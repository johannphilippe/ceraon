#ifndef FAUST_JIT_NODE_H
#define FAUST_JIT_NODE_H

#define FAUSTFLOAT double

#include "combinator3000.h"

#include "faust/dsp/libfaust.h"
#include "faust/dsp/llvm-dsp.h"
#include "faust/gui/MapUI.h"

template<typename Flt = double>
struct faust_jit_factory
{
    faust_jit_factory() {}
    ~faust_jit_factory()
    {
        deleteDSPFactory(m_factory);
    }

    static faust_jit_factory<Flt> *from_file(std::string path, std::vector<std::string> *opts = nullptr) {
        faust_jit_factory *f = new faust_jit_factory<Flt>();
        std::string _err(faust_jit_factory::err);
        std::vector<const char*> argv = faust_jit_factory::parse_args(opts);
        f->m_factory = createDSPFactoryFromFile(path, argv.size()-1, argv.data(), "", _err, -1);
        return f;
    }
    static faust_jit_factory<Flt> *from_string(std::string code, std::vector<std::string> *opts = nullptr) {
        faust_jit_factory *f = new faust_jit_factory<Flt>();
        std::string _err(faust_jit_factory::err);
        std::vector<const char*> argv = faust_jit_factory::parse_args(opts);
        f->m_factory = createDSPFactoryFromString("faust", code, argv.size()-1, argv.data(), "", _err, -1);
        return f;
    }
    static std::vector<const char *> parse_args(std::vector<std::string> *opts = nullptr)
    {
        std::vector<const char *> argv = {(char*)"-vec", (char*)"-double"};
        if(opts != nullptr && opts->size() > 0)
        {
            for(size_t i = 0; i < opts->size(); ++i)
                argv.push_back((char*)opts->at(i).data());
        }
        argv.push_back(nullptr);
        return argv;
    }

    llvm_dsp_factory *m_factory;
    constexpr static const char *err = "Faust LLVM DSP Factory creation error";
};

template<typename Flt = double>
struct faust_jit_node : public MapUI, public node<Flt>
{
    faust_jit_node(faust_jit_factory<Flt> *factory, size_t blocsize = 128, size_t samplerate = 48000) 
        : node<Flt>::node{1, 1, blocsize, samplerate}
    {
        std::cout << "instanciate " << std::endl;
        m_dsp = factory->m_factory->createDSPInstance();
        m_dsp->buildUserInterface(this);
        m_dsp->init(this->sample_rate);

        this->n_inputs = m_dsp->getNumInputs();
        this->n_outputs = m_dsp->getNumOutputs();

        this->outputs = new Flt*[this->n_outputs];
        for(size_t i = 0; i < this->n_outputs; ++i)
        {
            this->outputs[i] = new Flt[this->bloc_size];
        }
        // Create buffers
    }

    ~faust_jit_node()
    {
        delete m_dsp;
    }

    void process(node<Flt> *previous)
    {
        std::cout << "process " << std::endl;
        Flt **inputs = nullptr;
        if(this->n_inputs > 0) 
            inputs = previous->outputs;
        m_dsp->compute(this->bloc_size, inputs, this->outputs);
    }

    dsp *m_dsp;
};
#endif