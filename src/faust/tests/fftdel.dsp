import("stdfaust.lib");

freq = hslider("freq", 5, 1, 100, 0.1);
process = _, _, _ : _, *(os.osc(freq)), _;
