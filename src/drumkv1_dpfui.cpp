// drumkv1_dpfui.cpp
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

#include "drumkv1_dpfui.h"
#include "drumkv1_dpf.h"

#include <QWindow>
#include <QApplication>

//-------------------------------------------------------------------------
// DrumkV1PluginUI - DPF Plugin UI interface.
//

// --------------------------------
// NOTICE: About UI sizes
//
// DPF UI size is dynamic. It can be changed on runtime via setWidth() and setHeight().
// But different plug-in formats has different behaviors:
//
// * VST2/VST3: Size can be changed freely. Default UI size is not fundamental.
// * CLAP:		Must assume a default UI size, otherwise setWidth() and setHeight()
//				will not work.
//
// Since CLAP has limited size management feature, I apply a base UI size below, in the
// constructor of UI().
//
// Also please notice that the config macro DISTRHO_UI_DEFAULT_(WIDTH|HEIGHT) does not work.
// Assigning in UI() constructor instead.

START_NAMESPACE_DISTRHO

DrumkV1PluginUI::DrumkV1PluginUI()
	: UI(BASE_UI_HEIGHT, BASE_UI_WIDTH), fWidget(nullptr)
{
	// Print out DPF standalone mode state.
	const bool standalone = isStandalone();
	d_stdout("isStandalone %d", (int)standalone);

	// drumkv1 UI requires directly accessing synth instance.
	// This is discouraged by DPF, but drumkv1 do need this due to its designation.
	DrumkV1Plugin *fDspInstance = (DrumkV1Plugin*) UI::getPluginInstancePointer();

	fWidget = new drumkv1widget_dpf(fDspInstance->getSynthesizer(), this);

	// Get the recommended UI size, then apply it.
	const QSize& hint = fWidget->sizeHint();
	setWidth(hint.width());
	setHeight(hint.height());

	// Explicitly set window position to avoid misplace on some platforms (especially Windows)
	fWidget->move(0, 0);

	// Embed plug-in UI into host window.
	fParent = (WId) getParentWindowHandle();
	fWinId = fWidget->winId();	// Must require WinID first, otherwise plug-in will crash!
	if (fParent)
	{
		fWidget->windowHandle()->setParent(QWindow::fromWinId(fParent));
	}

	// Explicitly show UI. This is required when using external UI mode of DPF.
	fWidget->show();
}

DrumkV1PluginUI::~DrumkV1PluginUI()
{
	if (fWidget)
	{
		delete fWidget;
		fWidget = nullptr;
	}
}

/* --------------------------------------------------------------------------------------------------------
* DSP/Plugin Callbacks */

/**
	A parameter has changed on the plugin side.
	This is called by the host to inform the UI about parameter changes.
*/
void DrumkV1PluginUI::parameterChanged(uint32_t index, float value)
{
	fWidget->setUIParamValue(drumkv1::ParamIndex(index), value);
}

/* --------------------------------------------------------------------------------------------------------
* External Window overrides */

void DrumkV1PluginUI::focus()
{
	d_stdout("focus");

	fWidget->setFocus();
}

uintptr_t DrumkV1PluginUI::getNativeWindowHandle() const noexcept
{
	return (uintptr_t)fWidget->windowHandle()->winId();
}

void DrumkV1PluginUI::stateChanged(const char* key, const char* value)
{
}

void DrumkV1PluginUI::sizeChanged(uint width, uint height)
{
	UI::sizeChanged(width, height);

	if (fWidget != 0)
		fWidget->resize(width, height);
}

void DrumkV1PluginUI::titleChanged(const char* const title)
{
	d_stdout("titleChanged %s", title);

	DISTRHO_SAFE_ASSERT_RETURN(fWidget != 0,);
	fWidget->setWindowTitle(QString(title));
}

void DrumkV1PluginUI::transientParentWindowChanged(const uintptr_t winId)
{
	d_stdout("transientParentWindowChanged %lu", winId);

	DISTRHO_SAFE_ASSERT_RETURN(fWidget != 0,);
	// NOTICE: Seems not implemented by Qt
}

void DrumkV1PluginUI::visibilityChanged(const bool visible)
{
	d_stdout("visibilityChanged %d", visible);

	DISTRHO_SAFE_ASSERT_RETURN(fWidget != 0,);

	if (visible)
	{
		fWidget->show();
		fWidget->raise();
		fWidget->activateWindow();
	}
	else
		fWidget->hide();
}

void DrumkV1PluginUI::uiIdle()
{
	// d_stdout("uiIdle");

	if (fWidget)
	{
		QApplication::processEvents();
		return;
	}
}

/* ------------------------------------------------------------------------------------------------------------
 * UI entry point, called by DPF to create a new UI instance. */

UI* createUI()
{
	return new DrumkV1PluginUI();
}

END_NAMESPACE_DISTRHO


//-------------------------------------------------------------------------
// drumkv1_dpfui - Synth engine accessor: impl.
//

drumkv1_dpfui::drumkv1_dpfui(drumkv1_dpf *pSynth, DISTRHO::DrumkV1PluginUI *pluginUiInterface)
	: drumkv1_ui(pSynth, true),
	  m_plugin_ui(pluginUiInterface),
	  m_synth(pSynth)
{
}

void drumkv1_dpfui::write_function(drumkv1::ParamIndex index, float fValue) const
{
	m_plugin_ui->setParameterValue(index, fValue);
	m_plugin_ui->setState(DRUMKV1_STATE_KEY, m_synth->exportState());
}

// end of drumkv1_dpfui.cpp
