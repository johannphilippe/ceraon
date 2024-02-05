#include "../include/combinator3000_api.h"

node<double> *create_node(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new node<double>(inp, outp, blocsize, samplerate);
}

channel_adapter<double> *create_channel_adapter(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new channel_adapter<double>(inp, outp, blocsize, samplerate);
}

mixer<double> *create_mixer(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new mixer<double>(inp, outp, blocsize, samplerate);
}

upsampler<double> *create_upsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, 
    size_t num_cascade, size_t order, size_t steep)
{
    return new upsampler<double>(inp, outp, blocsize, samplerate, num_cascade, order, steep);    
}

downsampler<double> *create_downsampler(size_t inp, size_t outp, size_t blocsize, size_t samplerate, 
    size_t num_cascade, size_t order, size_t steep)
{
    return new downsampler<double>(inp, outp, blocsize, samplerate, num_cascade, order, steep);    
}

graph<double> *create_graph(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new graph<double>(inp, outp, blocsize, samplerate);
}

rtgraph<double> *create_rtgraph(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new rtgraph<double>(inp, outp, blocsize, samplerate);
}

#ifdef FFT_NODE
#include "fft/fft_node.h"
node<double> *create_fft_node(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new fft_node<double>(inp, outp, blocsize, samplerate);
}

node<double> *create_ifft_node(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new ifft_node<double>(inp, outp, blocsize, samplerate);
}
#else
node<double> *create_fft_node(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    throw std::runtime_error("FFT node is not enabled. Recompile to enable it.");
    return new node<double>(inp, outp, blocsize, samplerate);
}

node<double> *create_ifft_node(size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    throw std::runtime_error("FFT node is not enabled. Recompile to enable it.");
    return new node<double>(inp, outp, blocsize, samplerate);
}
#endif


#ifdef CSOUND_NODE
#include "csound/csound_node.h"
node<double> *create_csound_node(const char *csd_str, size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return new csound_node<double>(std::string(csd_str), inp, outp, blocsize, samplerate);
}
node<double> *create_csound_node_from_file(const char *csd_path, size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    return csound_node<double>::from_file(std::string(csd_path), inp, outp, blocsize, samplerate);
}
#else 
node<double> *create_csound_node(const char *csd_str, size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    throw std::runtime_error("Csound node is not enabled");
    return new node<double>(inp, outp, blocsize, samplerate);
}
node<double> *create_csound_node_from_file(const char *csd_path, size_t inp, size_t outp, size_t blocsize, size_t samplerate)
{
    throw std::runtime_error("Csound node is not enabled");
    return new node<double>(inp, outp, blocsize, samplerate);
}
#endif

#ifdef FAUST_JIT_NODE
#include "faust/faust_jit_node.h"
void *create_faust_jit_factory_from_file(const char *path)
{
    return (void *)faust_jit_factory<double>::from_file(path);
}
void *create_faust_jit_factory_from_string(const char *dsp_str, const char *dsp_name)
{
    return (void *)faust_jit_factory<double>::from_string(dsp_str, dsp_name);
}

void delete_faust_jit_factory(void *f)
{
    delete (faust_jit_factory<double> *)f;
}

node<double> *create_faust_jit_node(void *factory, size_t blocsize, size_t samplerate)
{
    return new faust_jit_node<double>( (faust_jit_factory<double> *)factory, blocsize, samplerate );
}
#else
void *create_faust_jit_factory_from_file(const char *path)
{
    throw std::runtime_error("Faust JIT node is not enabled");
    return (void *)nullptr;
}
void *create_faust_jit_factory_from_string(const char *dsp_str, const char *dsp_name)
{
    throw std::runtime_error("Faust JIT node is not enabled");
    return (void *)nullptr;
}

void delete_faust_jit_factory(void *f)
{
    throw std::runtime_error("Faust JIT node is not enabled");
}

node<double> *create_faust_jit_node(void *factory, size_t blocsize, size_t samplerate)
{
    throw std::runtime_error("Faust JIT node is not enabled");
    return new node<double>( (1, 1, blocsize, samplerate );
}
#endif

void delete_node(node<double> *n) {delete n;}
void delete_graph(graph<double> *g) {delete g;}

bool node_connect(node<double> *a, node<double> *b) {return a->connect(b);}
bool node_disconnect(node<double> *a, node<double> *b) {return a->disconnect(b);}
void node_process(node<double> *n, node<double> *previous) {n->process(previous);}

void csound_node_set_channel(void *cs_ptr, const char *name, double value)
{
#ifdef CSOUND_NODE 
    ((csound_node<double> *) cs_ptr)->SetChannel(name, value);
#endif
}

double csound_node_get_channel(void *cs_ptr, const char *name)
{

#ifdef CSOUND_NODE 
    return ((csound_node<double> *) cs_ptr)->GetChannel(name);
#else 
    throw std::runtime_error("Csound Node is not enabled");
    return 0;
#endif
}

void csound_node_compile_score(void *cs_ptr, const char *score)
{
#ifdef CSOUND_NODE 
    ((csound_node<double> *)cs_ptr)->ReadScore(score);
#endif
}

void csound_node_compile_orc(void *cs_ptr, const char *orc)
{
#ifdef CSOUND_NODE 
    ((csound_node<double> *)cs_ptr)->CompileOrc(orc);
#endif
}

void faust_jit_set_param(void *faust_dsp, const char *name, double value)
{
#ifdef FAUST_JIT_NODE
    std::string cpp_name(name);
    ((faust_jit_node<double> *)faust_dsp)->setParamValue(cpp_name, value);
#endif
}

double faust_jit_get_param(void *faust_dsp, const char *name)
{
#ifdef FAUST_JIT_NODE
    std::string cpp_name(name);
    return ((faust_jit_node<double> *)faust_dsp)->getParamValue(cpp_name);
#else
    throw std::runtime_error("Faust node is not enabled");
    return 0;
#endif
}

void graph_add_node(graph<double> *g, node<double> *n) {g->add_node(n);}
void graph_remove_node(graph<double> *g, node<double> *n) {g->remove_node(n);}
void graph_add_output(graph<double> *g, node<double> *o) {g->add_output(o);}
void graph_remove_output(graph<double> *g, node<double> *o) {g->remove_output(o);}

void graph_process_bloc(graph<double> *g) {g->process_bloc();}

void rtgraph_set_devices(rtgraph<double> *g, unsigned int in_device, unsigned int out_device) {g->set_devices(in_device, out_device);}

void rtgraph_start_stream(rtgraph<double> *g) {g->start_stream();}
void rtgraph_stop_stream(rtgraph<double> *g) {g->stop_stream();}
