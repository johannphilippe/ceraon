#ifndef COMBINATOR_H
#define COMBINATOR_H

//#define DISPLAY

#include<iostream>
#include<vector>
#include<mutex>
#include<memory>
#include"halfband/halfband.h"
#include"asciiplotter/asciiplotter.h"
/*
    Node is the main class. 
    It behaves like a linked list, where each node knows to which it is connected next (with pointers).
*/
template<typename Flt = double>
struct node
{
    node(size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000);
    virtual ~node();

    bool connect(node *n);
    bool disconnect(node *n);

    virtual void process(node<Flt> *previous);

    size_t n_inputs, n_outputs, bloc_size, sample_rate, n_nodes_in;
    std::vector<node *> connections;
    Flt **outputs;
};

template<typename Flt = double>
struct channel_adapter : public node<Flt>
{
    channel_adapter(size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000);
    virtual void process(node<Flt> *previous) override;
};

/*
    Node mixer is used to merge several nodes together. 
    It knows its number of 
*/
template<typename Flt = double>
struct mixer : public node<Flt>
{
    mixer(size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000);
    virtual void process(node<Flt> *previous) override;

    size_t process_count;
};

/*
    Simple upsample 
        * upsamples to *2 origin sample rate
        * results in *2 bloc size 
*/
template<typename Flt = double>
struct simple_upsampler : public node<Flt>
{
    simple_upsampler(size_t inp = 0, size_t outp = 0, size_t bloc_size = 128, size_t samplerate= 48000, size_t order = 10, size_t steep = 1);
    ~simple_upsampler();
    void process(node<Flt> *previous) override;

    size_t f_order, f_steep;
    std::vector<halfband *> filters;
};

template<typename Flt = double>
struct upsampler : public node<Flt>
{
    upsampler(size_t inp = 0, size_t outp = 0, size_t bloc_size = 128, size_t samplerate= 48000, size_t num_cascade = 1, size_t order = 10, size_t steep = 1);
    ~upsampler();
    void process(node<Flt> *previous) override;

    size_t n_cascade, f_order, f_steep, n_samps_iter;
    std::vector<simple_upsampler<Flt> *> upsamplers;
};

template<typename Flt = double>
struct downsampler : public node<Flt>
{
    downsampler(size_t inp = 0, size_t outp = 0, size_t bloc_size = 128, size_t samplerate= 48000, size_t num_cascade = 1, size_t order = 10, size_t steep = 1);
    ~downsampler();
    void process(node<Flt> *previous) override;

    size_t n_cascade, f_order, f_steep, n_samps_iter;
    std::vector<half_cascade *>decimators;
};

/*
    Graph 
    - Handles pointer to the starting nodes of one or several node chains 
    - Is passed to RtAudio engine to be called inside callback
*/


    /*
        Complexity here is to call parallel before to call sequential (for the case of mixer node )
            n1  -> n11  -> n111
                        -> n112
                -> n12
                        -> n121
            n2  -> n21
                        -> n211
                        -> n212
                -> n22
                        -> n221
                        -> n222
                        -> ... 
        At each sequential step, each grape can subdivide in several grapes
    */

// Todo : handle the case where first nodes are actually effects (they are called with inputs)
// Idea : * Add inp param, and take a basic "node" internally that takes input from rtengine and be previous call of  first nodes

template<typename Flt = double>
struct graph
{
    struct call_grape
    {
        node<Flt> *caller;
        std::vector< node<Flt> *> *callee;
    };

    graph(size_t inp = 0, size_t outp = 1, size_t blocsize = 128, size_t samplerate = 48000);
    // Safe
    void add_node(node<Flt> *n);
    void remove_node(node<Flt> *n);
    // Unsafe (it is automatically handled in add_node and remove_node)
    void add_output(node<Flt> *o);
    void remove_output(node<Flt> *o);

    void process_bloc();


    size_t n_inputs, n_outputs, bloc_size, sample_rate;
    std::vector<node<Flt>*> nodes;
    std::vector<call_grape> to_call, next_call;
    std::vector<call_grape> *to_call_ptr, *next_call_ptr;
    size_t grape_process_count;
    std::unique_ptr<mixer<Flt>> _mix;
    std::unique_ptr<node<Flt>> _input_node;
    std::recursive_mutex _mtx;
protected:

    bool has_same_call(node<Flt> *n, std::vector<node<Flt> *> *v);
    void _find_and_remove_out(node<Flt> *n);
    void _find_and_add_out(node<Flt> * n);
    void _rm_node(node<Flt> *n);
    void _process_grape();
};

template class graph<double>;
template class graph<float>;

#include "rtaudio/RtAudio.h"

static int rtgraph_callback(void *out_buffer, void *in_buffer, unsigned int nframes, double stream_time, RtAudioStreamStatus status, void *user_data);

template<typename Flt>
struct rtgraph : public graph<Flt>
{
    rtgraph(size_t inp = 1, size_t outp = 1, size_t blocsize = 128, size_t samplerate = 48000);

    void openstream();

    void list_devices();
    void set_devices(unsigned int input_device, unsigned int output_device);

    RtAudio dac;
    RtAudio::StreamParameters output_parameters, input_parameters;
    std::unique_ptr<RtAudio::StreamOptions> _options;
};

template class rtgraph<double>;

#endif