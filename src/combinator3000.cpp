#include "combinator3000.h"
#include<cstring>


template<typename Flt>
node<Flt>::node( size_t inp, size_t outp, size_t blocsize, size_t samplerate)
    : n_inputs(inp)
    , n_outputs(outp)
    , bloc_size(blocsize)
    , sample_rate(samplerate)
    , n_nodes_in(0)
{
    outputs = new Flt*[n_outputs];
    for(size_t i = 0; i < n_outputs; ++i)
        outputs[i] = new Flt[bloc_size];
}

template<typename Flt>
node<Flt>::~node()
{
    for(size_t i = 0; i < n_outputs; ++i)
        delete outputs[i];
    delete outputs;
}

template<typename Flt>
bool node<Flt>::connect(node<Flt> *n)
{
    if(this->n_outputs == n->n_inputs) {
        connections.push_back(n);
        n->n_nodes_in += 1;
        return true;
    } else { // if no input/output number match, we need an adapter node 
        channel_adapter<Flt> *a = new channel_adapter<Flt>(this->n_outputs, n->n_inputs);
        connections.push_back(a);
        a->n_nodes_in += 1;
        a->connect(n);
        return true;
    }
    return false;
}

template<typename Flt>
bool node<Flt>::disconnect(node<Flt> *n)
{
    for(size_t i = 0; i < connections.size(); ++i)
    {
        if(connections[i] == n) {
            connections[i]->n_nodes_in -= 1;
            connections.erase(connections.begin() + i);
            return true;
        }
    }
    return false;
}

template<typename Flt>
void node<Flt>::process(node<Flt> *previous)
{
    // Mix n_connected until modulo 0 is reached
    if(n_inputs > 0) 
    {
        // Copying inputs to outputs (input is the output of previous node)
        for(size_t ch = 0; ch < previous->n_outputs; ++ch)
            ::memcpy(outputs[ch], previous->outputs[ch], sizeof(Flt) * previous->bloc_size);
    }
}

template<typename Flt>
std::vector<node<Flt>*>* node<Flt>::process_next()
{
    // Processing next nodes
    for(auto & it : connections)
        it->process(this);

    return &connections;
}

template class node<double>;
template class node<float>;

