#ifndef COMBINATOR_H
#define COMBINATOR_H

//#define DISPLAY

#include<iostream>
#include<vector>
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

template<typename Flt = double>
struct graph
{
    graph() 
    {}

    void add_node(node<Flt> *n)
    {
        nodes.push_back(n);
    }

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
    struct call_grape
    {
        node<Flt> *caller;
        std::vector< node<Flt> *> *callee;
    };

    void process_bloc()
    {
        grape_process_count = 0;
        // Push base nodes to the call list
        next_call.push_back({nullptr, &nodes});
        to_call_ptr = &to_call;
        next_call_ptr = &next_call;
        _process_grape();
    }

    std::vector<node<Flt>*> nodes;
    std::vector<call_grape> to_call, next_call;
    std::vector<call_grape> *to_call_ptr, *next_call_ptr;

    size_t grape_process_count;
private:

    bool has_same_call(node<Flt> *n, std::vector<node<Flt> *> *v)
    {
        for(size_t i = 0; i < next_call_ptr->size(); ++i)
        {
            if(next_call_ptr->at(i).caller == n && next_call_ptr->at(i).callee == v)
                return true;
        }
        return false;
    }
    void _process_grape()
    {
        std::swap(to_call_ptr, next_call_ptr);
        next_call_ptr->clear();
        if(to_call_ptr->size() == 0) 
        {
            // End of block
            // Copy outputs of last nodes to graph outputs and exit
            return;
        }
        for(size_t i = 0; i < to_call_ptr->size(); ++i)
        {
            node<Flt> *caller = to_call_ptr->at(i).caller; // nullptr at first pass
            for(size_t j = 0; j < to_call_ptr->at(i).callee->size(); ++j)
            { 
                to_call_ptr->at(i).callee->at(j)->process(caller);
                std::vector<node<Flt> *> *nxt = &(to_call_ptr->at(i).callee->at(j)->connections);
                // And for each returned connections, (next called events), we add an element in next_call_ptr
                if(!has_same_call(to_call_ptr->at(i).callee->at(j), nxt))
                    next_call_ptr->push_back({to_call_ptr->at(i).callee->at(j), nxt});
            }
        }
        grape_process_count++;
        this->_process_grape();
    }
};

template class graph<double>;
template class graph<float>;

#endif