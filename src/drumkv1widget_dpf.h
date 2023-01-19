// drumkv1widget_dpf.h
//
/****************************************************************************
   Copyright (C) 2012-2022, rncbc aka Rui Nuno Capela.
   Copyright (C) 2023, AnClark Liu.
   All rights reserved.

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

#ifndef __drumkv1widget_dpf_h
#define __drumkv1widget_dpf_h

#include "drumkv1widget.h"

// Forward decls.
class drumkv1_dpf;
class drumkv1_dpfui;
namespace DISTRHO {
	class DrumkV1PluginUI;
}

//-------------------------------------------------------------------------
// drumkv1widget_lv2 - decl.
//

class drumkv1widget_dpf : public drumkv1widget
{
public:

	// Constructor.
	drumkv1widget_dpf(drumkv1_dpf *pSynth, DISTRHO::DrumkV1PluginUI *pPluginUiInterface);

	// Destructor.
	~drumkv1widget_dpf();

	// Param method. (Host -> UI)
	void setUIParamValue(drumkv1::ParamIndex paramIndex, float value);

protected:

	// Synth engine accessor.
	drumkv1_ui *ui_instance() const;

	// Param method. (UI -> Host)
	void updateParam(drumkv1::ParamIndex index, float fValue) const;

	// Show event handler.
	void showEvent(QShowEvent *pShowEvent);

	// Close event handler.
	void closeEvent(QCloseEvent *pCloseEvent);

private:

	// Instance variables.
	drumkv1_dpfui *m_pSynthUi;  // synth engine accessor

	int m_iShowEvent;
};

#endif  // __drumkv1widget_dpf_h

// end of drumkv1widget_dpf.h
