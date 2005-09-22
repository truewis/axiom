/***************************************************************************
                          kmultitabbar.h -  description
                             -------------------
    begin              :  2001
    copyright          : (C) 2001,2002,2003 by Joseph Wenninger <jowenn@kde.org>
 ***************************************************************************/

/***************************************************************************
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
 ***************************************************************************/

#ifndef _KMultitabbar_h_
#define _KMultitabbar_h_

#include "qt3support.h"

class KMultiTabBarButton;
class KMultiTabBarTab;

#ifdef QT4

#include <QWidget>
#include <QMenu>
//#include <QVBoxWidget>
#include <QString>
#include <QBoxLayout>
#include <QList>
#include <QScrollArea>
#include <QPushButton>

typedef QList<KMultiTabBarButton *> buttonList;
typedef QList<KMultiTabBarTab *> tabList;

#else

#include <qscrollview.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qabstractlayout.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qpushbutton.h>

typedef QPtrList<KMultiTabBarButton> buttonList;
typedef QPtrList<KMultiTabBarTab> tabList;

#endif


class QPixmap;
class QPainter;
class QFrame;

class KMultiTabBarPrivate;
class KMultiTabBarTabPrivate;
class KMultiTabBarButtonPrivate;
class KMultiTabBarInternal;

/**
 * A Widget for horizontal and vertical tabs.
 * It is possible to add normal buttons to the top/left
 * The handling if only one tab at a time or multiple tabs
 * should be raisable is left to the "user".
 *@author Joseph Wenninger
 */
class KMultiTabBar: public QWidget
{
	Q_OBJECT
public:
	//enum KMultiTabBarMode{ Horizontal, Vertical };
	enum KMultiTabBarPosition{DockLeft, DockRight, DockTop, DockBottom};

	/**
	 * VSNET == Visual Studio .Net like (only show the text of active tabs
	 * KDEV3 == Kdevelop 3 like (always show the text)
	 * KONQSBC == konqy's classic sidebar style (unthemed), this one is disabled
	 * 	at the moment, but will be renabled soon too
	 */
	enum KMultiTabBarStyle{VSNET=0, KDEV3=1, KONQSBC=2, KDEV3ICON=3,STYLELAST=0xffff};

	KMultiTabBar(Qt::Orientation o,QWidget *parent=0);
	virtual ~KMultiTabBar();

	/**
 	 * append  a new button to the button area. The button can later on be accessed with button(ID)
	 * eg for connecting signals to it
	 * @param pic a pixmap for the button
 	 * @param id an arbitraty ID value. It will be emitted in the clicked signal for identifying the button
	 *	if more than one button is connected to a signals.
	 * @param popup A popup menu which should be displayed if the button is clicked
	 * @param not_used_yet will be used for a popup text in the future
	 */
 	int appendButton(const QPixmap &pic,int id=-1,QMenu* popup=0,const QString& not_used_yet=QString::null);
	/** 
         * remove a button with the given ID
	 */
	void removeButton(int id);
	/**
	 * append a new tab to the tab area. It can be accessed lateron with tabb(id);
	 * @param pic a bitmap for the tab
	 * @param id an arbitrary ID which can be used later on to identify the tab
	 * @param text if a mode with text is used it will be the tab text, otherwise a mouse over hint
	 */
	int appendTab(const QPixmap &pic,int id=-1,const QString& text=QString::null);
	/**
	 * remove a tab with a given ID
	 */
	void removeTab(int id);
	/**
	 * set a tab to "raised"
	 * @param id The ID of the tab to manipulate
	 * @param state true == activated/raised, false == not active
	 */
	void setTab(int id ,bool state);
	/**
	 * return the state of a tab, identified by it's ID
	 */
	bool isTabRaised(int id) const;
	/**
	 * get a pointer to a button within the button area identified by its ID
	 */
	class KMultiTabBarButton *button(int id) const;

	/**
	 * get a pointer to a tab within the tab area, identiifed by its ID
	 */
	class KMultiTabBarTab *tab(int id) const;
	/**
	 * set the real position of the widget.
	 * @param pos if the mode is horizontal, only use top, bottom, if it is vertical use left or right
	 */
	void setPosition(KMultiTabBarPosition pos);
	/**
	 * set the display style of the tabs
	 */
	void setStyle(KMultiTabBarStyle style);
	/**
	 * be carefull, don't delete tabs yourself and don't delete the list itself
	 */
        tabList * tabs();
	/**
	 * be carefull, don't delete buttons yourself and don't delete the list itself
	 */
	buttonList * buttons();

	/**
	 * might vanish, not sure yet
	 */
	void showActiveTabTexts(bool show=true);
protected:
	friend class KMultiTabBarButton;
	virtual void fontChange( const QFont& );
	void updateSeparator();
private:
	class KMultiTabBarInternal *m_internal;
	QBoxLayout *m_l;
	QFrame *m_btnTabSep;
	buttonList m_buttons;
	KMultiTabBarPosition m_position;
	KMultiTabBarPrivate *d;
};

