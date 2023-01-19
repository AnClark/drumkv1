// drumkv1widget_dpf.cpp
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

#include "drumkv1widget_dpf.h"

#include "drumkv1_dpf.h"
#include "drumkv1_dpfui.h"

#include <QApplication>
#include <QFileInfo>
#include <QDir>

#include "drumkv1widget_palette.h"

#include <QCloseEvent>

#include <QStyleFactory>

#ifndef CONFIG_LIBDIR
#if defined(__x86_64__)
#define CONFIG_LIBDIR CONFIG_PREFIX "/lib64"
#else
#define CONFIG_LIBDIR CONFIG_PREFIX "/lib"
#endif
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define CONFIG_PLUGINSDIR CONFIG_LIBDIR "/qt5/plugins"
#else
#define CONFIG_PLUGINSDIR CONFIG_LIBDIR "/qt6/plugins"
#endif


//-------------------------------------------------------------------------
// drumkv1widget_lv2 - impl.
//

drumkv1widget_dpf::drumkv1widget_dpf ( drumkv1_dpf *pSynth, DISTRHO::DrumkV1PluginUI *pPluginUiInterface )
	: drumkv1widget()
{
	// Check whether under a dedicated application instance...
	QApplication *pApp = drumkv1_dpf::qapp_instance();
	if (pApp) {
		// Special style paths...
		if (QDir(CONFIG_PLUGINSDIR).exists())
			pApp->addLibraryPath(CONFIG_PLUGINSDIR);
	}

	// Custom color/style themes...
	drumkv1_config *pConfig = drumkv1_config::getInstance();
	if (pConfig) {
		if (!pConfig->sCustomColorTheme.isEmpty()) {
			QPalette pal;
			if (drumkv1widget_palette::namedPalette(
					pConfig, pConfig->sCustomColorTheme, pal))
				drumkv1widget::setPalette(pal);
		}
		if (!pConfig->sCustomStyleTheme.isEmpty()) {
			drumkv1widget::setStyle(
				QStyleFactory::create(pConfig->sCustomStyleTheme));
		}
	}

	// Initialize (user) interface stuff...
	m_pSynthUi = new drumkv1_dpfui(pSynth, pPluginUiInterface);

	// Initialise preset stuff...
	clearPreset();

	// Initial update, always...
	refreshElements();
	activateElement();

	//resetParamValues();
	resetParamKnobs(drumkv1::NUM_PARAMS);

	// May initialize the scheduler/work notifier.
	openSchedNotifier();
}


// Destructor.
drumkv1widget_dpf::~drumkv1widget_dpf (void)
{
	delete m_pSynthUi;
}


// Synth engine accessor.
drumkv1_ui *drumkv1widget_dpf::ui_instance (void) const
{
	return m_pSynthUi;
}

// Show event handler.
void drumkv1widget_dpf::showEvent ( QShowEvent *pShowEvent )
{
	drumkv1widget::showEvent(pShowEvent);

	++m_iShowEvent;

	drumkv1widget::updateElement();
}

// Close event handler.
void drumkv1widget_dpf::closeEvent ( QCloseEvent *pCloseEvent )
{
	drumkv1widget::closeEvent(pCloseEvent);
}

// Param method: Host -> UI.
// Render host's parameter values on UI. This is called by DPF UI's paramChanged() method.
void drumkv1widget_dpf::setUIParamValue(drumkv1::ParamIndex paramIndex, float value)
{
	if (paramIndex < drumkv1::NUM_ELEMENT_PARAMS && m_iShowEvent > 0) {
		drumkv1_ui *pDrumkUi = ui_instance();
		if (pDrumkUi) {
			const int iCurrentNote = pDrumkUi->currentElement();
			drumkv1_element *element = pDrumkUi->element(iCurrentNote);
			if (element)
				element->setParamValue(paramIndex, value);
		}
	}
	if (paramIndex >= drumkv1::NUM_ELEMENT_PARAMS || m_iShowEvent > 0)
		this->setParamValue(paramIndex, value);
}

// Param method: UI -> Host.
// This method sets host's param values from UI side.
void drumkv1widget_dpf::updateParam (
	drumkv1::ParamIndex index, float fValue ) const
{
	m_pSynthUi->setParamValue(index, fValue);
	m_pSynthUi->write_function(index, fValue);
}

// end of drumkv1widget_dpf.cpp
