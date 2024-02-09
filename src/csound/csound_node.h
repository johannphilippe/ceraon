#ifndef CSOUND_NODE_H
#define CSOUND_NODE_H

#include<memory>
#include "combinator3000.h"
#include "csound/csound.hpp"
#include "utilities.h"

template<typename Flt = double>
struct csound_node : public Csound, node<Flt>
{
    csound_node(std::string code, size_t inp = 0, size_t outp = 0, 
            size_t blocsize = 128, size_t samplerate = 48000)
        : node<Flt>::node(inp, outp, blocsize, samplerate)
        , process_cnt(0)
    {
        this->set_name("Csound");

        size_t err;
        // For some reason, if Csound does not "compiles" with this parameters, it is impossible to override from API
        std::string base_csd = "sr = 48000 \nksmps=128\nnchnls=1\nnchnls_i=1\n0dbfs=1\n";
        err = this->CompileOrc(base_csd.c_str());

        err = this->CompileCsdText(code.c_str());
        if(err != 0) 
            throw std::runtime_error("Csound compile with return code : " + std::to_string(err));
        this->SetHostImplementedAudioIO(1, 0);

        // Make sure parameters are overridden
        _params = std::make_unique<CSOUND_PARAMS>(); 
        this->GetParams(_params.get());
        _params->ksmps_override = this->bloc_size;
        _params->sample_rate_override = this->sample_rate;
        _params->nchnls_override = this->n_outputs;
        _params->nchnls_i_override = this->n_inputs;
        this->SetParams(_params.get());

        std::cout << "override : " << _params->ksmps_override << std::endl;

        err = this->Start();
        if(err != 0)
            throw std::runtime_error("Csound error when starting : " + std::to_string(err));
        
        std::cout << "Csound is ready to perform " << std::endl;
    }

    static csound_node<Flt> *from_file(std::string path, size_t inp = 0, size_t outp = 0, 
            size_t blocsize = 128, size_t samplerate = 48000)
    {
        std::string csd_str = read_file(path);
        return new csound_node<Flt>(csd_str, inp, outp, blocsize, samplerate);
    }

    void process(connection<Flt> &previous)
    {
        std::cout << "perform " << std::endl;

        if(this->n_inputs > 0)
        {
            for(size_t ch = previous.output_range.first, i = previous.input_offset;
                ch <= previous.output_range.second; ++ch, ++i)
            {
                for(size_t n = 0; n < this->bloc_size; ++n)
                    this->SetSpinSample(n, i, previous.target->outputs[ch][n]);
            }
            process_cnt = (process_cnt + 1) % this->n_nodes_in;
        }

        if(process_cnt == 0) 
        {
            int err = this->PerformKsmps();
            if(err != 0) 
                std::cout << "error with csound performance" << std::endl;

            for(size_t ch = 0; ch < this->n_outputs; ++ch)
            {
                for(size_t i = 0; i < this->bloc_size; ++i)
                {
                    this->outputs[ch][i] = this->GetSpoutSample(i, ch);
                }
            }
        }
    }

    size_t process_cnt;
    std::unique_ptr<CSOUND_PARAMS> _params;
};

#endif