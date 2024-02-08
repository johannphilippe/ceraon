#ifndef SNDFILE_NODE_H
#define SNDFILE_NODE_H

#include<iostream>
#include<memory>
#include"sndfile.hh"
#include"combinator3000.h"
#include"utilities.h"

template<typename Flt>
struct sndread_node : public node<Flt>
{
    sndread_node(std::string filepath, size_t blocsize = 128, size_t samplerate = 48000)
        : node<Flt>{0, 1, blocsize, samplerate}
    {
        this->set_name("SndRead");
        _sf = std::make_unique<SndfileHandle>(filepath, SFM_READ);
        if(_sf->samplerate() != samplerate)
            throw std::runtime_error("Samplerate of file does not match required samplerate");
        this->sample_rate = _sf->samplerate();
        this->n_outputs = _sf->channels();

        this->outputs = new Flt*[this->n_outputs];
        for(size_t i = 0; i < this->n_outputs; ++i)
            this->outputs[i] = new Flt[this->bloc_size];
        this->interleaved = new Flt[this->n_outputs * this->bloc_size];
    }

    void process(connection<Flt> &previous) override
    {
        size_t readcnt = _sf->readf(this->interleaved, this->bloc_size);
        for(size_t ch = 0; ch < this->n_outputs; ++ch)
        {
            for(size_t i = 0; i < this->bloc_size; ++i)
            {
                size_t index = i * this->n_outputs + ch;
                this->outputs[ch][i] = this->interleaved[index];
            }
        }
    }

    Flt *interleaved;
    std::unique_ptr<SndfileHandle> _sf;
};

template<typename Flt>
struct sndwrite_node : public node<Flt>
{
    sndwrite_node(std::string filepath, size_t inp = 1, size_t blocsize = 128, size_t samplerate = 48000)
        : node<Flt>::node(inp, inp, blocsize, samplerate)
        , process_cnt(0)
    {
        this->set_name("SndWrite");
        _sf = std::make_unique<SndfileHandle>(filepath, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_32, this->n_inputs, this->sample_rate);
        //Flt *rawmem = contiguous_memory(this->bloc_size, this->n_inputs, this->outputs);
        this->outputs = new Flt*[this->n_inputs];
        for(size_t i = 0; i < this->n_inputs; ++i)
            this->outputs[i] = new Flt[this->bloc_size];
        interleaved = new Flt[this->bloc_size * this->n_inputs];
    }

    void process(connection<Flt> &previous) override
    {
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ++ch, ++i)
        {
            for(size_t n = 0; n < this->bloc_size; ++n)
            {
                size_t index = n * this->n_inputs + i;
                interleaved[index] = previous.target->outputs[ch][n];
            }
            std::copy(previous.target->outputs[ch], previous.target->outputs[ch]+previous.target->bloc_size, this->outputs[ch]);
            process_cnt = (process_cnt + 1) % this->n_nodes_in;
        }
        if(process_cnt == 0) 
            _sf->writef(interleaved, this->bloc_size);
    }


    size_t process_cnt;
    Flt *interleaved;
    std::unique_ptr<SndfileHandle> _sf;
};

#endif