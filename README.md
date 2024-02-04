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

- Graph : must have a buffer, and take output when the "process block" is ending (so that only need to pass graph as user data)
- Bloc size adapter : useless. Any node could perfectly use a smaller bloc, and perform higher accuracy internally.

- Optimizations in general
  - The graph class must have some optim to do
  - A general memory pool to allocate buffers 
- Blocsize adapter (without resampling) - (?)
- Thread safety and threading model 
  - Thread locks
  - Idea that several graphs could be used on separated threads to improve performance (taking advantage of several cores)
- Prevent multiple identical connections (same node to same node) [and self connections ?, or enable feedback ?]

- FFT node 
- Faust LLVM node
- Csound node
- Supercollider node 
- Base nodes (gain, fadein, fadeout etc)
- RtAudio nodes
- For RtAudio, FFT & Faust LLVM (Csound and SC too) try to put options to build it or not (not always needed)
- API to create external nodes and build it (so API to link dynamik libraries at runtime)