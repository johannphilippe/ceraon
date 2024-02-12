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
        : node<Flt>::node(inp, outp, blocsize, samplerate, false)
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

    void process(connection<Flt> &previous, audio_context &ctx) override 
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

    void process(connection<Flt> &previous, audio_context &ctx) override 
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


// Information for OLA update and offsets
struct circular_info
{
    size_t position;
    bool update;
};

/*
    Implementation of OLA / WOLA FFT 
*/

template<typename Flt = double>
struct ola_fft : public node<Flt>
{
    ola_fft(size_t inp, size_t outp, size_t fftsize, size_t overlap, size_t samplerate)
        : node<Flt>::node(inp, outp, fftsize, samplerate, false)
        , fft_size(fftsize)
        , n_overlap(overlap)
        , hop_size(fft_size/n_overlap)
        , complex_size(audiofft::AudioFFT::ComplexSize(fft_size))
        , circular_offsets(inp)
        , ffts(inp)
    {
        if(!is_power_of_two(fft_size))
            throw std::runtime_error("OLA FFT size must be power of two");

        if(this->n_outputs  != (this->n_inputs * 3) )
            throw std::runtime_error("OLA FFT Output number must be 3 times input number");
        
        this->set_name("OLA FFFT");
        this->outputs = new Flt*[this->n_outputs];
        main_mem->alloc_channels<Flt>(complex_size, this->n_outputs, this->outputs);
        for(size_t ch = 0; ch < this->n_inputs; ++ch)
        {
            for(size_t n = 0; n < complex_size; ++n)
                this->outputs[ch*3+2][n] = n;
        }
        win_buffer = main_mem->alloc_buffer<Flt>(fft_size);
        input_buffers = new Flt*[this->n_inputs];
        main_mem->alloc_channels<Flt>(fft_size, this->n_inputs, input_buffers);
        std::memset(circular_offsets.data(), 0, sizeof(circular_info) * this->n_inputs);
        for(auto & it : ffts)
            it.init(fft_size);
    }
    
    ~ola_fft()
    {
        delete win_buffer;
        delete[] input_buffers;
    }

    void process(connection<Flt> &previous, audio_context &ctx) override
    {
        std::cout << "FFT Process" << std::endl;
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ++ch, ++i)
        {
            std::cout << circular_offsets[i].position << std::endl;
            if( (previous.target->bloc_size + circular_offsets[i].position) <= hop_size)
            {
                std::cout << "Condition ok" << std::endl;
                std::copy(previous.target->outputs[ch],
                    previous.target->outputs[ch] + previous.target->bloc_size, 
                    input_buffers[i] + (fft_size - hop_size + circular_offsets[i].position) );
                circular_offsets[i].position += previous.target->bloc_size;
                if(circular_offsets[i].position >= hop_size)
                {
                    circular_offsets[i].position %= hop_size;
                    circular_offsets[i].update = true;
                }
            } else  // graph bloc size is > to hop size, then we need to process FFT several times 
            {
                std::cout << "OLA FFT bloc size input must be power of two, and maximum half hop size (fft_size / overlap)" << std::endl;
                std::cout << "Skip processing " << std::endl;
            }

            if(circular_offsets[i].update)
            {
                std::cout << " >> FFT Update new computation" << std::endl;
                circular_offsets[i].update = false;
                for(size_t n = 0; n < fft_size; ++n)
                    win_buffer[n] = input_buffers[i][n] * hanning(n, fft_size);
                
                // Rotate input (could be replaced with circular buffer to prevent copy)
                std::copy(input_buffers[i]+hop_size, input_buffers[i]+fft_size, input_buffers[i]);
                ffts[i].fft(win_buffer, this->outputs[i*3], this->outputs[i*3+1]);
            }
        }
        std::cout << "FFT done " << std::endl;
    }

    Flt *win_buffer;
    Flt **input_buffers;
    size_t fft_size, n_overlap, hop_size, complex_size;
    std::vector<circular_info> circular_offsets;
    std::vector<audiofft::AudioFFT> ffts;
};

