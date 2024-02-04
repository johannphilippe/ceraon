import("stdfaust.lib");

freq = os.osc(0.2) : abs : *(500) : +(30);
process = os.sawtooth( freq ) * 0.1;