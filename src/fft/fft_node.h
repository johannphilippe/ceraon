#ifndef FFT_NODE_H
#define FFT_NODE_H

#include "combinator3000.h"
#include "AudioFFT/AudioFFT.h"
#include "utilities.h"
#include<cstring>
#include "comb_mem.h"

template<typename Flt = double>
struct fft_node : public node<Flt>
{
    fft_node(size_t inp = 1, size_t outp = 3, size_t blocsize = 128, size_t samplerate = 48000)
        : node<Flt>::node(node_init_mode::no_alloc, inp, outp, blocsize, samplerate)
        , ffts(inp)
    {
        if(this->n_outputs != (this->n_inputs * 3)) {
            throw std::runtime_error("FFT outputs must be 3 times its inputs");
        }

        this->set_name("FFT");
        complex_size = audiofft::AudioFFT::ComplexSize(this->bloc_size);
        this->outputs = new Flt*[this->n_outputs]; // For storing complex numbers
        for(size_t i = 0; i < this->n_outputs; ++i)
            this->outputs[i] = new Flt[complex_size];

        for(size_t ch = 0; ch < this->n_inputs; ++ch)
        {
            size_t index = ch*3+2;
            for(size_t i = 0; i < complex_size; ++i)
                this->outputs[index][i] =  i; //(i * this->sample_rate / this->bloc_size);

        }
        for(auto & it : ffts)
            it.init(this->bloc_size);
    }

    void process(connection<Flt> &previous) override 
    {
        for(size_t ch = previous.output_range.first, i = previous.input_offset; 
            ch <= previous.output_range.second; ++ch, ++i)
        {
            Flt *real = this->outputs[i*3];
            Flt *imag = this->outputs[i*3+1];
            const Flt *data = previous.target->outputs[ch];
            ffts[i].fft(data, real, imag);
        }
    }

    size_t complex_size;
    std::vector<audiofft::AudioFFT> ffts;
};

template<typename Flt = double >
struct ifft_node : public node<Flt>
{
    ifft_node(size_t inp = 3, size_t outp = 1, size_t blocsize = 128, size_t samplerate = 48000)
        : node<Flt>::node(inp, outp, blocsize, samplerate)
        , ffts(inp)
    {
        if(this->n_inputs != (this->n_outputs * 3)) {
            throw std::runtime_error("IFFT Inputs must be 3 times its outputs");
        }
        complex_size = audiofft::AudioFFT::ComplexSize(this->bloc_size);
        this->set_name("IFFT");
        for(auto & it : ffts)
            it.init(this->bloc_size);
    }

    void process(connection<Flt> &previous) override 
    {
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ch+=3, ++i)
        {
            const Flt *real = previous.target->outputs[ch*3];
            const Flt *imag = previous.target->outputs[ch*3+1];
            ffts[i].ifft(this->outputs[i], real, imag);
        }
    }

    size_t complex_size;
    std::vector<audiofft::AudioFFT> ffts;
};

/*
    FFT with overlap  and windowing
*/

template<typename Flt = double>
struct over_fft : public node<Flt>
{
    over_fft(size_t inp = 1, size_t outp = 3,  
        size_t blocsize = 1024, size_t samplerate = 48000, size_t overlap = 2)
        : node<Flt>::node(node_init_mode::no_alloc, inp, outp, blocsize, samplerate)
        , n_overlap(overlap)
        , win_count(n_overlap * 2 - 1)
        , hop_size(blocsize / n_overlap )
        , complex_size(audiofft::AudioFFT::ComplexSize(hop_size))
        , ffts(inp)
    {
        if(this->n_outputs != (this->n_inputs * 3)) {
            throw std::runtime_error("FFT outputs must be 3 times its inputs");
        }
        this->set_name("OverlapFFT");
        size_t nbufs = this->n_inputs * 3;
        this->outputs = new Flt*[nbufs];
        
        main_mem->alloc_channels<Flt>(this->bloc_size, nbufs, this->outputs);
        
        // Also need a single buffer to window, of size hop_size
        winbuf = new Flt[hop_size];
        // 
        for(size_t i = 0; i < this->n_inputs; ++i)
        {
            for(size_t w = 0; w < win_count; ++w)
            {
                size_t pos_offset = (w+1) * 3;
                size_t pos_in_buffer = (hop_size / 2) * w;
                for(size_t n = 0 ; n < complex_size; ++n)
                {
                    this->outputs[i*3+2][pos_in_buffer+pos_offset+n] = n;
                }

            }
        }

        for(auto & it : ffts)
            it.init(this->hop_size);

        std::cout << "OverlapFFT : " << hop_size << " & " << complex_size << std::endl;
    }

