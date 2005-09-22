/*
 * config_mgr.h - class configManager, a class for managing LMMS-configuration
 *
 * Linux MultiMedia Studio
 * Copyright (c) 2004-2005 Tobias Doerffel <tobydox@users.sourceforge.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */


#ifndef _CONFIG_MGR_H
#define _CONFIG_MGR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "qt3support.h"


#ifdef QT4

#include <QDialog>
#include <QMap>
#include <QVector>
#include <QPair>

#else

#include <qdialog.h>
#include <qmap.h>
#include <qvaluevector.h>
#include <qpair.h>

#endif


#include "lmms_main_win.h"


class QLineEdit;
class QLabel;
class QRadioButton;
class QHBoxLayout;
class QVBoxLayout;
class QFrame;


const QString PROJECTS_PATH = "projects/";
const QString PRESETS_PATH = "presets/";
const QString SAMPLES_PATH = "samples/";
const QString ARTWORK_PATH = "artwork/";
const QString LOCALE_PATH = "locale/";


class configManager : public QDialog 
{
	Q_OBJECT
public:
	static inline configManager * inst( void )
	{
		if( s_instanceOfMe == NULL )
		{
			s_instanceOfMe = new configManager();
		}
		return( s_instanceOfMe );
	}

	const QString & lmmsWorkingDir( void ) const
	{
		return( m_lmmsWorkingDir );
	}
	QString projectsDir( void ) const
	{
		return( m_lmmsWorkingDir + PROJECTS_PATH );
	}
	QString presetsDir( void ) const
	{
		return( m_lmmsWorkingDir + PRESETS_PATH );
	}
	QString samplesDir( void ) const
	{
		return( m_lmmsWorkingDir + SAMPLES_PATH );
	}
	QString artworkDir( void ) const
	{
		return( m_lmmsDataDir + ARTWORK_PATH );
	}
	QString localeDir( void ) const
	{
		return( m_lmmsDataDir + LOCALE_PATH );
	}

	const QString & value( const QString & _class,
					const QString & _attribute ) const;
	void setValue( const QString & _class, const QString & _attribute,
						const QString & _value );

	bool loadConfigFile( void );
	void saveConfigFile( void );


protected slots:
	void openWorkingDir( void );
	void setWorkingDir( const QString & _wd );

	virtual void accept( void );

        void backButtonClicked( void );
        void nextButtonClicked( void );
	void switchPage( csize _pg );
	void switchPage( QWidget * _pg );


private:
	static configManager * s_instanceOfMe;

	configManager( void );
	configManager( const configManager & _c );
	~configManager();

	void createWidgets( void );


	void FASTCALL addPage( QWidget * _w, const QString & _title );

	static void processFilesRecursively( const QString & _src_dir,
						const QString & _dst_dir,
		void( * _proc_func )( const QString & _src, const QString &
								_dst ) );


	const QString m_lmmsRcFile;
	QString m_lmmsWorkingDir;
	QString m_lmmsDataDir;

	typedef vvector<QPair<QString, QString> > stringPairVector;
	typedef QMap<QString, stringPairVector> settingsMap;
	settingsMap m_settings;


	QWidget * m_pageIntro;
	QWidget * m_pageWorkingDir;
	QWidget * m_pageFiles;

	QRadioButton * m_samplesCopyRB;
	QRadioButton * m_presetsCopyRB;
	QRadioButton * m_projectsCopyRB;

	QLineEdit * m_wdLineEdit;

	// wizard stuff
	vlist<QPair<QWidget *, QString> > m_pages;
	csize m_currentPage;
	QFrame * m_hbar;
	QWidget * m_contentWidget;
	QLabel * m_title;
	QPushButton * m_cancelButton;
	QPushButton * m_backButton;
	QPushButton * m_nextButton;
	QPushButton * m_finishButton;
	QHBoxLayout * m_buttonLayout;
	QHBoxLayout * m_mainLayout;
	QVBoxLayout * m_contentLayout;


	// lmmsMainWin-dtor is allowed to destroy us...
	friend lmmsMainWin::~lmmsMainWin();

} ;

#endif
