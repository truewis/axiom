/*
 * pattern.cpp - implementation of class pattern which holds notes
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

#include <Qt/QtXml>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>
#include <QImage>
#include <QMouseEvent>

#else

#include <qdom.h>
#include <qpopupmenu.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qimage.h>

#endif


#include "pattern.h"
#include "channel_track.h"
#include "templates.h"
#include "embed.h"
#include "piano_roll.h"
#include "track_container.h"
#include "rename_dialog.h"
#include "sample_buffer.h"
#include "audio_sample_recorder.h"
#include "song_editor.h"
#include "tooltip.h"


QPixmap * pattern::s_patternBg = NULL;
QPixmap * pattern::s_stepBtnOn = NULL;
QPixmap * pattern::s_stepBtnOff = NULL;
QPixmap * pattern::s_stepBtnOffLight = NULL;
QPixmap * pattern::s_frozen = NULL;




pattern::pattern ( channelTrack * _channel_track ) :
	trackContentObject( _channel_track ),
	m_channelTrack( _channel_track ),
	m_patternType( BEAT_PATTERN ),
	m_name( _channel_track->name() ),
	m_frozenPatternMutex(),
	m_frozenPattern( NULL ),
	m_freezeRecorder( NULL ),
	m_freezeStatusDialog( NULL ),
	m_freezeStatusUpdateTimer( NULL )
{
	initPixmaps();

	setFixedHeight( s_patternBg->height() + 4 );

#ifndef QT4
	// set background-mode for flicker-free redraw
	setBackgroundMode( Qt::NoBackground );
#endif

	if( m_patternType == BEAT_PATTERN )
	{
		for( int i = 0; i < MAX_BEATS_PER_TACT; ++i )
		{
			m_notes.push_back( new note( midiTime( 0 ),
							midiTime( i*4 ) ) );
		}
	}

	changeLength( length() );

	setAutoResizeEnabled( FALSE );

	toolTip::add( this,
		tr( "double-click to open this pattern in piano-roll" ) );
}




pattern::pattern( const pattern & _pat_to_copy ) :
	trackContentObject( _pat_to_copy.m_channelTrack ),
	m_channelTrack( _pat_to_copy.m_channelTrack ),
	m_patternType( _pat_to_copy.m_patternType ),
	m_name( "" ),
	m_frozenPatternMutex(),
	m_frozenPattern( NULL )
{
	initPixmaps();

	for( noteVector::const_iterator it = _pat_to_copy.m_notes.begin();
					it != _pat_to_copy.m_notes.end(); ++it )
	{
		m_notes.push_back( new note( **it ) );
	}

	setFixedHeight( s_patternBg->height() + 4 );

#ifndef QT4
	// set background-mode for flicker-free redraw
	setBackgroundMode( Qt::NoBackground );
#endif

	changeLength( length() );
	setAutoResizeEnabled( FALSE );
}




pattern::~pattern()
{
	if( pianoRoll::inst()->currentPattern() == this )
	{
		pianoRoll::inst()->setCurrentPattern( NULL );
	}
	for( noteVector::iterator it = m_notes.begin();
						it != m_notes.end(); ++it )
	{
		delete *it;
	}

	m_notes.clear();

	m_frozenPatternMutex.lock();
	delete m_frozenPattern;
	m_frozenPatternMutex.unlock();
}




void pattern::initPixmaps( void )
{
	if( s_patternBg == NULL )
	{
		s_patternBg = new QPixmap( embed::getIconPixmap(
							"pattern_bg" ) );
	}
	if( s_stepBtnOn == NULL )
	{
		s_stepBtnOn = new QPixmap( embed::getIconPixmap(
							"step_btn_on" ) );
	}
	if( s_stepBtnOff == NULL )
	{
		s_stepBtnOff = new QPixmap( embed::getIconPixmap(
							"step_btn_off" ) );
	}
	if( s_stepBtnOffLight == NULL )
	{
		s_stepBtnOffLight = new QPixmap( embed::getIconPixmap(
						"step_btn_off_light" ) );
	}
	if( s_frozen == NULL )
	{
		s_frozen = new QPixmap( embed::getIconPixmap( "frozen" ) );
	}
}




void pattern::movePosition( const midiTime & _pos )
{
	// patterns are always aligned on tact-boundaries
	trackContentObject::movePosition( midiTime( _pos.getTact(), 0 ) );
}




void pattern::constructContextMenu( QMenu * _cm )
{
#ifdef QT4
	QAction * a = new QAction( embed::getIconPixmap( "piano" ),
					tr( "Open in piano-roll" ), _cm );
	_cm->insertAction( _cm->actions()[0], a );
	connect( a, SIGNAL( triggered( bool ) ), this,
					SLOT( openInPianoRoll( bool ) ) );
#else
	_cm->insertItem( embed::getIconPixmap( "piano" ),
					tr( "Open in piano-roll" ),
					this, SLOT( openInPianoRoll() ),
								0, -1, 0 );
#endif
#ifdef QT4
	_cm->insertSeparator( _cm->actions()[1] );
#else
	_cm->insertSeparator( 1 );
#endif

#ifdef QT4
	_cm->addSeparator();
#else
	_cm->insertSeparator();
#endif
	_cm->addAction( embed::getIconPixmap( "edit_erase" ),
			tr( "Clear all notes" ), this, SLOT( clear() ) );
#ifdef QT4
	_cm->addSeparator();
#else
	_cm->insertSeparator();
#endif
	_cm->addAction( embed::getIconPixmap( "reload" ), tr( "Reset name" ),
						this, SLOT( resetName() ) );
	_cm->addAction( embed::getIconPixmap( "rename" ), tr( "Change name" ),
						this, SLOT( changeName() ) );
#ifdef QT4
	_cm->addSeparator();
#else
	_cm->insertSeparator();
#endif
	_cm->addAction( embed::getIconPixmap( "freeze" ),
		( m_frozenPattern != NULL )? tr( "Refreeze" ) : tr( "Freeze" ),
						this, SLOT( freeze() ) );
	_cm->addAction( embed::getIconPixmap( "unfreeze" ), tr( "Unfreeze" ),
						this, SLOT( unfreeze() ) );
}




void pattern::ensureBeatNotes( void )
{
	// make sure, that all step-note exist
	for( int i = 0; i < MAX_BEATS_PER_TACT; ++i )
	{
		bool found = FALSE;
		for( noteVector::iterator it = m_notes.begin();
						it != m_notes.end(); ++it )
		{
			if( ( *it )->pos() == i * 4 && ( *it )->length() <= 0 )
			{
				found = TRUE;
				break;
			}
		}
		if( found == FALSE )
		{
			addNote( note( midiTime( 0 ), midiTime( i * 4 ) ) );
		}
	}
}




void pattern::paintEvent( QPaintEvent * )
{
	changeLength( length() );

#ifdef QT4
	QPainter p( this );
#else
	// create pixmap for whole widget
	QPixmap pm( rect().size() );

	// and a painter for it
	QPainter p( &pm );
#endif

	for( Sint16 x = 2; x < width() - 1; x += 2 )
	{
		p.drawPixmap( x, 2, *s_patternBg );
	}
	p.setPen( QColor( 57, 69, 74 ) );
	p.drawLine( 0, 0, width(), 0 );
	p.drawLine( 0, 0, 0, height() );
	p.setPen( QColor( 120, 130, 140 ) );
	p.drawLine( 0, height() - 1, width() - 1, height() - 1 );
	p.drawLine( width() - 1, 0, width() - 1, height() - 1 );

	p.setPen( QColor( 0, 0, 0 ) );
	p.drawRect( 1, 1, width() - 2, height() - 2 );

	float ppt = pixelsPerTact();

	if( m_patternType == pattern::MELODY_PATTERN )
	{
		Sint16 central_key = 0;
		if( m_notes.size() > 0 )
		{
			// first determine the central tone so that we can 
			// display the area where most of the m_notes are
			Sint16 total_notes = 0;
			for( noteVector::iterator it = m_notes.begin();
						it != m_notes.end(); ++it )
			{
				if( ( *it )->length() > 0 )
				{
					central_key += ( *it )->key();
					++total_notes;
				}
			}

			if( total_notes > 0 )
			{
				central_key = central_key / total_notes;

				Sint16 central_y = s_patternBg->height() / 2;
				Sint16 y_base = central_y+TCO_BORDER_WIDTH-1;

				const Sint16 x_base = TCO_BORDER_WIDTH;

				p.setPen( QColor( 0, 0, 0 ) );
				for( tact tact_num = 1; tact_num <
						length().getTact(); ++tact_num )
				{
					p.drawLine( x_base + static_cast<int>(
								ppt*tact_num ),
							TCO_BORDER_WIDTH,
							x_base +
					static_cast<int>( ppt * tact_num ),
							height() - 2 *
							TCO_BORDER_WIDTH );
				}
				if( getTrack()->muted() )
				{
					p.setPen( QColor( 160, 160, 160 ) );
				}
				else if( m_frozenPattern != NULL )
				{
					p.setPen( QColor( 0x00, 0xE0, 0xFF ) );
				}
				else
				{
					p.setPen( QColor( 0xFF, 0xB0, 0x00 ) );
				}

				for( noteVector::iterator it = m_notes.begin();
						it != m_notes.end(); ++it )
				{
					Sint16 y_pos = central_key -
								( *it )->key();

					if( ( *it )->length() > 0 &&
							y_pos > -central_y &&
							y_pos < central_y )
					{
						Sint16 x1 = 2 * x_base +
		static_cast<int>( ( *it )->pos() * ppt / 64 );
						Sint16 x2 = x1 +
			static_cast<int>( ( *it )->length() * ppt / 64 );
						p.drawLine( x1, y_base+y_pos,
								x2,
								y_base+y_pos );

					}
				}
			}
		}
	}
	else if( m_patternType == pattern::BEAT_PATTERN && ppt >= 192 )
	{
		QPixmap stepon;
		QPixmap stepoff;
		QPixmap stepoffl;
#ifdef QT4
		stepon = s_stepBtnOn->scaled( width() / 16,
						s_stepBtnOn->height(),
						Qt::IgnoreAspectRatio,
						Qt::SmoothTransformation );
		stepoff = s_stepBtnOff->scaled( width() / 16,
						s_stepBtnOff->height(),
						Qt::IgnoreAspectRatio,
						Qt::SmoothTransformation );
		stepoffl = s_stepBtnOffLight->scaled( width() / 16,
						s_stepBtnOffLight->height(),
						Qt::IgnoreAspectRatio,
						Qt::SmoothTransformation );
#else
		stepon.convertFromImage( s_stepBtnOn->convertToImage().scale(
				width() / 16, s_stepBtnOn->height() ) );
		stepoff.convertFromImage( s_stepBtnOff->convertToImage().scale(
				width() / 16, s_stepBtnOff->height() ) );
		stepoffl.convertFromImage( s_stepBtnOffLight->convertToImage().
			scale( width() / 16, s_stepBtnOffLight->height() ) );
#endif
		for( noteVector::iterator it = m_notes.begin();
						it != m_notes.end(); ++it )
		{
			Sint16 no = it - m_notes.begin();
			Sint16 x = TCO_BORDER_WIDTH + static_cast<int>( no *
									ppt /
							m_notes.size() );
			Sint16 y = height() - s_stepBtnOn->height() - 1;
			if( ( *it )->length() < 0 )
			{
				p.drawPixmap( x, y, stepon );
			}
			else if( ( no / 4 ) % 2 )
			{
				p.drawPixmap( x, y, stepoff );
			}
			else
			{
				p.drawPixmap( x, y, stepoffl );
			}
		}
	}

	p.setFont( pointSize<7>( p.font() ) );
	p.setPen( QColor( 32, 240, 32 ) );
	p.drawText( 2, 9, m_name );
	if( m_frozenPattern != NULL )
	{
		p.setPen( QColor( 0, 224, 255 ) );
		p.drawRect( 0, 0, width(), height() - 1 );
		p.drawPixmap( 3, height() - s_frozen->height() - 4, *s_frozen );
	}

#ifndef QT4
	// blit drawn pixmap to actual widget
	bitBlt( this, rect().topLeft(), &pm );
#endif
}




void pattern::mousePressEvent( QMouseEvent * _me )
{
	if( _me->button() != Qt::LeftButton )
	{
		_me->ignore();
		return;
	}

	if( m_patternType == pattern::BEAT_PATTERN && pixelsPerTact() >= 192 &&
		_me->y() > height() - s_stepBtnOn->height() )
	{
		note * n = m_notes[( _me->x() - TCO_BORDER_WIDTH ) * 16 /
								width() ];
		if( n->length() < 0 )
		{
			n->setLength( 0 );
		}
		else
		{
			n->setLength( -64 );
		}
		songEditor::inst()->setModified();
		update();
		return;
	}
	trackContentObject::mousePressEvent( _me );
}




void pattern::mouseDoubleClickEvent( QMouseEvent * _me )
{
	if( _me->button() != Qt::LeftButton )
	{
		_me->ignore();
		return;
	}
	if( m_patternType == pattern::MELODY_PATTERN ||
		!( m_patternType == pattern::BEAT_PATTERN &&
		pixelsPerTact() >= 192 &&
		_me->y() > height() - s_stepBtnOn->height() ) )
	{
		openInPianoRoll();
	} 
}




void pattern::openInPianoRoll( void )
{
	openInPianoRoll( FALSE );
}




void pattern::openInPianoRoll( bool )
{
	pianoRoll::inst()->setCurrentPattern( this );
	pianoRoll::inst()->show();
	pianoRoll::inst()->setFocus();
	return;
}




void pattern::clear( void )
{
	clearNotes();
	ensureBeatNotes();
}




void pattern::resetName( void )
{
	m_name = m_channelTrack->name();
}




void pattern::changeName( void )
{
	renameDialog rename_dlg( m_name );
	rename_dlg.exec();
}




void pattern::freeze( void )
{
	if( songEditor::inst()->playing() )
	{
		QMessageBox::information( 0, tr( "Cannot freeze pattern" ),
						tr( "The pattern currently "
							"cannot be freezed "
							"because you're in "
							"play-mode. Please "
							"stop and try again!" ),
						QMessageBox::Ok );
		return;
	}
	if( m_channelTrack->muted() )
	{
		if( QMessageBox::
#if QT_VERSION >= 0x030200		
				 question
#else
				 information
#endif				 
		
					    ( 0, tr( "Channel muted" ),
						tr( "The channel this pattern "
							"belongs to is "
							"currently muted, so "
							"freezing makes no "
							"sense! Do you still "
							"want to continue?" ),
						QMessageBox::Yes,
						QMessageBox::No |
						QMessageBox::Default |
						QMessageBox::Escape ) ==
			QMessageBox::No )
		{
			return;
		}
	}

	// already frozen?
	if( m_frozenPattern != NULL )
	{
		// then unfreeze, before freezing it again
		unfreeze();
	}

	mixer::inst()->pause();

	bool b;
	m_freezeRecorder = new audioSampleRecorder(
						mixer::inst()->sampleRate(),
							DEFAULT_CHANNELS, b );
	mixer::inst()->setAudioDevice( m_freezeRecorder,
						mixer::inst()->highQuality() );

	songEditor::inst()->playPattern( this, FALSE );
	songEditor::playPos & ppp = songEditor::inst()->getPlayPos(
						songEditor::PLAY_PATTERN );
	ppp.setTact( 0 );
	ppp.setTact64th( 0 );
	ppp.setCurrentFrame( 0 );
	ppp.m_timeLineUpdate = FALSE;
	m_freezeStatusDialog = new patternFreezeStatusDialog;
	connect( m_freezeStatusDialog, SIGNAL( aborted() ), this,
							SLOT( abortFreeze() ) );

	m_freezeStatusUpdateTimer = new QTimer( this );
	connect( m_freezeStatusUpdateTimer, SIGNAL( timeout() ), this,
					SLOT( updateFreezeStatusDialog() ) );

	m_freezeStatusUpdateTimer->start( 50 );

	m_freezeStatusDialog->show();

	mixer::inst()->play();
}




void pattern::unfreeze( void )
{
	if( m_frozenPattern != NULL )
	{
		m_frozenPatternMutex.lock();
		delete m_frozenPattern;
		m_frozenPattern = NULL;
		m_frozenPatternMutex.unlock();
	}
}




void pattern::updateFreezeStatusDialog( void )
{
	m_freezeStatusDialog->setProgress( songEditor::inst()->getPlayPos(
						songEditor::PLAY_PATTERN ) *
							100 / length() );
	m_frozenPatternMutex.lock();

	// finishFreeze called?
	if( m_freezeRecorder == NULL )
	{
		// then we're done and destroy the timer and the dialog
		delete m_freezeStatusUpdateTimer;
		delete m_freezeStatusDialog;
		m_freezeStatusUpdateTimer = NULL;
		m_freezeStatusDialog = NULL;
	}

	m_frozenPatternMutex.unlock();
}




void pattern::finishFreeze( void )
{
	songEditor::inst()->stop();

	m_frozenPatternMutex.lock();

	m_freezeRecorder->createSampleBuffer( &m_frozenPattern );

	mixer::inst()->restoreAudioDevice();
	m_freezeRecorder = NULL;

	songEditor::inst()->getPlayPos( songEditor::PLAY_PATTERN
						).m_timeLineUpdate = TRUE;

	m_frozenPatternMutex.unlock();
}




void pattern::abortFreeze( void )
{
	songEditor::inst()->stop();

	m_frozenPatternMutex.lock();

	mixer::inst()->restoreAudioDevice();
	m_freezeRecorder = NULL;

	songEditor::inst()->getPlayPos( songEditor::PLAY_PATTERN
						).m_timeLineUpdate = TRUE;

	m_frozenPatternMutex.unlock();
}




void pattern::playFrozenData( sampleFrame * _ab, Uint32 _start_frame,
								Uint32 _frames )
{
	m_frozenPatternMutex.lock();
	if( m_frozenPattern != NULL )
	{
		m_frozenPattern->play( _ab, _start_frame, _frames );
	}
	m_frozenPatternMutex.unlock();
}





midiTime pattern::length( void ) const
{
	if( m_patternType == BEAT_PATTERN )
	{
		// TODO: remove this limitation later by adding 
		// "Add step to pattern"-option
		return( 64 );
	}

	Sint32 max_length = 0;

	for( noteVector::const_iterator it = m_notes.begin();
							it != m_notes.end();
									++it )
	{
		max_length = tMax<Sint32>( max_length, ( *it )->endPos() );
	}
	if( max_length % 64 == 0 )
	{
		return( midiTime( tMax<Sint32>( max_length, 64 ) ) );
	}
	return( midiTime( tMax( midiTime( max_length ).getTact() + 1, 1 ),
									0 ) );
}




note * pattern::addNote( const note & _new_note )
{
	note * new_note = new note( _new_note );

	if( m_notes.size() == 0 || m_notes.last()->pos() <= new_note->pos() )
	{
		m_notes.push_back( new_note );
	}
	else
	{
		// simple algorithm for inserting the note between two 
		// notes with smaller and greater position
		// maybe it could be optimized by starting in the middle and 
		// going forward or backward but note-inserting isn't that
		// time-critical since it is usually not done while playing...
		long new_note_abs_time = new_note->pos();
		noteVector::iterator it = m_notes.begin();

		while( it != m_notes.end() &&
					( *it )->pos() < new_note_abs_time )
		{
			++it;
		}

		m_notes.insert( it, new_note );
	}

	checkType();
	update();
	changeLength( length() );

	return( new_note );
}




void pattern::removeNote( const note * _note_to_del )
{
	noteVector::iterator it = m_notes.begin();
	while( it != m_notes.end() )
	{
		if( ( *it ) == _note_to_del )
		{
			delete *it;
			m_notes.erase( it );
			break;
		}
		++it;
	}

	checkType();
	update();
	changeLength( length() );
}




note * pattern::rearrangeNote( const note * _note_to_proc )
{
	// just rearrange the position of the note by removing it and adding 
	// a copy of it -> addNote inserts it at the correct position
	note copy_of_note( *_note_to_proc );
	removeNote( _note_to_proc );

	return( addNote( copy_of_note ) );
}




void pattern::clearNotes( void )
{
	for( noteVector::iterator it = m_notes.begin(); it != m_notes.end();
									++it )
	{
		delete *it;
	}

	m_notes.clear();
	checkType();
	update();
}




void pattern::setType( patternTypes _new_pattern_type )
{
	if( _new_pattern_type == BEAT_PATTERN ||
				_new_pattern_type == MELODY_PATTERN )
	{
		m_patternType = _new_pattern_type;
	}
}




note * pattern::noteAt( int _note_num )
{
	if( (csize) _note_num < m_notes.size() )
	{
		return( m_notes[_note_num] );
	}
	return( NULL );
}




void pattern::setNoteAt( int _note_num, note _new_note )
{
	if( static_cast<csize>( _note_num ) < m_notes.size() )
	{
		delete m_notes[_note_num];
		m_notes[_note_num] = new note( _new_note );
		checkType();
		update();
	}
}




void pattern::checkType( void )
{
	noteVector::iterator it = m_notes.begin();
	while( it != m_notes.end() )
	{
		if( ( *it )->length() > 0 )
		{
			setType( pattern::MELODY_PATTERN );
			return;
		}
		++it;
	}
	setType( pattern::BEAT_PATTERN );
}





void pattern::saveSettings( QDomDocument & _doc, QDomElement & _parent )
{
	QDomElement pattern_de = _doc.createElement( nodeName() );
	pattern_de.setAttribute( "type", QString::number( m_patternType ) );
	pattern_de.setAttribute( "name", m_name );
	if( _parent.nodeName() == "clipboard" )
	{
		pattern_de.setAttribute( "pos", QString::number( -1 ) );
	}
	else
	{
		pattern_de.setAttribute( "pos", QString::number(
							startPosition() ) );
	}
	pattern_de.setAttribute( "len", QString::number( length() ) );
	pattern_de.setAttribute( "frozen", QString::number(
						m_frozenPattern != NULL ) );
	_parent.appendChild( pattern_de );

	// now save settings of all notes
	for( noteVector::iterator it = m_notes.begin();
						it != m_notes.end(); ++it )
	{
		if( ( *it )->length() )
		{
			( *it )->saveSettings( _doc, pattern_de );
		}
	}
}




void pattern::loadSettings( const QDomElement & _this )
{
	unfreeze();

	m_patternType = static_cast<patternTypes>( _this.attribute( "type"
								).toInt() );
	m_name = _this.attribute( "name" );
	if( _this.attribute( "pos" ).toInt() >= 0 )
	{
		movePosition( _this.attribute( "pos" ).toInt() );
	}
	changeLength( midiTime( _this.attribute( "len" ).toInt() ) );

	clearNotes();

	QDomNode node = _this.firstChild();
	while( !node.isNull() )
	{
		if( node.isElement() )
		{
			note * n = new note();
			n->loadSettings( node.toElement() );
			m_notes.push_back( n );
		}
		node = node.nextSibling();
        }
	ensureBeatNotes();
/*	if( _this.attribute( "frozen" ).toInt() )
	{
		freeze();
	}*/
	update();
}




