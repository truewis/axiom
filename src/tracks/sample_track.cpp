/*
 * sample_track.cpp - implementation of class sampleTrack, a track which
 *                    provides arrangement of samples
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

#include <QPushButton>
#include <QPainter>
#include <Qt/QtXml>

#else

#include <qpushbutton.h>
#include <qpainter.h>
#include <qdom.h>

#endif


#include "sample_track.h"
#include "song_editor.h"
#include "name_label.h"
#include "embed.h"
#include "templates.h"
#include "buffer_allocator.h"
#include "tooltip.h"



sampleTCO::sampleTCO( track * _track ) :
	trackContentObject( _track ),
	m_sampleBuffer()
{
#ifndef QT4
	setBackgroundMode( Qt::NoBackground );
#endif

	setSampleFile( "" );

	// we need to receive bpm-change-events, because then we have to
	// change length of this TCO
	connect( songEditor::inst(), SIGNAL( bpmChanged( int ) ), this,
						SLOT( updateLength( int ) ) );
}




sampleTCO::~sampleTCO()
{
}




void sampleTCO::changeLength( const midiTime & _length )
{
	trackContentObject::changeLength( tMax( static_cast<Sint32>( _length ),
									64 ) );
}




void FASTCALL sampleTCO::play( sampleFrame * _ab, Uint32 _start_frame,
								Uint32 _frames )
{
	_start_frame = static_cast<Uint32>( tMax( 0.0f, _start_frame -
							startPosition() *
				songEditor::inst()->framesPerTact() / 64 ) );
	m_sampleBuffer.play( _ab, _start_frame, _frames );
}




const QString & sampleTCO::sampleFile( void ) const
{
	return( m_sampleBuffer.audioFile() );
}




void sampleTCO::setSampleFile( const QString & _sf )
{
	m_sampleBuffer.setAudioFile( _sf );
	updateLength();
	update();
	// set tooltip to filename so that user can see what sample this
	// sample-tco contains
	toolTip::add( this, ( m_sampleBuffer.audioFile() != "" ) ?
					m_sampleBuffer.audioFile() :
					tr( "double-click to select sample" ) );
}




void sampleTCO::updateLength( int )
{
	changeLength( getSampleLength() );
}




void sampleTCO::paintEvent( QPaintEvent * )
{
#ifdef QT4
	QPainter p( this );
	p.fillRect( rect(), QColor( 0, 64, 255 ) );
#else
	// create pixmap for whole widget
	QPixmap pm( rect().size() );
	pm.fill( QColor( 0, 64, 255 ) );
	// and a painter for it
	QPainter p( &pm );
#endif

	if( getTrack()->muted() )
	{
		p.setPen( QColor( 160, 160, 160 ) );
	}
	else
	{
		p.setPen( QColor( 0, 0, 128 ) );
	}
	p.drawRect( 0, 0, width(), height() );
	m_sampleBuffer.drawWaves( p, QRect( 1, 1, tMax( tMin( width() - 3,
					static_cast<int>( getSampleLength() *
						pixelsPerTact() / 64 ) ), 1 ),
						height() - 4 ) );
#ifndef QT4
	bitBlt( this, rect().topLeft(), &pm );
#endif
}




void sampleTCO::mouseDoubleClickEvent( QMouseEvent * )
{
	QString af = m_sampleBuffer.openAudioFile();
	if( af != "" && af != m_sampleBuffer.audioFile() )
	{
		setSampleFile( af );
		songEditor::inst()->setModified();
	}
}




midiTime sampleTCO::getSampleLength( void ) const
{
	return( static_cast<Sint32>( m_sampleBuffer.frames() /
					songEditor::inst()->framesPerTact() *
									64 ) );
}




void FASTCALL sampleTCO::saveSettings( QDomDocument & _doc, QDomElement & _parent )
{
	QDomElement sampletco_de = _doc.createElement( nodeName() );
	if( _parent.nodeName() == "clipboard" )
	{
		sampletco_de.setAttribute( "pos", QString::number( -1 ) );
	}
	else
	{
		sampletco_de.setAttribute( "pos", QString::number(
							startPosition() ) );
	}
	sampletco_de.setAttribute( "len", QString::number( length() ) );
	sampletco_de.setAttribute( "src", sampleFile() );
	// TODO: start- and end-frame
	_parent.appendChild( sampletco_de );
}




void FASTCALL sampleTCO::loadSettings( const QDomElement & _this )
{
	if( _this.attribute( "pos" ).toInt() >= 0 )
	{
		movePosition( _this.attribute( "pos" ).toInt() );
	}
	changeLength( _this.attribute( "len" ).toInt() );
	setSampleFile( _this.attribute( "src" ) );
}





/*

sampleTCOSettingsDialog::sampleTCOSettingsDialog( sampleTCO * _stco ) :
	QDialog(),
	m_sampleTCO( _stco )
{
	resize( 400, 300 );

	QVBoxWidget * vb0 = new QVBoxWidget( this );
	vb0->resize( 400, 300 );
	QHBoxWidget * hb00 = new QHBoxWidget( vb0 );
	m_fileLbl = new QLabel( _stco->sampleFile(), hb00 );
	QPushButton * open_file_btn = new QPushButton(
				embed::getIconPixmap( "fileopen" ), "", hb00 );
	connect( open_file_btn, SIGNAL( clicked() ), this,
						SLOT( openSampleFile() ) );

	QHBoxWidget * hb01 = new QHBoxWidget( vb0 );

	QPushButton * ok_btn = new QPushButton( tr( "OK" ), hb01 );
	ok_btn->setGeometry( 10, 0, 100, 32 );
	connect( ok_btn, SIGNAL( clicked() ), this, SLOT( accept() ) );

	QPushButton * cancel_btn = new QPushButton( tr( "Cancel" ), hb01 );
	cancel_btn->setGeometry( 120, 0, 100, 32 );
	connect( ok_btn, SIGNAL( clicked() ), this, SLOT( reject() ) );
	
}




sampleTCOSettingsDialog::~sampleTCOSettingsDialog()
{
}




void sampleTCOSettingsDialog::openSampleFile( void )
{
	QString af = m_sampleTCO->m_sampleBuffer.openAudioFile();
	if( af != "" )
	{
		setSampleFile( af );
	}
}




void sampleTCOSettingsDialog::setSampleFile( const QString & _f )
{
	m_fileLbl->setText( _f );
	m_sampleTCO->setSampleFile( _f );
	songEditor::inst()->setModified();
}
*/





