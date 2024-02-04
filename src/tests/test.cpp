#include<iostream>
#include<thread>

#include "combinator3000.h"
#include"faust_node.h"
#include"faust/osc.hpp"
#include"faust/filter.hpp"
#include"faust/square.hpp"

#include "sndfile.hh"

void simple_test()
{
    faust_node<osc, double> *o = new faust_node<osc, double>(1024);
    faust_node<filter, double> *f = new faust_node<filter, double>(1024);
    o->connect(f);

    graph<double> g;
    g.add_node(o);
    g.process_block();
}

// Issue : 
// The mixer childs are executed twice, since mixer is considered twice a caller
void mix_test()
{
    SndfileHandle outfile("/home/johann/Documents/tmp/sawsquare_mixer_filt.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_32, 1, 48000 );

    faust_node<osc, double> *o1 = new faust_node<osc, double>();
    faust_node<square, double> *o2 = new faust_node<square, double>();
    mixer_node<double> *m = new mixer_node<double>(1, 1);
    faust_node<filter, double> *f = new faust_node<filter, double>();

    // First mix signals 
    o1->connect(m);
    o2->connect(m);
    // Then send mixer output to filter
    m->connect(f);

    graph<double> g;
    g.add_node(o1);
    g.add_node(o2);

    // Buffer of 128 
    size_t dur = 10; // seconds 
    size_t nsamps_total = dur * 48000;
    size_t npasses = nsamps_total / dur;
    for(size_t i = 0; i < npasses; ++i) 
    {
        std::cout << "npasses : " << i << " / " << npasses << std::endl;
        g.process_block();
        outfile.writef(f->outputs[0], 128);
    }
    
}

void resampler_test()
{
    SndfileHandle tfile("/home/johann/Documents/tmp/temoin.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_32, 1, 48000 );
    SndfileHandle upfile("/home/johann/Documents/tmp/combinator_upsample.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_32, 1, 96000 );
    SndfileHandle downfile("/home/johann/Documents/tmp/combinator_downsample.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_32, 1, 48000 );

    faust_node<osc, double> *o = new faust_node<osc, double>(128, 48000);
    upsampler<double> *up = new upsampler<double>(1, 1, 256, 96000, 1, 10, 1);
    faust_node<filter, double> *f = new faust_node<filter, double>(256, 96000);
    downsampler<double> *down = new downsampler<double>(1, 1, 128, 48000, 1, 10, 1); 

    o->connect(up);
    up->connect(f);
    f->connect(down);

    graph<double> g;
    g.add_node(o);
    g.process_block();

    // Buffer of 128 
    size_t dur = 10; // seconds 
    size_t nsamps_total = dur * 48000;
    size_t npasses = nsamps_total / 128;
    for(size_t i = 0; i < npasses; ++i) 
    {
        std::cout << "npasses : " << i << " / " << npasses << std::endl;
        g.process_block();
        upfile.writef(f->outputs[0], 256);
        downfile.writef(down->outputs[0], 128);
        tfile.writef(o->outputs[0], 128);
    }
}

#include "rtaudio/RtAudio.h"
#include<vector>
#include<thread>
#include<chrono>

graph<double> *g_ptr;
faust_node<filter, double> *f_ptr;

int rt_callback( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
    std::cout << "callback " << std::endl;
    g_ptr->process_block();
    std::cout << "block processed " << std::endl;
    ::memcpy((double*)outputBuffer, f_ptr->outputs[0], sizeof(double) * f_ptr->bloc_size);
    return 0;
}

void rt_test()
{
    faust_node<osc, double> *o1 = new faust_node<osc, double>();
    faust_node<square, double> *o2 = new faust_node<square, double>();
    mixer_node<double> *m = new mixer_node<double>(1, 1);
    faust_node<filter, double> *f = new faust_node<filter, double>();
    graph<double> g;
    o1->connect(m);
    o2->connect(m);
    m->connect(f);
    g.add_node(o1);
    g.add_node(o2);

    std::cout << "nodes created " << std::endl;
    f_ptr = f;
    g_ptr = &g;

    RtAudio dac;
    RtAudio::StreamParameters parameters;
    parameters.deviceId = dac.getDefaultOutputDevice();
    parameters.nChannels = 1;
    parameters.firstChannel = 0;
    unsigned int sampleRate = 48000;
    unsigned int bufferFrames = 128; 

    std::cout << "RtAudio instanciated, params created " << std::endl;

    RtAudio::StreamOptions opts;
    opts.flags = RTAUDIO_NONINTERLEAVED;
    dac.openStream( &parameters, NULL, RTAUDIO_FLOAT64, sampleRate,
                        &bufferFrames, &rt_callback, (void *)&g , &opts);
    std::cout << "Stream opened " << std::endl;

    dac.startStream();
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main()
{
    //simple_test();
    //mix_test();
    //resampler_test();
    rt_test();
    return 0;
}