patternFreezeStatusDialog::patternFreezeStatusDialog( void )
{
	setWindowTitle( tr( "Freezing pattern..." ) );
#if QT_VERSION >= 0x030200
	setModal( TRUE );
#endif

	m_progressBar = new QProgressBar( this );
	m_progressBar->setGeometry( 10, 10, 200, 24 );
#ifdef QT4
	m_progressBar->setMaximum( 100 );
#else
	m_progressBar->setTotalSteps( 100 );
#endif
	m_progressBar->setTextVisible( FALSE );
	m_progressBar->show();
	m_cancelBtn = new QPushButton( embed::getIconPixmap( "cancel" ),
							tr( "Cancel" ), this );
	m_cancelBtn->setGeometry( 50, 38, 120, 28 );
	connect( m_cancelBtn, SIGNAL( clicked() ), this,
						SLOT( cancelBtnClicked() ) );
}




patternFreezeStatusDialog::~patternFreezeStatusDialog()
{
}





void patternFreezeStatusDialog::setProgress( int _p )
{
#ifdef QT4
	m_progressBar->setValue( _p );
#else
	m_progressBar->setProgress( _p );
#endif
}




void patternFreezeStatusDialog::closeEvent( QCloseEvent * _ce )
{
	_ce->ignore();
	cancelBtnClicked();
}




void patternFreezeStatusDialog::cancelBtnClicked( void )
{
	emit( aborted() );
}





#include "pattern.moc"

