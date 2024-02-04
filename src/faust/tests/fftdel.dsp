import("stdfaust.lib");

effect(d) = de.delay(d, d) : ve.korg35LPF(0.6, 3);

process = _,_ : effect(256), effect(128);