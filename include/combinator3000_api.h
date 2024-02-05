#ifndef COMBINATOR3000_API_H
#define COMBINATOR3000_API_H

#include "combinator3000.h"
#include "fft/fft_node.h"

/*
    Constructors
*/

node<double> *create_node(
        size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000
    );

channel_adapter<double> *create_channel_adapter(
        size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000
    );

mixer<double> *create_mixer(
        size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000
    );

upsampler<double> *create_upsampler(
        size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000, 
        size_t num_cascade = 1, size_t order = 10, size_t steep = 1
    );

downsampler<double> *create_downsampler(
        size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000,
        size_t num_cascade = 1, size_t order = 10, size_t steep = 1
    );

graph<double> *create_graph(
        size_t num_outputs = 1, size_t blocsize = 128, size_t samplerate = 48000
    );

/*
    Plugin nodes - only enabled if compiled (see CMake options)
*/
node<double> *create_fft_node(
        size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000
    );

node<double> *create_ifft_node(
        size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000 
    );

node<double> *create_csound_node(
        const char *csd_str, size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000
    );

node<double> *create_csound_node_from_file(
        const char *csd_file, size_t inp = 0, size_t outp = 0, size_t blocsize = 128, size_t samplerate = 48000
    );

void *create_faust_jit_factory_from_string(
        const char *dsp_str
    );

void *create_faust_jit_factory_from_file(
        const char *dsp_path
    );

void delete_faust_jit_factory(void *factory);

node<double> *create_faust_jit_node(
        void *factory, size_t blocsize = 128, size_t samplerate = 48000
    );

/*
    Destructors
*/
void delete_node(node<double> *n);
void delete_graph(graph<double> *g);


/*
    Node Methods
*/
bool node_connect(node<double> *n1, node<double> *n2);
bool node_disconnect(node<double> *n1, node<double> *n2);
void node_process(node<double> *n, node<double> *previous);

/*
    Graph Methods
*/
void graph_add_node(graph<double> *g, node<double> *n);
void graph_process_bloc(graph<double> *g);

#endif