template<typename Flt = double>
struct ola_ifft : public node<Flt>
{
    ola_ifft(size_t inp, size_t outp, size_t fftsize, size_t overlap, size_t samplerate)
        : node<Flt>::node(inp, outp, fftsize, samplerate, false)
        , fft_size(fftsize)
        , n_overlap(overlap)
        , hop_size(fft_size/n_overlap)
        , complex_size(audiofft::AudioFFT::ComplexSize(fft_size))
        , circular_offsets(inp)
        , ffts(inp)
    {
        if(!is_power_of_two(fft_size))
            throw std::runtime_error("OLA FFT size must be power of two");

        if(this->n_inputs  != (this->n_outputs * 3) )
            throw std::runtime_error("OLA FFT Output number must be 3 times input number");
        
        this->set_name("OLA IFFT");
        this->outputs = new Flt*[this->n_outputs];
        main_mem->alloc_channels<Flt>(hop_size, this->n_outputs, this->outputs);

        ola_buffers = new Flt*[n_overlap *this->n_outputs];
        main_mem->alloc_channels<Flt>(fft_size, n_overlap * this->n_outputs, ola_buffers);

        std::memset(circular_offsets.data(), 0, sizeof(circular_info) * this->n_inputs);
        for(auto & it : ffts)
            it.init(fft_size);
    }

    void process(connection<Flt> &previous, audio_context &ctx) override
    {
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ch+=3, ++i)
        {
            std::copy(this->outputs[i] + ctx.bloc_size, this->outputs[i] + this->hop_size, this->outputs[i]);

            circular_offsets[i].position += ctx.bloc_size; 
            std::cout << "IFFT : circular offset position : " << circular_offsets[i].position << std::endl;
            if(circular_offsets[i].position >= hop_size)
            {
                std::cout << "IFFT Update new computation" << std::endl;
                circular_offsets[i].position %= hop_size;

                // Rotate ola buffers  (also, could be replaced with circular index to prevent copy)
                for(size_t m = n_overlap-1; m > 0; m--)
                {
                    const size_t previous_idx = (i*n_overlap) + (m-1);
                    const size_t cur_idx = previous_idx + 1;;
                    std::copy(ola_buffers[previous_idx], 
                        ola_buffers[previous_idx] + fft_size, ola_buffers[cur_idx]);
                }
                
                ffts[i].ifft(ola_buffers[i*n_overlap], 
                    previous.target->outputs[ch], previous.target->outputs[ch+1]);
 
                // WOLA apply synthesis window
                for(size_t n = 0; n < fft_size; ++n)
                    ola_buffers[i*n_overlap][n] *= hanning(n, fft_size);
                
                for(size_t n = 0; n < hop_size; ++n)
                {
                    Flt sum = 0.0;
                    for(size_t m = 0; m < n_overlap; ++m)
                        sum += ola_buffers[i*n_overlap+m][n+(m*hop_size)];

                    sum /= (n_overlap / 2);
                    this->outputs[i][n] = sum;
                }
            }
        }
        std::cout << "IFFT done " << std::endl;
    }

    Flt **ola_buffers;
    size_t fft_size, n_overlap, hop_size, complex_size;
    std::vector<circular_info> circular_offsets;
    std::vector<audiofft::AudioFFT> ffts;
};

template<typename Flt = double>
struct circular_downbloc : public node<Flt>
{
    circular_downbloc(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
        : node<Flt>::node(inp, outp, blocsize, samplerate)
        , counts(this->n_inputs)
    {
        if(this->n_inputs != this->n_outputs)
            throw std::runtime_error("Circular downbloc : inputs and outputs must be equal");
        this->set_name("Circular downbloc");
    }

    void process(connection<Flt> &previous, audio_context &ctx)
    {
        for(size_t ch = previous.output_range.first, i = previous.input_offset;
            ch <= previous.output_range.second; ++ch, ++i)
        {
            std::copy(previous.target->outputs[ch] + counts[i], 
                previous.target->outputs[ch]+this->bloc_size + counts[i], 
                this->outputs[i]);
            counts[i] = (counts[i] + this->bloc_size) % previous.target->bloc_size;
        }
    }

    size_t down_factor;
    std::vector<size_t> counts;
};

#endif