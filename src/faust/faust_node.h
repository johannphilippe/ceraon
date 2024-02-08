#ifndef FAUST_NODE_H
#define FAUST_NODE_H

/*
    Faust basic node (works with Faust C++ generated code following a simple architecture)
*/
#define FAUSTFLOAT double

#include "combinator3000.h"
#include "asciiplotter/asciiplotter.h"
#include "faust/gui/MapUI.h"


template<class P, typename Flt = double>
struct faust_node : public node<Flt>, MapUI
{
    faust_node(size_t blocsize = 128, size_t samplerate = 48000) 
        : node<Flt>::node(processor.getNumInputs(), processor.getNumOutputs(), blocsize, samplerate)
        , process_cnt(0)
    {
        this->set_name("Faust");
        processor.buildUserInterface(this);
        processor.init(this->sample_rate);

        if(this->n_inputs > 0)
            inputs = new Flt*[this->n_inputs];
        else
            inputs = nullptr; 
    }
    void process(connection<Flt> &previous)
    {
        if(this->n_inputs > 0)
        {
            for(size_t ch = previous.output_range.first, i = previous.input_offset;
                ch <= previous.output_range.second; ++ch, ++i)
                    inputs[i] = previous.target->outputs[ch];
            process_cnt = (process_cnt + 1) % this->n_nodes_in;
        }

        if(process_cnt == 0) 
            processor.compute(this->bloc_size, inputs, this->outputs);
    }
    size_t process_cnt;
    Flt **inputs;
    P processor;
};

#endif