template<typename Flt>
channel_adapter<Flt>::channel_adapter(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
{
    size_t valid = std::max(this->n_inputs, this->n_outputs) % std::min(this->n_inputs, this->n_outputs);
    if(valid != 0)
        throw std::runtime_error("Error in channel adapter - outputs must be even divisor of inputs. Here" 
            + std::to_string(this->n_inputs) + " " 
            + std::to_string(this->n_outputs));
}


template<typename Flt>
void channel_adapter<Flt>::process(node<Flt> *previous)
{
    /*
        Cases : 
            * merging to 1 output : sum
            * splitting from 1 input : simple split 
            * if even divisor of inp/outp > simple to do (mix 2 by 2, etc)
            * else error ? 
    */
    Flt **inputs = previous->outputs;
    if(this->n_outputs == 1)
    {
        for(size_t i = 0; i < this->bloc_size; ++i)
        {
            Flt sum = 0;
            for(size_t ch = 0; ch < this->n_inputs; ++ch)
            {
               sum += previous->outputs[ch][i]; 
            }
            this->outputs[0][i] = sum;
        }
    } else if(this->n_inputs == 1)
    {
        for(size_t ch = 0; ch < this->n_outputs; ++ch)
        {
            ::memcpy(this->outputs[ch], previous->outputs[0], this->bloc_size * sizeof(Flt));
        }
    } else { // need to check if % is 0
        if(this->n_inputs > this->n_outputs && (this->n_inputs % this->n_outputs) == 0 )  
        {
            for(size_t o = 0; o < this->n_outputs; ++o)
            {
                for(size_t s = 0; s < this->bloc_size; ++s)
                {
                    Flt sum = 0;
                    for(size_t i = 0; i < this->n_inputs; i+= this->n_outputs)
                    {
                        sum += previous->outputs[i][s];
                    }
                    this->outputs[o][s] = sum;
                }
            }
        } else if(this->n_outputs > this->n_inputs && (this->n_outputs % this->n_inputs) == 0)
        {
            for(size_t o = 0; o < this->n_outputs; ++o)
            {
                size_t i = o % this->n_inputs;
                ::memcpy(this->outputs[o], previous->outputs[i], this->bloc_size * sizeof(Flt));
            }
        }
    }

    for(auto & it : this->connections)
        it->process(this);
}

template class channel_adapter<double>;
template class channel_adapter<float>;


template<typename Flt>
mixer_node<Flt>::mixer_node(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , process_count(0)
{}

template<typename Flt>
void mixer_node<Flt>::process(node<Flt> *previous)
{
    if(this->n_nodes_in == 0) {
        throw std::runtime_error("Mixer node must have at least one input (ideally two or more, it is a mixer)");
        return;
    }
    for(size_t ch = 0; ch < this->n_inputs; ++ch)
    {
        if(this->process_count == 0)
            ::memset(this->outputs[ch], 0, this->bloc_size * sizeof(Flt));
        for(size_t i = 0; i < this->bloc_size; ++i)
        {
            this->outputs[ch][i] += previous->outputs[ch][i];
        }
    }
    this->process_count = (this->process_count + 1) % this->n_nodes_in;
#ifdef DISPLAY
    if(this->process_count == 0)
    {
        AsciiPlotter plot("mixernode", 80, 15 );
        std::vector<double> xdata(this->bloc_size);
        for(size_t i = 0; i < this->bloc_size; ++i)
            xdata[i] = i;
        plot.addPlot(xdata, std::vector<double>(this->outputs[0], this->outputs[0] + this->bloc_size), "signal", '*');
        plot.legend();
        plot.show();

    }
#endif
}

template class mixer_node<double>;
template class mixer_node<float>;

template<typename Flt>
simple_upsampler<Flt>::simple_upsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, size_t order, size_t steep)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , f_order(order)
    , f_steep(steep)
{
    filters.resize(this->n_outputs);
    for(size_t i = 0; i < this->n_outputs; ++i)
        filters[i] = create_halfband(f_order, f_steep);
}

template<typename Flt>
simple_upsampler<Flt>::~simple_upsampler()
{
    for(size_t i = 0; i < this->n_outputs; ++i)
        destroy_halfband(filters[i]);
}

template<typename Flt>
void simple_upsampler<Flt>::process(node<Flt> *previous)
{
    for(size_t ch = 0; ch < this->n_inputs; ++ch)
    {
        for(size_t i = 0; i < previous->bloc_size; ++i)
        {
            this->outputs[ch][i*2] = process_halfband(filters[ch], previous->outputs[ch][i]);
            this->outputs[ch][i*2+1] = process_halfband(filters[ch], Flt(0.0) );
        }
    }
}

template class simple_upsampler<double>;

template<typename Flt>
upsampler<Flt>::upsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, size_t num_cascade, size_t order, size_t steep)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , n_cascade(num_cascade)
    , f_order(order)
    , f_steep(steep)
{
    size_t base_samplerate = this->sample_rate / (n_cascade * 2);
    size_t base_blocsize = this->bloc_size / (n_cascade * 2);
    upsamplers.resize(num_cascade);
    for(size_t i = 0; i < n_cascade; ++i)
    {
        base_samplerate *= 2;
        base_blocsize *= 2;
        this->upsamplers[i] = new simple_upsampler<Flt>(inp, outp, base_blocsize, base_samplerate, f_order, f_steep);
    }
}

template<typename Flt>
upsampler<Flt>::~upsampler()
{
    for(size_t i = 0; i < this->n_cascade; ++i)
        delete this->upsamplers[i];
}

template<typename Flt>
void upsampler<Flt>::process(node<Flt> *previous)
{
    node<Flt> *p = previous;
    for(size_t i = 0; i < n_cascade; i++)
    {
        upsamplers[i]->process(p);
        p = upsamplers[i];
    }
    for(size_t ch = 0; ch < this->n_outputs; ++ch)
        ::memcpy(this->outputs[ch], p->outputs[ch], this->bloc_size * sizeof(Flt));
}

template class upsampler<double>;

template<typename Flt>
downsampler<Flt>::downsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, size_t num_cascade, size_t order, size_t steep)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , n_cascade(num_cascade)
    , f_order(order)
    , f_steep(steep)
    , n_samps_iter(n_cascade * 2)
{
    decimators.resize(this->n_outputs);
    for(size_t i = 0; i < this->n_outputs; ++i)
        decimators[i] = create_half_cascade(n_cascade, f_order, f_steep);
}

template<typename Flt>
downsampler<Flt>::~downsampler()
{
    for(size_t i = 0; i < this->n_outputs; ++i)
        destroy_half_cascade(decimators[i]);
}

template<typename Flt>
void downsampler<Flt>::process(node<Flt> *previous)
{
    for(size_t ch = 0; ch < this->n_inputs; ++ch)
    {
        for(size_t i = 0; i < this->bloc_size; ++i)
        {
            size_t up = i * n_samps_iter;
            this->outputs[ch][i] = process_half_cascade(this->decimators[ch], &(previous->outputs[ch][up]));
        }
    }
}

template class downsampler<double>;

