/* ------------------------------------------------------------
name: "filter"
Code generated with Faust 2.71.0 (https://faust.grame.fr)
Compilation options: -a simple.arch -lang cpp -ct 1 -cn filter -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0
------------------------------------------------------------ */

#ifndef  __filter_H__
#define  __filter_H__

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
#include <math.h>

#ifndef FAUSTCLASS 
#define FAUSTCLASS filter
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

class filterSIG0 {
	
  private:
	
	int iVec0[2];
	int iRec4[2];
	
  public:
	
	int getNumInputsfilterSIG0() {
		return 0;
	}
	int getNumOutputsfilterSIG0() {
		return 1;
	}
	
	void instanceInitfilterSIG0(int sample_rate) {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec4[l1] = 0;
		}
	}
	
	void fillfilterSIG0(int count, double* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec0[0] = 1;
			iRec4[0] = (iVec0[1] + iRec4[1]) % 65536;
			table[i1] = std::sin(9.587379924285257e-05 * double(iRec4[0]));
			iVec0[1] = iVec0[0];
			iRec4[1] = iRec4[0];
		}
	}

};

static filterSIG0* newfilterSIG0() { return (filterSIG0*)new filterSIG0(); }
static void deletefilterSIG0(filterSIG0* dsp) { delete dsp; }

static double ftbl0filterSIG0[65536];

class filter : public dsp {
	
 private:
	
	int iVec1[2];
	int fSampleRate;
	double fConst1;
	double fRec5[2];
	double fConst2;
	double fRec0[2];
	double fRec2[2];
	double fRec3[2];
	
 public:
	filter() {}

	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
		m->declare("basics.lib/version", "1.12.0");
		m->declare("compile_options", "-a simple.arch -lang cpp -ct 1 -cn filter -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0");
		m->declare("filename", "filter.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.7.0");
		m->declare("name", "filter");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/version", "1.5.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
		m->declare("vaeffects.lib/korg35LPF:author", "Eric Tarr");
		m->declare("vaeffects.lib/korg35LPF:license", "MIT-style STK-4.3 license");
		m->declare("vaeffects.lib/name", "Faust Virtual Analog Filter Effect Library");
		m->declare("vaeffects.lib/version", "1.2.1");
	}

	virtual int getNumInputs() {
		return 1;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
		filterSIG0* sig0 = newfilterSIG0();
		sig0->instanceInitfilterSIG0(sample_rate);
		sig0->fillfilterSIG0(65536, ftbl0filterSIG0);
		deletefilterSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		double fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, double(fSampleRate)));
		fConst1 = 0.1 / fConst0;
		fConst2 = 6.283185307179586 / fConst0;
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iVec1[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec5[l3] = 0.0;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec0[l4] = 0.0;
		}
		for (int l5 = 0; l5 < 2; l5 = l5 + 1) {
			fRec2[l5] = 0.0;
		}
		for (int l6 = 0; l6 < 2; l6 = l6 + 1) {
			fRec3[l6] = 0.0;
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
	
	virtual filter* clone() {
		return new filter();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("filter");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* input0 = inputs[0];
		FAUSTFLOAT* output0 = outputs[0];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec1[0] = 1;
			double fTemp0 = ((1 - iVec1[1]) ? 0.0 : fConst1 + fRec5[1]);
			fRec5[0] = fTemp0 - std::floor(fTemp0);
			double fTemp1 = std::tan(fConst2 * std::pow(1e+01, 3.0 * std::fabs(ftbl0filterSIG0[std::max<int>(0, std::min<int>(int(65536.0 * fRec5[0]), 65535))]) + 1.0));
			double fTemp2 = fTemp1 + 1.0;
			double fTemp3 = 1.0 - fTemp1 / fTemp2;
			double fTemp4 = (double(input0[i0]) - fRec3[1]) * fTemp1;
			double fTemp5 = fTemp1 * ((fRec3[1] + (fTemp4 + 0.49347241269737024 * fRec0[1] * fTemp3 - fRec2[1]) / fTemp2) / (1.0 - 0.49347241269737024 * (fTemp1 * fTemp3 / fTemp2)) - fRec0[1]) / fTemp2;
			fRec0[0] = fRec0[1] + 2.0 * fTemp5;
			double fTemp6 = fRec0[1] + fTemp5;
			double fRec1 = fTemp6;
			fRec2[0] = fRec2[1] + 2.0 * (fTemp1 * (0.49347241269737024 * fTemp6 - fRec2[1]) / fTemp2);
			fRec3[0] = fRec3[1] + 2.0 * (fTemp4 / fTemp2);
			output0[i0] = FAUSTFLOAT(fRec1);
			iVec1[1] = iVec1[0];
			fRec5[1] = fRec5[0];
			fRec0[1] = fRec0[0];
			fRec2[1] = fRec2[0];
			fRec3[1] = fRec3[0];
		}
	}

};

/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/

// Factory API

/******************* END minimal-effect.cpp ****************/

#endif