    void process(connection<Flt> &previous) override
    {
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ++ch, ++i)
        {
            for(size_t w = 0; w < win_count; ++w)
            {
                size_t pos_in_buffer = (hop_size / 2) * w;
                size_t pos_offset = (w+1) * 3;
                for(size_t n = 0; n < hop_size; ++n)
                {
                    winbuf[n] = previous.target->outputs[ch][pos_in_buffer+n] * hanning(n, hop_size);
                }
                Flt *real = this->outputs[i*3] + pos_in_buffer + pos_offset;
                Flt *imag = this->outputs[i*3+1] + pos_in_buffer + pos_offset;
                ffts[i].fft(winbuf, real, imag);
            }
        }
    }
    Flt *winbuf;
    size_t n_overlap, win_count, hop_size, complex_size;
    std::vector<audiofft::AudioFFT> ffts;
};

template<typename Flt = double>
struct over_ifft : public node<Flt>
{
    over_ifft(size_t inp = 3, size_t outp = 1,  
        size_t blocsize = 1024, size_t samplerate = 48000, size_t overlap = 2)
        : node<Flt>::node(node_init_mode::no_alloc, inp, outp, blocsize, samplerate)
        , n_overlap(overlap)
        , win_count(n_overlap * 2 - 1)
        , hop_size(blocsize / (overlap))
        , complex_size(audiofft::AudioFFT::ComplexSize(hop_size))
        , process_count(0)
        , ffts(inp)
    {
        if(this->n_inputs != (this->n_outputs * 3)) {
            throw std::runtime_error("FFT outputs must be 3 times its inputs");
        }
        this->set_name("OverlapIFFT");
        // Number of buffers is : input_channels * 3 * win_count
        size_t nbufs = this->n_outputs * 3;
        this->outputs = new Flt*[nbufs];
        main_mem->alloc_channels<Flt>(this->bloc_size, nbufs, this->outputs);

        std::cout << "Allocated FFT " << std::endl;
        // Also need a single buffer to window, of size hop_size
        winbuf = (Flt*)main_mem->mem_reserve(sizeof(Flt)*hop_size);
        std::cout << "Allocate winbuf " << std::endl;

        for(auto & it : ffts)
            it.init(this->hop_size);
        std::cout << "OverlapIFFT : " << hop_size << " & " << complex_size << std::endl;
    }

    void process(connection<Flt> &previous) override 
    {
        if(process_count == 0)
        {
            for(size_t i = 0; i < this->n_outputs; ++i)
                std::memset(this->outputs[i], 0, sizeof(Flt) * this->bloc_size);   
        }
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ch+=3, ++i)
        {
            for(size_t w = 0; w < win_count; ++w)
            {
                size_t pos_in_buf = (hop_size / 2) * w ;
                size_t pos_offset = (w+1) * 3;

                Flt *real = previous.target->outputs[ch] + pos_in_buf + pos_offset;
                Flt *imag = previous.target->outputs[ch+1] + pos_in_buf + pos_offset;
                ffts[i].ifft(winbuf, real, imag);
                for(size_t n = 0; n < hop_size; ++n)
                {
                    this->outputs[i][n+pos_in_buf] += winbuf[n] * hanning(n, hop_size);
                }
            }
        }

        process_count = (process_count + 1) % this->n_nodes_in;
        // Clean out buffers when 
    }

    Flt *winbuf;
    size_t n_overlap, win_count, hop_size, complex_size, process_count;
    std::vector<audiofft::AudioFFT> ffts;
};

/*
    This is used to only take the constant energy part of overlap save ifft
        The dash part has constant energy 
       --------
      /\  /\  /\
     /  \/  \/  \
    /    \  /    \
    |____________|

*/

template<typename Flt = double>
struct over_ifft_downbloc : public node<Flt>
{
    over_ifft_downbloc(size_t inp = 1, size_t outp = 1,  
        size_t blocsize = 1024, size_t samplerate = 48000, size_t overlap = 2)
    : node<Flt>::node(node_init_mode::no_alloc, inp, outp, blocsize, samplerate)
    , n_overlap(overlap)
    , offset(blocsize/overlap/2)
    {
        this->bloc_size = blocsize - ( offset * 2);
        std::cout << "Allocate ifft downbloc" << std::endl;
        this->outputs = new Flt*[this->n_outputs];
        main_mem->alloc_channels<Flt>(this->bloc_size, this->n_outputs, this->outputs);
        std::cout << "over ifft created "<< std::endl;
    }

    void process(connection<Flt> &previous) override
    {
        std::cout << "over ifft down process " << std::endl;
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ++ch, ++i)
        {
            std::copy(previous.target->outputs[ch] + offset, 
                previous.target->outputs[ch] + (previous.target->bloc_size - offset), 
                this->outputs[i]);
        }
    }
    

    size_t n_overlap, offset, total_bloc;
};

#endif