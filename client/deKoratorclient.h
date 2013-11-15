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

#include <QAbstractButton>
#include <QBitmap>
#include <QImage>

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

enum ImageType {
    ImageOriginal,
    ImageRecolored,
    ImageTypeCount
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
    Q_OBJECT
public:
    DeKoratorFactory();
    virtual ~DeKoratorFactory();
    virtual KDecoration *createDecoration( KDecorationBridge *b ) override;
    virtual bool supports( KDecorationDefines::Ability ) const override;
    static QImage colorizedImage( const QImage &image, QColor color, QString colorizeMethod );
    QBitmap topLeftCornerBitmap_, topMidBitmap_, topRightCornerBitmap_, bottomLeftCornerBitmap_, bottomMidBitmap_, bottomRightCornerBitmap_;

public Q_SLOTS:
    void reset();

private:
    bool readConfig();
    void loadPixmaps();
    void colorizeDecoPixmaps( bool isActive );
    void colorizeButtonsPixmaps( bool isActive );
    void chooseRightPixmaps();
    void determineSizes();
    void prepareDecoWithBgCol();

private:

    bool colorizeActFrames_;
    bool colorizeInActFrames_;
    bool needInit_;
    bool needReload_;
    QString framesPath_;
    QString buttonsPath_;
    QString masksPath_;
public:
    bool useCustomButtonsColors_;
    bool customColorsActiveButtons_;
    bool customColorsInActiveButtons_;
    bool colorizeActButtons_;
    bool colorizeInActButtons_;
    QColor cusBtnCol_[ buttonTypeAllCount ];
    QImage decoImage[ decoCount ][ WindowActivationStateCount ][ ImageTypeCount ];
    QImage buttonImage[ buttonTypeAllCount ][ buttonStateCount ][ WindowActivationStateCount ][ ImageTypeCount ];
};












// DeKoratorButton //////////////////////////////////////////////////////////////

class DeKoratorButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit DeKoratorButton( DeKoratorClient *parent = 0, const char *name = 0,
                     const QString &tip = NULL, ButtonType type = ButtonHelp, buttonTypeAll btnType = restore );
    ~DeKoratorButton();

    void setPixmap( buttonTypeAll btnType );
    QSize sizeHint() const;
    Qt::MouseButton lastMousePress() const;
    void reset();

protected Q_SLOTS:
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












// DeKoratorClient //////////////////////////////////////////////////////////////

class DeKoratorClient : public KDecoration
{
    Q_OBJECT
public:
    DeKoratorClient( KDecorationBridge *b, KDecorationFactory *f );
    virtual ~DeKoratorClient();

    virtual void init() override;

    virtual void borders( int &l, int &r, int &t, int &b ) const override;
    virtual void resize( const QSize &size ) override;
    virtual QSize minimumSize() const override;
    virtual Position mousePosition( const QPoint &point ) const override;

private:
    void addButtons( QBoxLayout* layout, const QList<DecorationButton>& buttons );

    bool eventFilter( QObject *obj, QEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );
    void wheelEvent( QWheelEvent *e );
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent * );
    void showEvent( QShowEvent * );
    void doShape();
    void updateCaptionBuffer();


private Q_SLOTS:
    void maxButtonPressed();
    void menuButtonPressed();
    void aboveButtonPressed();
    void belowButtonPressed();
    void shadeButtonPressed();
    void keepAboveChange( bool );
    void keepBelowChange( bool );
    void menuButtonReleased();

    void activeChange();
    void desktopChange();
    void captionChange();
    void iconChange();
    void maximizeChange();
    void shadeChange();

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
};


} // namespace DeKorator

#endif // DEKORATORCLIENT_H
