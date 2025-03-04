// drumkv1_dpf.h
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

#ifndef __drumkv1_dpf_h
#define __drumkv1_dpf_h

#include "drumkv1.h"

#include "DistrhoPlugin.hpp"

#include <memory>

// Forward decls.
class QApplication;

//-------------------------------------------------------------------------
// drumkv1_dpf - Constants.
//

// Plugin states.
#define DRUMKV1_PRESET_NAME_FOR_DPF_STATE "DPF_PLUGIN_STATE"
#define DRUMKV1_STATE_KEY "drumkv1_state"
#define DRUMKV1_STATE_INDEX 0


//-------------------------------------------------------------------------
// drumkv1_dpf - Synthesizer interface decl.
//

class drumkv1_dpf : public drumkv1
{
public:

	drumkv1_dpf(double sample_rate);

	~drumkv1_dpf();

	enum PortIndex {

		MidiIn = 0,
		Notify,
		AudioInL,
		AudioInR,
		AudioOutL,
		AudioOutR,
		ParamBase
	};

	void loadState(const char* state_data);
	const char* exportState();

	void run(const float **inputs, float **outputs, uint32_t nframes, const MidiEvent* midiEvents, uint32_t midiEventCount);

	void activate();
	void deactivate();

	uint32_t urid_map(const char *uri) const;

	static void qapp_instantiate();
	static void qapp_cleanup();

	static QApplication *qapp_instance();

protected:

	void updatePreset(bool bDirty);
	void updateParam(drumkv1::ParamIndex index);
	void updateParams();

	void updateSample();

	void updateOffsetRange();

	void selectSample(int key);

	void updateTuning();

	bool state_changed();

private:

	static QApplication *g_qapp_instance;
	static unsigned int  g_qapp_refcount;
};


//-------------------------------------------------------------------------
// drumkv1Plugin - The main DPF plugin interface decl.
//

START_NAMESPACE_DISTRHO

class DrumkV1Plugin : public Plugin
{
	double fSampleRate = getSampleRate();
	std::unique_ptr<drumkv1_dpf> fSynthesizer = std::make_unique<drumkv1_dpf>(fSampleRate);

public:
	DrumkV1Plugin();
	~DrumkV1Plugin();

	drumkv1_dpf *getSynthesizer();

protected:
	// ----------------------------------------------------------------------------------------------------------------
	// Information

   /**
	  Get the plugin label.@n
	  This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
	*/
	const char* getLabel() const noexcept override
	{
		return "drumkv1";
	}

   /**
	  Get an extensive comment/description about the plugin.@n
	  Optional, returns nothing by default.
	*/
	const char* getDescription() const override
	{
		return "An old-school polyphonic additive synthesizer";
	}

   /**
	  Get the plugin author/maker.
	*/
	const char* getMaker() const noexcept override
	{
		return "rncbc aka. Rui Nuno Capela";
	}

   /**
	  Get the plugin license (a single line of text or a URL).@n
	  For commercial plugins this should return some short copyright information.
	*/
	const char* getLicense() const noexcept override
	{
		return "GPLv2";
	}

   /**
	  Get the plugin version, in hexadecimal.
	  @see d_version()
	*/
	uint32_t getVersion() const noexcept override
	{
		return d_version(0, 9, 28);
	}

   /**
	  Get the plugin unique Id.@n
	  This value is used by LADSPA, DSSI and VST plugin formats.
	  @see d_cconst()
	*/
	int64_t getUniqueId() const noexcept override
	{
		return d_cconst('d', 'r', 'k', '1');
	}

	// ----------------------------------------------------------------------------------------------------------------
	// Init

	void initState(uint32_t index, State& state) override;
	void initParameter(uint32_t index, Parameter& parameter) override;

	// ----------------------------------------------------------------------------------------------------------------
	// Internal data

	String getState(const char* key) const override;
	void setState(const char* key, const char* value) override;
	float getParameterValue(uint32_t index) const override;
	void setParameterValue(uint32_t index, float value) override;

	// ----------------------------------------------------------------------------------------------------------------
	// Audio/MIDI Processing

	void activate() override;
	void run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount) override;

	// ----------------------------------------------------------------------------------------------------------------
	// Callbacks (optional)

	void sampleRateChanged(double newSampleRate) override;

	// ----------------------------------------------------------------------------------------------------------------

	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumkV1Plugin)
};

END_NAMESPACE_DISTRHO

#endif// __drumkv1_dpf_h

// end of drumkv1_dpf.h

