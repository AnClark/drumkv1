// drumkv1_dpf.cpp
//
/****************************************************************************
   Copyright (C) 2023, AnClark Liu. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "drumkv1_dpf.h"
#include "drumkv1_param.h"
#include "drumkv1_config.h"
#include "drumkv1_sched.h"

#include <QApplication>
#include <QByteArray>
#include "QtXml/qdom.h"

// --------------------------------
// NOTICE: About plugin states
//
// Unlike synthv1 and padthv1, drumkv1 has a more sophisticated state.
// It has two layers of parameters: global parameters, element parameters.
//
// In synthv1 and padthv1, plug-in state is simple, so we can directly let DPF handle
// it in its own state manager, without using Vee-One's XML-based state configurator.
//
// However, in drumkv1, "element param" is a dedicated param collection for each sample.
// Simply depending on DPF's own state management is far not enough.
//
// So a better solution is to fetch and store states in drumkv1's own format,
// then access them via DPF's state API.

//-------------------------------------------------------------------------
// drumkv1_dpf - Constants.
//

// Parameter names - extracted from LV2 definition
static const char *ParamNames[drumkv1::NUM_PARAMS] = {
	"GEN1 Sample",
	"GEN1 Reverse",
	"GEN1 Offset",
	"GEN1 Offset Start",
	"GEN1 Offset End",
	"GEN1 Group",
	"GEN1 Coarse",
	"GEN1 Fine",
	"GEN1 Env.Time",
	"DCF1 Enabled",
	"DCF1 Cutoff",
	"DCF1 Resonance",
	"DCF1 Type",
	"DCF1 Slope",
	"DCF1 Envelope",
	"DCF1 Attack",
	"DCF1 Decay 1",
	"DCF1 Level 2",
	"DCF1 Decay 2",
	"LFO1 Enabled",
	"LFO1 Wave Shape",
	"LFO1 Wave Width",
	"LFO1 BPM",
	"LFO1 Rate",
	"LFO1 Sweep",
	"LFO1 Pitch",
	"LFO1 Cutoff",
	"LFO1 Resonance",
	"LFO1 Panning",
	"LFO1 Volume",
	"LFO1 Attack",
	"LFO1 Decay 1",
	"LFO1 Level 2",
	"LFO1 Decay 2",
	"DCA1 Enabled",
	"DCA1 Volume",
	"DCA1 Attack",
	"DCA1 Decay1",
	"DCA1 Level 2",
	"DCA1 Decay 2",
	"OUT1 Stereo Width",
	"OUT1 Panning",
	"OUT1 FX Send",
	"OUT1 Volume",
	"DEF1 Pitchbend",
	"DEF1 Modwheel",
	"DEF1 Pressure",
	"DEF1 Velocity",
	"DEF1 Channel",
	"DEF1 Note Off",
	"Chorus Wet",
	"Chorus Delay",
	"Chorus Feedback",
	"Chorus Rate",
	"Chorus Modulation",
	"Flanger Wet",
	"Flanger Delay",
	"Flanger Feedback",
	"Flanger Daft",
	"Phaser Wet",
	"Phaser Rate",
	"Phaser Feedback",
	"Phaser Depth",
	"Phaser Daft",
	"Delay Wet",
	"Delay Delay",
	"Delay Feedback",
	"Delay BPM",
	"Reverb Wet",
	"Reverb Room",
	"Reverb Damp",
	"Reverb Feedback",
	"Reverb Width",
	"Dynamic Compressor",
	"Dynamic Limiter"
};


//-------------------------------------------------------------------------
// drumkv1_dpf - Instantiation and cleanup.
//

QApplication *drumkv1_dpf::g_qapp_instance = nullptr;
unsigned int  drumkv1_dpf::g_qapp_refcount = 0;


drumkv1_dpf::drumkv1_dpf(double sample_rate): drumkv1(2, float(sample_rate))
{
}


drumkv1_dpf::~drumkv1_dpf()
{
}


void drumkv1_dpf::qapp_instantiate (void)
{
	if (qApp == nullptr && g_qapp_instance == nullptr) {
		static int s_argc = 1;
		static const char *s_argv[] = { DRUMKV1_TITLE, nullptr };
	#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
		::_putenv_s("QT_NO_GLIB", "1"); // Avoid glib event-loop...
	#else
		::setenv("QT_NO_GLIB", "1", 1); // Avoid glib event-loop...
	#endif
		g_qapp_instance = new QApplication(s_argc, (char **) s_argv);
	}

	if (g_qapp_instance) ++g_qapp_refcount;
}


void drumkv1_dpf::qapp_cleanup (void)
{
	if (g_qapp_instance && --g_qapp_refcount == 0) {
		delete g_qapp_instance;
		g_qapp_instance = nullptr;
	}
}


QApplication *drumkv1_dpf::qapp_instance (void)
{
	return g_qapp_instance;
}


//-------------------------------------------------------------------------
// drumkv1_dpf - methods impl.
//

void drumkv1_dpf::updatePreset ( bool /*bDirty*/ )
{
	// NOTICE: No need to tell DPF about preset changes, since DPF knows it
	//         when parameter changes from UI side.
	//         Also "synthesizer -> plug-in" access is not essential in DPF.
	//         See: drumkv1widget_dpf::updateParam.
}


