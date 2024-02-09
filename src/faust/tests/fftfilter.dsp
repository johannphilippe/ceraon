import("stdfaust.lib");
gain = hslider("gain", 1, 0, 1, 0.01);
// cut_bin = hslider("cut_bin", 1024, 0, 1024, 1);
// High−shelf cutoff frequency in Hz
cut = hslider("cut", 440, 0, 24000, 0.1);
// global variable set by the processor itself
fftSize = hslider("fftSize", 1024, 2, 16384, 1);
// FFT bin index of the cutoff frequency
cut_bin = cut / (ma.SR / fftSize);
process(real, imag, bin) = real * gain_bin, imag * gain_bin, bin 
with {
    // Check if the bin is lower than the cut_bin
    low = bin < cut_bin;
    // If lower, use the parameter value, else 1
    gain_bin = (low == 0) * gain + low;
};
