// drumkv1widget_control.cpp
//
/****************************************************************************
   Copyright (C) 2012-2020, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "drumkv1widget_control.h"
#include "drumkv1widget_controls.h"

#include "drumkv1_config.h"

#include "ui_drumkv1widget_control.h"

#include <QMessageBox>
#include <QPushButton>
#include <QLineEdit>


//----------------------------------------------------------------------------
// drumkv1widget_control -- UI wrapper form.

// Kind of singleton reference.
drumkv1widget_control *drumkv1widget_control::g_pInstance = nullptr;


// Constructor.
drumkv1widget_control::drumkv1widget_control ( QWidget *pParent )
	: QDialog(pParent), p_ui(new Ui::drumkv1widget_control), m_ui(*p_ui)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Make it auto-modeless dialog...
	QDialog::setAttribute(Qt::WA_DeleteOnClose);

	// Control types...
	m_ui.ControlTypeComboBox->clear();
	m_ui.ControlTypeComboBox->addItem(
		drumkv1_controls::textFromType(drumkv1_controls::CC),
		int(drumkv1_controls::CC));
	m_ui.ControlTypeComboBox->addItem(
		drumkv1_controls::textFromType(drumkv1_controls::RPN),
		int(drumkv1_controls::RPN));
	m_ui.ControlTypeComboBox->addItem(
		drumkv1_controls::textFromType(drumkv1_controls::NRPN),
		int(drumkv1_controls::NRPN));
	m_ui.ControlTypeComboBox->addItem(
		drumkv1_controls::textFromType(drumkv1_controls::CC14),
		int(drumkv1_controls::CC14));

	m_ui.ControlParamComboBox->setInsertPolicy(QComboBox::NoInsert);

	// Start clean.
	m_iControlParamUpdate = 0;

	m_iDirtyCount = 0;
	m_iDirtySetup = 0;

	// Populate param list.
	// activateControlType(m_ui.ControlTypeComboBox->currentIndex());

	// Try to fix window geometry.
	adjustSize();

	// UI signal/slot connections...
	QObject::connect(m_ui.ControlTypeComboBox,
		SIGNAL(activated(int)),
		SLOT(activateControlType(int)));
	QObject::connect(m_ui.ControlParamComboBox,
		SIGNAL(activated(int)),
		SLOT(changed()));
	QObject::connect(m_ui.ControlChannelSpinBox,
		SIGNAL(valueChanged(int)),
		SLOT(changed()));
	QObject::connect(m_ui.ControlLogarithmicCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(changed()));
	QObject::connect(m_ui.ControlInvertCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(changed()));
	QObject::connect(m_ui.ControlHookCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(changed()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(clicked(QAbstractButton *)),
		SLOT(clicked(QAbstractButton *)));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));

	// Pseudo-singleton reference setup.
	g_pInstance = this;
}


// Destructor.
drumkv1widget_control::~drumkv1widget_control (void)
{
	delete p_ui;
}


// Pseudo-singleton instance.
drumkv1widget_control *drumkv1widget_control::getInstance (void)
{
	return g_pInstance;
}


// Pseudo-constructor.
void drumkv1widget_control::showInstance (
	drumkv1_controls *pControls, drumkv1::ParamIndex index,
	const QString& sTitle, QWidget *pParent )
{
	drumkv1widget_control *pInstance = drumkv1widget_control::getInstance();
	if (pInstance)
		pInstance->close();

	pInstance = new drumkv1widget_control(pParent);
	pInstance->setWindowTitle(sTitle);
	pInstance->setControls(pControls, index);
	pInstance->show();
}


// Control accessors.
void drumkv1widget_control::setControls (
	drumkv1_controls *pControls, drumkv1::ParamIndex index )
{
	++m_iDirtySetup;

	m_pControls = pControls;
	m_index = index;
	m_key.status = drumkv1_controls::CC;

	unsigned int flags = 0;

	if (m_pControls) {
		const drumkv1_controls::Map& map = m_pControls->map();
		drumkv1_controls::Map::ConstIterator iter = map.constBegin();
		const drumkv1_controls::Map::ConstIterator& iter_end
			= map.constEnd();
		for ( ; iter != iter_end; ++iter) {
			const drumkv1_controls::Data& data = iter.value();
			if (drumkv1::ParamIndex(data.index) == m_index) {
				flags = data.flags;
				m_key = iter.key();
				break;
			}
		}
	}

	setControlKey(m_key);

	const bool bFloat = drumkv1_param::paramFloat(m_index);

	m_ui.ControlLogarithmicCheckBox->setChecked(
		(flags & drumkv1_controls::Logarithmic) && bFloat);
	m_ui.ControlLogarithmicCheckBox->setEnabled(bFloat);

	m_ui.ControlInvertCheckBox->setChecked(
		(flags & drumkv1_controls::Invert));
	m_ui.ControlInvertCheckBox->setEnabled(true);

	m_ui.ControlHookCheckBox->setChecked(
		(flags & drumkv1_controls::Hook) || !bFloat);
	m_ui.ControlHookCheckBox->setEnabled(bFloat);

	--m_iDirtySetup;

	m_iDirtyCount = 0;
}


drumkv1_controls *drumkv1widget_control::controls (void) const
{
	return m_pControls;
}


drumkv1::ParamIndex drumkv1widget_control::controlIndex (void) const
{
	return m_index;
}


// Pseudo-destructor.
void drumkv1widget_control::closeEvent ( QCloseEvent *pCloseEvent )
{
	// Pseudo-singleton reference setup.
	g_pInstance = nullptr;

	// Sure acceptance and probable destruction (cf. WA_DeleteOnClose).
	QDialog::closeEvent(pCloseEvent);
}


// Process incoming controller key event.
void drumkv1widget_control::setControlKey ( const drumkv1_controls::Key& key )
{
	setControlType(key.type());
	setControlParam(key.param);

	m_ui.ControlChannelSpinBox->setValue(key.channel());

	QPushButton *pResetButton
		= m_ui.DialogButtonBox->button(QDialogButtonBox::Reset);
	if (pResetButton && m_pControls)
		pResetButton->setEnabled(m_pControls->find_control(key) >= 0);
}


drumkv1_controls::Key drumkv1widget_control::controlKey (void) const
{
	drumkv1_controls::Key key;

	const drumkv1_controls::Type ctype = controlType();
	const unsigned short channel = controlChannel();

	key.status = ctype | (channel & 0x1f);
	key.param = controlParam();

	return key;
}


// Change settings (anything else slot).
void drumkv1widget_control::changed (void)
{
	if (m_iDirtySetup > 0)
		return;

#ifdef CONFIG_DEBUG_0
	qDebug("drumkv1widget_control::changed()");
#endif

	++m_iDirtyCount;

	stabilize();
}


// Reset settings (action button slot).
void drumkv1widget_control::clicked ( QAbstractButton *pButton )
{
#ifdef CONFIG_DEBUG_0
	qDebug("drumkv1widget_control::clicked(%p)", pButton);
#endif

	QDialogButtonBox::ButtonRole role
		= m_ui.DialogButtonBox->buttonRole(pButton);
	if ((role & QDialogButtonBox::ResetRole) == QDialogButtonBox::ResetRole)
		reset();
}


// Reset settings (Reset button slot).
void drumkv1widget_control::reset (void)
{
	if (m_pControls == nullptr)
		return;

#ifdef CONFIG_DEBUG_0
	qDebug("drumkv1widget_control::reset()");
#endif

	const int iIndex = m_pControls->find_control(m_key);
	if (iIndex < 0)
		return;

	// Unmap the existing controller....
	m_pControls->remove_control(m_key);

	// Save controls...
	drumkv1_config *pConfig = drumkv1_config::getInstance();
	if (pConfig)
		pConfig->saveControls(m_pControls);

	// Aint't dirty no more...
	m_iDirtyCount = 0;

	// Bail out...
	QDialog::accept();
	QDialog::close();
}


// Accept settings (OK button slot).
void drumkv1widget_control::accept (void)
{
	if (m_pControls == nullptr)
		return;

#ifdef CONFIG_DEBUG_0
	qDebug("drumkv1widget_control::accept()");
#endif

	// Unmap the existing controller....
	int iIndex = m_pControls->find_control(m_key);
	if (iIndex >= 0)
		m_pControls->remove_control(m_key);

	// Get new map settings...
	m_key = controlKey();

	// Check if already mapped to someone else...
	iIndex = m_pControls->find_control(m_key);
	if (iIndex >= 0 && drumkv1::ParamIndex(iIndex) != m_index) {
		if (QMessageBox::warning(this,
			QDialog::windowTitle(),
			tr("MIDI controller is already assigned.\n\n"
			"Do you want to replace the mapping?"),
			QMessageBox::Ok |
			QMessageBox::Cancel) == QMessageBox::Cancel)
			return;
	}

	// Unmap the existing controller....
	if (iIndex >= 0)
		m_pControls->remove_control(m_key);

	// Reset controller flags all te way...
	unsigned int flags = 0;

	if (m_ui.ControlLogarithmicCheckBox->isEnabled() &&
		m_ui.ControlLogarithmicCheckBox->isChecked())
		flags |= drumkv1_controls::Logarithmic;

	if (m_ui.ControlInvertCheckBox->isEnabled() &&
		m_ui.ControlInvertCheckBox->isChecked())
		flags |= drumkv1_controls::Invert;

	if (m_ui.ControlHookCheckBox->isEnabled() &&
		m_ui.ControlHookCheckBox->isChecked())
		flags |= drumkv1_controls::Hook;

	// Map the damn controller....
	drumkv1_controls::Data data;
	data.index = m_index;
	data.flags = flags;

	m_pControls->add_control(m_key, data);

	// Save controls...
	drumkv1_config *pConfig = drumkv1_config::getInstance();
	if (pConfig)
		pConfig->saveControls(m_pControls);

	// Aint't dirty no more...
	m_iDirtyCount = 0;

	// Just go with dialog acceptance...
	QDialog::accept();
	QDialog::close();
}


// Reject settings (Cancel button slot).
void drumkv1widget_control::reject (void)
{
#ifdef CONFIG_DEBUG_0
	qDebug("drumkv1widget_control::reject()");
#endif

	// Check if there's any pending changes...
	if (m_iDirtyCount > 0) {
		switch (QMessageBox::warning(this,
			QDialog::windowTitle(),
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			QMessageBox::Apply |
			QMessageBox::Discard |
			QMessageBox::Cancel)) {
		case QMessageBox::Discard:
			break;
		case QMessageBox::Apply:
			accept();
		default:
			return;
		}
	}

	// Just go with dialog rejection...
	QDialog::reject();
	QDialog::close();
}


// Perotected slots.
void drumkv1widget_control::activateControlType ( int iControlType )
{
#ifdef CONFIG_DEBUG_0
	qDebug("drumkv1widget_control::activateControlType(%d)", iControlType);
#endif

	updateControlType(iControlType);

	changed();
}

void drumkv1widget_control::editControlParamFinished (void)
{
	if (m_iControlParamUpdate > 0)
		return;

	++m_iControlParamUpdate;

	const QString& sControlParam
		= m_ui.ControlParamComboBox->currentText();

	bool bOk = false;
	sControlParam.toInt(&bOk);
	if (bOk) changed();

	--m_iControlParamUpdate;
}


void drumkv1widget_control::stabilize (void)
{
	const bool bValid = (m_iDirtyCount > 0);
	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bValid);
}


// Control type dependency refresh.
void drumkv1widget_control::updateControlType ( int iControlType )
{
	if (iControlType < 0)
		iControlType = m_ui.ControlTypeComboBox->currentIndex();

	const drumkv1_controls::Type ctype
		= controlTypeFromIndex(iControlType);

	const bool bOldEditable
		= m_ui.ControlParamComboBox->isEditable();
	const int iOldParam
		= m_ui.ControlParamComboBox->currentIndex();
	const QString sOldParam
		= m_ui.ControlParamComboBox->currentText();

	m_ui.ControlParamComboBox->clear();

	const QString sMask("%1 - %2");
	switch (ctype) {
	case drumkv1_controls::CC: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(false);
		const drumkv1widget_controls::Names& controllers
			= drumkv1widget_controls::controllerNames();
		for (unsigned short param = 0; param < 128; ++param) {
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(controllers.value(param)),
				int(param));
		}
		break;
	}
	case drumkv1_controls::RPN: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(true);
		const drumkv1widget_controls::Names& rpns
			= drumkv1widget_controls::rpnNames();
		drumkv1widget_controls::Names::ConstIterator iter = rpns.constBegin();
		const drumkv1widget_controls::Names::ConstIterator& iter_end
			= rpns.constEnd();
		for ( ; iter != iter_end; ++iter) {
			const unsigned short param = iter.key();
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(iter.value()),
				int(param));
		}
		break;
	}
	case drumkv1_controls::NRPN: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(true);
		const drumkv1widget_controls::Names& nrpns
			= drumkv1widget_controls::nrpnNames();
		drumkv1widget_controls::Names::ConstIterator iter = nrpns.constBegin();
		const drumkv1widget_controls::Names::ConstIterator& iter_end
			= nrpns.constEnd();
		for ( ; iter != iter_end; ++iter) {
			const unsigned short param = iter.key();
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(iter.value()),
				int(param));
		}
		break;
	}
	case drumkv1_controls::CC14: {
		if (m_ui.ControlParamTextLabel)
			m_ui.ControlParamTextLabel->setEnabled(true);
		m_ui.ControlParamComboBox->setEnabled(true);
		m_ui.ControlParamComboBox->setEditable(false);
		const drumkv1widget_controls::Names& control14s
			= drumkv1widget_controls::control14Names();
		for (unsigned short param = 1; param < 32; ++param) {
			m_ui.ControlParamComboBox->addItem(
				sMask.arg(param).arg(control14s.value(param)),
				int(param));
		}
		break;
	}
	default:
		break;
	}

	if (iOldParam >= 0 && iOldParam < m_ui.ControlParamComboBox->count())
		m_ui.ControlParamComboBox->setCurrentIndex(iOldParam);

	if (m_ui.ControlParamComboBox->isEditable()) {
		QObject::connect(m_ui.ControlParamComboBox->lineEdit(),
			SIGNAL(editingFinished()),
			SLOT(editControlParamFinished()));
		if (bOldEditable)
			m_ui.ControlParamComboBox->setEditText(sOldParam);
	}
}


void drumkv1widget_control::setControlType ( drumkv1_controls::Type ctype )
{
	const int iControlType = indexFromControlType(ctype);
	m_ui.ControlTypeComboBox->setCurrentIndex(iControlType);
	updateControlType(iControlType);
}


drumkv1_controls::Type drumkv1widget_control::controlType (void) const
{
	return controlTypeFromIndex(m_ui.ControlTypeComboBox->currentIndex());
}


void drumkv1widget_control::setControlParam ( unsigned short param )
{
	const int iControlParam = indexFromControlParam(param);
	if (iControlParam >= 0) {
		m_ui.ControlParamComboBox->setCurrentIndex(iControlParam);
	} else {
		const QString& sControlParam = QString::number(param);
		m_ui.ControlParamComboBox->setEditText(sControlParam);
	}
}


unsigned short drumkv1widget_control::controlParam (void) const
{
	if (m_ui.ControlParamComboBox->isEditable()) {
		unsigned short param = 0;
		const QString& sControlParam
			= m_ui.ControlParamComboBox->currentText();
		bool bOk = false;
		param = sControlParam.toInt(&bOk);
		if (bOk) return param;
	}

	return controlParamFromIndex(m_ui.ControlParamComboBox->currentIndex());
}


unsigned short drumkv1widget_control::controlChannel (void) const
{
	return m_ui.ControlChannelSpinBox->value();
}


drumkv1_controls::Type drumkv1widget_control::controlTypeFromIndex ( int iIndex ) const
{
	if (iIndex >= 0 && iIndex < m_ui.ControlTypeComboBox->count())
		return drumkv1_controls::Type(
			m_ui.ControlTypeComboBox->itemData(iIndex).toInt());
	else
		return drumkv1_controls::CC;
}


int drumkv1widget_control::indexFromControlType ( drumkv1_controls::Type ctype ) const
{
	return m_ui.ControlTypeComboBox->findData(int(ctype));
}


unsigned short drumkv1widget_control::controlParamFromIndex ( int iIndex ) const
{
	if (iIndex >= 0 && iIndex < m_ui.ControlParamComboBox->count())
		return m_ui.ControlParamComboBox->itemData(iIndex).toInt();
	else
		return 0;
}


int drumkv1widget_control::indexFromControlParam ( unsigned short param ) const
{
	return m_ui.ControlParamComboBox->findData(int(param));
}


// end of drumkv1widget_control.cpp