void drumkv1_dpf::updateParam ( drumkv1::ParamIndex index )
{
	// NOTICE: No need to tell DPF about param changes. Reason mentioned above.
	(void) index;
}


void drumkv1_dpf::updateParams (void)
{
	// NOTICE: No need to tell DPF about param changes. Reason mentioned above.
}


void drumkv1_dpf::updateTuning (void)
{
	// NOTICE: DPF does not support manual tuning. Will not implement it for now.
}


void drumkv1_dpf::updateSample (void)
{
	// NOTICE: No need to tell DPF about sample changes. Reason mentioned in updatePreset().
}

void drumkv1_dpf::updateOffsetRange (void)
{
	// NOTICE: No need to tell DPF about sample changes. Reason mentioned in updatePreset().
}

void drumkv1_dpf::selectSample (int key)
{
	drumkv1::setCurrentElementEx(key);
}


void drumkv1_dpf::activate (void)
{
	drumkv1::reset();
}


void drumkv1_dpf::deactivate (void)
{
	drumkv1::reset();
}


void drumkv1_dpf::run(const float **inputs, float **outputs, uint32_t nframes, const MidiEvent* midiEvents, uint32_t midiEventCount)
{
    const uint16_t nchannels = drumkv1::channels();

	uint32_t event_index;

    for (event_index = 0; event_index < midiEventCount; ++event_index)
    {
        drumkv1::process_midi((uint8_t*)midiEvents[event_index].data, midiEvents[event_index].size);
    }

	drumkv1::process((float**)inputs, outputs, nframes);
}



//-------------------------------------------------------------------------
// DrumkV1Plugin - DPF plugin interface.
//

START_NAMESPACE_DISTRHO

DrumkV1Plugin::DrumkV1Plugin()
	: Plugin(drumkv1::NUM_PARAMS, 0, 1) // parameters, programs, states
{
	drumkv1_dpf::qapp_instantiate();
}


DrumkV1Plugin::~DrumkV1Plugin()
{
	drumkv1_dpf::qapp_cleanup();
}


drumkv1_dpf* DrumkV1Plugin::getSynthesizer()
{
	return &(*fSynthesizer);	// Unique pointer -> standard pointer
}


void DrumkV1Plugin::loadStateToSynthesizer(const char *state_data)
{
	drumkv1_dpf *pSynthesizer = getSynthesizer();

	QDomDocument doc(DRUMKV1_TITLE);
	if (doc.setContent(QByteArray(state_data, strlen(state_data)))) {
		QDomElement eState = doc.documentElement();
	#if 1//DRUMKV1_LV2_LEGACY
		if (eState.tagName() == "elements")
			drumkv1_param::loadElements(pSynthesizer, eState, drumkv1_param::map_path());
		else
	#endif
		if (eState.tagName() == "state") {
			for (QDomNode nChild = eState.firstChild();
					!nChild.isNull();
						nChild = nChild.nextSibling()) {
				QDomElement eChild = nChild.toElement();
				if (eChild.isNull())
					continue;
				if (eChild.tagName() == "elements")
					drumkv1_param::loadElements(pSynthesizer, eChild, drumkv1_param::map_path());
				else
				if (eChild.tagName() == "tuning")
					drumkv1_param::loadTuning(pSynthesizer, eChild);
			}
		}
	}

	pSynthesizer->reset();
}


