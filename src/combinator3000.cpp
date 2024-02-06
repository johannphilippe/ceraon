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
    this->set_name("Node");
    outputs = new Flt*[n_outputs];
    for(size_t i = 0; i < n_outputs; ++i)
        outputs[i] = new Flt[bloc_size];
    
   // Contiguous memory
   //Flt * raw_mem = contiguous_memory(this->bloc_size, this->n_outputs, this->outputs);
}

template<typename Flt>
node<Flt>::~node()
{
    for(auto & it : connections)
        it->n_nodes_in -= 1;
    for(size_t i = 0; i < n_outputs; ++i)
        delete outputs[i];
    delete outputs;
}

template<typename Flt>
bool node<Flt>::connect(node<Flt> *n)
{
    if(this->n_outputs == n->n_inputs || n->handles_parallel()) {
        connections.push_back(n);
        n->n_nodes_in += 1;
        return true;
    } else { // if no input/output number match, we need an adapter node 
        channel_adapter<Flt> *a = 
            new channel_adapter<Flt>(this->n_outputs, n->n_inputs);
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
            std::copy(previous->outputs[ch], previous->outputs[ch] + previous->bloc_size, this->outputs[ch]);
            //::memcpy(outputs[ch], previous->outputs[ch], sizeof(Flt) * previous->bloc_size);
    }
}

template<typename Flt>
void node<Flt>::handles_parallel() {return false;}

template<typename Flt>
void node<Flt>::set_name(std::string n) {this->name = name_gen::concat(n);}

template<typename Flt>
std::string &node<Flt>::get_name() {return this->name;}

template class node<double>;
template class node<float>;

