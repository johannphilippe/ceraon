/* ------------------------------------------------------------
name: "fftfreeze"
Code generated with Faust 2.71.0 (https://faust.grame.fr)
Compilation options: -a simple.arch -lang cpp -ct 1 -cn fftfreeze -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0
------------------------------------------------------------ */

#ifndef  __fftfreeze_H__
#define  __fftfreeze_H__

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

#include "faust/gui/MapUI.h"
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

#ifndef FAUSTCLASS 
#define FAUSTCLASS fftfreeze
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

static double fftfreeze_faustpower2_f(double value) {
	return value * value;
}

class fftfreeze : public dsp {
	
 private:
	
	FAUSTFLOAT fCheckbox0;
	int IOTA0;
	double fVec0[16384];
	FAUSTFLOAT fHslider0;
	double fVec1[16384];
	double fRec0[2];
	double fVec2[16384];
	double fVec3[16384];
	double fRec1[2];
	FAUSTFLOAT fHslider1;
	int fSampleRate;
	
 public:
	fftfreeze() {}

	void metadata(Meta* m) { 
		m->declare("compile_options", "-a simple.arch -lang cpp -ct 1 -cn fftfreeze -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0");
		m->declare("delays.lib/name", "Faust Delay Library");
		m->declare("delays.lib/version", "1.1.0");
		m->declare("filename", "fftfreeze.dsp");
		m->declare("name", "fftfreeze");
	}

	virtual int getNumInputs() {
		return 3;
	}
	virtual int getNumOutputs() {
		return 3;
	}
	
	static void classInit(int sample_rate) {
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
	}
	
	virtual void instanceResetUserInterface() {
		fCheckbox0 = FAUSTFLOAT(0.0);
		fHslider0 = FAUSTFLOAT(1024.0);
		fHslider1 = FAUSTFLOAT(0.0);
	}
	
	virtual void instanceClear() {
		IOTA0 = 0;
		for (int l0 = 0; l0 < 16384; l0 = l0 + 1) {
			fVec0[l0] = 0.0;
		}
		for (int l1 = 0; l1 < 16384; l1 = l1 + 1) {
			fVec1[l1] = 0.0;
		}
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			fRec0[l2] = 0.0;
		}
		for (int l3 = 0; l3 < 16384; l3 = l3 + 1) {
			fVec2[l3] = 0.0;
		}
		for (int l4 = 0; l4 < 16384; l4 = l4 + 1) {
			fVec3[l4] = 0.0;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec1[l5] = 0.0;
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
	
	virtual fftfreeze* clone() {
		return new fftfreeze();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("fftfreeze");
		ui_interface->addHorizontalSlider("Reduce", &fHslider1, FAUSTFLOAT(0.0), FAUSTFLOAT(0.0), FAUSTFLOAT(2.0), FAUSTFLOAT(0.01));
		ui_interface->addHorizontalSlider("fftSize", &fHslider0, FAUSTFLOAT(1024.0), FAUSTFLOAT(2.0), FAUSTFLOAT(16384.0), FAUSTFLOAT(1.0));
		ui_interface->addCheckButton("freezeBtn", &fCheckbox0);
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* input1 = inputs[1];
		FAUSTFLOAT* input2 = inputs[2];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		FAUSTFLOAT* output2 = outputs[2];
		double fSlow0 = double(fCheckbox0);
		double fSlow1 = 1.0 - fSlow0;
		double fSlow2 = 0.5 * double(fHslider0);
		int iSlow3 = int(std::min<double>(16384.0, std::max<double>(0.0, fSlow2 + 1.0)));
		int iSlow4 = int(std::min<double>(16384.0, std::max<double>(0.0, fSlow2)));
		double fSlow5 = double(fHslider1);
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			double fTemp0 = double(input1[i0]);
			double fTemp1 = fSlow1 * fTemp0;
			fVec0[IOTA0 & 16383] = fTemp1;
			fVec1[IOTA0 & 16383] = fSlow0 * fRec0[1];
			fRec0[0] = fSlow0 * fVec1[(IOTA0 - iSlow4) & 16383] + fVec0[(IOTA0 - iSlow3) & 16383];
			double fTemp2 = double(input0[i0]);
			double fTemp3 = fSlow1 * fTemp2;
			fVec2[IOTA0 & 16383] = fTemp3;
			fVec3[IOTA0 & 16383] = fSlow0 * fRec1[1];
			fRec1[0] = fSlow0 * fVec3[(IOTA0 - iSlow4) & 16383] + fVec2[(IOTA0 - iSlow3) & 16383];
			double fTemp4 = std::sqrt(fftfreeze_faustpower2_f(fTemp2) + fftfreeze_faustpower2_f(fTemp0));
			double fTemp5 = std::max<double>(0.0, fSlow1 * fTemp4 + fSlow0 * (fTemp4 - fSlow5 * std::sqrt(fftfreeze_faustpower2_f(fRec1[0] + fTemp3) + fftfreeze_faustpower2_f(fRec0[0] + fTemp1))));
			double fTemp6 = std::atan2(fTemp0, fTemp2);
			output0[i0] = FAUSTFLOAT(std::cos(fTemp6) * fTemp5);
			output1[i0] = FAUSTFLOAT(std::sin(fTemp6) * fTemp5);
			output2[i0] = FAUSTFLOAT(double(input2[i0]));
			IOTA0 = IOTA0 + 1;
			fRec0[1] = fRec0[0];
			fRec1[1] = fRec1[0];
		}
	}

};

/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/

// Factory API

/******************* END minimal-effect.cpp ****************/

#endif
