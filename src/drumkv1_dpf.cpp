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

#include <QApplication>
#include <QtXml/qdom.h>

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

void drumkv1_dpf::loadState(const char *state_data)
{
	drumkv1_dpf *pDrumk = this;

	const bool running = pDrumk->running(false);

	pDrumk->setTuningEnabled(false);
	pDrumk->reset();

	static QHash<QString, drumkv1::ParamIndex> s_hash;
	if (s_hash.isEmpty()) {
		for (uint32_t i = drumkv1::NUM_ELEMENT_PARAMS; i < drumkv1::NUM_PARAMS; ++i) {
			const drumkv1::ParamIndex index = drumkv1::ParamIndex(i);
			s_hash.insert(drumkv1_param::paramName(index), index);
		}
	}

	QDomDocument doc(DRUMKV1_TITLE);
	if (doc.setContent(QByteArray(state_data))) {
		QDomElement ePreset = doc.documentElement();
		if (ePreset.tagName() == "preset") {
		//	&& ePreset.attribute("name") == fi.completeBaseName()) {
			for (QDomNode nChild = ePreset.firstChild();
					!nChild.isNull();
						nChild = nChild.nextSibling()) {
				QDomElement eChild = nChild.toElement();
				if (eChild.isNull())
					continue;
				if (eChild.tagName() == "params") {
					for (QDomNode nParam = eChild.firstChild();
							!nParam.isNull();
								nParam = nParam.nextSibling()) {
						QDomElement eParam = nParam.toElement();
						if (eParam.isNull())
							continue;
						if (eParam.tagName() == "param") {
							drumkv1::ParamIndex index = drumkv1::ParamIndex(
								eParam.attribute("index").toULong());
							const QString& sName = eParam.attribute("name");
							if (!sName.isEmpty()) {
								if (!s_hash.contains(sName))
									continue;
								index = s_hash.value(sName);
							}
							const float fValue = eParam.text().toFloat();
							pDrumk->setParamValue(index,
								drumkv1_param::paramSafeValue(index, fValue));
						}
					}
				}
				else
				if (eChild.tagName() == "elements") {
					drumkv1_param::loadElements(pDrumk, eChild);
				}
				else
				if (eChild.tagName() == "tuning") {
					drumkv1_param::loadTuning(pDrumk, eChild);
				}
			}
		}
	}

	pDrumk->stabilize();
	pDrumk->reset();
	pDrumk->running(running);
}


const char* drumkv1_dpf::exportState()
{
	drumkv1_dpf *pDrumk = this;

	pDrumk->stabilize();

	QDomDocument doc(DRUMKV1_TITLE);
	QDomElement ePreset = doc.createElement("preset");
	ePreset.setAttribute("name", DRUMKV1_PRESET_NAME_FOR_DPF_STATE);
	ePreset.setAttribute("version", CONFIG_BUILD_VERSION);

	QDomElement eElements = doc.createElement("elements");
	drumkv1_param::saveElements(pDrumk, doc, eElements, drumkv1_param::map_path(), false);
	ePreset.appendChild(eElements);

	QDomElement eParams = doc.createElement("params");
	for (uint32_t i = drumkv1::NUM_ELEMENT_PARAMS; i < drumkv1::NUM_PARAMS; ++i) {
		QDomElement eParam = doc.createElement("param");
		const drumkv1::ParamIndex index = drumkv1::ParamIndex(i);
		eParam.setAttribute("index", QString::number(i));
		eParam.setAttribute("name", drumkv1_param::paramName(index));
		const float fValue = pDrumk->paramValue(index);
		eParam.appendChild(doc.createTextNode(QString::number(fValue)));
		eParams.appendChild(eParam);
	}
	ePreset.appendChild(eParams);
	doc.appendChild(ePreset);

	if (pDrumk->isTuningEnabled()) {
		QDomElement eTuning = doc.createElement("tuning");
		drumkv1_param::saveTuning(pDrumk, doc, eTuning, false);
		ePreset.appendChild(eTuning);
	}

	const QByteArray data(doc.toByteArray());
    return data.constData();
}


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
	fprintf(stderr, ">> Trigger selectSample, key = %d\n", key);
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


void DrumkV1Plugin::initState(uint32_t index, State& state)
{
	if (index == DRUMKV1_STATE_INDEX) {
		state.key = DRUMKV1_STATE_KEY;
		state.hints = DRUMKV1_STATE_INDEX;
		state.defaultValue = String();
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
	fSynthesizer->setParamValue(currentParam, parameter.ranges.def);
}


String DrumkV1Plugin::getState(const char* key) const
{
	d_stderr2(">> Called getState()");

	if (strcmp(key, DRUMKV1_STATE_KEY) == 0) {
		return String(fSynthesizer->exportState());
	}

	return String();
}


void DrumkV1Plugin::setState(const char* key, const char* value)
{
	if (strcmp(key, DRUMKV1_STATE_KEY) == 0) {
		fSynthesizer->loadState(value);
	}
}


float DrumkV1Plugin::getParameterValue(uint32_t index) const
{
	return fSynthesizer->paramValue((drumkv1::ParamIndex)index);
}


void DrumkV1Plugin::setParameterValue(uint32_t index, float value)
{
	fSynthesizer->setParamValue((drumkv1::ParamIndex)index, value);
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
