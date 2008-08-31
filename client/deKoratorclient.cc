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





#include "deKoratorclient.h"

#include <qsettings.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3BoxLayout>
#include <QLabel>
#include <QWheelEvent>
#include <QPixmap>
#include <QMouseEvent>
#include <QShowEvent>
#include <Q3HBoxLayout>
#include <QEvent>
#include <Q3VBoxLayout>
#include <QPaintEvent>
#include <kiconloader.h>
#include <QTime>
#include <qdebug.h>
using namespace DeKorator;

// global constants

static int MARGIN = 4;
//static int GRUBMARGIN = 5;

static QColor STYLEBGCOL;

// frames
static int TITLESIZE = 0;
static int LEFTFRAMESIZE = 0;
static int BUTTOMFRAMESIZE = 0;
static int RIGHTFRAMESIZE = 0;

// deco
static int TOPLEFTCORNERWIDTH;
static int TOPRIGHTCORNERWIDTH;
static int LEFTTITLEWIDTH;
static int RIGHTTITLEWIDTH;
static int TOPLEFTFRAMEHEIGHT;
static int BOTTOMLEFTFRAMEHEIGHT;
static int TOPRIGHTFRAMEHEIGHT;
static int BOTTOMRIGHTFRAMEHEIGHT;
static int LEFTBOTTOMFRAMEWIDTH;
static int RIGHTBOTTOMFRAMEWIDTH;

// buttons
//static const int DECOSIZE = 24;

static int BUTTONSHEIGHT = 0;

static int BTNHELPEWIDTH = 0;
static int BTNMAXWIDTH = 0;
static int BTNCLOSEWIDTH = 0;
static int BTNMINWIDTH = 0;
static int BTNSTICKYWIDTH = 0;
static int BTNABOVEWIDTH = 0;
static int BTNBELOWWIDTH = 0;
static int BTNSHADEWIDTH = 0;
static int BTNMENUWIDTH = 0;

static int BTNHELPEHEIGHT = 0;
static int BTNMAXHEIGHT = 0;
static int BTNCLOSEHEIGHT = 0;
static int BTNMINHEIGHT = 0;
static int BTNSTICKYHEIGHT = 0;
static int BTNABOVEHEIGHT = 0;
static int BTNBELOWHEIGHT = 0;
static int BTNSHADEHEIGHT = 0;
static int BTNMENUHEIGHT = 0;

// masks
static int TOPLEFTMASKWIDTH = 0;
static int TOPMIDMASKWIDTH = 0;
static int TOPRIGHTMASKWIDTH = 0;
static int BOTTOMLEFTMASKWIDTH = 0;
static int BOTTOMMIDMASKWIDTH = 0;
static int BOTTOMRIGHTMASKWIDTH = 0;

static int BOTTOMLEFTMASKHEIGHT = 0;
static int BOTTOMMIDMASKHEIGHT = 0;
static int BOTTOMRIGHTMASKHEIGHT = 0;

// config
// misc
static Qt::Alignment TITLEALIGN = Qt::AlignHCenter;
static bool USEMENUEIMAGE = FALSE;
static bool IGNOREAPPICNCOL = FALSE;
static bool DBLCLKCLOSE = FALSE;
static bool SHOWBTMBORDER = FALSE;
static bool USESHDTEXT = FALSE;
static int ACTIVESHDTEXTX = FALSE;
static int ACTIVESHDTEXTY = FALSE;
static QColor ACTIVESHADECOLOR = QColor( 150, 150, 150 );
static int INACTIVESHDTEXTX = FALSE;
static int INACTIVESHDTEXTY = FALSE;
static QColor INACTIVESHADECOLOR = QColor( 150, 150, 150 );
static int BTNSHIFTX = 0;
static int BTNSHIFTY = 0;


// colors

static bool USEANIMATION = TRUE;
static QString ANIMATIONTYPE = "Intensity";

static int STEPS = 5;
static int INTERVAL = 5;
static int KEEPANIMATING = FALSE;

static QString BUTTONHOVERTYPE = "To Gray";
static float EFFECTAMOUNT = 5;
static QColor ACTIVEHIGHLIGHTCOLOR = QColor( 150, 150, 150 );
static QColor INACTIVEHIGHLIGHTCOLOR = QColor( 150, 150, 150 );
static QString DECOCOLORIZE = "Liquid Method";
static QString BUTTONSCOLORIZE = "Liquid Method";
//static QString BUTTONSHOVERCOLORIZE = "Liquid Method";
static const uint TIMERINTERVAL = 30; // msec
static const uint ANIMATIONSTEPS = 100;

//

// paths
static bool USEMASKS = FALSE;

// pix arrays
static QPixmap *DECOARR[ decoCount ][ pixTypeCount ];
static QPixmap *DECOPIXACTARR[ decoCount ];
static QPixmap *DECOPIXINACTARR[ decoCount ];

static QPixmap *BUTTONSARR[ buttonTypeAllCount ][ buttonStateCount ][ pixTypeCount ];
static QPixmap *BUTTONPIXACTARR[ buttonTypeAllCount ][ buttonStateCount ];
static QPixmap *BUTTONPIXINACTARR[ buttonTypeAllCount ][ buttonStateCount ];

//////////////////////////////////////////////////////////////////////////////
// DeKoratorFactory Class                                                     //
//////////////////////////////////////////////////////////////////////////////

bool DeKoratorFactory::initialized_ = FALSE;
bool DeKoratorFactory::colorizeActFrames_ = FALSE;
bool DeKoratorFactory::colorizeActButtons_ = FALSE;
bool DeKoratorFactory::colorizeInActFrames_ = FALSE;
bool DeKoratorFactory::colorizeInActButtons_ = FALSE;

bool DeKoratorFactory::useCustomButtonsColors_ = FALSE;
bool DeKoratorFactory::customColorsActiveButtons_ = FALSE;
bool DeKoratorFactory::customColorsInActiveButtons_ = FALSE;
QColor DeKoratorFactory::cusBtnCol_[ buttonTypeAllCount ];

bool DeKoratorFactory::needInit_ = FALSE;
bool DeKoratorFactory::needReload_ = FALSE;
QString DeKoratorFactory::framesPath_ = "";
QString DeKoratorFactory::buttonsPath_ = "";
QString DeKoratorFactory::masksPath_ = "";


extern "C" KDE_EXPORT KDecorationFactory* create_factory()
{
    return new DeKorator::DeKoratorFactory();
}

//////////////////////////////////////////////////////////////////////////////
// DeKoratorFactory()
// ----------------
// Constructor
DeKoratorFactory::DeKoratorFactory()
{
    readConfig();
    initialized_ = TRUE;

    initPixmaps();

    loadPixmaps();

    bool isActive = false;
    if ( colorizeInActFrames_ )
        colorizeDecoPixmaps( isActive );
    if ( colorizeInActButtons_ )
        colorizeButtonsPixmaps( isActive );

    isActive = true;
    if ( colorizeActFrames_ )
        colorizeDecoPixmaps( isActive );
    if ( colorizeActButtons_ )
        colorizeButtonsPixmaps( isActive );

    prepareDecoWithBgCol();

    chooseRightPixmaps();
}

