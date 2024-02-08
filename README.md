# Combinator 3000

Audio graph generator for multi-environment combination.
Experimental / in development.

## Mechanism 

Main component is `node`. A Node is an audio processor. 
It has : 
    * A number of inputs and outputs
    * An output buffer for samples processing (input buffer is always owned by previous node)
    * A `process` method (compute)
    * A vector of `connection` (represent output connections)
    * A `connect` method to connect a node to another

Nodes can be put in a `graph` object or a `rtgraph`. 
A `graph` handles a list of `node` (starting of chain) and generates an internal list of events to trigger, 
automatically finding which node is connected to which. 
Nodes must be connected together BEFORE the "starting node" of the chain is put in the graph.

## Dependencies 

Some nodes won't compile unless you have installed : 
- Faust (with libfaust and LLVM support)
- Csound 
- libsndfile

## TODO 

- Add Nodegroup ?
- Move AudioFFT to copy rather than submodule (keep double version instead of float)

- Move everything (node<Flt> * and connection *) to shared pointer as soon as possible 
- Graph drawing (add Faust graph generator)
- Optimizations in general
  - The graph class must have some optim to do
  - A general memory pool to allocate buffers 
  - In the `graph::process_bloc` function, while performing, it could gather information on nodes to optimize (in realtime)
- Blocsize adapter (without resampling - could be implemented like a circular buffer)
- Thread safety and threading model 
  - Thread locks
  - Idea that several graphs could be used on separated threads to improve performance (taking advantage of several cores)

- Done - FFT node 
- Done - Faust LLVM node
- Done - Csound node
- Done - Sndfile node : still need to 
  - Expose to API
  - Add loop mode (optional)
  - Add change sndfile mode (through function)
  
- API to create external nodes (C++ or Faust) and build it (so API to link dynamic libraries at runtime)