sampleTrack::sampleTrack( trackContainer * _tc )
	: track( _tc )
{
	getTrackWidget()->setFixedHeight( 32 );

	m_trackLabel = new nameLabel( tr( "Sample track" ),
						getTrackSettingsWidget(),
						embed::getIconPixmap(
							"sample_track" ) );
	m_trackLabel->setGeometry( 1, 1, DEFAULT_SETTINGS_WIDGET_WIDTH-2, 29 );
	m_trackLabel->show();

	_tc->updateAfterTrackAdd();
}




sampleTrack::~sampleTrack()
{
}




track::trackTypes sampleTrack::trackType( void ) const
{
	return( SAMPLE_TRACK );
}




bool FASTCALL sampleTrack::play( const midiTime & _start, Uint32 _start_frame,
					Uint32 _frames, Uint32 _frame_base,
							Sint16 /*_tco_num*/ )
{
	vlist<trackContentObject *> tcos;
	getTCOsInRange( tcos, _start, _start+static_cast<Sint32>( _frames * 64 /
					songEditor::inst()->framesPerTact() ) );

	if ( tcos.size() == 0 )
	{
		return( FALSE );
	}

	sampleFrame * buf = bufferAllocator::alloc<sampleFrame>( _frames );

	volumeVector v = { 1.0f, 1.0f
#ifndef DISABLE_SURROUND
					, 1.0f, 1.0f
#endif
			} ;
	float fpt = songEditor::inst()->framesPerTact();

	for( vlist<trackContentObject *>::iterator it = tcos.begin();
							it != tcos.end(); ++it )
	{
		sampleTCO * st = dynamic_cast<sampleTCO *>( *it );
		if( st != NULL )
		{
			st->play( buf, _start_frame +
					static_cast<Uint32>( _start.getTact() *
									fpt ),
					_frames );
			mixer::inst()->addBuffer( buf, _frames, _frame_base +
							static_cast<Uint32>(
					st->startPosition().getTact64th() *
							fpt / 64.0f ), v );
		}
	}

	bufferAllocator::free( buf );

	return( TRUE );
}




trackContentObject * sampleTrack::createTCO( const midiTime & )
{
	return( new sampleTCO( this ) );
}





void sampleTrack::saveTrackSpecificSettings( QDomDocument & _doc,
							QDomElement & _parent )
{
	QDomElement st_de = _doc.createElement( nodeName() );
	st_de.setAttribute( "name", m_trackLabel->text() );
	_parent.appendChild( st_de );
}




void sampleTrack::loadTrackSpecificSettings( const QDomElement & _this )
{
	m_trackLabel->setText( _this.attribute( "name" ) );
}




#include "sample_track.moc"

