// drumkv1_config.cpp
//
/****************************************************************************
   Copyright (C) 2012-2022, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "drumkv1_config.h"

#include "drumkv1_programs.h"
#include "drumkv1_controls.h"

#include <QFileInfo>


//-------------------------------------------------------------------------
// drumkv1_config - Prototype settings structure (pseudo-singleton).
//

// Singleton instance accessor (static).
drumkv1_config *drumkv1_config::g_pSettings = nullptr;

drumkv1_config *drumkv1_config::getInstance (void)
{
	return g_pSettings;
}


// Constructor.
drumkv1_config::drumkv1_config (void)
	: QSettings(DRUMKV1_DOMAIN, DRUMKV1_TITLE)
{
	g_pSettings = this;

	load();
}


// Default destructor.
drumkv1_config::~drumkv1_config (void)
{
	save();

	g_pSettings = nullptr;
}


// Preset utility methods.
QString drumkv1_config::presetGroup (void) const
{
	return "/Presets/";
}


QString drumkv1_config::presetFile ( const QString& sPreset )
{
	QSettings::beginGroup(presetGroup());
	const QString sPresetFile(QSettings::value(sPreset).toString());
	QSettings::endGroup();
	return sPresetFile;
}


void drumkv1_config::setPresetFile (
	const QString& sPreset, const QString& sPresetFile )
{
	QSettings::beginGroup(presetGroup());
	QSettings::setValue(sPreset, sPresetFile);
	QSettings::endGroup();
}


void drumkv1_config::removePreset ( const QString& sPreset )
{
	QSettings::beginGroup(presetGroup());
	const QString& sPresetFile = QSettings::value(sPreset).toString();
	if (QFileInfo(sPresetFile).exists())
		QFile(sPresetFile).remove();
	QSettings::remove(sPreset);
	QSettings::endGroup();
}


QStringList drumkv1_config::presetList (void)
{
	QStringList list;
	QSettings::beginGroup(presetGroup());
	QStringListIterator iter(QSettings::childKeys());
	while (iter.hasNext()) {
		const QString& sPreset = iter.next();
		if (QFileInfo(QSettings::value(sPreset).toString()).exists())
			list.append(sPreset);
	}
	QSettings::endGroup();
	return list;
}


// Programs utility methods.
QString drumkv1_config::programsGroup (void) const
{
	return "/Programs";
}

QString drumkv1_config::bankPrefix (void) const
{
	return "/Bank_";
}


void drumkv1_config::loadPrograms ( drumkv1_programs *pPrograms )
{
	pPrograms->clear_banks();

	QSettings::beginGroup(programsGroup());

	const QStringList& bank_keys = QSettings::childKeys();
	QStringListIterator bank_iter(bank_keys);
	while (bank_iter.hasNext()) {
		const QString& bank_key = bank_iter.next();
		uint16_t bank_id = bank_key.toInt();
		const QString& bank_name
			= QSettings::value(bank_key).toString();
		drumkv1_programs::Bank *pBank = pPrograms->add_bank(bank_id, bank_name);
		QSettings::beginGroup(bankPrefix() + bank_key);
		const QStringList& prog_keys = QSettings::childKeys();
		QStringListIterator prog_iter(prog_keys);
		while (prog_iter.hasNext()) {
			const QString& prog_key = prog_iter.next();
			uint16_t prog_id = prog_key.toInt();
			const QString& prog_name
				= QSettings::value(prog_key).toString();
			pBank->add_prog(prog_id, prog_name);
		}
		QSettings::endGroup();
	}

	QSettings::endGroup();

	pPrograms->enabled(bProgramsEnabled);
}


void drumkv1_config::savePrograms ( drumkv1_programs *pPrograms )
{
	bProgramsEnabled = pPrograms->enabled();

	clearPrograms();

	QSettings::beginGroup(programsGroup());

	const drumkv1_programs::Banks& banks = pPrograms->banks();
	drumkv1_programs::Banks::ConstIterator bank_iter = banks.constBegin();
	const drumkv1_programs::Banks::ConstIterator& bank_end = banks.constEnd();
	for ( ; bank_iter != bank_end; ++bank_iter) {
		drumkv1_programs::Bank *pBank = bank_iter.value();
		const QString& bank_key = QString::number(pBank->id());
		const QString& bank_name = pBank->name();
		QSettings::setValue(bank_key, bank_name);
		QSettings::beginGroup(bankPrefix() + bank_key);
		const drumkv1_programs::Progs& progs = pBank->progs();
		drumkv1_programs::Progs::ConstIterator prog_iter = progs.constBegin();
		const drumkv1_programs::Progs::ConstIterator& prog_end = progs.constEnd();
		for ( ; prog_iter != prog_end; ++prog_iter) {
			drumkv1_programs::Prog *pProg = prog_iter.value();
			const QString& prog_key = QString::number(pProg->id());
			const QString& prog_name = pProg->name();
			QSettings::setValue(prog_key, prog_name);
		}
		QSettings::endGroup();
	}

	QSettings::endGroup();
	QSettings::sync();
}


void drumkv1_config::clearPrograms (void)
{
	QSettings::beginGroup(programsGroup());

	const QStringList& bank_keys = QSettings::childKeys();
	QStringListIterator bank_iter(bank_keys);
	while (bank_iter.hasNext()) {
		const QString& bank_key = bank_iter.next();
		QSettings::beginGroup(bankPrefix() + bank_key);
		const QStringList& prog_keys = QSettings::childKeys();
		QStringListIterator prog_iter(prog_keys);
		while (prog_iter.hasNext()) {
			const QString& prog_key = prog_iter.next();
			QSettings::remove(prog_key);
		}
		QSettings::endGroup();
		QSettings::remove(bank_key);
	}

	QSettings::endGroup();
}


// Programs utility methods.
QString drumkv1_config::controlsGroup (void) const
{
	return "/Controllers";
}

QString drumkv1_config::controlPrefix (void) const
{
	return "/Control";
}


void drumkv1_config::loadControls ( drumkv1_controls *pControls )
{
	pControls->clear();

	QSettings::beginGroup(controlsGroup());

	const QStringList& keys = QSettings::childKeys();
	QStringListIterator iter(keys);
	while (iter.hasNext()) {
		const QString& sKey = '/' + iter.next();
		const QStringList& clist = sKey.split('_');
		if (clist.at(0) == controlPrefix()) {
			const unsigned short channel
				= clist.at(1).toInt();
			const drumkv1_controls::Type ctype
				= drumkv1_controls::typeFromText(clist.at(2));
			drumkv1_controls::Key key;
			key.status = ctype | (channel & 0x1f);
			key.param = clist.at(3).toInt();
			const QStringList& vlist
				= QSettings::value(sKey).toStringList();
			drumkv1_controls::Data data;
			data.index = vlist.at(0).toInt();
			if (vlist.count() > 1)
				data.flags = vlist.at(1).toInt();
			pControls->add_control(key, data);
		}
	}

	QSettings::endGroup();

	pControls->enabled(bControlsEnabled);
}


void drumkv1_config::saveControls ( drumkv1_controls *pControls )
{
	bControlsEnabled = pControls->enabled();

	clearControls();

	QSettings::beginGroup(controlsGroup());

	const drumkv1_controls::Map& map = pControls->map();
	drumkv1_controls::Map::ConstIterator iter = map.constBegin();
	const drumkv1_controls::Map::ConstIterator& iter_end = map.constEnd();
	for ( ; iter != iter_end; ++iter) {
		const drumkv1_controls::Key& key = iter.key();
		QString sKey = controlPrefix();
		sKey += '_' + QString::number(key.channel());
		sKey += '_' + drumkv1_controls::textFromType(key.type());
		sKey += '_' + QString::number(key.param);
		const drumkv1_controls::Data& data = iter.value();
		QStringList vlist;
		vlist.append(QString::number(data.index));
		vlist.append(QString::number(data.flags));
		QSettings::setValue(sKey, vlist);
	}

	QSettings::endGroup();
	QSettings::sync();
}


void drumkv1_config::clearControls (void)
{
	QSettings::beginGroup(controlsGroup());

	const QStringList& keys = QSettings::childKeys();
	QStringListIterator iter(keys);
	while (iter.hasNext()) {
		const QString& key = iter.next();
		QSettings::remove(key);
	}

	QSettings::endGroup();
}


// Explicit I/O methods.
void drumkv1_config::load (void)
{
	QSettings::beginGroup("/Default");
	sPreset = QSettings::value("/Preset").toString();
	sPresetDir = QSettings::value("/PresetDir").toString();
	sSampleDir = QSettings::value("/SampleDir").toString();
	iKnobDialMode = QSettings::value("/KnobDialMode", 0).toInt();
	iKnobEditMode = QSettings::value("/KnobEditMode", 0).toInt();
	iFrameTimeFormat = QSettings::value("/FrameTimeFormat", 0).toInt();
	fRandomizePercent = QSettings::value("/RandomizePercent", 20.0f).toFloat();
	bUseGMDrumNames = QSettings::value("/UseGMDrumNames", true).toBool();
	bControlsEnabled = QSettings::value("/ControlsEnabled", false).toBool();
	bProgramsEnabled = QSettings::value("/ProgramsEnabled", false).toBool();
	QSettings::endGroup();

	QSettings::beginGroup("/Dialogs");
	bProgramsPreview = QSettings::value("/ProgramsPreview", false).toBool();
	bUseNativeDialogs = QSettings::value("/UseNativeDialogs", false).toBool();
	// Run-time special non-persistent options.
	bDontUseNativeDialogs = !bUseNativeDialogs;
	QSettings::endGroup();

	QSettings::beginGroup("/Custom");
	sCustomColorTheme = QSettings::value("/ColorTheme").toString();
	sCustomStyleTheme = QSettings::value("/StyleTheme").toString();
	QSettings::endGroup();

	// Micro-tuning options.
	QSettings::beginGroup("/Tuning");
	bTuningEnabled = QSettings::value("/Enabled", false).toBool();
	fTuningRefPitch = float(QSettings::value("/RefPitch", 440.0).toDouble());
	iTuningRefNote = QSettings::value("/RefNote", 69).toInt();
	sTuningScaleDir = QSettings::value("/ScaleDir").toString();
	sTuningScaleFile = QSettings::value("/ScaleFile").toString();
	sTuningKeyMapDir = QSettings::value("/KeyMapDir").toString();
	sTuningKeyMapFile = QSettings::value("/KeyMapFile").toString();
	QSettings::endGroup();
}


void drumkv1_config::save (void)
{
	QSettings::beginGroup("/Program");
	QSettings::setValue("/Version", CONFIG_BUILD_VERSION);
	QSettings::endGroup();

	QSettings::beginGroup("/Default");
	QSettings::setValue("/Preset", sPreset);
	QSettings::setValue("/PresetDir", sPresetDir);
	QSettings::setValue("/SampleDir", sSampleDir);
	QSettings::setValue("/KnobDialMode", iKnobDialMode);
	QSettings::setValue("/KnobEditMode", iKnobEditMode);
	QSettings::setValue("/FrameTimeFormat", iFrameTimeFormat);
	QSettings::setValue("/RandomizePercent", fRandomizePercent);
	QSettings::setValue("/UseGMDrumNames", bUseGMDrumNames);
	QSettings::setValue("/ControlsEnabled", bControlsEnabled);
	QSettings::setValue("/ProgramsEnabled", bProgramsEnabled);
	QSettings::endGroup();

	QSettings::beginGroup("/Dialogs");
	QSettings::setValue("/ProgramsPreview", bProgramsPreview);
	QSettings::setValue("/UseNativeDialogs", bUseNativeDialogs);
	QSettings::endGroup();

	QSettings::beginGroup("/Custom");
	QSettings::setValue("/ColorTheme", sCustomColorTheme);
	QSettings::setValue("/StyleTheme", sCustomStyleTheme);
	QSettings::endGroup();

	// Micro-tuning options.
	QSettings::beginGroup("/Tuning");
	QSettings::setValue("/Enabled", bTuningEnabled);
	QSettings::setValue("/RefPitch", double(fTuningRefPitch));
	QSettings::setValue("/RefNote", iTuningRefNote);
	QSettings::setValue("/ScaleDir", sTuningScaleDir);
	QSettings::setValue("/ScaleFile", sTuningScaleFile);
	QSettings::setValue("/KeyMapDir", sTuningKeyMapDir);
	QSettings::setValue("/KeyMapFile", sTuningKeyMapFile);
	QSettings::endGroup();

	QSettings::sync();
}


// end of drumkv1_config.cpp

