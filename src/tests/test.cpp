#include<iostream>
#include<thread>

#include "combinator3000.h"
#include"faust/faust_node.h"
#include "faust/faust_jit_node.h"
#include"faust/tests/osc.hpp"
#include"faust/tests/filter.hpp"
#include"faust/tests/square.hpp"
#include "faust/tests/fftdel.hpp"
#include "faust/tests/fftfreeze.hpp"
#include "csound/csound_node.h"
#include "fft/fft_node.h"

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

void simple_fft_test()
{
    SndfileHandle wf("/home/johann/Documents/tmp/fft.wav", SFM_WRITE, SF_FORMAT_WAV |SF_FORMAT_PCM_24, 1, 48000);

    faust_node<osc, double> *o1 = new faust_node<osc, double>();
    fft_node<double> *_fft = new fft_node<double>(1, 2, 128, 48000);
    ifft_node<double> *_ifft = new ifft_node<double>(2, 1, 128, 48000);

    std::cout << "Nodes init OK " << std::endl;

    o1->connect(_fft);
    _fft->connect(_ifft);

    std::cout << "Nodes connected " << std::endl;

    graph<double> g;
    g.add_node(o1);

    std::cout << "Graph init OK " << std::endl;

    size_t dur = 10; // seconds 
    size_t nsamps_total = dur * 48000;
    size_t npasses = nsamps_total / 128;
    for(size_t i = 0; i < npasses; ++i)
    {
        g.process_block();
        wf.writef(_ifft->outputs[0], 128);
    }
}

void fft_denoise_test()
{
    faust_node<osc, double> *o1 = new faust_node<osc, double>();
    upsampler<double> *up = new upsampler<double>(1, 1, 512, 192000, 2, 10, 1);
    
    fft_node<double> *_fft = new fft_node<double>(1, 3, 512, 192000);
    faust_node<fftfreeze, double> *_delfft = new faust_node<fftfreeze, double>(255, 192000);
    ifft_node<double> *_ifft = new ifft_node<double>(3, 1, 512, 192000);

    downsampler<double> *down = new downsampler<double>(1, 1, 128, 48000, 2, 10, 1);
    
    _delfft->setParamValue("fftSize", 512);
    //_delfft->setParamValue("freezeBtn", 1);

    o1->connect(up);
    up->connect(_fft);
    _fft->connect(_delfft);
    _delfft->connect(_ifft);
    _ifft->connect(down);

    graph<double> g;
    g.add_node(o1);

    SndfileHandle wf("/home/johann/Documents/tmp/fft_delay.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_24, 1, 48000);
    size_t dur = 10; // seconds 
    size_t nsamps_total = dur * 48000;
    size_t npasses = nsamps_total / 128;
    for(size_t i = 0; i < npasses; ++i)
    {
        if(i < npasses/2 )
        {
            _delfft->setParamValue("freezeBtn", 1);
        } else 
            _delfft->setParamValue("freezeBtn", 0);
        g.process_block();
        wf.writef(down->outputs[0], 128);
    }
}


void faust_jit_test()
{
    SndfileHandle wf("/home/johann/Documents/tmp/faust_jit.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_24, 1, 48000);

    faust_jit_factory<double> *fac = faust_jit_factory<double>::from_string("import(\"stdfaust.lib\"); process = os.sawtooth(100) * 0.2;");
    faust_jit_node<double> *dsp = new faust_jit_node<double>(fac, 128, 48000);
    faust_node<filter, double> *filt = new faust_node<filter, double>(128, 48000);

    dsp->connect(filt);

    graph<double> g;
    g.add_node(dsp);

    size_t dur = 10; // seconds 
    size_t nsamps_total = dur * 48000;
    size_t npasses = nsamps_total / 128;
    for(size_t i = 0; i < npasses; ++i)
    {
        g.process_block();
        wf.writef(filt->outputs[0], 128);
    }
}

void csound_faust_test()
{
    faust_jit_factory<double> *fac = faust_jit_factory<double>::from_string("import(\"stdfaust.lib\"); freq = hslider(\"freq\", 100, 50, 1000, 0.1); process = os.sawtooth(freq) * 0.2;");
    faust_jit_node<double> *fdsp = new faust_jit_node<double>(fac, 128, 48000);
    fdsp->setParamValue("freq", 300);
    std::string csd = "" \
    "<CsoundSynthesizer>\n" \
    "<CsOptions>\n" \
    "</CsOptions>\n" \
    "<CsInstruments> \n" \
        "sr = 44100 \n" \
        "ksmps = 41 \n" \
        "nchnls = 8 \n" \
        "nchnls_i = 12 \n" \
    "instr 1 \n" \
        "ain = inch(1) \n" \
        "adel = abs(oscili:a(0.02, 0.5)) \n" \
        "kfb  = 0.7 \n" \
        "ao = flanger(ain, adel, kfb) \n" \
        "outch 1, ao \n" \
    "endin \n" \
    "</CsInstruments> \n" \
    "<CsScore> \n" \
        "f 0 z \n" \
        "i 1 0 -1 \n" \
    "</CsScore> \n" \
    "</CsoundSynthesizer> \n";
    std::cout << csd << std::endl;
    csound_node<double> *csn = new csound_node<double>(csd, 1, 1, 128, 48000);

    std::cout << "faust inout " <<  fdsp->n_inputs << " & " << fdsp->n_outputs << std::endl;

    std::cout << "connecting " << std::endl;

    fdsp->connect(csn);

    std::cout << "connected " << std::endl;
    graph<> g;
    g.add_node(fdsp);

    std::cout << "added to graph " << std::endl;

    SndfileHandle wf("/home/johann/Documents/tmp/csound_faust_jit.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_24, 1, 48000);
    size_t dur = 10; // seconds 
    size_t nsamps_total = dur * 48000;
    size_t npasses = nsamps_total / 128;
    for(size_t i = 0; i < npasses; ++i)
    {
        std::cout << "bloc graph " << std::endl;
        g.process_block();
        wf.writef(csn->outputs[0], 128);
    }
}

int main()
{
    //simple_test();
    //mix_test();
    //resampler_test();
    //rt_test();
    //simple_fft_test();
    //fft_denoise_test();
    //faust_jit_test();
    csound_faust_test();
    return 0;
}