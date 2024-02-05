# Combinator 3000

Audio graph generator for multi-environment combination.
Experimental / in development / and still a concept.

## Mechanism 

Main component is `node`. A Node is an audio processor. 
It has : 
    * A `process` method (compute)
    * A vector of `node` (next step of graph)
    * A number of inputs and outputs
    * An output buffer for samples processing (input buffer is always owned by previous node)

```
node_1  -> node_effect1
        -> node_effect2
        -> ...

node_2  -> node_effect21
        -> node_effect22
        -> ...

```

For now, I should find a way to mix nodes in a graph. As described above, it can be splitted, but not merged.
It would be cool to be able to detect that without a "prepare" function, so it can be connected/disconnected in realtime.


Luckily : 
* The number of outputs channels of a node may be the same as the number of input channels of the next (connected) ones.
But
* It may not be. In such case, a channel_adapter node is used (automatically). If number of outputs and inputs (of next node) is even divisor (3 and 6 for example), 
then the operation can happen. Else, it raises an error.
* Same applies for bloc_size (adapter if not the same) and samplerate (resampler can be user to upsample or downsample)

## Dependencies 


## Submodules 

- RtAudio
- AudioFFT (optional)


## TODO 

- Change strategy for graph : 
  - Add a list of ordered events to process 
  - In the function generating the list, remove ALL duplicates (due to several connects)
  - Refresh the list when add node or remove node

- Case of a graph that upsamples and does not downsample : output bloc size is > to input bloc size : handle it 

- Bloc size adapter : useless. Any node could perfectly use a smaller bloc, and perform higher accuracy internally.

- Move to shared pointer as soon as possible 
- Graph drawing (patchbook, graphviz, or similar)

- Optimizations in general
  - The graph class must have some optim to do
  - A general memory pool to allocate buffers 
  - In the `graph::process_bloc` function, while performing, it could gather information on nodes to optimize (in realtime)
- Blocsize adapter (without resampling) - (?)
- Thread safety and threading model 
  - Thread locks
  - Idea that several graphs could be used on separated threads to improve performance (taking advantage of several cores)
- Prevent multiple identical connections (same node to same node) [and self connections ?, or enable feedback ?]

- Done - FFT node 
- Done - Faust LLVM node
- Done - Csound node -> Add from file constructor
- Supercollider node  ? (not sure there is a public API to handle audio)
- Convolution FIR Node (with overlap add) ? 
- Base nodes (gain, fadein, fadeout etc)
- RtAudio nodes
- API to create external nodes and build it (so API to link dynamik libraries at runtime)