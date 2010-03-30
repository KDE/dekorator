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

#include <kdecorationfactory.h>
#include <kdecoration.h>

#include <QtGui/QAbstractButton>
#include <QtGui/QBitmap>
#include <QtGui/QImage>

class QBoxLayout;
class QHBoxLayout;
class QSpacerItem;
class QVBoxLayout;


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
    bottomLeftFrame,
    leftBottomFrame,
    midBottomFrame,
    rightBottomFrame,
    topRightFrame,
    midRightFrame,
    bottomRightFrame,
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
    tabClose,
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
    ButtonTabClose,
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

enum WindowActivationState {
    WindowActive = 0,
    WindowInactive,
    WindowActivationStateCount
};

// DeKoratorFactory /////////////////////////////////////////////////////////////

class DeKoratorFactory: public KDecorationFactory
{
public:
    DeKoratorFactory();
    virtual ~DeKoratorFactory();
    virtual KDecoration *createDecoration( KDecorationBridge *b );
    virtual bool reset( unsigned long changed );
    virtual bool supports( KDecorationDefines::Ability ) const;
    static QImage colorizedImage( const QImage &image, QColor color, QString colorizeMethod );
    static bool initialized();
    QBitmap topLeftCornerBitmap_, topMidBitmap_, topRightCornerBitmap_, bottomLeftCornerBitmap_, bottomMidBitmap_, bottomRightCornerBitmap_;

private:
    bool readConfig();
    void loadPixmaps();
    void colorizeDecoPixmaps( bool isActive );
    void colorizeButtonsPixmaps( bool isActive );
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
    QImage decoImage[ decoCount ][ pixTypeCount ];
    QImage buttonImage[ buttonTypeAllCount ][ buttonStateCount ][ pixTypeCount ];
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
    DeKoratorButton( DeKoratorClient *parent = 0, const char *name = 0,
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
    QImage chooseRightHoverEffect( const QImage &img, QString res );

private:
    int buttonWidth_;
    DeKoratorClient *client_;
    ButtonType type_;
    Qt::MouseButton lastmouse_;
    int decoPixHeight_;
    bool hover_;
    QTimer *animTmr;
    uint animProgress;
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
    void addButtons( QBoxLayout* layout, const QString& buttons );

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
    QVBoxLayout *mainLayout_;
    QHBoxLayout *titleLayout_ ;
    QHBoxLayout *midLayout_;
    QSpacerItem *leftTitleBarSpacer_, *titleBarSpacer_, *rightTitleBarSpacer_, *leftSpacer_, *rightSpacer_, *bottomSpacer_;
    bool closing_;
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
