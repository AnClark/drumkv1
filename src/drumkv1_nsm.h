// drumkv1_nsm.h
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

#ifndef __drumkv1_nsm_h
#define __drumkv1_nsm_h

#include "drumkv1_config.h"

#include <QObject>

#ifdef CONFIG_LIBLO
#include <lo/lo.h>
#endif


//---------------------------------------------------------------------------
// drumkv1_nsm - NSM OSC client agent.

class drumkv1_nsm : public QObject
{
	Q_OBJECT

public:

	// Constructor.
	drumkv1_nsm(const QString& nsm_url, QObject *pParent = 0);

	// Destructor.
	~drumkv1_nsm();

	// Session activation accessor.
	bool is_active() const;

	// Session manager accessors.
	const QString& manager() const;
	const QString& capabilities() const;

	// Session client accessors.
	const QString& path_name() const;
	const QString& display_name() const;
	const QString& client_name() const;

	// Session client methods.
	void announce(const QString& app_name, const QString& capabilities);
	void dirty(bool is_dirty);
	void visible(bool is_visible);
	void progress(float percent);
	void message(int priority, const QString& mesg);

	// Status/error codes
	enum ReplyCode
	{
		ERR_OK               =  0,
		ERR_GENERAL          = -1,
		ERR_INCOMPATIBLE_API = -2,
		ERR_BLACKLISTED      = -3,
		ERR_LAUNCH_FAILED    = -4,
		ERR_NO_SUCH_FILE     = -5,
		ERR_NO_SESSION_OPEN  = -6,
		ERR_UNSAVED_CHANGES  = -7,
		ERR_NOT_NOW          = -8,
		ERR_BAD_PROJECT      = -9,
		ERR_CREATE_FAILED    = -10
	};

	// Session client reply methods.
	void open_reply(ReplyCode reply_code = ERR_OK);
	void save_reply(ReplyCode reply_code = ERR_OK);

	// Server methods response methods.
	void nsm_announce_error(
		const char *mesg);

	void nsm_announce_reply(
		const char *mesg,
		const char *manager,
		const char *capabilities);

	void nsm_open(
		const char *path_name,
		const char *display_name,
		const char *client_name);

	void nsm_save();
	void nsm_loaded();
	void nsm_show();
	void nsm_hide();

protected:

	void reply(const QString& path, ReplyCode reply_code);

signals:

	// Session client callbacks.
	void active(bool is_active);
	void open();
	void save();
	void loaded();
	void show();
	void hide();

private:

	// Instance variables.
#ifdef CONFIG_LIBLO
	lo_address m_address;
	lo_server_thread m_thread;
	lo_server  m_server;
#endif
	bool       m_active;
	bool       m_dirty;
	QString    m_manager;
	QString    m_capabilities;
	QString    m_path_name;
	QString    m_display_name;
	QString    m_client_name;
};


#endif // __drumkv1_nsm_h

// end of drumkv1_nsm.h
