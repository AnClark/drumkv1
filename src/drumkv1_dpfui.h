// drumkv1_dpfui.h
//
/****************************************************************************
   Copyright (C) 2022-2023, AnClark Liu. All rights reserved.

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

#ifndef __drumkv1_dpfui_h
#define __drumkv1_dpfui_h

#include "drumkv1widget_dpf.h"

#include "DistrhoUI.hpp"
//#include "ResizeHandle.hpp" // Not found on DPF?

// Forward decls.
class drumkv1_dpf;

// Constants.
static const int BASE_UI_WIDTH = 1100;
static const int BASE_UI_HEIGHT = 510;

// -----------------------------------------------------------------------------------------------------------
// DrumkV1PluginUI - DPF Plugin UI interface.

START_NAMESPACE_DISTRHO

class DrumkV1PluginUI : public UI {

	drumkv1widget_dpf *fWidget;
	WId fWinId;
	WId fParent;

	//ResizeHandle fResizeHandle;

	// ----------------------------------------------------------------------------------------------------------------

public:
	DrumkV1PluginUI();
	~DrumkV1PluginUI();

protected:
	// ----------------------------------------------------------------------------------------------------------------
	// DSP/Plugin Callbacks

	void parameterChanged(uint32_t index, float value) override;
	//void programLoaded(uint32_t index) override;
	void stateChanged(const char* key, const char* value) override;

	// ----------------------------------------------------------------------------------------------------------------
	// External window overrides

	void focus() override;
	uintptr_t getNativeWindowHandle() const noexcept override;
	void sizeChanged(uint width, uint height) override;
	void titleChanged(const char* const title) override;
	void transientParentWindowChanged(const uintptr_t winId) override;
	void visibilityChanged(const bool visible) override;
	void uiIdle() override;

private:
	// ----------------------------------------------------------------------------------------------------------------
	// Internal Procedures

	void _initParameterProperties();

	DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumkV1PluginUI)
};

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO

//-------------------------------------------------------------------------
// drumkv1_dpfui - Synth engine accessor: decl.
//

class drumkv1_dpfui : public drumkv1_ui
{
public:

	// Constructor.
	drumkv1_dpfui(drumkv1_dpf *pSynth, DISTRHO::DrumkV1PluginUI *pluginUiInterface);

	// Accessors.
	void write_function(drumkv1::ParamIndex index, float fValue) const;

private:
	DISTRHO::DrumkV1PluginUI *m_plugin_ui;
	drumkv1_dpf *m_synth;
};

#endif// __drumkv1_dpfui_h

// end of drumkv1_dpfui.h
