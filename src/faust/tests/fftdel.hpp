/* ------------------------------------------------------------
name: "fftdel"
Code generated with Faust 2.71.0 (https://faust.grame.fr)
Compilation options: -a simple.arch -lang cpp -ct 1 -cn fftdel -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0
------------------------------------------------------------ */

#ifndef  __fftdel_H__
#define  __fftdel_H__

/************************************************************************
 IMPORTANT NOTE : this file contains two clearly delimited sections :
 the ARCHITECTURE section (in two parts) and the USER section. Each section
 is governed by its own copyright and license. Please check individually
 each section for license and copyright information.
 *************************************************************************/

/******************* BEGIN minimal-effect.cpp ****************/
/************************************************************************
 FAUST Architecture File
 Copyright (C) 2003-2019 GRAME, Centre National de Creation Musicale
 ---------------------------------------------------------------------
 This Architecture section is free software; you can redistribute it
 and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3 of
 the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; If not, see <http://www.gnu.org/licenses/>.
 
 EXCEPTION : As a special exception, you may create a larger work
 that contains this FAUST architecture section and distribute
 that work under terms of your choice, so long as this FAUST
 architecture section is not modified.
 
 ************************************************************************
 ************************************************************************/
 
#include <algorithm>

#include "faust/gui/UI.h"
#include "faust/gui/meta.h"
#include "faust/dsp/dsp.h"

#if defined(SOUNDFILE)
#include "faust/gui/SoundUI.h"
#endif

using std::max;
using std::min;

/******************************************************************************
 *******************************************************************************
 
 VECTOR INTRINSICS
 
 *******************************************************************************
 *******************************************************************************/


/********************END ARCHITECTURE SECTION (part 1/2)****************/

/**************************BEGIN USER SECTION **************************/

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS fftdel
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif


class fftdel : public dsp {
	
 private:
	
	int fSampleRate;
	double fConst0;
	double fConst2;
	double fConst4;
	int IOTA0;
	double fVec0[512];
	double fConst5;
	double fConst6;
	double fConst7;
	double fRec0[2];
	double fRec1[2];
	double fRec2[2];
	double fVec1[256];
	double fRec4[2];
	double fRec5[2];
	double fRec6[2];
	
 public:
	fftdel() {}

	void metadata(Meta* m) { 
		m->declare("compile_options", "-a simple.arch -lang cpp -ct 1 -cn fftdel -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.1.0");
		m->declare("filename", "fftdel.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.7.0");
		m->declare("name", "fftdel");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("vaeffects.lib/korg35LPF:author", "Eric Tarr");
		m->declare("vaeffects.lib/korg35LPF:license", "MIT-style STK-4.3 license");
		m->declare("vaeffects.lib/name", "Faust Virtual Analog Filter Effect Library");
		m->declare("vaeffects.lib/version", "1.2.1");
	}

	virtual int getNumInputs() {
		return 2;
	}
	virtual int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::tan(3964.4219162949976 / std::min<double>(1.92e+05, std::max<double>(1.0, double(fSampleRate))));
		double fConst1 = fConst0 + 1.0;
		fConst2 = fConst0 / fConst1;
		double fConst3 = 1.0 - fConst2;
		fConst4 = 0.49347241269737024 * fConst3;
		fConst5 = 1.0 / fConst1;
		fConst6 = 1.0 / (1.0 - 0.49347241269737024 * (fConst0 * fConst3 / fConst1));
		fConst7 = 2.0 * fConst2;
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
		IOTA0 = 0;
		for (int l0 = 0; l0 < 512; l0 = l0 + 1) {
			fVec0[l0] = 0.0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			fRec0[l1] = 0.0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec1[l2] = 0.0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec2[l3] = 0.0;
		}
		for (int l4 = 0; l4 < 256; l4 = l4 + 1) {
			fVec1[l4] = 0.0;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec4[l5] = 0.0;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec5[l6] = 0.0;
		}
		for (int l7 = 0; l7 < 2; l7 = l7 + 1) {
			fRec6[l7] = 0.0;
		}
	}
	
	virtual void init(int sample_rate) {
		classInit(sample_rate);
		instanceInit(sample_rate);
	}
	
	virtual void instanceInit(int sample_rate) {
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
	}
	
	virtual fftdel* clone() {
		return new fftdel();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("fftdel");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			fVec0[IOTA0 & 511] = double(input0[i0]);
			double fTemp0 = fVec0[(IOTA0 - 256) & 511] - fRec2[1];
			double fTemp1 = fConst6 * (fRec2[1] + fConst5 * (fConst0 * fTemp0 + fConst4 * fRec0[1] - fRec1[1])) - fRec0[1];
			fRec0[0] = fRec0[1] + fConst7 * fTemp1;
			double fTemp2 = fRec0[1] + fConst2 * fTemp1;
			fRec1[0] = fRec1[1] + fConst7 * (0.49347241269737024 * fTemp2 - fRec1[1]);
			fRec2[0] = fRec2[1] + fConst7 * fTemp0;
			double fRec3 = fTemp2;
			output0[i0] = FAUSTFLOAT(fRec3);
			fVec1[IOTA0 & 255] = double(input1[i0]);
			double fTemp3 = fVec1[(IOTA0 - 128) & 255] - fRec6[1];
			double fTemp4 = fConst6 * (fRec6[1] + fConst5 * (fConst0 * fTemp3 + fConst4 * fRec4[1] - fRec5[1])) - fRec4[1];
			fRec4[0] = fRec4[1] + fConst7 * fTemp4;
			double fTemp5 = fRec4[1] + fConst2 * fTemp4;
			fRec5[0] = fRec5[1] + fConst7 * (0.49347241269737024 * fTemp5 - fRec5[1]);
			fRec6[0] = fRec6[1] + fConst7 * fTemp3;
			double fRec7 = fTemp5;
			output1[i0] = FAUSTFLOAT(fRec7);
			IOTA0 = IOTA0 + 1;
			fRec0[1] = fRec0[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec4[1] = fRec4[0];
			fRec5[1] = fRec5[0];
			fRec6[1] = fRec6[0];
		}
	}

};

/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/

// Factory API

/******************* END minimal-effect.cpp ****************/

#endif
