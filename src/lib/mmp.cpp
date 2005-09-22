/*
 * mmp.cpp - implementation of class multimediaProject
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


#include "qt3support.h"

#ifdef QT4

#include <QFile>
#include <QMessageBox>

#else

#include <qfile.h>
#include <qmessagebox.h>

#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include "mmp.h"
#include "song_editor.h"



multimediaProject::typeDescStruct multimediaProject::s_types[multimediaProject::PROJ_TYPE_COUNT] =
{
	{ multimediaProject::UNKNOWN, "unknown" },
	{ multimediaProject::SONG_PROJECT, "song" },
	{ multimediaProject::CHANNEL_SETTINGS, "channelsettings" },
	{ multimediaProject::EFFECT_SETTINGS, "effectsettings" },
	{ multimediaProject::VIDEO_PROJECT, "video" },
	{ multimediaProject::BURN_PROJECT, "burn" },
	{ multimediaProject::PLAYLIST, "playlist" }
} ;



multimediaProject::multimediaProject( projectTypes _project_type ) :
	QDomDocument( "multimedia-project" ),
	m_content(),
	m_head(),
	m_type( _project_type )
{
	QDomElement root = createElement( "multimediaproject" );
	root.setAttribute( "version", MMP_VERSION_STRING );
	root.setAttribute( "type", typeName( _project_type ) );
	root.setAttribute( "creator", "Linux MultiMedia Studio (LMMS)" );
	root.setAttribute( "creatorversion", VERSION );
	appendChild( root );

	m_head = createElement( "head" );
	root.appendChild( m_head );

	m_content = createElement( typeName( _project_type ) );
	root.appendChild( m_content );

}




multimediaProject::multimediaProject( const QString & _in_file_name ) :
	QDomDocument(),
	m_content(),
	m_head()
{
	QFile in_file( _in_file_name );
#ifdef QT4
	if( !in_file.open( QIODevice::ReadOnly ) )
#else
	if( !in_file.open( IO_ReadOnly ) )
#endif
	{
		QMessageBox::critical( NULL,
					songEditor::tr( "Could not open file" ),
					songEditor::tr( "Could not open "
							"file %1. You probably "
							"have no rights to "
							"read this file.\n"
							"Please make sure you "
							"have at least read-"
							"access to the file "
							"and try again."
						).arg( _in_file_name ) );
		return;
	}

	QString error_msg;
	int line;
	int col;
	if( !setContent( &in_file, &error_msg, &line, &col ) )
	{
		QMessageBox::critical( NULL, songEditor::tr( "Error in "
							"multimedia-project" ),
					songEditor::tr( "The multimedia-"
							"project %1 seems to "
							"contain errors. LMMS "
							"will try its best "
							"to recover as much as "
							"possible data from "
							"this file."
						).arg( _in_file_name ) );
		return;
	}
	in_file.close();


	QDomElement root = documentElement();
	m_type = type( root.attribute( "type" ) );

	QDomNode node = root.firstChild();
	while( !node.isNull() )
	{
		if( node.isElement() )
		{
			if( node.nodeName() == "head" )
			{
				m_head = node.toElement();
			}
			else if( node.nodeName() == typeName( m_type ) )
			{
				m_content = node.toElement();
			}
		}
		node = node.nextSibling();
	}
}




multimediaProject::~multimediaProject()
{
}




bool multimediaProject::writeFile( const QString & _fn, bool _overwrite_check )
{
	QString xml = "<?xml version=\"1.0\"?>\n" + toString(
#if QT_VERSION >= 0x030100
								2
#endif
									);
	QString fn = _fn;
	if( type() == CHANNEL_SETTINGS )
	{
		if( fn.section( '.', -2, -1 ) != "cs.xml" )
		{
			fn += ".cs.xml";
		}
	}
	else if( fn.section( '.',-1 ) != "xml" )
	{
		fn += ".xml";
	}


	QFile outfile( fn );
	if( _overwrite_check == TRUE &&
		outfile.exists() == TRUE &&
		QMessageBox::
#if QT_VERSION >= 0x030200
		question
#else
		information
#endif
				( NULL,
					songEditor::tr( "File already exists" ),
					songEditor::tr( "The file %1 already "
							"exists.\nDo you want "
							"to overwrite it?"
						).arg( fn ),
					QMessageBox::Yes, QMessageBox::No )
			== QMessageBox::No )
	{
		return( FALSE );
	}


#ifdef QT4
	if( !outfile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
#else
	if( !outfile.open( IO_WriteOnly | IO_Truncate ) )
#endif
	{
		QMessageBox::critical( NULL, songEditor::tr( "Could not write "
								"file" ),
					songEditor::tr( "Could not write file "
							"%1. You probably are "
							"not permitted to "
							"write to this file. "
							"Please make sure you "
							"have write-access to "
							"the file and try "
							"again."
						).arg( fn ) );
		return( FALSE );
	}
#ifdef QT4
	outfile.write( xml.toAscii().constData(), xml.length() );
#else
	outfile.writeBlock( xml.ascii(), xml.length() );
#endif
	outfile.close();

	return( TRUE );
}




multimediaProject::projectTypes multimediaProject::typeOfFile(
							const QString & _fn )
{
	multimediaProject m( _fn );
	return( m.type() );
}




multimediaProject::projectTypes multimediaProject::type(
						const QString & _type_name )
{
	for( int i = 0; i < PROJ_TYPE_COUNT; ++i )
	{
		if( s_types[i].m_name == _type_name )
		{
			return( static_cast<multimediaProject::projectTypes>(
									i ) );
		}
	}
	return( UNKNOWN );
}




QString multimediaProject::typeName( projectTypes _project_type )
{
	if( _project_type >= UNKNOWN && _project_type < PROJ_TYPE_COUNT )
	{
		return( s_types[_project_type].m_name );
	}
	return( s_types[UNKNOWN].m_name );
}

