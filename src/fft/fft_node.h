#ifndef FFT_NODE_H
#define FFT_NODE_H

#include "combinator3000.h"
#include "AudioFFT/AudioFFT.h"
#include "utilities.h"

/*
    Notes on overlap add : 
        * Calculate buffer size with (buffersize/overlap) shift every time
        * So the actual input size is (buffersize / overlap)
        * So use overlap to increase the buffer size instead of "reducing"

    So here
        * if Bsize is 512
        * Actual buffer is 1024
        * Shift 512 every time (keep track of the ancient data)
        * 
        * 
        * Number of FFT frames is overlap * 2 - 1
*/


/*template<typename Flt>
struct fft_node : public node<Flt>
{
    // Number of output channels to allow interleaved real/imaginaries data
    // blocsize is complexsize (find input size with previous->bloc_size or with `(bloc_size-1)*2` )
    fft_node(size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000, size_t num_overlap = 2)
        : node<Flt>::node{inp, outp, blocsize, samplerate}
        , n_overlap(num_overlap)
        , hop_size(blocsize/num_overlap)
        , n_overlap_frames(num_overlap * 2 - 1)
        , ffts(inp)
    {
        this->hop_complex_size = audiofft::AudioFFT::ComplexSize(this->hop_size);
        
        // Init buffers

        // Overlap specific temp buffers 
        this->overlap_space = new Flt[this->hop_size];
        this->overlap_bufs = new Flt*[this->n_overlap_frames * 2];
        for(size_t i = 0; i < this->n_overlap_frames; ++i)
            this->overlap_bufs[i] = new Flt[this->hop_complex_size];

        // Outputs 
        this->outputs = new Flt*[this->n_outputs*3]; // For storing complex numbers
        for(size_t i = 0; i < this->n_outputs*3; ++i)
            this->outputs[i] = new Flt[this->hop_complex_size];
        
        // Calc bins
        for(size_t i = 0; i < this->hop_complex_size; ++i)
            this->outputs[2][i] = i *(this->sample_rate / this->hop_size);

        for(auto & it : ffts)
            it.init(this->hop_size);
    }

    void process(node<Flt> *previous) override 
    {
        // For each channel
        for(size_t ch = 0; ch < this->n_inputs; ++ch)
        {
            ::memset(this->outputs[ch*3], 0, this->hop_complex_size * sizeof(Flt));
            ::memset(this->outputs[ch*3+1], 0, this->hop_complex_size * sizeof(Flt));

            // For each overlap pass
            for(size_t i = 0; i < this->n_overlap_frames; ++i)
            {
                const Flt *data = previous->outputs[ch] + (this->hop_size * i);
                size_t start = this->hop_size * i;
                // Copy piece by piece and window

                // Only window if overlap > 1
                if(this->n_overlap > 1) {
                    for(size_t n = 0; n < this->hop_size; ++n)
                    {
                        size_t index = n + start;
                        Flt w = hanning(n, this->hop_size);
                        this->overlap_space[n] = previous->outputs[index] * w;   
                    }
                } else // Simply copy
                {
                    ::memcpy(this->overlap_space, previous->outputs, this->hop_size * sizeof(Flt));
                }

                // Process FFT 
                Flt *real = this->overlap_bufs[i*2];
                Flt *imag = this->overlap_bufs[i*2+1];
                ffts[ch].fft(this->overlap_space, real, imag);
                
                // Sum it to output 
                for(size_t j = 0; j < this->hop_complex_size; ++j)
                {
                    this->outputs[ch*3][j] += this->overlap_bufs[i*2][j]; 
                    this->outputs[ch*3+1][j] += this->overlap_bufs[i*2+1][j];
                }
            }
        }
    }

    size_t n_overlap, hop_size, n_overlap_frames;
    std::vector<audiofft::AudioFFT> ffts;
    Flt *overlap_space;
    Flt **overlap_bufs;
};
*/
template<typename Flt = double>
struct fft_node : public node<Flt>
{
    // Number of output channels to allow interleaved real/imaginaries data
    // blocsize is complexsize (find input size with previous->bloc_size or with `(bloc_size-1)*2` )
    fft_node(size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000)
        : node<Flt>::node{inp, outp, blocsize, samplerate}
        , ffts(inp)
    {
        size_t complex_size = audiofft::AudioFFT::ComplexSize(this->bloc_size);
        this->outputs = new Flt*[this->n_outputs*2]; // For storing complex numbers
        for(size_t i = 0; i < this->n_outputs*2; ++i)
            this->outputs[i] = new Flt[complex_size];
            
        for(auto & it : ffts)
            it.init(this->bloc_size);
    }

    void process(node<Flt> *previous) override 
    {
        for(size_t i = 0; i < this->n_inputs; ++i)
        {
            Flt *real = this->outputs[i*2];
            Flt *imag = this->outputs[i*2+1];
            const Flt *data = previous->outputs[i];

            ffts[i].fft(data, real, imag);
        }
    }

    std::vector<audiofft::AudioFFT> ffts;
};

template<typename Flt = double >
struct ifft_node : public node<Flt>
{
    ifft_node(size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000)
        : node<Flt>::node(inp, outp, blocsize, samplerate)
        , ffts(inp)
    {
        for(auto & it : ffts)
            it.init(this->bloc_size);
    }

    void process(node<Flt> *previous) override 
    {
        for(size_t i = 0; i < this->n_outputs; ++i)
        {
            const Flt *real = previous->outputs[i*3];
            const Flt *imag = previous->outputs[i*3+1];
            ffts[i].ifft(this->outputs[i], real, imag);
        }
    }

    std::vector<audiofft::AudioFFT> ffts;
};

//template class fft_node<double>;
//template class ifft_node<double>;
#endif