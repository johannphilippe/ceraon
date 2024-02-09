#ifndef COMBINATOR_H
#define COMBINATOR_H

//#define DISPLAY

#include<iostream>
#include<vector>
#include<mutex>
#include<memory>
#include"halfband/halfband.h"
#include"asciiplotter/asciiplotter.h"
#include "utilities.h"

/*
    Node is the main half-abstract base class for DSP nodes
    It behaves like a linked list, where each node knows to which it is connected next (with pointers).
*/
enum class node_init_mode
{
    no_alloc = 0,
    alloc = 1
};

template<typename Flt = double>
struct node
{

    struct connection
    {
        node<Flt> *target = nullptr;
        std::pair<size_t, size_t> output_range = {0, 0};
        size_t input_offset = 0;
        node<Flt> *parent = nullptr;

        size_t get_num_outputs() {return output_range.second - output_range.first + 1;}
        static std::vector<connection> vec_from_nodes(std::vector<node<Flt>*> &n) {
            std::vector<connection> nv(n.size());
            for(size_t i = 0; i < n.size(); ++i)
                nv.push_back(connection{n[i]});
            return nv;
        }
    };

    node(size_t inp = 0, size_t outp = 0, 
        size_t blocsize = 128, size_t samplerate = 48000);
    node(node_init_mode init_memory, size_t inp = 0, size_t outp = 0, 
        size_t blocsize = 128, size_t samplerate = 48000);
    virtual ~node();

    bool connect(node *n, bool adapt_channels = true);
    bool connect(connection n, bool adapt_channels = true);
    bool disconnect(node *n);
    
    void set_name(std::string n);
    std::string &get_name();

    virtual void process(connection &previous);

    /* 
        Keep these four members at the beginning of the struct 
        (some childs init without constructor call)
    */
    size_t n_inputs, n_outputs, bloc_size, sample_rate;
    
    size_t n_nodes_in;
    std::vector<connection> connections;
    Flt **outputs;
    std::string name;
};

template<typename Flt>
using connection = typename node<Flt>::connection;

/*
    Channel adapter merges or sums channels to enable
    to connect nodes with different number of ins/outs
*/
template<typename Flt = double>
struct channel_adapter : public node<Flt>
{
    channel_adapter(size_t inp = 0, size_t outp = 0, 
        size_t blocsize = 128, size_t samplerate = 48000);
    virtual void process(connection<Flt> &previous) override;
    size_t process_count;
};

/*
    Bloc size adapter
    It has an internal circular buffer to increase bloc size while keeping sample rate
*/
template<typename Flt = double>
struct upbloc : public node<Flt>
{
    upbloc(size_t inp = 1, size_t outp = 1, 
        size_t blocsize = 128, size_t samplerate = 48000);

    void process(connection<Flt> &previous) override;
};

template class upbloc<float>;
template class upbloc<double>;

template<typename Flt = double>
struct downbloc : public node<Flt>
{
    downbloc(size_t inp = 1, size_t outp = 1, 
        size_t blocsize = 128, size_t samplerate = 48000);

    void process(connection<Flt> &previous) override;
};

template class downbloc<float>;
template class downbloc<double>;

/*
    Node mixer is used to merge several nodes outputs together
*/
template<typename Flt = double>
struct mixer : public node<Flt>
{
    mixer(size_t inp = 0, size_t outp = 0, 
        size_t blocsize = 128, size_t samplerate = 48000);
    virtual void process(connection<Flt> &previous) override;

    size_t process_count;
};

/*
    Simple upsample 
        * upsamples to * 2 origin sample rate
        * results in * 2 bloc size 
*/
template<typename Flt = double>
struct simple_upsampler : public node<Flt>
{
    simple_upsampler(size_t inp = 0, size_t outp = 0, 
        size_t bloc_size = 128, size_t samplerate= 48000, 
        size_t order = 10, size_t steep = 1);
    ~simple_upsampler();
    void process(connection<Flt> &revious) override;

    size_t f_order, f_steep;
    std::vector<halfband *> filters;
};

