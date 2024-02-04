#!/bin/bash

faust -double -a simple.arch -cn filter filter.dsp -o filter.hpp
faust -double -a simple.arch -cn osc osc.dsp -o osc.hpp
faust -double -a simple.arch -cn square square.dsp -o square.hpp