const char* DrumkV1Plugin::exportStateFromSynthesizer()
{
	drumkv1_dpf *pSynthesizer = getSynthesizer();

	QDomDocument doc(DRUMKV1_TITLE);
	QDomElement eState = doc.createElement("state");

	QDomElement eElements = doc.createElement("elements");
	drumkv1_param::saveElements(pSynthesizer, doc, eElements, drumkv1_param::map_path());
	eState.appendChild(eElements);

	if (getSynthesizer()->isTuningEnabled()) {
		QDomElement eTuning = doc.createElement("tuning");
		drumkv1_param::saveTuning(pSynthesizer, doc, eTuning);
		eState.appendChild(eTuning);
	}

	doc.appendChild(eState);

	const QByteArray data(doc.toByteArray());
	const char *value = data.constData();
	size_t size = data.size();

	return value;
}


void DrumkV1Plugin::initState(uint32_t index, State& state)
{
	if (index == 0) {
		state.key = "drumkv1_state";
		state.label = "drumkv1 State";
		state.hints = kStateIsHostWritable;
		state.defaultValue = "";
	}
}


void DrumkV1Plugin::initParameter(uint32_t index, Parameter& parameter)
{
	drumkv1::ParamIndex currentParam = (drumkv1::ParamIndex)index;

	parameter.hints = kParameterIsAutomatable;

	parameter.name = ParamNames[index];
	parameter.shortName = drumkv1_param::paramName(currentParam);
	parameter.symbol = drumkv1_param::paramName(currentParam);
	parameter.ranges.def = drumkv1_param::paramDefaultValue(currentParam);
	parameter.ranges.min = drumkv1_param::paramMinValue(currentParam);
	parameter.ranges.max = drumkv1_param::paramMaxValue(currentParam);

	if (drumkv1_param::paramBool(currentParam)) {
		parameter.hints |= kParameterIsBoolean;
	}
	else if (drumkv1_param::paramInt(currentParam)) {
		parameter.hints |= kParameterIsInteger;
	}

	// Apply default parameter values for drumkv1 explicitly,
	// since DPF cannot apply it automatically
	fParameters[index] = parameter.ranges.def;
	fSynthesizer->setParamValue(currentParam, parameter.ranges.def);
}


String DrumkV1Plugin::getState(const char* key)
{
	d_stderr2(">> getState() invoked. key = %s", key);

	if (strcmp(key, "drumkv1_state") == 0) {
		d_stderr(">>> Save state to host: \n %s", exportStateFromSynthesizer());
		return String(exportStateFromSynthesizer());
	}

	return String();
}


void DrumkV1Plugin::setState(const char* key, const char* value)
{
	d_stderr2(">> setState() invoked. key = %s", key);

	if (strcmp(key, "drumkv1_state") == 0) {
		d_stderr(">>> Read state from host: \n%s", value);
		loadStateToSynthesizer(value);
	}
}


float DrumkV1Plugin::getParameterValue(uint32_t index) const
{
	return fParameters[index];
}


void DrumkV1Plugin::setParameterValue(uint32_t index, float value)
{
	if (index > drumkv1::GEN1_SAMPLE) {
		fParameters[index] = value;
		fSynthesizer->setParamValue((drumkv1::ParamIndex)index, value);
	}
}


void DrumkV1Plugin::activate()
{
	fSynthesizer->activate();
}


void DrumkV1Plugin::run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount) 
{
	fSynthesizer->run(inputs, outputs, frames, midiEvents, midiEventCount);
}


void DrumkV1Plugin::sampleRateChanged(double newSampleRate)
{
	fSynthesizer->setSampleRate(newSampleRate);
}


Plugin* createPlugin()
{
	d_stderr2(">> Creating plugin...");
	return new DrumkV1Plugin();
}

END_NAMESPACE_DISTRHO

// end of drumkv1_dpf.cpp