/**
 * This class should never be created except with the appendButton call of KMultiTabBar
 */
class KMultiTabBarButton: public QPushButton
{
	Q_OBJECT
public:
	KMultiTabBarButton(const QPixmap& pic,const QString&, QMenu *popup,
		int id,QWidget *parent, KMultiTabBar::KMultiTabBarPosition pos, KMultiTabBar::KMultiTabBarStyle style);
	KMultiTabBarButton(const QString&, QMenu *popup,
		int id,QWidget *parent, KMultiTabBar::KMultiTabBarPosition pos, KMultiTabBar::KMultiTabBarStyle style);
	virtual  ~KMultiTabBarButton();
	int id() const;

public slots:
	/**
	 * this is used internaly, but can be used by the user, if (s)he wants to
	 * It the according call of KMultiTabBar is invoked though this modifications will be overwritten
	 */
	void setPosition(KMultiTabBar::KMultiTabBarPosition);
        /**
         * this is used internaly, but can be used by the user, if (s)he wants to
         * It the according call of KMultiTabBar is invoked though this modifications will be overwritten
         */
	void setStyle(KMultiTabBar::KMultiTabBarStyle);

        /**
	 * modify the text of the button
         */
	void setText(const QString &);

	QSize sizeHint() const;

protected:
	KMultiTabBar::KMultiTabBarPosition m_position;
	KMultiTabBar::KMultiTabBarStyle m_style;
	QString m_text;
	virtual void hideEvent( class QHideEvent*);
	virtual void showEvent( class QShowEvent*);
private:
	int m_id;
	KMultiTabBarButtonPrivate *d;
signals:
	/**
	 * this is emitted if  the button is clicked
	 * @param id	the ID identifying the button
	 */
	void clicked(int id);
protected slots:
	virtual void slotClicked();
};

/**
 * This class should never be created except with the appendTab call of KMultiTabBar
 */
class KMultiTabBarTab: public KMultiTabBarButton
{
	Q_OBJECT
public:
	KMultiTabBarTab(const QPixmap& pic,const QString&,int id,QWidget *parent,
		KMultiTabBar::KMultiTabBarPosition pos,KMultiTabBar::KMultiTabBarStyle style);
	virtual ~KMultiTabBarTab();
	/**
	 * set the active state of the tab
	 * @param  state true==active false==not active
	 */
	void setState(bool state);
	/**
	 * choose if the text should always be displayed
	 * this is only used in classic mode if at all
	 */
	void showActiveTabText(bool show);
	void resize(){ setSize( neededSize() ); }
#ifndef QT4
	virtual inline const QColorGroup & palette( void ) const
	{
		return( colorGroup() );
	}
#endif 
private:
	bool m_showActiveTabText;
	int m_expandedSize;
	KMultiTabBarTabPrivate *d;
protected:
	friend class KMultiTabBarInternal;
	void setSize(int);
	int neededSize();
	void updateState();
	virtual void drawButton(QPainter *);
	virtual void drawButtonLabel(QPainter *);
	void drawButtonStyled(QPainter *);
	void drawButtonClassic(QPainter *);
protected slots:
	virtual void slotClicked();
	void setTabsPosition(KMultiTabBar::KMultiTabBarPosition);

public slots:
	virtual void setIcon(const QString&);
	virtual void setIcon(const QPixmap&);
};



class KMultiTabBarInternal: public Q3ScrollView
{
        Q_OBJECT
public:
        KMultiTabBarInternal(QWidget *parent,Qt::Orientation o);
        int appendTab(const QPixmap &,int=-1,const QString& =QString::null);
        KMultiTabBarTab *tab(int) const;
        void removeTab(int);
        void setPosition(enum KMultiTabBar::KMultiTabBarPosition pos);
        void setStyle(enum KMultiTabBar::KMultiTabBarStyle style);
        void showActiveTabTexts(bool show);
        tabList * tabs(){return &m_tabs;}
#ifndef QT4
	virtual inline const QColorGroup & palette( void ) const
	{
		return( colorGroup() );
	}
#endif 
private:
        friend class KMultiTabBar;
        QWidget *box;
	QBoxLayout *mainLayout;
        tabList m_tabs;
        enum KMultiTabBar::KMultiTabBarPosition m_position;
        bool m_showActiveTabTexts;
        enum  KMultiTabBar::KMultiTabBarStyle m_style;
	int m_expandedTabSize;
	int m_lines;
protected:
        virtual void drawContents ( QPainter *, int, int, int, int);

        /**
         * [contentsM|m]ousePressEvent are reimplemented from QScrollView
         * in order to ignore all mouseEvents on the viewport, so that the
         * parent can handle them.
         */
        virtual void contentsMousePressEvent(QMouseEvent *);
        virtual void mousePressEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);
};

#endif