/*
    Full upsampler
    - num_cascade sets the number of upsampling (times 2) processes 
        (e.g. 1 means one upsampling, so 48000 becomes 96000, 2 means 48000 becomes 192000)
    - order is the filter order
    - Steep is to enable the steepness of filters (1 or 0)
    - blocsize and samplerate must be the target (if you upsample to 96k Hz, set samplerate to 96000 
        and set blocsize to twice the previous node)
*/
template<typename Flt = double>
struct upsampler : public node<Flt>
{
    upsampler(size_t inp = 0, size_t outp = 0, 
        size_t bloc_size = 128, size_t samplerate= 48000, 
        size_t num_cascade = 1, size_t order = 10, size_t steep = 1);
    ~upsampler();
    void process(connection<Flt> &previous) override;

    size_t n_cascade, f_order, f_steep, n_samps_iter;
    std::vector<simple_upsampler<Flt>*> upsamplers;
};

/*
    Downsampler, inverse of upsampler
    Its blocsize and samplerate must be the target 
    (if you upsample from 96000 to 48000, set 48000 samplerate and half bloc size as previous node)
*/
template<typename Flt = double>
struct downsampler : public node<Flt>
{
    downsampler(size_t inp = 0, size_t outp = 0, 
        size_t bloc_size = 128, size_t samplerate= 48000, 
        size_t num_cascade = 1, size_t order = 10, size_t steep = 1);
    ~downsampler();
    void process(connection<Flt> &previous) override;

    size_t n_cascade, f_order, f_steep, n_samps_iter;
    std::vector<half_cascade *>decimators;
};

/*
    Main structure for non-realtime Graph
    A graph must be filled with at least one node (the starting nodes)
    Nodes must be connected together before you add them to graph
    Then it will automatically find which node in the chain must be connected to output
*/
template<typename Flt = double>
struct graph
{
    struct call_grape
    {
        connection<Flt> *caller;
        std::vector<connection<Flt>> *callee;
    };

    struct call_t
    {
        node<Flt> *callee;
        connection<Flt> caller_ctx;
        bool operator==(call_t &other) {
            if(this->callee == other.callee && this->caller_ctx.target == other.caller_ctx.target)
                return true;
            if(this->caller_ctx.target == nullptr || other.caller_ctx.target == nullptr) 
                return false;
            return false;
        }
    };

    graph(size_t inp = 0, size_t outp = 1, 
        size_t blocsize = 128, size_t samplerate = 48000);
    // Safe
    void add_node(node<Flt> *n);
    void add_nodes(std::vector<node<Flt>*> n);
    void remove_node(node<Flt> *n);
    // Unsafe (it is automatically handled in add_node and remove_node)
    //void add_output(node<Flt> *o);
    //void remove_output(node<Flt> *o);

    void process_bloc();

    std::string generate_patchbook_code();
    void generate_faust_diagram();

    size_t n_inputs, n_outputs, bloc_size, sample_rate;
    std::vector<node<Flt>*> nodes;


    std::shared_ptr<mixer<Flt>> _mix;
    std::shared_ptr<node<Flt>> _input_node;
    connection<Flt>  _input_connection;

    std::vector<connection<Flt> > _node_connections;

    std::recursive_mutex _mtx;
protected:
    std::vector<call_grape> to_call, next_call;
    std::vector<call_grape> *to_call_ptr, *next_call_ptr;

    void _generate_patchbook_code( std::string &s);
    
    //std::vector<call_event> call_list;
    std::vector<call_t> call_list;
    void _generate_event_list();
    void _remove_duplicates();
    void _find_and_remove_out(node<Flt> *n);
    void _find_and_add_out(node<Flt> * n);
    void _rm_node(node<Flt> *n);
    void _process_grape();
};

template class graph<double>;
template class graph<float>;

#include "rtaudio/RtAudio.h"

/*
    Same as graph, with realtime audio input/output
    Call start_stream or stop_stream to start/stop performance
*/
static int rtgraph_callback(void *out_buffer, void *in_buffer, 
    unsigned int nframes, double stream_time, 
    RtAudioStreamStatus status, void *user_data);

template<typename Flt>
struct rtgraph : public graph<Flt>
{
    rtgraph(size_t inp = 1, size_t outp = 1, 
        size_t blocsize = 128, size_t samplerate = 48000);

    void start_stream();
    void stop_stream();

    void list_devices();
    void set_devices(unsigned int input_device, unsigned int output_device);

    RtAudio dac;
    RtAudio::StreamParameters output_parameters, input_parameters;
    std::shared_ptr<RtAudio::StreamOptions> _options;
};

template<typename Flt>
using call_event = typename graph<Flt>::call_event;

template class rtgraph<double>;

#endif