//////////////////////////////////////////////////////////////////////////////
// ~DeKoratorFactory()
// -----------------
// Destructor
DeKoratorFactory::~DeKoratorFactory()
{
    initialized_ = FALSE;
    int i, j, k;

    for ( i = 0; i < decoCount; i++ )
    {
        for ( j = 0; j < pixTypeCount; j++ )
        {
            if ( DECOARR[ i ][ j ] )
                delete DECOARR[ i ][ j ];
        }
    }

    for ( i = 0; i < buttonTypeAllCount; i++ )
    {
        for ( j = 0; j < buttonStateCount; j++ )
        {
            for ( k = 0; k < pixTypeCount; k++ )
            {
                if ( BUTTONSARR[ i ][ j ][ k ] )
                    delete BUTTONSARR[ i ][ j ][ k ];
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// createDecoration()
// -----------------
// Create the decoration
KDecoration* DeKoratorFactory::createDecoration( KDecorationBridge* b )
{
    return new DeKoratorClient( b, this );
}

//////////////////////////////////////////////////////////////////////////////
// reset()
// -------
// Reset the handler. Returns TRUE if decorations need to be remade, FALSE if
// only a repaint is necessary
bool DeKoratorFactory::reset( unsigned long changed )
{
    //    captionBufferDirty_ = true;
    // read in the configuration
    initialized_ = FALSE;
    bool confchange = readConfig();
    initialized_ = TRUE;


    if ( confchange || ( changed & ( SettingDecoration | SettingButtons | SettingBorder | SettingColors ) ) )
    {
        //qWarning( "aaaaaaaaaaaaa" );
        if ( DeKoratorFactory::needReload_ )
        {
            loadPixmaps();

            //             bool isActive = false;
            //             if ( colorizeInActFrames_ )
            //                 colorizeDecoPixmaps( isActive );
            //             if ( colorizeInActButtons_ )
            //                 colorizeButtonsPixmaps( isActive );
            //
            //             isActive = true;
            //             if ( colorizeActFrames_ )
            //                 colorizeDecoPixmaps( isActive );
            //             if ( colorizeActButtons_ )
            //                 colorizeButtonsPixmaps( isActive );
            //
            //             prepareDecoWithBgCol();
            //
            //             chooseRightPixmaps();
            //
            //             return TRUE;
        }

        //if ( DeKoratorFactory::needInit_ )
        {
            bool isActive = false;
            if ( colorizeInActFrames_ )
                colorizeDecoPixmaps( isActive );
            if ( colorizeInActButtons_ )
                colorizeButtonsPixmaps( isActive );

            isActive = true;
            if ( colorizeActFrames_ )
                colorizeDecoPixmaps( isActive );
            if ( colorizeActButtons_ )
                colorizeButtonsPixmaps( isActive );

            prepareDecoWithBgCol();

            chooseRightPixmaps();

            //return TRUE;
        }
        return TRUE;
    }
    else
    {
        resetDecorations( changed );
        return FALSE;
    }
}

//////////////////////////////////////////////////////////////////////////////
// readConfig()
// ------------
// Read in the configuration file
bool DeKoratorFactory::readConfig()
{

    // create a config object
    KConfig conf( "kwindeKoratorrc" );
    QColor col = QColor( 150, 150, 150 );

    // grab settings

    // misc settings
    KConfigGroup config = KConfigGroup(&conf, "MISC" );

    Qt::Alignment oldalign = TITLEALIGN;
    QString value = config.readEntry( "TitleAlignment", "AlignHCenter" );
    if ( value == "AlignLeft" )
        TITLEALIGN = Qt::AlignLeft;
    else if ( value == "AlignHCenter" )
        TITLEALIGN = Qt::AlignHCenter;
    else if ( value == "AlignRight" )
        TITLEALIGN = Qt::AlignRight;

    bool oldUseMenuImage = USEMENUEIMAGE;
    USEMENUEIMAGE = config.readEntry( "UseMenuImage", false );

    bool oldIgnoreAppIcnCol = IGNOREAPPICNCOL;
    IGNOREAPPICNCOL = config.readEntry( "IgnoreAppIconCol", false );

//    bool oldDblClkClose = DBLCLKCLOSE;
    DBLCLKCLOSE = config.readEntry( "DblClkClose", false );

    bool oldShowBtmBorder = SHOWBTMBORDER;
    SHOWBTMBORDER = config.readEntry( "ShowBtmBorder", false );

    bool oldUseShdtext = USESHDTEXT;
    USESHDTEXT = config.readEntry( "UseShdtext", false );

    int oldActiveShdtextX = ACTIVESHDTEXTX;
    ACTIVESHDTEXTX = config.readEntry( "ActiveShdtextX", 0 );

    int oldActiveShdtextY = ACTIVESHDTEXTY;
    ACTIVESHDTEXTY = config.readEntry( "ActiveShdtextY", 0 );

    QColor oldActiveShadeColor = ACTIVESHADECOLOR;
    ACTIVESHADECOLOR = config.readEntry( "ActiveShadowColor", col );

    int oldInActiveShdtextX = INACTIVESHDTEXTX;
    INACTIVESHDTEXTX = config.readEntry( "InActiveShdtextX", 0 );

    int oldInActiveShdtextY = INACTIVESHDTEXTY;
    INACTIVESHDTEXTY = config.readEntry( "InActiveShdtextY", 0 );

    QColor oldInActiveShadeColor = INACTIVESHADECOLOR;
    INACTIVESHADECOLOR = config.readEntry( "InActiveShadowColor", col );


    //int oldBtnShiftX = BTNSHIFTX;
    BTNSHIFTX = config.readEntry( "BtnShiftX", 0 );

    //int oldBtnShiftY = BTNSHIFTY;
    BTNSHIFTY = config.readEntry( "BtnShiftY", 0 );


    // color's settings
    config = KConfigGroup(&conf, "COLORS" );

    bool oldColorizeActFrames = DeKoratorFactory::colorizeActFrames_;
    DeKoratorFactory::colorizeActFrames_ = config.readEntry( "ColorizeActFrames", false );

    bool oldColorizeActButtons = DeKoratorFactory::colorizeActButtons_;
    DeKoratorFactory::colorizeActButtons_ = config.readEntry( "ColorizeActButtons", false );

    bool oldColorizeInActFrames = DeKoratorFactory::colorizeInActFrames_;
    DeKoratorFactory::colorizeInActFrames_ = config.readEntry( "ColorizeInActFrames", false );

    bool oldColorizeInActButtons = DeKoratorFactory::colorizeInActButtons_;
    DeKoratorFactory::colorizeInActButtons_ = config.readEntry( "ColorizeInActButtons", false );

    USEANIMATION = config.readEntry( "UseAnimation", false );

    ANIMATIONTYPE = config.readEntry( "AnimationType", "Liquid Method" );

    STEPS = config.readEntry( "AnimSteps", 5 );
    INTERVAL = config.readEntry( "AnimInterval", 30 );
    KEEPANIMATING = config.readEntry( "KeepAnimating", false );

    BUTTONHOVERTYPE = config.readEntry( "ButtonsHoverType", "To Gray" );

    EFFECTAMOUNT = ( ( float ) config.readEntry( "EffectAmount", 5 ) / 10 );

    ACTIVEHIGHLIGHTCOLOR = config.readEntry( "ActiveHighlightColor", col );

    INACTIVEHIGHLIGHTCOLOR = config.readEntry( "InActiveHighlightColor", col );

    QString oldDecoColorize = DECOCOLORIZE;
    DECOCOLORIZE = config.readEntry( "DecoColorize", "Liquid Method" );

    QString oldButtonsColorize = BUTTONSCOLORIZE;
    BUTTONSCOLORIZE = config.readEntry( "ButtonsColorize", "Liquid Method" );
    /*
        QString oldButtonsHoverColorize = BUTTONSHOVERCOLORIZE;
        BUTTONSHOVERCOLORIZE = config.readEntry( "ButtonsHoverColorize", "Liquid Method" );*/

    // buttons settings
    config = KConfigGroup(&conf, "BUTTONS" );

    bool oldUseCustomButtonsColors = DeKoratorFactory::useCustomButtonsColors_;
    DeKoratorFactory::useCustomButtonsColors_ = config.readEntry( "UseCustomButtonsColors", false );

    bool oldCustomColorsActiveButtons = DeKoratorFactory::customColorsActiveButtons_;
    DeKoratorFactory::customColorsActiveButtons_ = config.readEntry( "customColorsActiveButtons", false );

    bool oldCustomColorsInActiveButtons = DeKoratorFactory::customColorsInActiveButtons_;
    DeKoratorFactory::customColorsInActiveButtons_ = config.readEntry( "customColorsInActiveButtons", false );

    // load buttons colors
    QColor colArr[ buttonTypeAllCount ];
    for ( int i = 0 ; i < buttonTypeAllCount ; i++ )
    {
        colArr[ i ] = DeKoratorFactory::cusBtnCol_[ i ];
    }
    DeKoratorFactory::cusBtnCol_[ restore ] = config.readEntry( "RestoreButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ help ] = config.readEntry( "HelpButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ max ] = config.readEntry( "MaxButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ min ] = config.readEntry( "MinButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ close ] = config.readEntry( "CloseButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ sticky ] = config.readEntry( "StickyButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ stickydown ] = config.readEntry( "StickyDownButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ above ] = config.readEntry( "AboveButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ abovedown ] = config.readEntry( "AboveDownButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ below ] = config.readEntry( "BelowButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ belowdown ] = config.readEntry( "BelowDownButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ shade ] = config.readEntry( "ShadeButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ shadedown ] = config.readEntry( "ShadeDownButtonColor", col );
    DeKoratorFactory::cusBtnCol_[ menu ] = config.readEntry( "MenuButtonColor", col );

    bool cusColChanged = false;
    for ( int i = 0 ; i < buttonTypeAllCount ; i++ )
    {
        if ( colArr[ i ] != DeKoratorFactory::cusBtnCol_[ i ] )
        {
            cusColChanged = true;
            i = buttonTypeAllCount;
            qWarning( "true" );
        }

    }

    // path's settings
    config = KConfigGroup(&conf, "PATHS" );

    QString oldFramesPath = DeKoratorFactory::framesPath_;
    DeKoratorFactory::framesPath_ = config.readEntry( "FramesPath", "" );

    QString oldButtonsPath = DeKoratorFactory::buttonsPath_;
    DeKoratorFactory::buttonsPath_ = config.readEntry( "ButtonsPath", "" );

    bool oldUseMasks = USEMASKS;
    USEMASKS = config.readEntry( "UseMasks", false );

    QString oldMasksPath = DeKoratorFactory::masksPath_;
    DeKoratorFactory::masksPath_ = config.readEntry( "MasksPath", "" );


    //style backgrond
    QColor oldStyleBgCol = STYLEBGCOL;

    QString colStr = QSettings().readEntry( "/Qt/Palette/active", "aaaa" );
    colStr = colStr.section( "#", 3, 3 ) ;
    colStr.insert( 0, '#' );
    colStr.truncate( 7 );

    QColor c;
    c.setNamedColor( colStr );
    STYLEBGCOL = c;



    if ( oldalign == TITLEALIGN &&
            oldUseMenuImage == USEMENUEIMAGE &&
            oldIgnoreAppIcnCol == IGNOREAPPICNCOL &&
            oldShowBtmBorder == SHOWBTMBORDER &&
            oldUseShdtext == USESHDTEXT &&
            oldActiveShdtextX == ACTIVESHDTEXTX &&
            oldActiveShdtextY == ACTIVESHDTEXTY &&
            oldActiveShadeColor == ACTIVESHADECOLOR &&
            oldInActiveShdtextX == INACTIVESHDTEXTX &&
            oldInActiveShdtextY == INACTIVESHDTEXTY &&
            oldInActiveShadeColor == INACTIVESHADECOLOR &&
            oldColorizeActFrames == DeKoratorFactory::colorizeActFrames_ &&
            oldColorizeActButtons == DeKoratorFactory::colorizeActButtons_ &&
            oldColorizeInActFrames == DeKoratorFactory::colorizeInActFrames_ &&
            oldColorizeInActButtons == DeKoratorFactory::colorizeInActButtons_ &&
            oldDecoColorize == DECOCOLORIZE &&
            oldButtonsColorize == BUTTONSCOLORIZE &&
            oldUseCustomButtonsColors == DeKoratorFactory::useCustomButtonsColors_ &&
            oldCustomColorsActiveButtons == DeKoratorFactory::customColorsActiveButtons_ &&
            oldCustomColorsInActiveButtons == DeKoratorFactory::customColorsInActiveButtons_ &&
            cusColChanged == false &&
            oldFramesPath == DeKoratorFactory::framesPath_ &&
            oldButtonsPath == DeKoratorFactory::buttonsPath_ &&
            oldMasksPath == DeKoratorFactory::masksPath_ &&
            oldUseMasks == USEMASKS &&
            oldStyleBgCol == STYLEBGCOL
       )
        return FALSE;
    else
    {
        //         if ( oldColorizeActFrames != DeKoratorFactory::colorizeActFrames_ ||
        //                 oldColorizeActButtons != DeKoratorFactory::colorizeActButtons_ ||
        //                 oldColorizeInActFrames != DeKoratorFactory::colorizeInActFrames_ ||
        //                 oldColorizeInActButtons != DeKoratorFactory::colorizeInActButtons_ ||
        //                 oldDecoColorize != DECOCOLORIZE ||
        //                 oldButtonsColorize != BUTTONSCOLORIZE ||
        //                 cusColChanged == true ||
        //                 oldStyleBgCol != STYLEBGCOL
        //            )
        //             DeKoratorFactory::needInit_ = TRUE;
        //         else
        //             DeKoratorFactory::needInit_ = FALSE;

        if ( oldFramesPath != DeKoratorFactory::framesPath_ ||
                oldButtonsPath != DeKoratorFactory::buttonsPath_ ||
                oldMasksPath != DeKoratorFactory::masksPath_
           )
            DeKoratorFactory::needReload_ = TRUE;
        else
            DeKoratorFactory::needReload_ = FALSE;

        return TRUE;
    }
}

//////////////////////////////////////////////////////////////////////////////
// loadPixmaps()
// ------------
// Read in the configuration file
void DeKoratorFactory::loadPixmaps()
{
    QString decoPixDir = DeKoratorFactory::framesPath_;
    QString btnPixDir = DeKoratorFactory::buttonsPath_;
    QString masksPixDir = DeKoratorFactory::masksPath_;
    qDebug() << "Frames path: " << decoPixDir;

    // deco
    // top bar from left to right
    DECOARR[ topLeftCorner ][ orig ] ->load( decoPixDir + "/topLeftCornerBg.png" );
    DECOARR[ leftButtons ][ orig ] ->load( decoPixDir + "/leftButtonsBg.png" );
    DECOARR[ leftTitle ][ orig ] ->load( decoPixDir + "/leftTitleBg.png" );
    DECOARR[ midTitle ][ orig ] ->load( decoPixDir + "/midTitleBg.png" );
    qDebug() << "loaded midTitleBg.png" << DECOARR[ midTitle ][ orig ]->width() << DECOARR[ midTitle ][ orig ]->height();
    DECOARR[ rightTitle ][ orig ] ->load( decoPixDir + "/rightTitleBg.png" );
    DECOARR[ rightButtons ][ orig ] ->load( decoPixDir + "/rightButtonsBg.png" );
    DECOARR[ topRightCorner ][ orig ] ->load( decoPixDir + "/topRightCornerBg.png" );
    // left frame from top to buttom
    DECOARR[ topLeftFrame ][ orig ] ->load( decoPixDir + "/topLeftFrameBg.png" );
    DECOARR[ midLeftFrame ][ orig ] ->load( decoPixDir + "/midLeftFrameBg.png" );
    DECOARR[ buttomLeftFrame ][ orig ] ->load( decoPixDir + "/bottomLeftFrameBg.png" );
    // buttom frame from left to right
    DECOARR[ leftButtomFrame ][ orig ] ->load( decoPixDir + "/leftBottomFrameBg.png" );
    DECOARR[ midButtomFrame ][ orig ] ->load( decoPixDir + "/midBottomFrameBg.png" );
    DECOARR[ rightButtomFrame ][ orig ] ->load( decoPixDir + "/rightBottomFrameBg.png" );
    // right frame from top to buttom
    DECOARR[ topRightFrame ][ orig ] ->load( decoPixDir + "/topRightFrameBg.png" );
    DECOARR[ midRightFrame ][ orig ] ->load( decoPixDir + "/midRightFrameBg.png" );
    DECOARR[ buttomRightFrame ][ orig ] ->load( decoPixDir + "/bottomRightFrameBg.png" );


    // buttons
    BUTTONSARR[ restore ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonRestore.png" );
    BUTTONSARR[ restore ][ press ][ normal ] ->load( btnPixDir + "/press/buttonRestorePress.png" );
    BUTTONSARR[ restore ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonRestoreHover.png" );

    BUTTONSARR[ help ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonHelp.png" );
    BUTTONSARR[ help ][ press ][ normal ] ->load( btnPixDir + "/press/buttonHelpPress.png" );
    BUTTONSARR[ help ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonHelpHover.png" );

    BUTTONSARR[ max ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonMax.png" );
    BUTTONSARR[ max ][ press ][ normal ] ->load( btnPixDir + "/press/buttonMaxPress.png" );
    BUTTONSARR[ max ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonMaxHover.png" );

    BUTTONSARR[ min ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonMin.png" );
    BUTTONSARR[ min ][ press ][ normal ] ->load( btnPixDir + "/press/buttonMinPress.png" );
    BUTTONSARR[ min ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonMinHover.png" );

    BUTTONSARR[ close ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonClose.png" );
    BUTTONSARR[ close ][ press ][ normal ] ->load( btnPixDir + "/press/buttonClosePress.png" );
    BUTTONSARR[ close ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonCloseHover.png" );

    BUTTONSARR[ sticky ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonSticky.png" );
    BUTTONSARR[ sticky ][ press ][ normal ] ->load( btnPixDir + "/press/buttonStickyDownPress.png" );
    BUTTONSARR[ sticky ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonStickyHover.png" );

    BUTTONSARR[ stickydown ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonStickyDown.png" );
    BUTTONSARR[ stickydown ][ press ][ normal ] ->load( btnPixDir + "/press/buttonStickyDownPress.png" );
    BUTTONSARR[ stickydown ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonStickyDownHover.png" );

    BUTTONSARR[ above ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonAbove.png" );
    BUTTONSARR[ above ][ press ][ normal ] ->load( btnPixDir + "/press/buttonAbovePress.png" );
    BUTTONSARR[ above ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonAboveHover.png" );

    BUTTONSARR[ abovedown ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonAboveDown.png" );
    BUTTONSARR[ abovedown ][ press ][ normal ] ->load( btnPixDir + "/press/buttonAboveDownPress.png" );
    BUTTONSARR[ abovedown ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonAboveDownHover.png" );

    BUTTONSARR[ below ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonBelow.png" );
    BUTTONSARR[ below ][ press ][ normal ] ->load( btnPixDir + "/press/buttonBelowPress.png" );
    BUTTONSARR[ below ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonBelowHover.png" );

    BUTTONSARR[ belowdown ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonBelowDown.png" );
    BUTTONSARR[ belowdown ][ press ][ normal ] ->load( btnPixDir + "/press/buttonBelowDownPress.png" );
    BUTTONSARR[ belowdown ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonBelowDownHover.png" );

    BUTTONSARR[ shade ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonShade.png" );
    BUTTONSARR[ shade ][ press ][ normal ] ->load( btnPixDir + "/press/buttonShadePress.png" );
    BUTTONSARR[ shade ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonShadeHover.png" );

    BUTTONSARR[ shadedown ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonShadeDown.png" );
    BUTTONSARR[ shadedown ][ press ][ normal ] ->load( btnPixDir + "/press/buttonShadeDownPress.png" );
    BUTTONSARR[ shadedown ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonShadeDownHover.png" );

    BUTTONSARR[ menu ][ regular ][ normal ] ->load( btnPixDir + "/normal/buttonMenu.png" );
    BUTTONSARR[ menu ][ press ][ normal ] ->load( btnPixDir + "/press/buttonMenuPress.png" );
    BUTTONSARR[ menu ][ hover ][ normal ] ->load( btnPixDir + "/hover/buttonMenuHover.png" );


    //masks
    topLeftCornerBitmap_ = QBitmap( masksPixDir + "/topLeftCornerBitmap.png" );
    topMidBitmap_ = QBitmap( masksPixDir + "/topMidBitmap.png" );
    topRightCornerBitmap_ = QBitmap( masksPixDir + "/topRightCornerBitmap.png" );
    buttomLeftCornerBitmap_ = QBitmap( masksPixDir + "/buttomLeftCornerBitmap.png" );
    buttomMidBitmap_ = QBitmap( masksPixDir + "/buttomMidBitmap.png" );
    buttomRightCornerBitmap_ = QBitmap( masksPixDir + "/buttomRightCornerBitmap.png" );

    determineSizes();

    //prepareDecoWithBgCol();

}

//////////////////////////////////////////////////////////////////////////////
// determineSizes()
// ------------
//
void DeKoratorFactory::determineSizes()
{
    //MARGIN = midButtomFrameBg_.height();

    // frames
    TITLESIZE = DECOARR[ midTitle ][ orig ] ->height();
    LEFTFRAMESIZE = DECOARR[ midLeftFrame ][ orig ] ->width();
    BUTTOMFRAMESIZE = DECOARR[ midButtomFrame ][ orig ] ->height();
    RIGHTFRAMESIZE = DECOARR[ midRightFrame ][ orig ] ->width();

    TOPLEFTCORNERWIDTH = DECOARR[ topLeftCorner ][ orig ] ->width();
    TOPRIGHTCORNERWIDTH = DECOARR[ topRightCorner ][ orig ] ->width();
    LEFTTITLEWIDTH = DECOARR[ leftTitle ][ orig ] ->width();
    RIGHTTITLEWIDTH = DECOARR[ rightTitle ][ orig ] ->width();
    TOPLEFTFRAMEHEIGHT = DECOARR[ topLeftFrame ][ orig ] ->height();
    BOTTOMLEFTFRAMEHEIGHT = DECOARR[ buttomLeftFrame ][ orig ] ->height();
    TOPRIGHTFRAMEHEIGHT = DECOARR[ topRightFrame ][ orig ] ->height();
    BOTTOMRIGHTFRAMEHEIGHT = DECOARR[ buttomRightFrame ][ orig ] ->height();
    LEFTBOTTOMFRAMEWIDTH = DECOARR[ leftButtomFrame ][ orig ] ->width();
    RIGHTBOTTOMFRAMEWIDTH = DECOARR[ rightButtomFrame ][ orig ] ->width();


    // buttons
    BUTTONSHEIGHT = TITLESIZE;

    BTNHELPEWIDTH = BUTTONSARR[ help ][ regular ][ normal ] ->width();
    BTNMAXWIDTH = BUTTONSARR[ max ][ regular ][ normal ] ->width();
    BTNCLOSEWIDTH = BUTTONSARR[ close ][ regular ][ normal ] ->width();
    BTNMINWIDTH = BUTTONSARR[ min ][ regular ][ normal ] ->width();
    BTNSTICKYWIDTH = BUTTONSARR[ sticky ][ regular ][ normal ] ->width();
    BTNABOVEWIDTH = BUTTONSARR[ above ][ regular ][ normal ] ->width();
    BTNBELOWWIDTH = BUTTONSARR[ below ][ regular ][ normal ] ->width();
    BTNSHADEWIDTH = BUTTONSARR[ shade ][ regular ][ normal ] ->width();
    BTNMENUWIDTH = BUTTONSARR[ menu ][ regular ][ normal ] ->width();

    BTNHELPEHEIGHT = BUTTONSARR[ help ][ regular ][ normal ] ->height();
    BTNMAXHEIGHT = BUTTONSARR[ max ][ regular ][ normal ] ->height();
    BTNCLOSEHEIGHT = BUTTONSARR[ close ][ regular ][ normal ] ->height();
    BTNMINHEIGHT = BUTTONSARR[ min ][ regular ][ normal ] ->height();
    BTNSTICKYHEIGHT = BUTTONSARR[ sticky ][ regular ][ normal ] ->height();
    BTNABOVEHEIGHT = BUTTONSARR[ above ][ regular ][ normal ] ->height();
    BTNBELOWHEIGHT = BUTTONSARR[ below ][ regular ][ normal ] ->height();
    BTNSHADEHEIGHT = BUTTONSARR[ shade ][ regular ][ normal ] ->height();
    BTNMENUHEIGHT = BUTTONSARR[ menu ][ regular ][ normal ] ->height();

    // masks
    TOPLEFTMASKWIDTH = topLeftCornerBitmap_.width();
    TOPMIDMASKWIDTH = topMidBitmap_.width();
    TOPRIGHTMASKWIDTH = topRightCornerBitmap_.width();
    BOTTOMLEFTMASKWIDTH = buttomLeftCornerBitmap_.width();
    BOTTOMMIDMASKWIDTH = buttomMidBitmap_.width();
    BOTTOMRIGHTMASKWIDTH = buttomRightCornerBitmap_.width();

    BOTTOMLEFTMASKHEIGHT = buttomLeftCornerBitmap_.height();
    BOTTOMMIDMASKHEIGHT = buttomMidBitmap_.height();
    BOTTOMRIGHTMASKHEIGHT = buttomRightCornerBitmap_.height();

}


//////////////////////////////////////////////////////////////////////////////
// colorizeDecoPixmaps()
// ------------
//
void DeKoratorFactory::colorizeDecoPixmaps( bool isActive )
{
    int i;
    QColor col = options() ->palette( KDecoration::ColorTitleBar, isActive ).background();

    if ( isActive )
    {
        for ( i = 0; i < decoCount; i++ )
        {
            *( DECOARR[ i ][ actCol ] ) = *( DECOARR[ i ][ orig ] );
            colorizePixmap( DECOARR[ i ][ actCol ], col, DECOCOLORIZE );
        }
    }
    else
    {
        for ( i = 0; i < decoCount; i++ )
        {
            *( DECOARR[ i ][ inActCol ] ) = *( DECOARR[ i ][ orig ] );
            colorizePixmap( DECOARR[ i ][ inActCol ], col, DECOCOLORIZE );
        }

    }

    prepareDecoWithBgCol();
}

//////////////////////////////////////////////////////////////////////////////
// colorizeButtonsPixmaps()
// ------------
//
void DeKoratorFactory::colorizeButtonsPixmaps( bool isActive )
{
    int i, j;
    QColor col = options() ->palette( KDecoration::ColorButtonBg, isActive ).background();

    if ( isActive )
    {
        if ( useCustomButtonsColors_ && customColorsActiveButtons_ )
        {
            for ( i = 0; i < buttonTypeAllCount; i++ )
            {
                for ( j = 0; j < buttonStateCount; j++ )
                {
                    *( BUTTONSARR[ i ][ j ][ actCol ] ) = *( BUTTONSARR[ i ][ j ][ normal ] );
                    colorizePixmap( BUTTONSARR[ i ][ j ][ actCol ], cusBtnCol_[ i ], BUTTONSCOLORIZE );
                }
            }
        }
        else
        {
            for ( i = 0; i < buttonTypeAllCount; i++ )
            {
                for ( j = 0; j < buttonStateCount; j++ )
                {
                    *( BUTTONSARR[ i ][ j ][ actCol ] ) = *( BUTTONSARR[ i ][ j ][ normal ] );
                    colorizePixmap( BUTTONSARR[ i ][ j ][ actCol ], col, BUTTONSCOLORIZE );
                }
            }
        }
    }
    else
    {
        if ( ( useCustomButtonsColors_ && customColorsInActiveButtons_ ) )
        {
            for ( i = 0; i < buttonTypeAllCount; i++ )
            {
                for ( j = 0; j < buttonStateCount; j++ )
                {
                    *( BUTTONSARR[ i ][ j ][ inActCol ] ) = *( BUTTONSARR[ i ][ j ][ normal ] );
                    colorizePixmap( BUTTONSARR[ i ][ j ][ inActCol ], cusBtnCol_[ i ], BUTTONSCOLORIZE );
                }
            }
        }
        else
        {
            for ( i = 0; i < buttonTypeAllCount; i++ )
            {
                for ( j = 0; j < buttonStateCount; j++ )
                {
                    *( BUTTONSARR[ i ][ j ][ inActCol ] ) = *( BUTTONSARR[ i ][ j ][ normal ] );
                    colorizePixmap( BUTTONSARR[ i ][ j ][ inActCol ], col, BUTTONSCOLORIZE );
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// colorizePixmap()
// ------------
//
void DeKoratorFactory::colorizePixmap( QPixmap *pix, QColor c, QString colorizeMethod )
{
    QImage img;
    if ( colorizeMethod == "Liquid Method" )
    {
        img = pix->convertToImage();
        //KIconEffect::toGray( img, 1.0 );

        if ( img.depth() != 32 )
            img = img.convertDepth( 32 );
        QImage *dest = new QImage( img.width(), img.height(), 32 );
        dest->setAlphaBuffer( TRUE );
        unsigned int *data = ( unsigned int * ) img.bits();
        unsigned int *destData = ( unsigned int* ) dest->bits();
        int total = img.width() * img.height();
        int current;
        int delta;
        int destR, destG, destB, alpha;
        int srcR = c.red() + 100;
        int srcG = c.green() + 100;
        int srcB = c.blue() + 100;

        for ( current = 0; current < total; ++current )
        {
            alpha = qAlpha( data[ current ] );

            //         qWarning( "--------------------------" );
            //         qWarning( "red - %d", qRed( data[ current ] ) );
            //         qWarning( "green - %d", qRed( data[ current ] ) );
            //         qWarning( "blue - %d", qRed( data[ current ] ) );

            delta = 255 - qGray( data[ current ] );

            destR = srcR - delta;
            destG = srcG - delta;
            destB = srcB - delta;

            if ( destR < 0 )
                destR = 0;
            if ( destG < 0 )
                destG = 0;
            if ( destB < 0 )
                destB = 0;
            if ( destR > 255 )
                destR = 255;
            if ( destG > 255 )
                destG = 255;
            if ( destB > 255 )
                destB = 255;
            destData[ current ] = qRgba( destR, destG, destB, alpha );
        }
        pix->convertFromImage( *dest );
    }
    else if ( colorizeMethod == "Kde Method" )
    {
        img = pix->convertToImage();
        KIconEffect::colorize( img, c, 1.0 );
        pix->convertFromImage( img, 0 );
    }
    else if ( colorizeMethod == "Hue Adgustment" )
    {
        img = pix->convertToImage();
        //KIconEffect::toGray( img, 1.0 );

        if ( img.depth() != 32 )
            img = img.convertDepth( 32 );

        //         QImage *dest;
        //         *dest = img;
        QImage *dest = new QImage( img.width(), img.height(), 32 );
        dest->setAlphaBuffer( TRUE );
        unsigned int *data = ( unsigned int * ) img.bits();
        unsigned int *destData = ( unsigned int* ) dest->bits();
        int total = img.width() * img.height();
        int current;
//        int delta;
        int destR, destG, destB, alpha;
//        int srcR = c.red();
//        int srcG = c.green();
//        int srcB = c.blue();
        int h, s, v, ch;
//        int *r, *g, *b, *a;
        QColor col ;
//        float srcPercent, destPercent;

        for ( current = 0; current < total; ++current )
        {
            //             int qRed ( data[ current ] );
            //             int qGreen ( data[ current ] );
            //             int qBlue ( data[ current ] );
            //             alpha = qAlpha( destData[ current ] );
            //
            //             //destData[ current ] = data[ current ];
            //
            //             QColor col = destData[ current ];
            //             col.getHsv( h, s, v );
            //             //*h += 50;
            //
            //             col.setHsv( 50, *s, *v );
            //
            //             col.getRgb( r, g, b );

            col.setRgb( data[ current ] );
            //col = Qt::red;
            c.hsv( &h, &s, &v );
            ch = h;
            col.hsv( &h, &s, &v );
            //             s = 0;
            //v += 100;
            col.setHsv( ch, s, v );
            destR = col.red();
            destG = col.green();
            destB = col.blue();
            alpha = qAlpha( data[ current ] );


            //             if ( destR < 0 )
            //                 destR = 0;
            //             if ( destG < 0 )
            //                 destG = 0;
            //             if ( destB < 0 )
            //                 destB = 0;
            //             if ( destR > 255 )
            //                 destR = 255;
            //             if ( destG > 255 )
            //                 destG = 255;
            //             if ( destB > 255 )
            //                 destB = 255;

            destData[ current ] = qRgba( destR, destG, destB, alpha );
            //destData[ current ] = data[ current ];
        }
        pix->convertFromImage( *dest,                                                             /*Qt::ColorOnly | Qt::DiffuseDither | Qt::DiffuseAlphaDither |*/ Qt::AvoidDither );
    }
}


//////////////////////////////////////////////////////////////////////////////
// colorizePixmap()
//
void DeKoratorFactory::initPixmaps()
{
    int i, j, k;

    for ( i = 0; i < decoCount; i++ )
    {
        for ( j = 0; j < pixTypeCount; j++ )
        {
            DECOARR[ i ][ j ] = new QPixmap();
        }
    }

    for ( i = 0; i < buttonTypeAllCount; i++ )
    {
        for ( j = 0; j < buttonStateCount; j++ )
        {
            for ( k = 0; k < pixTypeCount; k++ )
            {
                BUTTONSARR[ i ][ j ][ k ] = new QPixmap();
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// colorizePixmap()
//
void DeKoratorFactory::chooseRightPixmaps()
{
    int i, j;

    if ( DeKoratorFactory::colorizeActFrames_ )
    {
        for ( i = 0; i < decoCount; i++ )
        {
            DECOPIXACTARR[ i ] = DECOARR[ i ][ actCol ];
        }
    }
    else
    {
        for ( i = 0; i < decoCount; i++ )
        {
            DECOPIXACTARR[ i ] = DECOARR[ i ][ normal ];
        }
    }

    if ( DeKoratorFactory::colorizeInActFrames_ )
    {
        for ( i = 0; i < decoCount; i++ )
        {
            DECOPIXINACTARR[ i ] = DECOARR[ i ][ inActCol ];
        }
    }
    else
    {
        for ( i = 0; i < decoCount; i++ )
        {
            DECOPIXINACTARR[ i ] = DECOARR[ i ][ normal ];
        }
    }



    if ( DeKoratorFactory::colorizeActButtons_ )
    {

        for ( i = 0; i < buttonTypeAllCount; i++ )
        {
            for ( j = 0; j < buttonStateCount; j++ )
            {
                BUTTONPIXACTARR[ i ][ j ] = BUTTONSARR[ i ][ j ][ actCol ];
            }
        }
    }
    else
    {
        for ( i = 0; i < buttonTypeAllCount; i++ )
        {
            for ( j = 0; j < buttonStateCount; j++ )
            {
                BUTTONPIXACTARR[ i ][ j ] = BUTTONSARR[ i ][ j ][ normal ];
            }
        }
    }


    if ( DeKoratorFactory::colorizeInActButtons_ )
    {
        for ( i = 0; i < buttonTypeAllCount; i++ )
        {
            for ( j = 0; j < buttonStateCount; j++ )
            {
                BUTTONPIXINACTARR[ i ][ j ] = BUTTONSARR[ i ][ j ][ inActCol ];
            }
        }
    }
    else
    {
        for ( i = 0; i < buttonTypeAllCount; i++ )
        {
            for ( j = 0; j < buttonStateCount; j++ )
            {
                BUTTONPIXINACTARR[ i ][ j ] = BUTTONSARR[ i ][ j ][ normal ];
            }
        }
    }

}

//////////////////////////////////////////////////////////////////////////////
// prepareDecoWithBbCol()
//
void DeKoratorFactory::prepareDecoWithBgCol()
{
    int i;
    QPainter painter;
    QPixmap tempPix;
    QWidget widget;
    QColor col = widget.colorGroup().background();


    if ( DeKoratorFactory::colorizeActFrames_ )
    {
        for ( i = 0 ; i < decoCount ; i++ )
        {
            tempPix.resize( DECOARR[ i ][ orig ] ->width(), DECOARR[ i ][ orig ] ->height() );

            tempPix.fill( col );

            painter.begin( &tempPix );
            {
                painter.drawPixmap( 0, 0, *( DECOARR[ i ][ actCol ] ) );
            }
            painter.end();

            *( DECOARR[ i ][ actCol ] ) = tempPix;
        }
    }
    else
    {
        for ( i = 0 ; i < decoCount ; i++ )
        {
            tempPix.resize( DECOARR[ i ][ orig ] ->width(), DECOARR[ i ][ orig ] ->height() );

            tempPix.fill( col );

            painter.begin( &tempPix );
            {
                painter.drawPixmap( 0, 0, *( DECOARR[ i ][ orig ] ) );
            }
            painter.end();

            *( DECOARR[ i ][ normal ] ) = tempPix;
        }
    }

    if ( DeKoratorFactory::colorizeInActFrames_ )
    {
        for ( i = 0 ; i < decoCount ; i++ )
        {
            tempPix.resize( DECOARR[ i ][ orig ] ->width(), DECOARR[ i ][ orig ] ->height() );

            tempPix.fill( col );

            painter.begin( &tempPix );
            {
                painter.drawPixmap( 0, 0, *( DECOARR[ i ][ inActCol ] ) );
            }
            painter.end();

            *( DECOARR[ i ][ inActCol ] ) = tempPix;
        }
    }
    else
    {
        for ( i = 0 ; i < decoCount ; i++ )
        {
            tempPix.resize( DECOARR[ i ][ orig ] ->width(), DECOARR[ i ][ orig ] ->height() );

            tempPix.fill( col );

            painter.begin( &tempPix );
            {
                painter.drawPixmap( 0, 0, *( DECOARR[ i ][ orig ] ) );
            }
            painter.end();

            *( DECOARR[ i ][ normal ] ) = tempPix;
        }
    }


    //     if ( !DeKoratorFactory::colorizeActFrames_ || !DeKoratorFactory::colorizeInActFrames_ )
    //     {
    //         for ( i = 0 ; i < decoCount ; i++ )
    //         {
    //             tempPix.resize( DECOARR[ i ][ orig ] ->width(), DECOARR[ i ][ orig ] ->height() );
    //
    //             tempPix.fill( col );
    //
    //             painter.begin( &tempPix );
    //             {
    //                 painter.drawPixmap( 0, 0, *( DECOARR[ i ][ orig ] ) );
    //             }
    //             painter.end();
    //
    //             *( DECOARR[ i ][ normal ] ) = tempPix;
    //         }
    //     }
}






//////////////////////////////////////////////////////////////////////////////
// DeKoratorButton Class                                                      //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// DeKoratorButton()
// ---------------
// Constructor
DeKoratorButton::DeKoratorButton( bool isLeft, int buttonWidth, int buttonHeight, DeKoratorClient * parent, const char * name,
                                  const QString & tip, ButtonType type, buttonTypeAll btnType )
        : Q3Button( parent->widget(), name ), isLeft_( isLeft ), buttonWidth_( buttonWidth ), client_( parent ), type_( type ), lastmouse_( Qt::NoButton ), decoPixHeight_( buttonHeight )
{
    //decoPixInAct_ = buttonPixInAct;
    animProgress = 0;
    hover_ = FALSE;
    setBackgroundMode( Qt::NoBackground );
    setFixedSize( buttonWidth_, BUTTONSHEIGHT );
    setCursor( Qt::arrowCursor );



    //if ( buttonPix )
    setPixmap( btnType );
    QToolTip::add
        ( this, tip );
    animTmr = new QTimer( this );
    connect( animTmr, SIGNAL( timeout() ), this, SLOT( animate() ) );
}

//////////////////////////////////////////////////////////////////////////////
// DeKoratorButton()
// ---------------
// destructor
DeKoratorButton::~DeKoratorButton()
{
    //if ( deco_ ) delete deco_;
}

//////////////////////////////////////////////////////////////////////////////
// setPixmap()

// -----------
// Set the button decoration
void DeKoratorButton::setPixmap( buttonTypeAll btnType )
{
    //   decoPix_ = buttonPix;
    //   decoPixPress_ = buttonPixPress;
    //   decoPixHover_ = buttonPixHover;
    //   decoPixInAct_ = buttonPixInAct;

    //   btnPixAct_ = btnPixAct;
    //   btnPixInAct_ = btnPixInAct;
    btnType_ = btnType;


    //     btnPixAct_ = btnPixAct;
    //     btnPixInAct_ = btnPixInAct;

    //decoPix_->setMask(*deco_);
    //   }
    //   else
    //   {
    //     deco_ = new QBitmap(DECOSIZE, DECOSIZE, bitmap, TRUE);
    //     deco_->setMask(*deco_);
    //   }
    repaint( FALSE );
}

//////////////////////////////////////////////////////////////////////////////
// sizeHint()
// ----------
// Return size hint
QSize DeKoratorButton::sizeHint() const
{
    return QSize( buttonWidth_, BUTTONSHEIGHT );
}

//////////////////////////////////////////////////////////////////////////////
// enterEvent()
// ------------
// Mouse has entered the button
void DeKoratorButton::enterEvent( QEvent * e )
{
    // if we wanted to do mouseovers, we would keep track of it here
    Q3Button::enterEvent( e );
    s = STEPS;
    hover_ = TRUE;
    setCursor( Qt::PointingHandCursor );

    if ( USEANIMATION )
        animate();
    else
        repaint( FALSE );
}

//////////////////////////////////////////////////////////////////////////////
// leaveEvent()
// ------------
// Mouse has left the button
void DeKoratorButton::leaveEvent( QEvent * e )
{
    // if we wanted to do mouseovers, we would keep track of it here

    Q3Button::leaveEvent( e );
    //STEPS = s;
    hover_ = FALSE;
    unsetCursor ();

    if ( USEANIMATION )
        animate();
    else
        repaint( FALSE );
}

//////////////////////////////////////////////////////////////////////////////
// mousePressEvent()
// -----------------
// Button has been pressed
void DeKoratorButton::mousePressEvent( QMouseEvent * e )
{
    lastmouse_ = e->button();

    // translate and pass on mouse event
    int button = Qt::LeftButton;
    if ( ( type_ != ButtonMax ) && ( e->button() != Qt::LeftButton ) )
    {
        button = Qt::NoButton; // middle & right buttons inappropriate
    }
    QMouseEvent me( e->type(), e->pos(), e->globalPos(),
                    button, e->state() );
    Q3Button::mousePressEvent( &me );
}

//////////////////////////////////////////////////////////////////////////////
// mouseReleaseEvent()
// -----------------
// Button has been released
void DeKoratorButton::mouseReleaseEvent( QMouseEvent * e )
{
    lastmouse_ = e->button();

    // translate and pass on mouse event
    int button = Qt::LeftButton;
    if ( ( type_ != ButtonMax ) && ( e->button() != Qt::LeftButton ) )
    {
        button = Qt::NoButton; // middle & right buttons inappropriate
    }
    QMouseEvent me( e->type(), e->pos(), e->globalPos(), button, e->state() );
    Q3Button::mouseReleaseEvent( &me );
}

//////////////////////////////////////////////////////////////////////////////
// drawButton()
// ------------
// Draw the button
void DeKoratorButton::drawButton( QPainter * painter )
{
    if ( !DeKoratorFactory::initialized() )
        return ;

    QColorGroup group;
    int dx = 0, dy = 0;
    bool act = client_->isActive();
    QImage buttonImgBak;

    // fill background
    if ( isLeft_ )
        painter->drawTiledPixmap( rect(), act ? *( DECOPIXACTARR[ leftButtons ] ) : *( DECOPIXINACTARR[ leftButtons ] ) );
    else
        painter->drawTiledPixmap( rect(), act ? *( DECOPIXACTARR[ rightButtons ] ) : *( DECOPIXINACTARR[ rightButtons ] ) );

    // apply app icon effects
    if ( type_ == ButtonMenu && !USEMENUEIMAGE )
    {
        dy = ( height() - 16 ) / 2;

        QPixmap appIconPix = client_->icon().pixmap( 16, QIcon::Normal );

        if ( !IGNOREAPPICNCOL )
        {
            if ( act )
            {
                if ( client_->decoFactory_->useCustomButtonsColors_ && client_->decoFactory_->customColorsActiveButtons_ )
                {
                    DeKoratorFactory::colorizePixmap( &appIconPix, client_->decoFactory_->cusBtnCol_[ menu ], BUTTONSCOLORIZE );
                }
                else if ( client_->decoFactory_->colorizeActButtons_ )
                {
                    QColor col = client_->decoFactory_->options() ->palette( KDecoration::ColorButtonBg, act ).background();

                    DeKoratorFactory::colorizePixmap( &appIconPix, col, BUTTONSCOLORIZE );
                }
            }
            else
            {
                if ( client_->decoFactory_->useCustomButtonsColors_ && client_->decoFactory_->customColorsInActiveButtons_ )
                {
                    DeKoratorFactory::colorizePixmap( &appIconPix, client_->decoFactory_->cusBtnCol_[ menu ], BUTTONSCOLORIZE );
                }
                else if ( client_->decoFactory_->colorizeInActButtons_ )
                {
                    QColor col = client_->decoFactory_->options() ->palette( KDecoration::ColorButtonBg, act ).background();

                    DeKoratorFactory::colorizePixmap( &appIconPix, col, BUTTONSCOLORIZE );
                }
            }
        }
        buttonImgBak = appIconPix;

    }
    else
    {
        dy = ( BUTTONSHEIGHT - decoPixHeight_ ) / 2;
    }


    // down
    if ( isDown() )
    {
        if ( !( type_ == ButtonMenu && !USEMENUEIMAGE ) )
        {
            buttonImgBak = act ? *( BUTTONPIXACTARR[ btnType_ ][ press ] ) : *( BUTTONPIXINACTARR[ btnType_ ][ press ] );
        }
        dx += BTNSHIFTX;
        dy += BTNSHIFTY;

        chooseRightHoverEffect( &buttonImgBak, BUTTONHOVERTYPE );
    }
    // hover
    else if ( hover_ )
    {
        if ( !( type_ == ButtonMenu && !USEMENUEIMAGE ) )
        {
            buttonImgBak = act ? *( BUTTONPIXACTARR[ btnType_ ][ hover ] ) : *( BUTTONPIXINACTARR[ btnType_ ][ hover ] );
        }
        chooseRightHoverEffect( &buttonImgBak, USEANIMATION ? ANIMATIONTYPE : BUTTONHOVERTYPE );
    }
    // regular
    else
    {
        if ( !( type_ == ButtonMenu && !USEMENUEIMAGE ) )
        {
            buttonImgBak = act ? *( BUTTONPIXACTARR[ btnType_ ][ regular ] ) : *( BUTTONPIXINACTARR[ btnType_ ][ regular ] );
        }

        if ( USEANIMATION && animProgress > 0 )
            chooseRightHoverEffect( &buttonImgBak, ANIMATIONTYPE );
    }

    painter->drawPixmap( dx, dy, QPixmap::fromImage(buttonImgBak) );


    if ( client_->isShade() && !SHOWBTMBORDER )
    {
        painter->setPen( QColor( 70, 70, 70 ) );
        painter->drawLine( 0, BUTTONSHEIGHT - 1, buttonWidth_ - 1, BUTTONSHEIGHT - 1 );
    }
}

//////////////////////////////////////////////////////////////////////////////
// chooseRightHoverEffect()
// ------------
//
QImage* DeKoratorButton::chooseRightHoverEffect( QImage * img, QString res )
{

    if ( USEANIMATION && !isDown() )
    {
        float f = ( ( float ) animProgress / 100 );

        if ( res == "Intensity" )
        {
            f /= 1.5;

            * img = Blitz::intensity ( *img, f );
        }
        else if ( res == "Blend" )
        {
            f /= 1.7;
            QColor col = client_->isActive() ? ACTIVEHIGHLIGHTCOLOR : INACTIVEHIGHLIGHTCOLOR;

         /*   * img = Blitz::blend ( col, *img, f );*/
        }
        else if ( res == "Solarize" )
        {
         /*   Blitz::solarize ( *img, 100 - animProgress ); */
        }
        else if ( res == "Fade" )
        {
            f /= 3;
            QColor col = client_->isActive() ? ACTIVEHIGHLIGHTCOLOR : INACTIVEHIGHLIGHTCOLOR;

            * img = Blitz::fade ( *img, f, col );
        }
        else if ( res == "Trashed" )
        {
            * img = Blitz::fade ( *img, animProgress, Qt::black );
        }
        else if ( res == "Hue Shift" )
        {
            int h, s, v;
            QColor col ;
            int shift = ( int ) ( animProgress * 3.5 );
            col.setRgb( 255, 0, 0 );
            col.hsv( &h, &s, &v );
            col.setHsv( shift, s, v );

            QPixmap pix;
            pix.convertFromImage( *img, Qt::AvoidDither );
            DeKoratorFactory::colorizePixmap( &pix, col, "Hue Adgustment" );
            *img = pix.convertToImage();
        }
    }
    else
    {
        if ( BUTTONHOVERTYPE == "To Gray" )
            KIconEffect::toGray( *img, EFFECTAMOUNT );
        else if ( BUTTONHOVERTYPE == "Colorize" )
        {
            QColor col = client_->isActive() ? ACTIVEHIGHLIGHTCOLOR : INACTIVEHIGHLIGHTCOLOR;

            QPixmap pix;
            pix.convertFromImage( *img, Qt::AvoidDither );
            DeKoratorFactory::colorizePixmap( &pix, col, BUTTONSCOLORIZE );
            *img = pix.convertToImage();
        }
        else if ( BUTTONHOVERTYPE == "DeSaturate" )
            KIconEffect::deSaturate( *img, EFFECTAMOUNT );
        else if ( BUTTONHOVERTYPE == "To Gamma" )
            KIconEffect::toGamma( *img, EFFECTAMOUNT );
    }
    return img;
}


//////////////////////////////////////////////////////////////////////////////
// animate()
// ------------
//
void DeKoratorButton::animate()
{
    animTmr->stop();

    if ( hover_ )
    {
        if ( KEEPANIMATING )
        {
            animProgress += s;

            if ( animProgress >= ANIMATIONSTEPS || animProgress <= 0 )
            {
                s *= -1;
            }
            animTmr->start( INTERVAL, true ); // single-shot
        }
        else
        {
            if ( !KEEPANIMATING )
            {
                if ( animProgress < ANIMATIONSTEPS )
                    animProgress += STEPS;
                else
                {
                    animProgress = ANIMATIONSTEPS;
                }
            }
            if ( animProgress < ANIMATIONSTEPS )
                animTmr->start( INTERVAL, true ); // single-shot
        }
    }
    else
    {
        if ( animProgress > 0 )
            animProgress -= STEPS;
        else
        {
            animProgress = 0;
        }
        if ( animProgress > 0 )
            animTmr->start( INTERVAL, true ); // single-shot
    }
    //qWarning( "STEPS: %d", STEPS );
    //qWarning( "animProgress: %d", animProgress );
    //qWarning( "INTERVAL: %d", INTERVAL );
    repaint( false );
}










//////////////////////////////////////////////////////////////////////////////
// DeKoratorClient Class                                                      //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// DeKoratorClient()
// ---------------
// Constructor
DeKoratorClient::DeKoratorClient( KDecorationBridge * b, KDecorationFactory * f )
        : KDecoration( b, f ), mainLayout_( NULL ), titleLayout_( NULL ), midLayout_( NULL ) , leftTitleBarSpacer_( NULL ), titleBarSpacer_( NULL ), rightTitleBarSpacer_( NULL ), leftSpacer_( NULL ), rightSpacer_( NULL ), bottomSpacer_( NULL ), captionBufferDirty_( true )
{
    //captionBufferDirty_ = true;
    //maskDirty_ = true;
    decoFactory_ = ( DeKoratorFactory* ) f;
}

DeKoratorClient::~DeKoratorClient()
{
    for ( int n = 0; n < ButtonTypeCount; n++ )
    {
        if ( button[ n ] )
            delete button[ n ];
    }
}

//////////////////////////////////////////////////////////////////////////////
// init()
// ------
// Actual initializer for class


// basic layout:

//  __________________________________________________________________________
// | leftTitleBarSpacer | btns | titleBarSpacer_ | bts | rightTitleBarSpacer |
// |____________________|______|_________________|_____|_____________________|
// | |                                                                     | |
// | |                         contentsFake                                | |
// | |                                                                     | |
// |leftSpacer                                                    rightSpacer|
// |_|_____________________________________________________________________|_|
// |                                 bottomSpacer                            |
// |_________________________________________________________________________|
//

void DeKoratorClient::init()
{
    createMainWidget( Qt::WNoAutoErase );
    widget() ->installEventFilter( this );

    // for flicker-free redraws
    widget() ->setBackgroundMode( Qt::NoBackground );

    // layouts
    delete mainLayout_;
    delete titleLayout_;
    delete midLayout_;
    // spacers
    delete leftTitleBarSpacer_;
    delete titleBarSpacer_;
    delete rightTitleBarSpacer_;
    delete leftSpacer_;
    delete rightSpacer_;
    delete bottomSpacer_;
    /*  topLeftCornerBg[ normal ] ->load( decoPixDir + "/topLeftCornerBg.png" );
            leftButtonsBg[ normal ] ->load( decoPixDir + "/leftButtonsBg.png" );
            leftTitleBg[ normal ] ->load( decoPixDir + "/leftTitleBg.png" );
            midTitleBg[ normal ] ->load( decoPixDir + "/midTitleBg.png" );
            rightTitleBg[ normal ] ->load( decoPixDir + "/rightTitleBg.png" );
            rightButtonsBg[ normal ] ->load( decoPixDir + "/rightButtonsBg.png" );
            topRightCornerBg[ normal ] ->load( decoPixDir + "/topRightCornerBg.png" );
            // left frame from top to buttom
            topLeftFrameBg[ normal ] ->load( decoPixDir + "/topLeftFrameBg.png" );
            midLeftFrameBg[ normal ] ->load( decoPixDir + "/midLeftFrameBg.png" );
            buttomLeftFrameBg[ normal ] ->load( decoPixDir + "/bottomLeftFrameBg.png" );
            // buttom frame from left to right
            leftButtomFrameBg[ normal ] ->load( decoPixDir + "/leftBottomFrameBg.png" );
            midButtomFrameBg[ normal ] ->load( decoPixDir + "/midBottomFrameBg.png" );
            rightButtomFrameBg[ normal ] ->load( decoPixDir + "/rightBottomFrameBg.png" );
            // right frame from top to buttom
            topRightFrameBg[ normal ] ->load( decoPixDir + "/topRightFrameBg.png" );
            midRightFrameBg[ normal ] ->load( decoPixDir + "/midRightFrameBg.png" );
            buttomRightFrameBg[ normal ] ->load( decoPixDir + "/bottomRightFrameBg.png" );*/

    // layouts
    mainLayout_ = new Q3VBoxLayout( widget(), 0, 0 );
    titleLayout_ = new Q3HBoxLayout( mainLayout_, 0, 0 );
    midLayout_ = new Q3HBoxLayout( mainLayout_, 0, 0 );

    // spacers
    leftTitleBarSpacer_ = new QSpacerItem( TOPLEFTCORNERWIDTH, TITLESIZE,
                                           QSizePolicy::Fixed, QSizePolicy::Fixed );
    titleBarSpacer_ = new QSpacerItem( 1, TITLESIZE, QSizePolicy::Expanding,
                                       QSizePolicy::Fixed );
    rightTitleBarSpacer_ = new QSpacerItem( TOPRIGHTCORNERWIDTH, TITLESIZE,
                                            QSizePolicy::Fixed, QSizePolicy::Fixed );
    leftSpacer_ = new QSpacerItem( LEFTFRAMESIZE, 1,
                                   QSizePolicy::Fixed, QSizePolicy::Expanding );
    rightSpacer_ = new QSpacerItem( RIGHTFRAMESIZE, 1,
                                    QSizePolicy::Fixed, QSizePolicy::Expanding );
    bottomSpacer_ = new QSpacerItem( 1, ( !isSetShade() || SHOWBTMBORDER ) ? BUTTOMFRAMESIZE : 0,
                                     QSizePolicy::Expanding, QSizePolicy::Fixed );


    // setup layout

    //left spacer
    titleLayout_->addItem( leftTitleBarSpacer_ );
    // setup titlebar buttons
    for ( int n = 0; n < ButtonTypeCount; n++ )
        button[ n ] = 0;
    addButtons( titleLayout_, options() ->titleButtonsLeft(), TRUE );
    titleLayout_->addItem( titleBarSpacer_ );
    addButtons( titleLayout_, options() ->titleButtonsRight(), FALSE );
    //right spacer
    titleLayout_->addItem( rightTitleBarSpacer_ );

    // mid layout
    midLayout_->addItem( leftSpacer_ );
    if ( isPreview() )
    {
        midLayout_->addWidget( new QLabel( "<center><b>" + i18n( "Dekorator preview (Version 0.3)" ) + "</b></center>", widget() ) );
    }
    else
    {
        midLayout_->addItem( new QSpacerItem( 0, 0 ) );
    }
    midLayout_->addItem( rightSpacer_ );


    //Bottom
    //if(!isSetShade() || SHOWBTMBORDER)
    mainLayout_->addItem( bottomSpacer_ );

    // connections
    connect( this, SIGNAL( keepAboveChanged( bool ) ), SLOT( keepAboveChange( bool ) ) );
    connect( this, SIGNAL( keepBelowChanged( bool ) ), SLOT( keepBelowChange( bool ) ) );

    captionBufferDirty_ = true;
    //maskDirty_ = true;
    widget() ->update( titleBarSpacer_->geometry() );
}

//////////////////////////////////////////////////////////////////////////////
// addButtons()
// ------------
// Add buttons to title layout
void DeKoratorClient::addButtons( Q3BoxLayout * layout, const QString & s, bool isLeft )
{
    //const unsigned char * bitmap;
//    QPixmap * pix1, *pix2, *pix3, *pix4;
    QString tip;
    buttonTypeAll btnType;

    if ( s.length() > 0 )
    {
        for ( int n = 0; n < s.length(); n++ )
        {
            switch ( s[ n ].toAscii() )
            {
            case 'M':
                // Menu button
                if ( !button[ ButtonMenu ] )
                {
                    button[ ButtonMenu ] =
                        new DeKoratorButton( isLeft, ( BTNMENUWIDTH < 16 ) ? 16 : BTNMENUWIDTH, BTNMENUHEIGHT, this, "menu", i18n( "Menu" ), ButtonMenu, menu );
                    connect( button[ ButtonMenu ], SIGNAL( pressed() ),
                             this, SLOT( menuButtonPressed() ) );
                    connect( button[ ButtonMenu ], SIGNAL( released() ), this, SLOT( menuButtonReleased() ) );

                    layout->addWidget( button[ ButtonMenu ] );
                }
                break;

            case 'S':
                // Sticky button
                if ( !button[ ButtonSticky ] )
                {
                    bool d = isOnAllDesktops();
                    if ( d )
                    {
                        btnType = stickydown;
                    }
                    else
                    {
                        //             pix1 = &( decoFactory_->buttonStickyPix_ );
                        //             pix2 = &( decoFactory_->buttonStickyPressPix_ );
                        //             pix3 = &( decoFactory_->buttonStickyHoverPix_ );
                        //             pix4 = &( decoFactory_->buttonStickyDownPixInAct_ );


                        btnType = sticky;
                    }
                    button[ ButtonSticky ] =
                        new DeKoratorButton( isLeft, BTNSTICKYWIDTH, BTNSTICKYHEIGHT, this, "sticky", d ? i18n( "Sticky" ) : i18n( "Un-Sticky" ), ButtonSticky, btnType );
                    connect( button[ ButtonSticky ], SIGNAL( clicked() ),
                             this, SLOT( toggleOnAllDesktops() ) );
                    layout->addWidget( button[ ButtonSticky ] );
                }
                break;

            case 'H':
                // Help button
                if ( ( !button[ ButtonHelp ] ) && providesContextHelp() )
                {
                    button[ ButtonHelp ] =
                        new DeKoratorButton( isLeft, BTNHELPEWIDTH, BTNHELPEHEIGHT, this, "help-contents", i18n( "Help" ),
                                             ButtonHelp, help );
                    connect( button[ ButtonHelp ], SIGNAL( clicked() ),
                             this, SLOT( showContextHelp() ) );
                    layout->addWidget( button[ ButtonHelp ] );
                }
                break;

            case 'I':
                // Minimize button
                if ( ( !button[ ButtonMin ] ) && isMinimizable() )
                {
                    button[ ButtonMin ] =
                        new DeKoratorButton( isLeft, BTNMINWIDTH, BTNMINHEIGHT, this, "iconify", i18n( "Minimize" ), ButtonMin, min );
                    connect( button[ ButtonMin ], SIGNAL( clicked() ),
                             this, SLOT( minimize() ) );
                    layout->addWidget( button[ ButtonMin ] );
                }
                break;

            case 'A':
                // Maximize button
                if ( ( !button[ ButtonMax ] ) && isMaximizable() )
                {
                    bool m = ( maximizeMode() == MaximizeFull );
                    if ( m )
                    {
                        //            pix1 = &( decoFactory_->buttonRestorePix_ );
                        //             pix2 = &( decoFactory_->buttonRestorePressPix_ );
                        //             pix3 = &( decoFactory_->buttonRestoreHoverPix_ );
                        //             pix4 = &( decoFactory_->buttonRestorePixInAct_ );


                        btnType = restore;
                    }
                    else
                    {
                        //             pix1 = &( decoFactory_->buttonMaxPix_ );
                        //             pix2 = &( decoFactory_->buttonMaxPressPix_ );
                        //             pix3 = &( decoFactory_->buttonMaxHoverPix_ );
                        //             pix4 = &( decoFactory_->buttonMaxPixInAct_ );


                        btnType = max;
                    }
                    button[ ButtonMax ] =
                        new DeKoratorButton( isLeft, BTNMAXWIDTH, BTNMAXHEIGHT, this, "maximize", m ? i18n( "Restore" ) : i18n( "Maximize" ),
                                             ButtonMax, btnType );
                    connect( button[ ButtonMax ], SIGNAL( clicked() ),
                             this, SLOT( maxButtonPressed() ) );
                    layout->addWidget( button[ ButtonMax ] );
                }
                break;

            case 'X':
                // Close button
                if ( ( !button[ ButtonClose ] ) && isCloseable() )
                {
                    button[ ButtonClose ] =
                        new DeKoratorButton( isLeft, BTNCLOSEWIDTH, BTNCLOSEHEIGHT, this, "close", i18n( "Close" ),
                                             ButtonClose, close );
                    connect( button[ ButtonClose ], SIGNAL( clicked() ),
                             this, SLOT( closeWindow() ) );
                    layout->addWidget( button[ ButtonClose ] );
                }
                break;

            case 'F':
                // Above button
                if ( ( !button[ ButtonAbove ] ) )
                {
                    bool a = keepAbove();
                    if ( a )
                    {
                        btnType = abovedown;
                    }
                    else
                    {
                        btnType = above;
                    }
                    //           pix1 = &( decoFactory_->buttonAbovePix_ );
                    button[ ButtonAbove ] =
                        new DeKoratorButton( isLeft, BTNABOVEWIDTH, BTNABOVEHEIGHT, this, "above",
                                             i18n( "Keep Above Others" ), ButtonAbove, btnType );
                    connect( button[ ButtonAbove ], SIGNAL( clicked() ),
                             this, SLOT( aboveButtonPressed() ) );
                    layout->addWidget( button[ ButtonAbove ] );
                }
                break;

            case 'B':
                // Below button
                if ( ( !button[ ButtonBelow ] ) )
                {
                    bool b = keepBelow();
                    if ( b )
                    {
                        btnType = belowdown;
                    }
                    else
                    {
                        btnType = below;
                    }
                    //          pix1 = &( decoFactory_->buttonBelowPix_ );
                    button[ ButtonBelow ] =
                        new DeKoratorButton( isLeft, BTNBELOWWIDTH, BTNBELOWHEIGHT, this, "below",
                                             i18n( "Keep Below Others" ), ButtonBelow, btnType );
                    connect( button[ ButtonBelow ], SIGNAL( clicked() ),
                             this, SLOT( belowButtonPressed() ) );
                    layout->addWidget( button[ ButtonBelow ] );
                }
                break;

            case 'L':
                // Shade button
                if ( ( !button[ ButtonShade && isShadeable() ] ) )
                {
                    bool s = isSetShade();
                    if ( s )
                    {
                        btnType = shadedown;
                    }
                    else
                    {
                        btnType = shade;
                    }
                    button[ ButtonShade ] =
                        new DeKoratorButton( isLeft, BTNSHADEWIDTH, BTNSHADEHEIGHT, this, "shade", s ? i18n( "Unshade" ) : i18n( "Shade" ),
                                             ButtonShade, btnType );
                    connect( button[ ButtonShade ], SIGNAL( clicked() ),
                             this, SLOT( shadeButtonPressed() ) );
                    layout->addWidget( button[ ButtonShade ] );
                }
                break;

            case '_':
                // Spacer item
                layout->addSpacing( 3 );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// activeChange()
// --------------
// window active state has changed
void DeKoratorClient::activeChange()
{
    for ( int n = 0; n < ButtonTypeCount; n++ )
        if ( button[ n ] )
            button[ n ] ->reset();
    widget() ->repaint( FALSE );
}

//////////////////////////////////////////////////////////////////////////////
// captionChange()
// ---------------
// The title has changed
void DeKoratorClient::captionChange()
{
    captionBufferDirty_ = true;
    widget() ->repaint( titleBarSpacer_->geometry() );
}

//////////////////////////////////////////////////////////////////////////////
// desktopChange()
// ---------------
// Called when desktop/sticky changes
void DeKoratorClient::desktopChange()
{
    bool d = isOnAllDesktops();
    buttonTypeAll btnType;

    if ( d )
    {
        btnType = stickydown;
    }
    else
    {
        btnType = sticky;
    }
    if ( button[ ButtonSticky ] )
    {
        button[ ButtonSticky ] ->setPixmap( btnType );
        QToolTip::remove
            ( button[ ButtonSticky ] );
        QToolTip::add
            ( button[ ButtonSticky ], d ? i18n( "Sticky" ) : i18n( "Un-Sticky" ) );
    }
}

//////////////////////////////////////////////////////////////////////////////
// iconChange()
// ------------
// The title has changed
void DeKoratorClient::iconChange()
{
    if ( !USEMENUEIMAGE && button[ ButtonMenu ] )
    {
        //button[ ButtonMenu ] ->setPixmap( 0 );
        button[ ButtonMenu ] ->repaint( FALSE );
    }
}

//////////////////////////////////////////////////////////////////////////////
// maximizeChange()
// ----------------
// Maximized state has changed
void DeKoratorClient::maximizeChange()
{
    bool m = ( maximizeMode() == MaximizeFull );
    buttonTypeAll btnType;

    if ( m )
    {
        btnType = restore;
    }
    else
    {
        btnType = max;
    }
    if ( button[ ButtonMax ] )
    {
        button[ ButtonMax ] ->setPixmap( btnType );
        QToolTip::remove
            ( button[ ButtonMax ] );
        QToolTip::add
            ( button[ ButtonMax ], m ? i18n( "Restore" ) : i18n( "Maximize" ) );
    }
}

//////////////////////////////////////////////////////////////////////////////
// shadeChange()
// -------------
// Called when window shading changes
void DeKoratorClient::shadeChange()
{
    bool s = isSetShade();
    buttonTypeAll btnType;

    if ( s )
    {
        btnType = shadedown;
    }
    else
    {
        btnType = shade;
    }
    if ( button[ ButtonShade ] )
    {
        button[ ButtonShade ] ->setPixmap( btnType );
        QToolTip::remove
            ( button[ ButtonShade ] );
        QToolTip::add
            ( button[ ButtonShade ], s ? i18n( "Unshade" ) : i18n( "Shade" ) );
    }

    //mainlayout_->setRowSpacing( 3, isSetShade() ? 0 : MARGIN );
    for ( int n = 0; n < ButtonTypeCount; n++ )
        if ( button[ n ] )
            button[ n ] ->reset();
    widget() ->repaint( FALSE );

    //mainlayout_->setRowSpacing( 3, isSetShade() ? 0 : MARGIN );
    //     if ( DeKoratorFactory::useMasks_ )
    //         doShape();
}

//////////////////////////////////////////////////////////////////////////////
// keepAboveChange()
// ------------
// The above state has changed
void DeKoratorClient::keepAboveChange( bool a )
{
    buttonTypeAll btnType;

    if ( a )
    {
        btnType = abovedown;
    }
    else
    {
        btnType = above;
    }
    if ( button[ ButtonAbove ] )
    {
        button[ ButtonAbove ] ->setPixmap( btnType );
    }
}

//////////////////////////////////////////////////////////////////////////////
// keepBelowChange()
// ------------
// The below state has changed
void DeKoratorClient::keepBelowChange( bool b )
{
    buttonTypeAll btnType;

    if ( b )
    {
        btnType = belowdown;
    }
    else
    {
        btnType = below;
    }
    if ( button[ ButtonBelow ] )
    {
        button[ ButtonBelow ] ->setPixmap( btnType );
    }
}

//////////////////////////////////////////////////////////////////////////////
// borders()
// ----------
// Get the size of the borders
void DeKoratorClient::borders( int & l, int & r, int & t, int & b ) const
{
    l = LEFTFRAMESIZE;
    r = RIGHTFRAMESIZE;
    t = TITLESIZE ;
    //     if ( SHOWBTMBORDER )
    //b = 10;
    //     else
    //         b = isShade() ? 0 : BUTTOMFRAMESIZE;
    //b = SHOWBTMBORDER ? BUTTOMFRAMESIZE : isShade() ? 1 : BUTTOMFRAMESIZE;
    if ( !isShade() || SHOWBTMBORDER )
    {
        b = BUTTOMFRAMESIZE;
        bottomSpacer_->changeSize( 1, BUTTOMFRAMESIZE, QSizePolicy::Expanding, QSizePolicy::Fixed );
    }
    else
    {
        b = 0;
        bottomSpacer_->changeSize( 1, b, QSizePolicy::Expanding, QSizePolicy::Fixed );
    }
    widget() ->layout() ->activate();
}

//////////////////////////////////////////////////////////////////////////////
// resize()
// --------
// Called to resize the window
void DeKoratorClient::resize( const QSize & size )
{
    widget() ->resize( size );
}

//////////////////////////////////////////////////////////////////////////////
// minimumSize()
// -------------
// Return the minimum allowable size for this window
QSize DeKoratorClient::minimumSize() const
{
    QSize size = widget() ->minimumSize();
    size.setWidth( size.width() + TOPLEFTCORNERWIDTH + TOPRIGHTCORNERWIDTH );

    return size;

}

//////////////////////////////////////////////////////////////////////////////
// mousePosition()
// ---------------
// Return logical mouse position
KDecoration::Position DeKoratorClient::mousePosition( const QPoint & point ) const
{
    //    bool res = TRUE;
    Position pos;
    if ( isShade() )
    {
        return PositionCenter;
    }

    if ( point.y() <= 5 )
    {
        // inside top frame
        if ( point.x() <= LEFTFRAMESIZE )
            pos = PositionTopLeft;
        else if ( point.x() >= ( width() - RIGHTFRAMESIZE ) )
            pos = PositionTopRight;
        else
            pos = PositionTop;
    }
    else if ( point.y() >= ( height() - BUTTOMFRAMESIZE ) )
    {
        // inside handle
        if ( point.x() <= LEFTFRAMESIZE )
            pos = PositionBottomLeft;
        else if ( point.x() >= ( width() - RIGHTFRAMESIZE ) )
            pos = PositionBottomRight;
        else
            pos = PositionBottom;
    }
    else if ( point.x() <= LEFTFRAMESIZE )
    {
        // on left frame
        if ( point.y() <= TITLESIZE )
            pos = PositionTopLeft;
        else if ( point.y() >= ( height() - BUTTOMFRAMESIZE ) )
            pos = PositionBottomLeft;
        else
            pos = PositionLeft;
    }
    else if ( point.x() >= width() - RIGHTFRAMESIZE )
    {
        // on right frame
        if ( point.y() <= TITLESIZE )
            pos = PositionTopRight;
        else if ( point.y() >= ( height() - BUTTOMFRAMESIZE ) )
            pos = PositionBottomRight;
        else
            pos = PositionRight;
    }
    else
    {
        // inside the frame
        pos = PositionCenter;
    }
    return pos;

    //     const int corner = 24;
    //     Position pos;
    //
    //     if (point.y() <= MARGIN) {
    //         // inside top frame
    //         if (point.x() <= corner)                 pos = PositionTopLeft;
    //         else if (point.x() >= (width()-corner))  pos = PositionTopRight;
    //         else                                     pos = PositionTop;
    //     } else if (point.y() >= (height()-MARGIN*2)) {
    //         // inside handle
    //         if (point.x() <= corner)                 pos = PositionBottomLeft;
    //         else if (point.x() >= (width()-corner))  pos = PositionBottomRight;
    //         else                                     pos = PositionBottom;
    //     } else if (point.x() <= MARGIN) {
    //         // on left frame
    //         if (point.y() <= corner)                 pos = PositionTopLeft;
    //         else if (point.y() >= (height()-corner)) pos = PositionBottomLeft;
    //         else                                     pos = PositionLeft;
    //     } else if (point.x() >= width()-MARGIN) {
    //         // on right frame
    //         if (point.y() <= corner)                 pos = PositionTopRight;
    //         else if (point.y() >= (height()-corner)) pos = PositionBottomRight;
    //         else                                     pos = PositionRight;
    //     } else {
    //         // inside the frame
    //         pos = PositionCenter;
    //     }
    //     return pos;

}


//////////////////////////////////////////////////////////////////////////////
// eventFilter()
// -------------
// Event filter
bool DeKoratorClient::eventFilter( QObject * obj, QEvent * e )
{
    if ( obj != widget() )
        return FALSE;

    switch ( e->type() )
    {

    case QEvent::MouseButtonDblClick:
        {
            mouseDoubleClickEvent( static_cast<QMouseEvent *>( e ) );
            return TRUE;
        }
    case QEvent::Wheel:
        {
            wheelEvent( static_cast<QWheelEvent *>( e ) );
            return TRUE;
        }
    case QEvent::MouseButtonPress:
        {
            processMousePressEvent( static_cast<QMouseEvent *>( e ) );
            if ( USEMASKS )
                doShape();
            return TRUE;
        }
    case QEvent::Paint:
        {
            paintEvent( static_cast<QPaintEvent *>( e ) );
            return TRUE;
        }
    case QEvent::Resize:
        {
            resizeEvent( static_cast<QResizeEvent *>( e ) );
            return TRUE;
        }
    case QEvent::Show:
        {
            showEvent( static_cast<QShowEvent *>( e ) );
            return TRUE;
        }
    default:
        {
            return FALSE;
        }
    }

    return FALSE;
}

//////////////////////////////////////////////////////////////////////////////
// mouseDoubleClickEvent()
// -----------------------
// Doubleclick on title
void DeKoratorClient::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( titleBarSpacer_->geometry().contains( e->pos() ) )
        titlebarDblClickOperation();
}

//////////////////////////////////////////////////////////////////////////////
// wheelEvent()
// -----------------------
//
void DeKoratorClient::wheelEvent( QWheelEvent *e )
{
    if ( titleLayout_->geometry().contains( e->pos() ) )
        titlebarMouseWheelOperation( e->delta() );
}


//////////////////////////////////////////////////////////////////////////////
// paintEvent()
// ------------
// Repaint the window
void DeKoratorClient::paintEvent( QPaintEvent* )
{
    if ( !DeKoratorFactory::initialized() )
        return ;
    if ( TITLESIZE )
    {
        if ( captionBufferDirty_ && USESHDTEXT )
            updateCaptionBuffer();

        QPainter painter( widget() );
        int tx, ty, tw, th;
        int w = width() , h = height();
        QRect titleR( titleBarSpacer_->geometry() );
        QRect leftTitleR( leftTitleBarSpacer_->geometry() );
        QRect rightTitleR( rightTitleBarSpacer_->geometry() );
        titleR.rect( &tx, &ty, &tw, &th );
        QRect rect;

        QPainter painter2;
        QPixmap pix( w, TITLESIZE );
        painter2.begin( &pix );
        {
            // topLeftCorner

            rect.setRect( 0, 0, TOPLEFTCORNERWIDTH, TITLESIZE );
            painter2.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ topLeftCorner ] ) : *( DECOPIXINACTARR[ topLeftCorner ] ) );

            //         rect.setRect( 0, 0, TOPLEFTCORNERWIDTH, TITLESIZE );
            //         painter2.drawTiledPixmap( rect, isActive() ? decoFactory_->topLeftCornerBg_ : decoFactory_->topLeftCornerBgInAct_ );

            // Space under the left button group
            painter2.drawTiledPixmap( leftTitleR.right() + 1, titleR.top(),
                                      ( titleR.left() - 1 ) - leftTitleR.right(), titleR.height(), isActive() ? *( DECOPIXACTARR[ leftButtons ] ) : *( DECOPIXINACTARR[ leftButtons ] ) );

            if ( tw > 0 )
            {
                //qWarning("%d",titleR.width());

                //leftTitleR
                rect.setRect( tx, 0, qMin( LEFTTITLEWIDTH, tw ), TITLESIZE );

                painter2.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ leftTitle ] ) : *( DECOPIXINACTARR[ leftTitle ] ) );

                //             rect.setRect( tx, 0, qMin( LEFTTITLEWIDTH, tw ), TITLESIZE );
                //             painter2.drawTiledPixmap( rect, isActive() ? decoFactory_->leftTitleBg_ : decoFactory_->leftTitleBgInAct_ );

                //midTitle
                if ( tw > LEFTTITLEWIDTH + RIGHTTITLEWIDTH )
                {
                    rect.setRect( tx + LEFTTITLEWIDTH, 0, tw - ( RIGHTTITLEWIDTH + LEFTTITLEWIDTH ), th );

                    painter2.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ midTitle ] ) : *( DECOPIXINACTARR[ midTitle ] ) );

                    //                 painter2.drawTiledPixmap( rect, isActive() ? decoFactory_->midTitleBg_ : decoFactory_->midTitleBgInAct_ );
                }

                //rightTitleR
                if ( tw > LEFTTITLEWIDTH )
                {
                    rect.setRect( qMax( tx + tw - RIGHTTITLEWIDTH, tx + LEFTTITLEWIDTH ), 0, qMin( RIGHTTITLEWIDTH, tw - LEFTTITLEWIDTH ), th );

                    painter2.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? *( DECOPIXACTARR[ rightTitle ] ) : *( DECOPIXINACTARR[ rightTitle ] ), ( tw > LEFTTITLEWIDTH + RIGHTTITLEWIDTH ) ? 0 : LEFTTITLEWIDTH - ( tw - RIGHTTITLEWIDTH ), 0 );

                    //                 painter2.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? decoFactory_->rightTitleBg_ : decoFactory_->rightTitleBgInAct_, ( tw > LEFTTITLEWIDTH + RIGHTTITLEWIDTH ) ? 0 : LEFTTITLEWIDTH - ( tw - RIGHTTITLEWIDTH ), 0 );
                }
            }

            // Space under the right button group
            painter2.drawTiledPixmap( titleR.right() + 1, titleR.top(),
                                      ( rightTitleR.left() - 1 ) - titleR.right(), titleR.height(), isActive() ? *( DECOPIXACTARR[ rightButtons ] ) : *( DECOPIXINACTARR[ rightButtons ] ) );


            //topRightCorner
            rect.setRect( widget() ->width() - TOPRIGHTCORNERWIDTH, 0, TOPRIGHTCORNERWIDTH, TITLESIZE );

            painter2.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ topRightCorner ] ) : *( DECOPIXINACTARR[ topRightCorner ] ) );

            // painter2.drawTiledPixmap( rect, isActive() ? decoFactory_->topRightCornerBg_ : decoFactory_->topRightCornerBgInAct_ );


            QString c( caption() );
            QFontMetrics fm( options() ->font( isActive() ) );
            int captionWidth = fm.width( c );
            // shadow text effect
            if ( USESHDTEXT )
            {

                int captionHeight = fm.height( );
                int dx = 0, dy = 0;

                if ( TITLEALIGN == Qt::AlignLeft || captionWidth > titleR.width() )
                {
                    dx = tx + 1 + ( isActive() ? ACTIVESHDTEXTX : INACTIVESHDTEXTX ) ;
                    dy = ( TITLESIZE / 2 ) - ( captionHeight / 2 ) + ( isActive() ? ACTIVESHDTEXTY : INACTIVESHDTEXTY ) ;
                }
                else if ( TITLEALIGN == Qt::AlignHCenter )
                {
                    dx = tx + ( tw / 2 ) - ( ( captionWidth + ( MARGIN * 2 ) ) / 2 ) + ( isActive() ? ACTIVESHDTEXTX : INACTIVESHDTEXTX ) ;
                    dy = ( TITLESIZE / 2 ) - ( captionHeight / 2 ) + ( isActive() ? ACTIVESHDTEXTY : INACTIVESHDTEXTY ) ;
                }
                else if ( TITLEALIGN == Qt::AlignRight )
                {
                    dx = tx + tw - ( captionWidth + ( MARGIN * 2 ) ) - 1 + ( isActive() ? ACTIVESHDTEXTX : INACTIVESHDTEXTX ) ;
                    dy = ( TITLESIZE / 2 ) - ( captionHeight / 2 ) + ( isActive() ? ACTIVESHDTEXTY : INACTIVESHDTEXTY ) ;
                }

                painter2.drawImage( dx, dy, isActive() ? activeShadowImg_ : inActiveShadowImg_, 0, 0, isActive() ? qMin( activeShadowImg_.width(), titleR.width() ) : qMin( inActiveShadowImg_.width(), titleR.width() ), isActive() ? activeShadowImg_.height() : inActiveShadowImg_.height() );
            }


            //draw titleR text
            painter2.setFont( options() ->font( isActive(), FALSE ) );
            painter2.setPen( options() ->color( KDecoration::ColorFont, isActive() ) );

            Qt::Alignment titleAlignBak = TITLEALIGN;
            if ( captionWidth > titleR.width() )
                titleAlignBak = Qt::AlignLeft;

            painter2.drawText( tx + MARGIN, ty, tw - ( MARGIN * 2 ), th, titleAlignBak | Qt::AlignVCenter, caption() );
        }
        painter2.end();
        painter.drawPixmap( 0, 0, pix );


        // draw frames
        if ( ! isShade() || SHOWBTMBORDER )
        {

            if ( h - TITLESIZE - BUTTOMFRAMESIZE > 0 )
            {
                int leftRightFramesHeight = h - TITLESIZE - BUTTOMFRAMESIZE ;

                //left frame
                //top
                rect.setRect( 0, TITLESIZE , LEFTFRAMESIZE, qMin( TOPLEFTFRAMEHEIGHT, leftRightFramesHeight ) );

                painter.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ topLeftFrame ] ) : *( DECOPIXINACTARR[ topLeftFrame ] ) );

                // mid
                if ( leftRightFramesHeight > TOPLEFTFRAMEHEIGHT + BOTTOMLEFTFRAMEHEIGHT )
                {
                    rect.setRect( 0, TITLESIZE + TOPLEFTFRAMEHEIGHT , LEFTFRAMESIZE, leftRightFramesHeight - TOPLEFTFRAMEHEIGHT - BOTTOMLEFTFRAMEHEIGHT );

                    painter.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ midLeftFrame ] ) : *( DECOPIXINACTARR[ midLeftFrame ] ) );

                    //                 painter.drawTiledPixmap( rect, isActive() ? decoFactory_->midLeftFrameBg_ : decoFactory_->midLeftFrameBgInAct_ );
                }

                // buttom
                if ( leftRightFramesHeight > TOPLEFTFRAMEHEIGHT )
                {
                    rect.setRect( 0, qMax( h - BUTTOMFRAMESIZE - BOTTOMLEFTFRAMEHEIGHT, TITLESIZE + TOPLEFTFRAMEHEIGHT ) , LEFTFRAMESIZE, qMin( BOTTOMLEFTFRAMEHEIGHT, leftRightFramesHeight - TOPLEFTFRAMEHEIGHT ) );

                    painter.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? *( DECOPIXACTARR[ buttomLeftFrame ] ) : *( DECOPIXINACTARR[ buttomLeftFrame ] ), 0, ( leftRightFramesHeight > TOPLEFTFRAMEHEIGHT + BOTTOMLEFTFRAMEHEIGHT ) ? 0 : TITLESIZE + TOPLEFTFRAMEHEIGHT - ( h - BUTTOMFRAMESIZE - BOTTOMLEFTFRAMEHEIGHT ) );

                    //                 painter.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? decoFactory_->buttomLeftFrameBg_ : decoFactory_->buttomLeftFrameBgInAct_, 0, ( leftRightFramesHeight > TOPLEFTFRAMEHEIGHT + BOTTOMLEFTFRAMEHEIGHT ) ? 0 : TITLESIZE + TOPLEFTFRAMEHEIGHT - ( h - BUTTOMFRAMESIZE - BOTTOMLEFTFRAMEHEIGHT ) );
                }


                //rightFrame
                // top
                rect.setRect( w - RIGHTFRAMESIZE, TITLESIZE , RIGHTFRAMESIZE, qMin( TOPRIGHTFRAMEHEIGHT, leftRightFramesHeight ) );

                painter.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ topRightFrame ] ) : *( DECOPIXINACTARR[ topRightFrame ] ) );

                //             painter.drawTiledPixmap( rect, isActive() ? decoFactory_->topRightFrameBg_ : decoFactory_->topRightFrameBgInAct_ );

                // mid
                if ( leftRightFramesHeight > TOPRIGHTFRAMEHEIGHT + BOTTOMRIGHTFRAMEHEIGHT )
                {
                    rect.setRect( w - RIGHTFRAMESIZE, TITLESIZE + TOPRIGHTFRAMEHEIGHT, RIGHTFRAMESIZE, leftRightFramesHeight - TOPRIGHTFRAMEHEIGHT - BOTTOMRIGHTFRAMEHEIGHT );

                    painter.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ midRightFrame ] ) : *( DECOPIXINACTARR[ midRightFrame ] ) );

                    //                 painter.drawTiledPixmap( rect, isActive() ? decoFactory_->midRightFrameBg_ : decoFactory_->rightFrameBgInAct_ );
                }

                // bottom
                if ( leftRightFramesHeight > TOPRIGHTFRAMEHEIGHT )
                {
                    rect.setRect( w - RIGHTFRAMESIZE, qMax( h - BUTTOMFRAMESIZE - BOTTOMRIGHTFRAMEHEIGHT, TITLESIZE + TOPRIGHTFRAMEHEIGHT ) , RIGHTFRAMESIZE, qMin( BOTTOMRIGHTFRAMEHEIGHT, leftRightFramesHeight - TOPRIGHTFRAMEHEIGHT ) );

                    painter.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? *( DECOPIXACTARR[ buttomRightFrame ] ) : *( DECOPIXINACTARR[ buttomRightFrame ] ), 0, ( leftRightFramesHeight > TOPRIGHTFRAMEHEIGHT + BOTTOMRIGHTFRAMEHEIGHT ) ? 0 : TITLESIZE + TOPRIGHTFRAMEHEIGHT - ( h - BUTTOMFRAMESIZE - BOTTOMRIGHTFRAMEHEIGHT ) );

                    //                 painter.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? decoFactory_->buttomRightFrameBg_ : decoFactory_->buttomRightFrameBgInAct_, 0, ( leftRightFramesHeight > TOPRIGHTFRAMEHEIGHT + BOTTOMRIGHTFRAMEHEIGHT ) ? 0 : TITLESIZE + TOPRIGHTFRAMEHEIGHT - ( h - BUTTOMFRAMESIZE - BOTTOMRIGHTFRAMEHEIGHT ) );
                }
            }


            // buttom frame
            if ( w > 0 )
            {            // left
                rect.setRect( 0 , h - BUTTOMFRAMESIZE, qMin( LEFTBOTTOMFRAMEWIDTH, w ) , BUTTOMFRAMESIZE );

                painter.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ leftButtomFrame ] ) : *( DECOPIXINACTARR[ leftButtomFrame ] ) );

                // mid
                if ( w > LEFTBOTTOMFRAMEWIDTH + RIGHTBOTTOMFRAMEWIDTH )
                {
                    rect.setRect( LEFTBOTTOMFRAMEWIDTH , h - BUTTOMFRAMESIZE, w - LEFTBOTTOMFRAMEWIDTH - RIGHTBOTTOMFRAMEWIDTH, BUTTOMFRAMESIZE );

                    painter.drawTiledPixmap( rect, isActive() ? *( DECOPIXACTARR[ midButtomFrame ] ) : *( DECOPIXINACTARR[ midButtomFrame ] ) );

                    //                 painter.drawTiledPixmap( rect, isActive() ? decoFactory_->midButtomFrameBg_ : decoFactory_->midButtomFrameBgInAct_ );
                }

                // right
                if ( w > LEFTBOTTOMFRAMEWIDTH )
                {
                    rect.setRect( qMax( w - RIGHTBOTTOMFRAMEWIDTH, LEFTBOTTOMFRAMEWIDTH ) , h - BUTTOMFRAMESIZE, qMin( RIGHTBOTTOMFRAMEWIDTH, w - LEFTBOTTOMFRAMEWIDTH ) , BUTTOMFRAMESIZE );

                    painter.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? *( DECOPIXACTARR[ rightButtomFrame ] ) : *( DECOPIXINACTARR[ rightButtomFrame ] ), ( w > LEFTBOTTOMFRAMEWIDTH + RIGHTBOTTOMFRAMEWIDTH ) ? 0 : LEFTBOTTOMFRAMEWIDTH - ( w - RIGHTBOTTOMFRAMEWIDTH ), 0 );

                    //                 painter.drawTiledPixmap( rect.x(), rect.y(), rect.width(), rect.height(), isActive() ? decoFactory_->rightButtomFrameBg_ : decoFactory_->rightButtomFrameBgInAct_, ( w > LEFTBOTTOMFRAMEWIDTH + RIGHTBOTTOMFRAMEWIDTH ) ? 0 : LEFTBOTTOMFRAMEWIDTH - ( w - RIGHTBOTTOMFRAMEWIDTH ), 0 );
                }
            }
        }
        else
        {
            painter.setPen( QColor( 70, 70, 70 ) );
            painter.drawLine( 0 , h - 1, w, h - 1 );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// update_captionBuffer()
// ------------
//
void DeKoratorClient::updateCaptionBuffer()
{
    if ( !DeKoratorFactory::initialized() )
        return ;

    QPainter painter;
    QString c( caption() );
    QFontMetrics fm( options() ->font( isActive() ) );
    int captionWidth = fm.width( c );
    int captionHeight = fm.height( );
    QPixmap textPixmap;

    // prepare the shadow
    textPixmap = QPixmap( captionWidth + ( MARGIN * 2 ) , captionHeight ); // 2*2 px shadow space
    textPixmap.fill( QColor( 0, 0, 0 ) );
    textPixmap.setMask( textPixmap.createHeuristicMask( TRUE ) );
    painter.begin( &textPixmap );
    painter.setFont( options() ->font( isActive(), FALSE ) );
    painter.setPen( Qt::white );
    painter.drawText( textPixmap.rect(), Qt::AlignCenter, caption() );
    painter.end();


    ShadowEngine se;
    //if ( isActive() )
    activeShadowImg_ = se.makeShadow( textPixmap, ACTIVESHADECOLOR );
    //else
    inActiveShadowImg_ = se.makeShadow( textPixmap, INACTIVESHADECOLOR );

    captionBufferDirty_ = false;
}

//////////////////////////////////////////////////////////////////////////////
// resizeEvent()
// -------------
// Window is being resized
void DeKoratorClient::resizeEvent( QResizeEvent *e )
{
    if ( widget() ->isShown() )
    {
        QRegion region = widget() ->rect();
        region = region.subtract( titleBarSpacer_->geometry() );
        widget() ->erase( region );
    }
    if ( USEMASKS )
    {

        if ( oldSize_ != e->size() )
        {
            //qWarning("QResizeEvent");
            sizeChanged = true;
        }
        else
            sizeChanged = false;

        oldSize_ = e->size();

        // 		if(e->type() == QEvent::Resize)
        // 		{
        //
        // 			//oldSize((QResizeEvent*)e)->size());
        // 			qWarning( "resizeEvent" );
        // 		}

        //        maskDirty_ = true;
        doShape();
    }
}

//////////////////////////////////////////////////////////////////////////////
// showEvent()
// -----------
// Window is being shown
void DeKoratorClient::showEvent( QShowEvent * )
{
    widget() ->repaint();
    if ( USEMASKS )
        doShape();
}

//////////////////////////////////////////////////////////////////////////////
// maxButtonPressed()
// -----------------
// Max button was pressed
void DeKoratorClient::maxButtonPressed()
{
    if ( button[ ButtonMax ] )
    {
#if KDE_IS_VERSION(3, 3, 0)
        maximize( button[ ButtonMax ] ->lastMousePress() );
#else

        switch ( button[ ButtonMax ] ->lastMousePress() )
        {
        case Qt::MidButton:
            maximize( maximizeMode() ^ MaximizeVertical );
            break;
        case Qt::RightButton:
            maximize( maximizeMode() ^ MaximizeHorizontal );
            break;
        default:
            ( maximizeMode() == MaximizeFull ) ? maximize( MaximizeRestore )
            : maximize( MaximizeFull );
        }
#endif

    }
}

//////////////////////////////////////////////////////////////////////////////
// shadeButtonPressed()
// -----------------
// Shade button was pressed
void DeKoratorClient::shadeButtonPressed()
{
    if ( button[ ButtonShade ] )
    {
        setShade( !isSetShade() );
    }
}

//////////////////////////////////////////////////////////////////////////////
// aboveButtonPressed()
// -----------------
// Above button was pressed
void DeKoratorClient::aboveButtonPressed()
{
    if ( button[ ButtonAbove ] )
    {
        setKeepAbove( !keepAbove() );
    }
}

//////////////////////////////////////////////////////////////////////////////
// belowButtonPressed()
// -----------------
// Below buttQt::green; //on was pressed
void DeKoratorClient::belowButtonPressed()
{
    if ( button[ ButtonBelow ] )
    {
        setKeepBelow( !keepBelow() );
    }
}

//////////////////////////////////////////////////////////////////////////////
// menuButtonPressed()
// -------------------
// Menu button was pressed (popup the menu)
void DeKoratorClient::menuButtonPressed()
{
    //     if ( button[ ButtonMenu ] )
    //     {
    //         QPoint p( button[ ButtonMenu ] ->rect().bottomLeft().x(),
    //                   button[ ButtonMenu ] ->rect().bottomLeft().y() );
    //         KDecorationFactory* f = factory();
    //         showWindowMenu( button[ ButtonMenu ] ->mapToGlobal( p ) );
    //         if ( !f->exists( this ) ) return ; // decoration was destroyed
    //         button[ ButtonMenu ] ->setDown( FALSE );
    //     }

    static QTime * t = NULL;
    static DeKoratorClient* lastClient = NULL;
    if ( t == NULL )
        t = new QTime;
    bool dbl = ( lastClient == this && t->elapsed() <= QApplication::doubleClickInterval() );
    lastClient = this;
    t->start();
    if ( !dbl || !DBLCLKCLOSE )
    {
        QPoint p( button[ ButtonMenu ] ->rect().bottomLeft().x(),
                  button[ ButtonMenu ] ->rect().bottomLeft().y() );
        KDecorationFactory* f = factory();
        showWindowMenu( button[ ButtonMenu ] ->mapToGlobal( p ) );
        if ( !f->exists( this ) )
            return ; // decoration was destroyed
        button[ ButtonMenu ] ->setDown( FALSE );
    }
    else
        closing_ = true;
}

//////////////////////////////////////////////////////////////////////////////
// menuButtonReleased()
// -------------------
//
void DeKoratorClient::menuButtonReleased()
{
    if ( closing_ )
        closeWindow();
}

//////////////////////////////////////////////////////////////////////////////
// doShape()
// -------------------
//
void DeKoratorClient::doShape()
{
    int w = width();
    int h = height();
    QRegion mask( 0, 0, w, h );

    if ( sizeChanged )
    {
        //qWarning("doShape");

        // top left
        QRegion mtr;
        QRegion m = QRegion( decoFactory_->topLeftCornerBitmap_ );
        mask -= QRegion( m );


        // top mid
        if ( TOPMIDMASKWIDTH >= 1 )
        {
            int pos = TOPLEFTMASKWIDTH;
            int rep = ( w - TOPLEFTMASKWIDTH - TOPRIGHTMASKWIDTH ) / TOPMIDMASKWIDTH;
            m = QRegion( decoFactory_->topMidBitmap_ );
            QRegion mBak = m;

            for ( int i = 0 ; i < rep ; i++ )
            {
                m = mBak;
                m.translate( TOPLEFTMASKWIDTH + ( i * TOPMIDMASKWIDTH ), 0 );
                mask -= QRegion( m );
                pos += TOPMIDMASKWIDTH;
            }

            m = mBak;
            mtr = mBak;
            mtr.translate( w - pos - TOPRIGHTMASKWIDTH, 0 );
            m -= mtr;
            m.translate( pos, 0 );
            mask -= m;
        }

        {
            //top right
            m = QRegion( decoFactory_->topRightCornerBitmap_ );
            m.translate( width() - TOPRIGHTMASKWIDTH, 0 );
            mask -= QRegion( m );
        }


        if ( !isShade() || SHOWBTMBORDER )
        {
            //buttom left
            m = QRegion( decoFactory_->buttomLeftCornerBitmap_ );
            m.translate( 0, h - BOTTOMLEFTMASKHEIGHT );
            mask -= QRegion( m );

            // bottom mid
            if ( BOTTOMMIDMASKWIDTH >= 1 )
            {
                int pos = BOTTOMLEFTMASKWIDTH;
                int rep = ( w - BOTTOMLEFTMASKWIDTH - BOTTOMRIGHTMASKWIDTH ) / BOTTOMMIDMASKWIDTH;
                int hm = h - BOTTOMMIDMASKHEIGHT;
                m = QRegion( decoFactory_->buttomMidBitmap_ );
                QRegion mBak = m;

                for ( int i = 0 ; i < rep ; i++ )
                {
                    m = mBak;
                    m.translate( BOTTOMLEFTMASKWIDTH + ( i * BOTTOMMIDMASKWIDTH ), hm );
                    mask -= QRegion( m );
                    pos += BOTTOMMIDMASKWIDTH;
                }

                m = mBak;
                mtr = mBak;
                mtr.translate( w - pos - BOTTOMRIGHTMASKWIDTH, 0 );
                m -= mtr;
                m.translate( pos, hm );
                mask -= m;
            }

            //buttom right
            m = QRegion( decoFactory_->buttomRightCornerBitmap_ );
            m.translate( width() - BOTTOMRIGHTMASKWIDTH, h - BOTTOMRIGHTMASKHEIGHT );
            mask -= QRegion( m );
        }
        mask_ = mask;
    }

    setMask( mask_ );
}


#include "deKoratorclient.moc"
