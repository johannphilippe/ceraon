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
    {
        this->set_name("Faust");
        processor.buildUserInterface(this);
        processor.init(this->sample_rate);
    }
    void process(node<Flt> *previous)
    {
        Flt **inputs = nullptr;
        if(this->n_inputs > 0) 
            inputs = previous->outputs;
        processor.compute(this->bloc_size, inputs, this->outputs);

#ifdef DISPLAY
        AsciiPlotter plot("Node", 80, 15);
        std::vector<double> xdata(this->bloc_size);
        for(size_t i = 0; i < this->bloc_size; ++i)
            xdata[i] = i;
        plot.addPlot(xdata, std::vector<double>(this->outputs[0], this->outputs[0] + this->bloc_size), "signal", '*');
        plot.legend();
        plot.show();
#endif
    }

    void set_param()
    {
        
    }

    P processor;
};

#endif