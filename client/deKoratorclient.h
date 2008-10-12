///////////////////////////////////////////////////////////////////////
// -------------------
// DeKorator window decoration for KDE
// -------------------
// Copyright (C) 2005  moty rahamim <moty.rahamim@gmail.com>
//
// Based on Example-0.8, some ideas and code have been taken from plastik and KCMicons
//
// Example window decoration for KDE
// Copyright (c) 2003, 2004 David Johnson <david@usermode.org>
//
// Plastik KWindowSystem window decoration
// Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>
//
// KCMicons for KDE
// Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.
///////////////////////////////////////////////////////////////////////


#ifndef DEKORATORCLIENT_H
#define DEKORATORCLIENT_H

#include <QtGui/QAbstractButton>
#include <qbitmap.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qcursor.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3BoxLayout>
#include <QShowEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QEvent>
#include <Q3VBoxLayout>

#include <kdecoration.h>
#include <kdecorationfactory.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdeversion.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kiconeffect.h>
#include <qimageblitz.h>

#include "shadow.h"

class QSpacerItem;
class QPoint;

namespace DeKorator
{

class DeKoratorClient;

enum decoType {
    topLeftCorner = 0,
    leftButtons,
    leftTitle,
    midTitle,
    rightTitle,
    rightButtons,
    topRightCorner,
    topLeftFrame,
    midLeftFrame,
    buttomLeftFrame,
    leftButtomFrame,
    midButtomFrame,
    rightButtomFrame,
    topRightFrame,
    midRightFrame,
    buttomRightFrame,
    decoCount
};

enum buttonTypeAll {
    restore = 0,
    help,
    max,
    min,
    close,
    sticky,
    stickydown,
    above,
    abovedown,
    below,
    belowdown,
    shade,
    shadedown,
    menu,
    buttonTypeAllCount
};

enum ButtonType {
    ButtonHelp = 0,
    ButtonMax,
    ButtonMin,
    ButtonClose,
    ButtonMenu,
    ButtonSticky,
    ButtonAbove,
    ButtonBelow,
    ButtonShade,
    ButtonTypeCount
};

enum pixType {
    normal = 0,
    actCol,
    inActCol,
    orig,
    pixTypeCount
};

enum buttonState {
    regular = 0,
    hover,
    press,
    buttonStateCount
};

// DeKoratorFactory /////////////////////////////////////////////////////////////

class DeKoratorFactory: public KDecorationFactory
{
public:
    DeKoratorFactory();
    virtual ~DeKoratorFactory();
    virtual KDecoration *createDecoration( KDecorationBridge *b );
    virtual bool reset( unsigned long changed );
    virtual bool supports( KDecorationDefines::Ability ) const { return true; }
    static void colorizePixmap( QPixmap *pix, QColor col, QString colorizeMethod );
    static bool initialized();
	QBitmap topLeftCornerBitmap_, topMidBitmap_, topRightCornerBitmap_, buttomLeftCornerBitmap_, buttomMidBitmap_, buttomRightCornerBitmap_;

private:
    bool readConfig();
    void loadPixmaps();
    void colorizeDecoPixmaps( bool isActive );
    void colorizeButtonsPixmaps( bool isActive );
    void initPixmaps();
    void chooseRightPixmaps();
    void determineSizes();
    void prepareDecoWithBgCol();

private:

    static bool colorizeActFrames_;
    static bool colorizeInActFrames_;
    static bool needInit_;
	static bool needReload_;
    static QString framesPath_;
    static QString buttonsPath_;
    static QString masksPath_;
public:
    static bool initialized_;
    static bool useCustomButtonsColors_;
    static bool customColorsActiveButtons_;
    static bool customColorsInActiveButtons_;
    static bool colorizeActButtons_;
    static bool colorizeInActButtons_;
    static QColor cusBtnCol_[ buttonTypeAllCount ];
};

inline bool DeKoratorFactory::initialized()
{
    return initialized_;
}












// DeKoratorButton //////////////////////////////////////////////////////////////

class DeKoratorButton : public QAbstractButton
{
    Q_OBJECT
public:
    DeKoratorButton( bool isLeft, int buttonWidth, int buttonHeight, DeKoratorClient *parent = 0, const char *name = 0,
                     const QString &tip = NULL, ButtonType type = ButtonHelp, buttonTypeAll btnType = restore );
    ~DeKoratorButton();

    void setPixmap( buttonTypeAll btnType );
    QSize sizeHint() const;
    Qt::MouseButton lastMousePress() const;
    void reset();

protected slots:
    void animate();

private:
    void enterEvent( QEvent *e );
    void leaveEvent( QEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void paintEvent( QPaintEvent *e );
    QImage* chooseRightHoverEffect( QImage *img, QString res );

private:
    bool isLeft_;
    int buttonWidth_;
    DeKoratorClient *client_;
    ButtonType type_;
    //QPixmap *decoPix_, *decoPixPress_, *decoPixHover_, *decoPixInAct_;
    Qt::MouseButton lastmouse_;
	int decoPixHeight_;
    bool hover_;
    QTimer *animTmr;
    uint animProgress;
    QPixmap *btnPixAct_, *btnPixInAct_;
    buttonTypeAll btnType_;
int s;
};

inline Qt::MouseButton DeKoratorButton::lastMousePress() const
{
    return lastmouse_;
}

inline void DeKoratorButton::reset()
{
    repaint();
}












// DeKoratorClient //////////////////////////////////////////////////////////////

class DeKoratorClient : public KDecoration
{
    Q_OBJECT
public:
    DeKoratorClient( KDecorationBridge *b, KDecorationFactory *f );
    virtual ~DeKoratorClient();

    virtual void init();

    virtual void activeChange();
    virtual void desktopChange();
    virtual void captionChange();
    virtual void iconChange();
    virtual void maximizeChange();
    virtual void shadeChange();

    virtual void borders( int &l, int &r, int &t, int &b ) const;
    virtual void resize( const QSize &size );
    virtual QSize minimumSize() const;
    virtual Position mousePosition( const QPoint &point ) const;

private:
    void addButtons( Q3BoxLayout* layout, const QString& buttons, bool isLeft );

    bool eventFilter( QObject *obj, QEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );
	void wheelEvent( QWheelEvent *e );
	void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent * );
    void showEvent( QShowEvent * );
    void doShape();
    void updateCaptionBuffer();


private slots:
    void maxButtonPressed();
    void menuButtonPressed();
    void aboveButtonPressed();
    void belowButtonPressed();
    void shadeButtonPressed();
    void keepAboveChange( bool );
    void keepBelowChange( bool );
    void menuButtonReleased();

private:
    DeKoratorButton *button[ ButtonTypeCount ];
    Q3VBoxLayout *mainLayout_;
    Q3HBoxLayout *titleLayout_ ;
    Q3HBoxLayout *midLayout_;
    QSpacerItem *leftTitleBarSpacer_, *titleBarSpacer_, *rightTitleBarSpacer_, *leftSpacer_, *rightSpacer_, *bottomSpacer_;
    bool closing_;
    QPixmap *captionBufferPix_;
    bool captionBufferDirty_;
    QImage activeShadowImg_, inActiveShadowImg_;
	QRegion mask_;
	bool sizeChanged;
	QSize oldSize_;
public:
    DeKoratorFactory *decoFactory_;
};


} // namespace DeKorator

#endif // DEKORATORCLIENT_H
