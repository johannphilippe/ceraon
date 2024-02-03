/* ------------------------------------------------------------
name: "osc"
Code generated with Faust 2.71.0 (https://faust.grame.fr)
Compilation options: -a simple.arch -lang cpp -ct 1 -cn osc -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0
------------------------------------------------------------ */

#ifndef  __osc_H__
#define  __osc_H__

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
#define FAUSTCLASS osc
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

class oscSIG0 {
	
  private:
	
	int iVec0[2];
	int iRec2[2];
	
  public:
	
	int getNumInputsoscSIG0() {
		return 0;
	}
	int getNumOutputsoscSIG0() {
		return 1;
	}
	
	void instanceInitoscSIG0(int sample_rate) {
		for (int l0 = 0; l0 < 2; l0 = l0 + 1) {
			iVec0[l0] = 0;
		}
		for (int l1 = 0; l1 < 2; l1 = l1 + 1) {
			iRec2[l1] = 0;
		}
	}
	
	void filloscSIG0(int count, double* table) {
		for (int i1 = 0; i1 < count; i1 = i1 + 1) {
			iVec0[0] = 1;
			iRec2[0] = (iVec0[1] + iRec2[1]) % 65536;
			table[i1] = std::sin(9.587379924285257e-05 * double(iRec2[0]));
			iVec0[1] = iVec0[0];
			iRec2[1] = iRec2[0];
		}
	}

};

static oscSIG0* newoscSIG0() { return (oscSIG0*)new oscSIG0(); }
static void deleteoscSIG0(oscSIG0* dsp) { delete dsp; }

static double ftbl0oscSIG0[65536];

class osc : public dsp {
	
 private:
	
	int iVec1[2];
	int fSampleRate;
	double fConst0;
	double fConst1;
	double fRec3[2];
	double fConst2;
	double fRec0[2];
	
 public:
	osc() {}

	void metadata(Meta* m) { 
		m->declare("basics.lib/name", "Faust Basic Element Library");
		m->declare("basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
		m->declare("basics.lib/version", "1.12.0");
		m->declare("compile_options", "-a simple.arch -lang cpp -ct 1 -cn osc -es 1 -mcd 16 -mdd 1024 -mdy 33 -double -ftz 0");
		m->declare("filename", "osc.dsp");
		m->declare("maths.lib/author", "GRAME");
		m->declare("maths.lib/copyright", "GRAME");
		m->declare("maths.lib/license", "LGPL with exception");
		m->declare("maths.lib/name", "Faust Math Library");
		m->declare("maths.lib/version", "2.7.0");
		m->declare("name", "osc");
		m->declare("oscillators.lib/name", "Faust Oscillator Library");
		m->declare("oscillators.lib/saw2ptr:author", "Julius O. Smith III");
		m->declare("oscillators.lib/saw2ptr:license", "STK-4.3");
		m->declare("oscillators.lib/version", "1.5.0");
		m->declare("platform.lib/name", "Generic Platform Library");
		m->declare("platform.lib/version", "1.3.0");
	}

	virtual int getNumInputs() {
		return 0;
	}
	virtual int getNumOutputs() {
		return 1;
	}
	
	static void classInit(int sample_rate) {
		oscSIG0* sig0 = newoscSIG0();
		sig0->instanceInitoscSIG0(sample_rate);
		sig0->filloscSIG0(65536, ftbl0oscSIG0);
		deleteoscSIG0(sig0);
	}
	
	virtual void instanceConstants(int sample_rate) {
		fSampleRate = sample_rate;
		fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, double(fSampleRate)));
		fConst1 = 0.2 / fConst0;
		fConst2 = 1.0 / fConst0;
	}
	
	virtual void instanceResetUserInterface() {
	}
	
	virtual void instanceClear() {
		for (int l2 = 0; l2 < 2; l2 = l2 + 1) {
			iVec1[l2] = 0;
		}
		for (int l3 = 0; l3 < 2; l3 = l3 + 1) {
			fRec3[l3] = 0.0;
		}
		for (int l4 = 0; l4 < 2; l4 = l4 + 1) {
			fRec0[l4] = 0.0;
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
	
	virtual osc* clone() {
		return new osc();
	}
	
	virtual int getSampleRate() {
		return fSampleRate;
	}
	
	virtual void buildUserInterface(UI* ui_interface) {
		ui_interface->openVerticalBox("osc");
		ui_interface->closeBox();
	}
	
	virtual void compute(int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
		FAUSTFLOAT* output0 = outputs[0];
		for (int i0 = 0; i0 < count; i0 = i0 + 1) {
			iVec1[0] = 1;
			double fTemp0 = ((1 - iVec1[1]) ? 0.0 : fConst1 + fRec3[1]);
			fRec3[0] = fTemp0 - std::floor(fTemp0);
			double fTemp1 = std::max<double>(2.220446049250313e-16, std::fabs(5e+02 * std::fabs(ftbl0oscSIG0[std::max<int>(0, std::min<int>(int(65536.0 * fRec3[0]), 65535))]) + 3e+01));
			double fTemp2 = fRec0[1] + fConst2 * fTemp1;
			double fTemp3 = fTemp2 + -1.0;
			int iTemp4 = fTemp3 < 0.0;
			fRec0[0] = ((iTemp4) ? fTemp2 : fTemp3);
			double fRec1 = ((iTemp4) ? fTemp2 : fTemp2 + fTemp3 * (1.0 - fConst0 / fTemp1));
			output0[i0] = FAUSTFLOAT(0.1 * (2.0 * fRec1 + -1.0));
			iVec1[1] = iVec1[0];
			fRec3[1] = fRec3[0];
			fRec0[1] = fRec0[0];
		}
	}

};

/***************************END USER SECTION ***************************/

/*******************BEGIN ARCHITECTURE SECTION (part 2/2)***************/

// Factory API

/******************* END minimal-effect.cpp ****************/

#endif