template<typename Flt>
channel_adapter<Flt>::channel_adapter(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
{
    this->set_name("Channel adapter");
    size_t valid = std::max(this->n_inputs, this->n_outputs) 
            % std::min(this->n_inputs, this->n_outputs);
    if(valid != 0)
        throw std::runtime_error(
                "Error in channel adapter - outputs must be even divisor of inputs. Here" 
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
            std::copy(previous->outputs[0], previous->outputs[0] + this->bloc_size, this->outputs[ch]);
    } else { // need to check if % is 0
        if(this->n_inputs > this->n_outputs && ((this->n_inputs % this->n_outputs) == 0 ) )  
        {
            for(size_t o = 0; o < this->n_outputs; ++o)
            {
                for(size_t s = 0; s < this->bloc_size; ++s)
                {
                    Flt sum = 0;
                    for(size_t i = o; i < this->n_inputs; i+= this->n_outputs)
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
                std::copy(previous->outputs[i], previous->outputs[i] + this->bloc_size, this->outputs[o]);
            }
        }
    }
}

template class channel_adapter<double>;
template class channel_adapter<float>;


template<typename Flt>
mixer<Flt>::mixer(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , process_count(0)
{
    this->set_name("Mixer");
    
    for(size_t ch = 0; ch < this->n_inputs; ++ch)
        std::memset(this->outputs[ch], 0, this->bloc_size * sizeof(Flt));
}

template<typename Flt>
void mixer<Flt>::process(node<Flt> *previous)
{
    if(this->n_nodes_in == 0) {
        throw std::runtime_error("Mixer node must have at least one input (ideally two or more, it is a mixer)");
        return;
    }
    for(size_t ch = 0; ch < this->n_inputs; ++ch)
    {
        if(this->process_count == 0)
            std::memset(this->outputs[ch], 0, this->bloc_size * sizeof(Flt));
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

template class mixer<double>;
template class mixer<float>;

template<typename Flt>
parallelizer<Flt>::parallelizer(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , current_channel(0)
{}

template<typename Flt>
void parallelizer<Flt>::process(node<Flt> *previous) 
{
    if(this->process_count == 0)
        this->current_channel = 0;
    for(size_t ch = 0; ch < previous->n_outputs; ++ch)
    {
        std::copy(previous->outputs[ch], previous->outputs[ch] + previous->bloc_size, 
                this->outputs[current_channel]);
        current_channel = (current_channel + 1) % this->n_inputs;
    }    

    this->process_count = (this->process_count + 1) % this->n_nodes_in;
}

template<typename Flt>
bool parallelizer<Flt>::handles_parallel() {return true;}

template class parallelizer<double>;
template class parallelizer<float>;

template<typename Flt>
simple_upsampler<Flt>::simple_upsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, 
        size_t order, size_t steep)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , f_order(order)
    , f_steep(steep)
{
    this->set_name("Simple upsampler");
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
upsampler<Flt>::upsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, 
        size_t num_cascade, size_t order, size_t steep)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , n_cascade(num_cascade)
    , f_order(order)
    , f_steep(steep)
{
    this->set_name("Upsampler");
    size_t base_samplerate = this->sample_rate / (n_cascade * 2);
    size_t base_blocsize = this->bloc_size / (n_cascade * 2);
    upsamplers.resize(num_cascade);
    for(size_t i = 0; i < n_cascade; ++i)
    {
        base_samplerate *= 2;
        base_blocsize *= 2;
        this->upsamplers[i] = new simple_upsampler<Flt>(inp, outp, base_blocsize, base_samplerate, 
                f_order, f_steep);
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
        std::copy(p->outputs[ch], p->outputs[ch] + this->bloc_size, this->outputs[ch]);
        //::memcpy(this->outputs[ch], p->outputs[ch], this->bloc_size * sizeof(Flt));
}

template class upsampler<double>;

template<typename Flt>
downsampler<Flt>::downsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, 
        size_t num_cascade, size_t order, size_t steep)
    : node<Flt>::node(inp, outp, blocsize, samplerate)
    , n_cascade(num_cascade)
    , f_order(order)
    , f_steep(steep)
    , n_samps_iter(n_cascade * 2)
{
    this->set_name("Downsampler");
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

/*
    Graph methods
*/

template<typename Flt>
graph<Flt>::graph(size_t inp, size_t outp, size_t blocsize, size_t samplerate) 
    : n_inputs(inp)
    , n_outputs(outp)
    , bloc_size(blocsize)
    , sample_rate(samplerate)
{
    to_call.reserve(128);
    next_call.reserve(128);
    call_list.reserve(256);
    _mix = std::make_unique<mixer<Flt> >(outp, outp, blocsize, samplerate); 
    _input_node = std::make_unique<node<Flt>>(inp, inp, blocsize, samplerate);

    _mix->set_name("GraphOutput");
    _input_node->set_name("GraphInputs");
}

template<typename Flt>
void graph<Flt>::process_bloc()
{
    /*
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    // Push base nodes to the call list
    node<Flt> *ptr = (_input_node->n_inputs > 0) ? _input_node.get() : nullptr;
    next_call.push_back({ptr, &nodes});
    to_call_ptr = &to_call;
    next_call_ptr = &next_call;
    _process_grape();
    */

    for(size_t i = 0; i < call_list.size(); ++i)
    {
        call_list[i].callee->process(call_list[i].caller);
    }
}

template<typename Flt>
void graph<Flt>::add_node(node<Flt> *n)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    nodes.push_back(n);
    _find_and_add_out(n);
    _generate_event_list();
}

template<typename Flt>
void graph<Flt>::add_nodes(std::vector<node<Flt>*> n)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    nodes.insert(nodes.end(), n.begin(), n.end());
    for(auto & it : n)
        _find_and_add_out(it);
    _generate_event_list();
}

template<typename Flt>
void graph<Flt>::remove_node(node<Flt> *n)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    _find_and_remove_out(n);
    _rm_node(n);
    _generate_event_list();
    return;
}

template<typename Flt>
void graph<Flt>::add_output(node<Flt> *o)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    o->connect(_mix.get());
}

template<typename Flt>
void graph<Flt>::remove_output(node<Flt> *o)
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    o->disconnect(_mix.get());
}

template<typename Flt>
std::string graph<Flt>::generate_patchbook_code()
{
    std::string code("");
    _generate_patchbook_code(code);
    return code;
}

template<typename Flt>
void graph<Flt>::_generate_patchbook_code(std::string &c)
{
    for(auto & it : call_list)
    {
        if(it.caller == nullptr)
            continue;
        for(size_t ch = 0; ch < it.caller->n_outputs; ++ch) 
        {
            std::string s = "- " + it.caller->get_name() 
                + " (Out" + std::to_string(ch) + ") >> " 
                + it.callee->get_name() 
                + "(In" + std::to_string(ch) + ")\n";
            c.append(s);
        }
    }
}

template<typename Flt>
void graph<Flt>::_find_and_remove_out(node<Flt> *n)
{
    std::vector<node<Flt> *> *_iter_list = &n->connections;
    for(size_t i = 0; i < _iter_list->size(); ++i)
    {
        if(_iter_list->at(i) == _mix.get()) 
        {
            _iter_list->at(i)->disconnect(_mix.get());
        } else if(_iter_list->at(i)->connections.size() > 0) 
        {
            _find_and_remove_out(_iter_list->at(i));
        }
    }
}

template<typename Flt>
void graph<Flt>::_find_and_add_out(node<Flt> * n)
{
    std::vector<node<Flt> *> *_iter_list = &n->connections;
    if(_iter_list->size() == 0 ) {
        if(n->n_outputs > 0 ) {
            n->connect(_mix.get());
        }
        return;
    }
    for(size_t i = 0; i < _iter_list->size(); ++i)
    {
        if(_iter_list->at(i)->connections.size() == 0 
            && _iter_list->at(i)->n_outputs > 0)  // End of chain
        {
            _iter_list->at(i)->connect(_mix.get());
        } else 
        {
            bool found = false;
            for(size_t n = 0; n < _iter_list->at(i)->connections.size(); ++n)
            {
                if(_iter_list->at(i)->connections[n] == _mix.get())
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                _find_and_add_out(_iter_list->at(i));
        }
    }
}

template<typename Flt>
void graph<Flt>::_rm_node(node<Flt> *n)
{
    for(size_t i = 0; i < nodes.size(); ++i)
    {
        if(n == nodes[i])
        {
            nodes.erase(nodes.begin() + i);
            return;
        }   
    }
}

template<typename Flt>
void graph<Flt>::_process_grape()
{
    std::swap(to_call_ptr, next_call_ptr);
    next_call_ptr->clear();
    if(to_call_ptr->size() == 0) 
    {
        return;
    }
    for(size_t i = 0; i < to_call_ptr->size(); ++i)
    {
        node<Flt> *caller = to_call_ptr->at(i).caller; // nullptr at first pass

        for(size_t j = 0; j < to_call_ptr->at(i).callee->size(); ++j)
        { 
            call_list.push_back({caller, to_call_ptr->at(i).callee->at(j)});
            std::vector<node<Flt> *> *nxt = &(to_call_ptr->at(i).callee->at(j)->connections);
            next_call_ptr->push_back({to_call_ptr->at(i).callee->at(j), nxt});
        }
    }
    this->_process_grape();
}

template<typename Flt>
void graph<Flt>::_remove_duplicates()
{
    std::vector<int> to_erase(32);
    for(size_t i = 0; i < call_list.size(); ++i)
    {
        for(size_t j = 0; j < call_list.size(); ++j)
        {
            if(j == i)
                continue;
            if(call_list[i].caller == call_list[j].caller 
                && call_list[i].callee == call_list[j].callee)
            {
                std::cout << "remove duplicate : " << call_list[j].caller->get_name() << " -> "  << call_list[j].callee->get_name() << std::endl;
                call_list.erase(call_list.begin() + j);
                --j;
            }
        }
    }
}

template<typename Flt>
void graph<Flt>::_generate_event_list()
{
    std::lock_guard<std::recursive_mutex> lock(_mtx);
    call_list.clear();
    node<Flt> *ptr = (_input_node->n_inputs > 0) ? _input_node.get() : nullptr;
    next_call.push_back({ptr, &nodes});
    to_call_ptr = &to_call;
    next_call_ptr = &next_call;
    _process_grape();
    _remove_duplicates();
}

/*
    Rtgraph engine
*/

template<typename Flt>
rtgraph<Flt>::rtgraph(size_t inp, size_t outp, size_t blocsize, size_t  samplerate)
    : graph<Flt>::graph(inp, outp, blocsize, samplerate)
{
    output_parameters.nChannels = this->n_outputs;
    output_parameters.firstChannel = 0;
    output_parameters.deviceId = dac.getDefaultOutputDevice();
    input_parameters.nChannels = this->n_inputs;
    input_parameters.firstChannel = 0;
    input_parameters.deviceId = dac.getDefaultInputDevice();
    _options = std::make_unique<RtAudio::StreamOptions>();
    _options.get()->flags = RTAUDIO_NONINTERLEAVED | RTAUDIO_MINIMIZE_LATENCY | RTAUDIO_SCHEDULE_REALTIME;
    _options.get()->priority = 99;

    dac.showWarnings(false);
}

template<typename Flt>
void rtgraph<Flt>::list_devices()
{
    std::vector<unsigned int> ids = dac.getDeviceIds();
    RtAudio::DeviceInfo info;
    for(auto & it : ids)
    {
        info = dac.getDeviceInfo(it);
        std::cout << it << " - " << info.name << std::endl;
        std::cout << "\tID : " << info.ID << std::endl;
        std::cout << "\tinputs : " << info.inputChannels << std::endl;
        std::cout << "\toutputs : " << info.outputChannels << std::endl;
        std::cout << "\tpreferred SR : " << info.preferredSampleRate << std::endl;
    }
}

template<typename Flt>
void rtgraph<Flt>::set_devices(unsigned int input_device, unsigned int output_device)
{
    input_parameters.deviceId = input_device;
    output_parameters.deviceId = output_device;
}

template<typename Flt>
void rtgraph<Flt>::start_stream()
{
    if(dac.isStreamOpen())
    {
        std::cout << "Stream is already open" << std::endl;
        return;
    }
    RtAudio::StreamParameters *in_param_ptr = (this->n_inputs > 0) ? (&input_parameters) : nullptr;
    dac.openStream(&output_parameters, in_param_ptr, RTAUDIO_FLOAT64, 
            (unsigned int)this->sample_rate, (unsigned int *)&this->bloc_size, 
            &rtgraph_callback, (void *)this, _options.get());
    dac.startStream();
}

template<typename Flt>
void rtgraph<Flt>::stop_stream()
{
    if(dac.isStreamOpen()) {
        dac.stopStream();
    }
}

int rtgraph_callback(void *out_buffer, void *in_buffer, 
        unsigned int nframes, double stream_time, RtAudioStreamStatus status, void *user_data)
{
    rtgraph<double> *_graph = (rtgraph<double> *)user_data;
    std::lock_guard<std::recursive_mutex> lock(_graph->_mtx);
    double *inputs = (double *)in_buffer;
    double *outputs = (double *) out_buffer;

    for(size_t i = 0; i < _graph->n_inputs; ++i)
    {
        size_t offset = _graph->bloc_size * i;
        std::copy(inputs+offset, inputs+offset + nframes, _graph->_input_node->outputs[i]);
    }
    _graph->process_bloc();
    for(size_t o = 0; o < _graph->n_outputs; ++o)
    {
        size_t offset = _graph->bloc_size * o;
        std::copy(_graph->_mix->outputs[o],  _graph->_mix->outputs[o] + _graph->bloc_size, outputs + offset);
    }
    return 0;
}