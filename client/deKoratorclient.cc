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

#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KIconEffect>
#include <KDE/KLocale>

#include <QtCore/QSettings>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QVBoxLayout>

#include <qimageblitz.h>

#include "shadow.h"

using namespace DeKorator;


// global constants

static int MARGIN = 4;
//static int GRUBMARGIN = 5;

static QColor STYLEBGCOL;

// frames
static int TITLESIZE = 0;
static int LEFTFRAMESIZE = 0;
static int BOTTOMFRAMESIZE = 0;
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

static QSize buttonSize[ButtonTypeCount];

static const char * const buttonGlyphName[] =
{
    "Restore", "Help", "Max", "Min", "Close",
    "Sticky", "StickyDown", "Above", "AboveDown",
    "Below", "BelowDown", "Shade", "ShadeDown",
    "Menu", "TabClose"
};


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
static bool changeCursorOverButtons = false;
static bool USEMENUEIMAGE = false;
static bool IGNOREAPPICNCOL = false;
static bool DBLCLKCLOSE = false;
static bool SHOWBTMBORDER = false;
static bool showMaximizedBorders = false;
static bool USESHDTEXT = false;
static int ACTIVESHDTEXTX = 0;
static int ACTIVESHDTEXTY = 0;
static QColor ACTIVESHADECOLOR = QColor( 150, 150, 150 );
static int INACTIVESHDTEXTX = 0;
static int INACTIVESHDTEXTY = 0;
static QColor INACTIVESHADECOLOR = QColor( 150, 150, 150 );
static int BTNSHIFTX = 0;
static int BTNSHIFTY = 0;


// colors

static bool USEANIMATION = true;
static QString ANIMATIONTYPE = "Intensity";

static int STEPS = 5;
static int INTERVAL = 5;
static bool KEEPANIMATING = false;

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
static bool USEMASKS = false;

// pix arrays
static QPixmap decoPixmap[ decoCount ][ WindowActivationStateCount ];
static QImage buttonStateImage[ buttonTypeAllCount ][ buttonStateCount ][ WindowActivationStateCount ];

//////////////////////////////////////////////////////////////////////////////
// DeKoratorFactory Class                                                     //
//////////////////////////////////////////////////////////////////////////////

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
    colorizeActFrames_ = false;
    colorizeActButtons_ = false;
    colorizeInActFrames_ = false;
    colorizeInActButtons_ = false;

    useCustomButtonsColors_ = false;
    customColorsActiveButtons_ = false;
    customColorsInActiveButtons_ = false;

    needInit_ = false;
    needReload_ = false;

    readConfig();

    loadPixmaps();

    chooseRightPixmaps();
}

//////////////////////////////////////////////////////////////////////////////
// ~DeKoratorFactory()
// -----------------
// Destructor
DeKoratorFactory::~DeKoratorFactory()
{
}

//////////////////////////////////////////////////////////////////////////////
// supports()
// -----------------
// inform kwin which features our decoration supports
bool DeKoratorFactory::supports( KDecorationDefines::Ability ability ) const
{
    switch (ability)
    {
        // announce
        case AbilityAnnounceButtons:
        case AbilityAnnounceColors:
        // buttons
        case AbilityButtonMenu:
        case AbilityButtonOnAllDesktops:
        case AbilityButtonSpacer:
        case AbilityButtonHelp:
        case AbilityButtonMinimize:
        case AbilityButtonMaximize:
        case AbilityButtonClose:
        case AbilityButtonAboveOthers:
        case AbilityButtonBelowOthers:
        case AbilityButtonShade:
        case AbilityButtonResize:
        // colors
        case AbilityColorTitleBack:
        case AbilityColorTitleFore:
        case AbilityColorTitleBlend:
        case AbilityColorFrame:
        case AbilityColorHandle:
        case AbilityColorButtonBack:
        case AbilityColorButtonFore:
            return true;
        default:
            return false;
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
// Reset the handler. Returns true if decorations need to be remade, false if
// only a repaint is necessary
bool DeKoratorFactory::reset( unsigned long changed )
{
    //    captionBufferDirty_ = true;
    // read in the configuration
    bool confchange = readConfig();

    if ( confchange || ( changed & ( SettingDecoration | SettingButtons | SettingBorder | SettingColors ) ) )
    {
        if ( DeKoratorFactory::needReload_ )
        {
            loadPixmaps();
        }
        chooseRightPixmaps();
        return true;
    }
    else
    {
        resetDecorations( changed );
        return false;
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

    bool oldChangeCursorOverButtons = changeCursorOverButtons;
    changeCursorOverButtons = config.readEntry( "ChangeCursorOverButtons", false );

    bool oldUseMenuImage = USEMENUEIMAGE;
    USEMENUEIMAGE = config.readEntry( "UseMenuImage", false );

    bool oldIgnoreAppIcnCol = IGNOREAPPICNCOL;
    IGNOREAPPICNCOL = config.readEntry( "IgnoreAppIconCol", false );

//    bool oldDblClkClose = DBLCLKCLOSE;
    DBLCLKCLOSE = config.readEntry( "DblClkClose", false );

    bool oldShowBtmBorder = SHOWBTMBORDER;
    SHOWBTMBORDER = config.readEntry( "ShowBtmBorder", false );

    bool oldShowMaximizedBorders = showMaximizedBorders;
    showMaximizedBorders = config.readEntry( "ShowMaximizedBorders", false );

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
    for ( int i = 0 ; i < buttonTypeAllCount ; ++i )
    {
        colArr[ i ] = DeKoratorFactory::cusBtnCol_[ i ];
        DeKoratorFactory::cusBtnCol_[ i ] = config.readEntry( QLatin1String( buttonGlyphName[ i ] ) + "ButtonColor", col );
    }

    bool cusColChanged = false;
    for ( int i = 0 ; i < buttonTypeAllCount ; ++i )
    {
        if ( colArr[ i ] != DeKoratorFactory::cusBtnCol_[ i ] )
        {
            cusColChanged = true;
            i = buttonTypeAllCount;
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


    //style background
    QColor oldStyleBgCol = STYLEBGCOL;

    QPalette palette;
    STYLEBGCOL = palette.color(QPalette::Window);



    if ( oldalign == TITLEALIGN &&
            oldChangeCursorOverButtons == changeCursorOverButtons &&
            oldUseMenuImage == USEMENUEIMAGE &&
            oldIgnoreAppIcnCol == IGNOREAPPICNCOL &&
            oldShowBtmBorder == SHOWBTMBORDER &&
            oldShowMaximizedBorders == showMaximizedBorders &&
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
        return false;
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
        //             DeKoratorFactory::needInit_ = true;
        //         else
        //             DeKoratorFactory::needInit_ = false;

        if ( oldFramesPath != DeKoratorFactory::framesPath_ ||
                oldButtonsPath != DeKoratorFactory::buttonsPath_ ||
                oldMasksPath != DeKoratorFactory::masksPath_
           )
            DeKoratorFactory::needReload_ = true;
        else
            DeKoratorFactory::needReload_ = false;

        return true;
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

    // deco
    static const char * const decoPixName[] =
    {
        "topLeftCorner", "leftButtons", "leftTitle", "midTitle", "rightTitle", "rightButtons", "topRightCorner",
        "topLeftFrame", "midLeftFrame", "bottomLeftFrame",
        "leftBottomFrame", "midBottomFrame", "rightBottomFrame",
        "topRightFrame", "midRightFrame", "bottomRightFrame"
    };

    for ( int i = 0; i < decoCount; ++i )
    {
        decoImage[ i ][ WindowActive ][ ImageOriginal ] = QImage( decoPixDir + '/' + decoPixName[i] + "Bg.png" );
    }

    // 0.1 compatibility
    if ( decoImage[ midLeftFrame ][ WindowActive ][ ImageOriginal ].isNull() )
        decoImage[ midLeftFrame ][ WindowActive ][ ImageOriginal ].load( decoPixDir + "/leftFrameBg.png" );
    if ( decoImage[ midRightFrame ][ WindowActive ][ ImageOriginal ].isNull() )
        decoImage[ midRightFrame ][ WindowActive ][ ImageOriginal ].load( decoPixDir + "/rightFrameBg.png" );
    if ( decoImage[ midBottomFrame ][ WindowActive ][ ImageOriginal ].isNull() )
    {
        decoImage[ leftBottomFrame ][ WindowActive ][ ImageOriginal ].load( decoPixDir + "/buttomLeftCornerBg.png" );
        decoImage[ midBottomFrame ][ WindowActive ][ ImageOriginal ].load( decoPixDir + "/buttomFrameBg.png" );
        decoImage[ rightBottomFrame ][ WindowActive ][ ImageOriginal ].load( decoPixDir + "/buttomRightCornerBg.png" );
    }

    for ( int i = 0; i < decoCount; ++i )
    {
        decoImage[ i ][ WindowInactive ][ ImageOriginal ] = QImage( decoPixDir + "/inactive/" + decoPixName[i] + "Bg.png" );
        if ( decoImage[ i ][ WindowInactive ][ ImageOriginal ].isNull() )
        {
            decoImage[ i ][ WindowInactive ][ ImageOriginal ] = decoImage[ i ][ WindowActive ][ ImageOriginal ];
        }
    }

    // buttons
    static const char * const buttonStateName[] =
    {
        "", "Hover", "Press"
    };

    static const char * const buttonStatePath[] =
    {
        "/normal/button", "/hover/button", "/press/button"
    };

    for ( int i = 0; i < buttonTypeAllCount; ++i )
    {
        QImage buttonsImage( btnPixDir + "/buttons" + buttonGlyphName[i] + ".png");

        if ( !buttonsImage.isNull() && (buttonsImage.width() % 6) == 0 )
        {
            // we got 6 images (active normal/hover/press, inactive normal/hover/press)
            int buttonWidth = buttonsImage.width() / 6;
            for ( int j = 0; j < buttonStateCount; ++j )
            {
                buttonImage[ i ][ j ][ WindowActive ][ ImageOriginal ] = buttonsImage.copy( j * buttonWidth, 0, buttonWidth, buttonsImage.height() );
                buttonImage[ i ][ j ][ WindowInactive ][ ImageOriginal ] = buttonsImage.copy( ( j + 3 ) * buttonWidth, 0, buttonWidth, buttonsImage.height() );
            }
        }
        else
        // 0.2 compatibility
        {
            for ( int j = 0; j < buttonStateCount; ++j )
            {
                buttonImage[ i ][ j ][ WindowActive ][ ImageOriginal ] = QImage( btnPixDir + buttonStatePath[j] + buttonGlyphName[i] + buttonStateName[j] + ".png" );
                buttonImage[ i ][ j ][ WindowInactive ][ ImageOriginal ] = QImage();
            }
        }

        // 0.1 compatibility
        if ( buttonImage[ i ][ regular][ WindowActive ][ ImageOriginal ].isNull() )
        {
            buttonImage[ i ][ regular ][ WindowActive ][ ImageOriginal ].load( btnPixDir + "/button" + buttonGlyphName[i] + ".png" );
            if (buttonImage[ i ][ regular ][ WindowActive ][ ImageOriginal ].isNull() )
            {
                if (i == 0)
                {
                    for ( int j = 0; j < buttonStateCount; ++j )
                    {
                        buttonImage[ i ][ j ][ WindowActive ][ ImageOriginal ] = QImage( btnPixDir + buttonStatePath[j] + buttonGlyphName[2] + buttonStateName[j] + ".png" );
                    }
                    if ( buttonImage[ i ][ regular][ WindowActive ][ ImageOriginal ].isNull() )
                    {
                        buttonImage[ i ][ regular ][ WindowActive ][ ImageOriginal ].load( btnPixDir + "/button" + buttonGlyphName[2] + ".png" );
                    }
                }
            }
            for ( int j = hover; j < buttonStateCount; ++j )
            {
                buttonImage[ i ][ j ][ WindowActive ][ ImageOriginal ] = buttonImage[ i ][ regular ][ WindowActive ][ ImageOriginal ];
            }
        }

        for ( int j = 0; j < buttonStateCount; ++j )
        {
            if ( buttonImage[ i ][ j ][ WindowInactive ][ ImageOriginal ].isNull() )
            {
                buttonImage[ i ][ j ][ WindowInactive ][ ImageOriginal ] = buttonImage[ i ][ j ][ WindowActive ][ ImageOriginal ];
            }
        }
    }

    //masks
    topLeftCornerBitmap_ = QBitmap( masksPixDir + "/topLeftCornerBitmap.png" );
    topMidBitmap_ = QBitmap( masksPixDir + "/topMidBitmap.png" );
    topRightCornerBitmap_ = QBitmap( masksPixDir + "/topRightCornerBitmap.png" );
    bottomLeftCornerBitmap_ = QBitmap( masksPixDir + "/bottomLeftCornerBitmap.png" );
    bottomMidBitmap_ = QBitmap( masksPixDir + "/bottomMidBitmap.png" );
    bottomRightCornerBitmap_ = QBitmap( masksPixDir + "/bottomRightCornerBitmap.png" );

    // 0.2 compatibility
    if ( bottomLeftCornerBitmap_.isNull() )
        bottomLeftCornerBitmap_ = QBitmap( masksPixDir + "/buttomLeftCornerBitmap.png" );
    if ( bottomMidBitmap_.isNull() )
        bottomMidBitmap_ = QBitmap( masksPixDir + "/buttomMidBitmap.png" );
    if ( bottomRightCornerBitmap_.isNull() )
        bottomRightCornerBitmap_ = QBitmap( masksPixDir + "/buttomRightCornerBitmap.png" );

    determineSizes();
}

//////////////////////////////////////////////////////////////////////////////
// determineSizes()
// ------------
//
void DeKoratorFactory::determineSizes()
{
    //MARGIN = midBottomFrameBg_.height();

    // frames
    TITLESIZE = decoImage[ midTitle ][ WindowActive ][ ImageOriginal ].height();
    LEFTFRAMESIZE = decoImage[ midLeftFrame ][ WindowActive ][ ImageOriginal ].width();
    BOTTOMFRAMESIZE = decoImage[ midBottomFrame ][ WindowActive ][ ImageOriginal ].height();
    RIGHTFRAMESIZE = decoImage[ midRightFrame ][ WindowActive ][ ImageOriginal ].width();

    TOPLEFTCORNERWIDTH = decoImage[ topLeftCorner ][ WindowActive ][ ImageOriginal ].width();
    TOPRIGHTCORNERWIDTH = decoImage[ topRightCorner ][ WindowActive ][ ImageOriginal ].width();
    LEFTTITLEWIDTH = decoImage[ leftTitle ][ WindowActive ][ ImageOriginal ].width();
    RIGHTTITLEWIDTH = decoImage[ rightTitle ][ WindowActive ][ ImageOriginal ].width();
    TOPLEFTFRAMEHEIGHT = decoImage[ topLeftFrame ][ WindowActive ][ ImageOriginal ].height();
    BOTTOMLEFTFRAMEHEIGHT = decoImage[ bottomLeftFrame ][ WindowActive ][ ImageOriginal ].height();
    TOPRIGHTFRAMEHEIGHT = decoImage[ topRightFrame ][ WindowActive ][ ImageOriginal ].height();
    BOTTOMRIGHTFRAMEHEIGHT = decoImage[ bottomRightFrame ][ WindowActive ][ ImageOriginal ].height();
    LEFTBOTTOMFRAMEWIDTH = decoImage[ leftBottomFrame ][ WindowActive ][ ImageOriginal ].width();
    RIGHTBOTTOMFRAMEWIDTH = decoImage[ rightBottomFrame ][ WindowActive ][ ImageOriginal ].width();


    // buttons
    BUTTONSHEIGHT = TITLESIZE;

    buttonSize[ ButtonHelp ] = buttonImage[ help ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonMax ] = buttonImage[ max ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonMin ] = buttonImage[ min ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonClose ] = buttonImage[ close ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonMenu ] = buttonImage[ menu ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonSticky ] = buttonImage[ sticky ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonAbove ] = buttonImage[ above ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonBelow ] = buttonImage[ below ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonShade ] = buttonImage[ shade ][ regular ][ WindowActive ][ ImageOriginal ].size();
    buttonSize[ ButtonTabClose ] = buttonImage[ tabClose ][ regular ][ WindowActive ][ ImageOriginal ].size();

    // masks
    TOPLEFTMASKWIDTH = topLeftCornerBitmap_.width();
    TOPMIDMASKWIDTH = topMidBitmap_.width();
    TOPRIGHTMASKWIDTH = topRightCornerBitmap_.width();
    BOTTOMLEFTMASKWIDTH = bottomLeftCornerBitmap_.width();
    BOTTOMMIDMASKWIDTH = bottomMidBitmap_.width();
    BOTTOMRIGHTMASKWIDTH = bottomRightCornerBitmap_.width();

    BOTTOMLEFTMASKHEIGHT = bottomLeftCornerBitmap_.height();
    BOTTOMMIDMASKHEIGHT = bottomMidBitmap_.height();
    BOTTOMRIGHTMASKHEIGHT = bottomRightCornerBitmap_.height();

}


//////////////////////////////////////////////////////////////////////////////
// colorizeDecoPixmaps()
// ------------
//
void DeKoratorFactory::colorizeDecoPixmaps( bool isActive )
{
    QColor col = options() ->palette( KDecoration::ColorTitleBar, isActive ).background().color();
    WindowActivationState wa = isActive ? WindowActive : WindowInactive;

    for ( int i = 0; i < decoCount; ++i )
    {
        decoImage[ i ][ wa ][ ImageRecolored ] = colorizedImage( decoImage[ i ][ wa ][ ImageOriginal ], col, DECOCOLORIZE );
    }
}

//////////////////////////////////////////////////////////////////////////////
// colorizeButtonsPixmaps()
// ------------
//
void DeKoratorFactory::colorizeButtonsPixmaps( bool isActive )
{
    QColor col = options() ->palette( KDecoration::ColorButtonBg, isActive ).background().color();
    WindowActivationState wa = isActive ? WindowActive : WindowInactive;

    bool customColors = useCustomButtonsColors_ && ( isActive ? customColorsActiveButtons_ : customColorsInActiveButtons_ );

    for ( int i = 0; i < buttonTypeAllCount; ++i )
    {
        for ( int j = 0; j < buttonStateCount; ++j )
        {
            buttonImage[ i ][ j ][ wa ][ ImageRecolored ] = colorizedImage( buttonImage[ i ][ j ][ wa ][ ImageOriginal ], customColors ? cusBtnCol_[ i ] : col, BUTTONSCOLORIZE );
        }
    }
}


//////////////////////////////////////////////////////////////////////////////
// colorizedImage()
// ------------
//
QImage DeKoratorFactory::colorizedImage( const QImage &image, QColor c, QString colorizeMethod )
{
    if ( colorizeMethod == "Liquid Method" )
    {
        QImage img = image;

        if ( img.depth() != 32 )
            img = img.convertToFormat( QImage::Format_ARGB32 );
        QImage dest( img.width(), img.height(), QImage::Format_ARGB32 );
        unsigned int *data = ( unsigned int * ) img.bits();
        unsigned int *destData = ( unsigned int* ) dest.bits();
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
        return dest;
    }
    else if ( colorizeMethod == "KDE Method" )
    {
        QImage img = image;
        KIconEffect::colorize( img, c, 1.0 );
        return img;
    }
    else if ( colorizeMethod == "Hue Adjustment" )
    {
        QImage img = image;

        if ( img.depth() != 32 )
            img = img.convertToFormat( QImage::Format_ARGB32 );

        QImage dest( img.width(), img.height(), QImage::Format_ARGB32 );
        unsigned int *data = ( unsigned int * ) img.bits();
        unsigned int *destData = ( unsigned int* ) dest.bits();
        int total = img.width() * img.height();
        int current;
        int destR, destG, destB, alpha;
        int h, s, v, ch;
        c.getHsv( &h, &s, &v );
        ch = h;
        QColor col ;

        for ( current = 0; current < total; ++current )
        {
            col.setRgb( data[ current ] );
            col.getHsv( &h, &s, &v );
            col.setHsv( ch, s, v );
            destR = col.red();
            destG = col.green();
            destB = col.blue();
            alpha = qAlpha( data[ current ] );

            destData[ current ] = qRgba( destR, destG, destB, alpha );
        }
        return dest;
    }
    return image;
}

//////////////////////////////////////////////////////////////////////////////
// chooseRightPixmaps()
//
void DeKoratorFactory::chooseRightPixmaps()
{
    int i, j;

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

    for ( i = 0; i < decoCount; ++i )
    {
        decoPixmap[ i ][ WindowActive ] = QPixmap::fromImage( decoImage[ i ][ WindowActive ][ ImageRecolored ] );
        decoPixmap[ i ][ WindowInactive ] = QPixmap::fromImage( decoImage[ i ][ WindowInactive ][ ImageRecolored] );
    }

    for ( i = 0; i < buttonTypeAllCount; ++i )
    {
        for ( j = 0; j < buttonStateCount; ++j )
        {
            buttonStateImage[ i ][ j ][ WindowActive ] = buttonImage[ i ][ j ][ WindowActive ][ DeKoratorFactory::colorizeActButtons_ ? ImageRecolored : ImageOriginal ];
            buttonStateImage[ i ][ j ][ WindowInactive ] = buttonImage[ i ][ j ][ WindowInactive ][ DeKoratorFactory::colorizeInActButtons_ ? ImageRecolored : ImageOriginal ];
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// prepareDecoWithBgCol()
//
void DeKoratorFactory::prepareDecoWithBgCol()
{
    int i;

    for ( i = 0 ; i < decoCount ; ++i )
    {
        decoImage[ i ][ WindowActive ][ ImageRecolored ] = decoImage[ i ][ WindowActive ][ DeKoratorFactory::colorizeActFrames_ ? ImageRecolored : ImageOriginal ];
        decoImage[ i ][ WindowInactive ][ ImageRecolored ] = decoImage[ i ][ WindowInactive ][ DeKoratorFactory::colorizeInActFrames_ ? ImageRecolored : ImageOriginal ];
    }
}






//////////////////////////////////////////////////////////////////////////////
// DeKoratorButton Class                                                      //
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// DeKoratorButton()
// ---------------
// Constructor
DeKoratorButton::DeKoratorButton( DeKoratorClient * parent, const char * name,
                                  const QString & tip, ButtonType type, buttonTypeAll btnType )
        : QAbstractButton( parent->widget() ), buttonWidth_( buttonSize[type].width() ), client_( parent ), type_( type ), lastmouse_( Qt::NoButton ), decoPixHeight_( buttonSize[type].height() )
{
    animProgress = 0;
    hover_ = false;
    setAttribute( Qt::WA_NoSystemBackground, true );
    if (type == ButtonMenu)
        buttonWidth_ = qMax( 16, buttonWidth_ );
    setFixedSize( buttonWidth_, BUTTONSHEIGHT );
    setCursor( Qt::ArrowCursor );
    setObjectName( QString::fromAscii(name) );

    setPixmap( btnType );
    setToolTip( tip );
    animTmr = new QTimer( this );
    animTmr->setSingleShot( true );
    connect( animTmr, SIGNAL( timeout() ), this, SLOT( animate() ) );
}

//////////////////////////////////////////////////////////////////////////////
// DeKoratorButton()
// ---------------
// destructor
DeKoratorButton::~DeKoratorButton()
{
}

//////////////////////////////////////////////////////////////////////////////
// setPixmap()

// -----------
// Set the button decoration
void DeKoratorButton::setPixmap( buttonTypeAll btnType )
{
    btnType_ = btnType;
    update();
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
    QAbstractButton::enterEvent( e );
    s = STEPS;
    hover_ = true;
    if ( changeCursorOverButtons )
        setCursor( Qt::PointingHandCursor );

    if ( USEANIMATION )
        animate();
    else
        update();
}

//////////////////////////////////////////////////////////////////////////////
// leaveEvent()
// ------------
// Mouse has left the button
void DeKoratorButton::leaveEvent( QEvent * e )
{
    // if we wanted to do mouseovers, we would keep track of it here

    QAbstractButton::leaveEvent( e );
    //STEPS = s;
    hover_ = false;
    if ( changeCursorOverButtons )
        unsetCursor ();

    if ( USEANIMATION )
        animate();
    else
        update();
}

//////////////////////////////////////////////////////////////////////////////
// mousePressEvent()
// -----------------
// Button has been pressed
void DeKoratorButton::mousePressEvent( QMouseEvent * e )
{
    lastmouse_ = e->button();

    // translate and pass on mouse event
    Qt::MouseButton button = Qt::LeftButton;
    if ( ( type_ != ButtonMax ) && ( e->button() != Qt::LeftButton ) )
    {
        button = Qt::NoButton; // middle & right buttons inappropriate
    }
    QMouseEvent me( e->type(), e->pos(), e->globalPos(),
                    button, e->buttons(), e->modifiers() );
    QAbstractButton::mousePressEvent( &me );
}

//////////////////////////////////////////////////////////////////////////////
// mouseReleaseEvent()
// -----------------
// Button has been released
void DeKoratorButton::mouseReleaseEvent( QMouseEvent * e )
{
    lastmouse_ = e->button();

    // translate and pass on mouse event
    Qt::MouseButton button = Qt::LeftButton;
    if ( ( type_ != ButtonMax ) && ( e->button() != Qt::LeftButton ) )
    {
        button = Qt::NoButton; // middle & right buttons inappropriate
    }
    QMouseEvent me( e->type(), e->pos(), e->globalPos(), button, e->buttons(), e->modifiers() );
    QAbstractButton::mouseReleaseEvent( &me );
}

//////////////////////////////////////////////////////////////////////////////
// paintEvent()
// ------------
// Draw the button
void DeKoratorButton::paintEvent( QPaintEvent * /*e*/ )
{
    int dx = 0, dy = 0;
    bool act = client_->isActive();
    WindowActivationState waState = act ? WindowActive : WindowInactive;
    QImage image;
    QPainter painter( this );

    // apply app icon effects
    if ( type_ == ButtonMenu && !USEMENUEIMAGE )
    {
        dy = ( height() - 16 ) / 2;

        QPixmap appIconPix = client_->icon().pixmap( 16, QIcon::Normal );
        image = appIconPix.toImage();

        if ( !IGNOREAPPICNCOL )
        {
            DeKoratorFactory *factory = static_cast<DeKoratorFactory *>( client_->factory() );

            if ( factory->useCustomButtonsColors_ && ( act ? factory->customColorsActiveButtons_ : factory->customColorsInActiveButtons_ ) )
            {
                image = DeKoratorFactory::colorizedImage( image, factory->cusBtnCol_[ menu ], BUTTONSCOLORIZE );
            }
            else if ( act ? factory->colorizeActButtons_ : factory->colorizeInActButtons_ )
            {
                QColor col = factory->options() ->palette( KDecoration::ColorButtonBg, act ).color( QPalette::Window );

                image = DeKoratorFactory::colorizedImage( image, col, BUTTONSCOLORIZE );
            }
        }

    }
    else
    {
        dy = ( BUTTONSHEIGHT - decoPixHeight_ ) / 2;

        image = buttonStateImage[ btnType_ ][ isDown() ? press : hover_ ? hover : regular ][ waState ];
    }


    // down
    if ( isDown() )
    {
        dx += BTNSHIFTX;
        dy += BTNSHIFTY;

        image = chooseRightHoverEffect( image, BUTTONHOVERTYPE );
    }
    // hover
    else if ( hover_ )
    {
        image = chooseRightHoverEffect( image, USEANIMATION ? ANIMATIONTYPE : BUTTONHOVERTYPE );
    }
    // regular
    else
    {
        if ( USEANIMATION && animProgress > 0 )
            image = chooseRightHoverEffect( image, ANIMATIONTYPE );
    }

    painter.drawImage( dx, dy, image );


    if ( client_->isShade() && !SHOWBTMBORDER )
    {
        painter.setPen( QColor( 70, 70, 70 ) );
        painter.drawLine( 0, BUTTONSHEIGHT - 1, buttonWidth_ - 1, BUTTONSHEIGHT - 1 );
    }
}

//////////////////////////////////////////////////////////////////////////////
// chooseRightHoverEffect()
// ------------
//
QImage DeKoratorButton::chooseRightHoverEffect( const QImage &image, QString res )
{
    QImage img = image;

    if ( USEANIMATION && !isDown() )
    {
        float f = ( ( float ) animProgress / 100 );

        if ( res == "Intensity" )
        {
            f /= 1.5;

            img = Blitz::intensity ( img, f );
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

            img = Blitz::fade ( img, f, col );
        }
        else if ( res == "Trashed" )
        {
            img = Blitz::fade ( img, animProgress, Qt::black );
        }
        else if ( res == "Hue Shift" )
        {
            int h, s, v;
            QColor col ;
            int shift = ( int ) ( animProgress * 3.5 );
            col.setRgb( 255, 0, 0 );
            col.getHsv( &h, &s, &v );
            col.setHsv( shift, s, v );

            img = DeKoratorFactory::colorizedImage( img, col, "Hue Adjustment" );
        }
    }
    else
    {
        if ( BUTTONHOVERTYPE == "To Gray" )
            KIconEffect::toGray( img, EFFECTAMOUNT );
        else if ( BUTTONHOVERTYPE == "Colorize" )
        {
            QColor col = client_->isActive() ? ACTIVEHIGHLIGHTCOLOR : INACTIVEHIGHLIGHTCOLOR;

            img = DeKoratorFactory::colorizedImage( img, col, BUTTONSCOLORIZE );
        }
        else if ( BUTTONHOVERTYPE == "DeSaturate" )
            KIconEffect::deSaturate( img, EFFECTAMOUNT );
        else if ( BUTTONHOVERTYPE == "To Gamma" )
            KIconEffect::toGamma( img, EFFECTAMOUNT );
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
            animTmr->start( INTERVAL );
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
                animTmr->start( INTERVAL );
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
            animTmr->start( INTERVAL );
    }
    update();
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
}

DeKoratorClient::~DeKoratorClient()
{
    for ( int n = 0; n < ButtonTypeCount; ++n )
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
    createMainWidget();
    widget() ->installEventFilter( this );

    widget() ->setAttribute( Qt::WA_NoSystemBackground, true );

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

    // layouts
    mainLayout_ = new QVBoxLayout( widget() );
    mainLayout_->setSpacing( 0 );
    mainLayout_->setContentsMargins( 0, 0, 0, 0 );
    titleLayout_ = new QHBoxLayout();
    titleLayout_->setSpacing( 0 );
    titleLayout_->setContentsMargins( 0, 0, 0, 0 );
    mainLayout_->addLayout( titleLayout_ );
    midLayout_ = new QHBoxLayout();
    midLayout_->setSpacing( 0 );
    midLayout_->setContentsMargins( 0, 0, 0, 0 );
    mainLayout_->addLayout( midLayout_ );

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
    bottomSpacer_ = new QSpacerItem( 1, ( !isSetShade() || SHOWBTMBORDER ) ? BOTTOMFRAMESIZE : 0,
                                     QSizePolicy::Expanding, QSizePolicy::Fixed );

    if (maximizeMode() == MaximizeFull && !showMaximizedBorders )
    {
        leftTitleBarSpacer_->changeSize( 0, TITLESIZE, QSizePolicy::Fixed, QSizePolicy::Fixed );
        rightTitleBarSpacer_->changeSize( 0, TITLESIZE, QSizePolicy::Fixed, QSizePolicy::Fixed );
        leftSpacer_->changeSize( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding );
        rightSpacer_->changeSize( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding );
        bottomSpacer_->changeSize( 0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed );
    }

    // setup layout

    //left spacer
    titleLayout_->addItem( leftTitleBarSpacer_ );
    // setup titlebar buttons
    for ( int n = 0; n < ButtonTypeCount; ++n )
        button[ n ] = 0;
    addButtons( titleLayout_, options() ->titleButtonsLeft() );
    titleLayout_->addItem( titleBarSpacer_ );
    addButtons( titleLayout_, options() ->titleButtonsRight() );
    //right spacer
    titleLayout_->addItem( rightTitleBarSpacer_ );

    // mid layout
    midLayout_->addItem( leftSpacer_ );
    if ( isPreview() )
    {
        QWidget *previewLabel = new QLabel( "<p align=\"center\"><b>" + i18n( "deKorator")
            + "</b> - " + i18n("Themable window decorator for KDE" ) + "</p><p align=\"center\">"
            + i18n("Click Configure icon to manage themes") + "</p>", widget() );
        previewLabel->setAutoFillBackground( true );
        midLayout_->addWidget( previewLabel );
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
}

//////////////////////////////////////////////////////////////////////////////
// addButtons()
// ------------
// Add buttons to title layout
void DeKoratorClient::addButtons( QBoxLayout * layout, const QString & s )
{
    QString tip;
    buttonTypeAll btnType;

    if ( s.length() > 0 )
    {
        for ( int n = 0; n < s.length(); ++n )
        {
            switch ( s[ n ].toAscii() )
            {
            case 'M':
                // Menu button
                if ( !button[ ButtonMenu ] )
                {
                    button[ ButtonMenu ] =
                        new DeKoratorButton( this, "menu", i18n( "Menu" ), ButtonMenu, menu );
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
                        btnType = sticky;
                    }
                    button[ ButtonSticky ] =
                        new DeKoratorButton( this, "sticky", d ? i18n( "Sticky" ) : i18n( "Un-Sticky" ), ButtonSticky, btnType );
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
                        new DeKoratorButton( this, "help-contents", i18n( "Help" ),
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
                        new DeKoratorButton( this, "iconify", i18n( "Minimize" ), ButtonMin, min );
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
                        btnType = restore;
                    }
                    else
                    {
                        btnType = max;
                    }
                    button[ ButtonMax ] =
                        new DeKoratorButton( this, "maximize", m ? i18n( "Restore" ) : i18n( "Maximize" ),
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
                        new DeKoratorButton( this, "close", i18n( "Close" ),
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
                    button[ ButtonAbove ] =
                        new DeKoratorButton( this, "above",
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
                    button[ ButtonBelow ] =
                        new DeKoratorButton( this, "below",
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
                        new DeKoratorButton( this, "shade", s ? i18n( "Unshade" ) : i18n( "Shade" ),
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
    for ( int n = 0; n < ButtonTypeCount; ++n )
        if ( button[ n ] )
            button[ n ] ->reset();
    widget() ->update();
}

//////////////////////////////////////////////////////////////////////////////
// captionChange()
// ---------------
// The title has changed
void DeKoratorClient::captionChange()
{
    captionBufferDirty_ = true;
    widget() ->update( titleBarSpacer_->geometry() );
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
        button[ ButtonSticky ] ->setToolTip( d ? i18n( "Sticky" ) : i18n( "Un-Sticky" ) );
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
        button[ ButtonMenu ] ->update();
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
        button[ ButtonMax ] ->setToolTip( m ? i18n( "Restore" ) : i18n( "Maximize" ) );
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
        button[ ButtonShade ] ->setToolTip( s ? i18n( "Unshade" ) : i18n( "Shade" ) );
    }

    //mainlayout_->setRowSpacing( 3, isSetShade() ? 0 : MARGIN );
    for ( int n = 0; n < ButtonTypeCount; ++n )
        if ( button[ n ] )
            button[ n ] ->reset();

    //mainlayout_->setRowSpacing( 3, isSetShade() ? 0 : MARGIN );
    sizeChanged = true;
    if ( USEMASKS )
        doShape();
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
    bool maximized = maximizeMode() == MaximizeFull && !showMaximizedBorders;

    if ( maximized )
    {
        l = 0;
        r = 0;
    }
    else
    {
        l = LEFTFRAMESIZE;
        r = RIGHTFRAMESIZE;
    }
    t = TITLESIZE ;
    //     if ( SHOWBTMBORDER )
    //b = 10;
    //     else
    //         b = isShade() ? 0 : BOTTOMFRAMESIZE;
    //b = SHOWBTMBORDER ? BOTTOMFRAMESIZE : isShade() ? 1 : BOTTOMFRAMESIZE;
    if ( !maximized && (!isShade() || SHOWBTMBORDER ) )
    {
        b = BOTTOMFRAMESIZE;
        bottomSpacer_->changeSize( 1, BOTTOMFRAMESIZE, QSizePolicy::Expanding, QSizePolicy::Fixed );
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
    if (maximizeMode() == MaximizeFull && !showMaximizedBorders )
    {
        leftTitleBarSpacer_->changeSize( 0, TITLESIZE, QSizePolicy::Fixed, QSizePolicy::Fixed );
        rightTitleBarSpacer_->changeSize( 0, TITLESIZE, QSizePolicy::Fixed, QSizePolicy::Fixed );
        leftSpacer_->changeSize( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding );
        rightSpacer_->changeSize( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding );
        bottomSpacer_->changeSize( 0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed );
    }
    else
    {
        leftTitleBarSpacer_->changeSize( TOPLEFTCORNERWIDTH, TITLESIZE, QSizePolicy::Fixed, QSizePolicy::Fixed );
        rightTitleBarSpacer_->changeSize( TOPRIGHTCORNERWIDTH, TITLESIZE, QSizePolicy::Fixed, QSizePolicy::Fixed );
        leftSpacer_->changeSize( LEFTFRAMESIZE, 1, QSizePolicy::Fixed, QSizePolicy::Expanding );
        rightSpacer_->changeSize( LEFTFRAMESIZE, 1, QSizePolicy::Fixed, QSizePolicy::Expanding );
        bottomSpacer_->changeSize( 1, BOTTOMFRAMESIZE, QSizePolicy::Expanding, QSizePolicy::Fixed );
    }
    widget() ->layout() ->invalidate();
    widget() ->layout() ->activate();
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
    int pos = PositionCenter;
    if ( isShade() )
    {
        return Position( pos );
    }

    if ( point.x() >= ( width() - qMax( TITLESIZE, RIGHTFRAMESIZE ) ) )
    {
        pos |= PositionRight;
    }
    else if ( point.x() <= qMax( TITLESIZE, LEFTFRAMESIZE ) )
    {
        pos |= PositionLeft;
    }

    if ( point.y() >= ( height() - qMax( TITLESIZE, BOTTOMFRAMESIZE ) ) )
    {
        pos |= PositionBottom;
    }
    else if ( point.y() <= ( pos == PositionCenter ? 3 : TITLESIZE ) )
    {
        pos |= PositionTop;
    }

    return Position( pos );
}


//////////////////////////////////////////////////////////////////////////////
// eventFilter()
// -------------
// Event filter
bool DeKoratorClient::eventFilter( QObject * obj, QEvent * e )
{
    if ( obj != widget() )
        return false;

    switch ( e->type() )
    {

    case QEvent::MouseButtonDblClick:
        {
            mouseDoubleClickEvent( static_cast<QMouseEvent *>( e ) );
            return true;
        }
    case QEvent::Wheel:
        {
            wheelEvent( static_cast<QWheelEvent *>( e ) );
            return true;
        }
    case QEvent::MouseButtonPress:
        {
            processMousePressEvent( static_cast<QMouseEvent *>( e ) );
            return true;
        }
    case QEvent::Paint:
        {
            paintEvent( static_cast<QPaintEvent *>( e ) );
            return true;
        }
    case QEvent::Resize:
        {
            resizeEvent( static_cast<QResizeEvent *>( e ) );
            return true;
        }
    case QEvent::Show:
        {
            showEvent( static_cast<QShowEvent *>( e ) );
            return true;
        }
    default:
        {
            return false;
        }
    }

    return false;
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


void drawHorizontalTiled(QPainter *p, const QRect &rect,
                         const QPixmap &left, const QPixmap &mid, const QPixmap &right)
{
    if (rect.isEmpty()) {
        return;
    }
    int leftSize = left.width();
    int rightSize = right.width();
    int sum = leftSize + mid.width() + rightSize;
    if (mid.width() > 0) {
        if (rect.width() >= sum) {
            p->drawTiledPixmap(QRect(rect.left() + leftSize, rect.top(), rect.width() - leftSize - rightSize, rect.height()), mid);
        } else {
            leftSize = leftSize * rect.width() / sum;
            rightSize = rightSize * rect.width() / sum;
            p->drawPixmap(QRect(rect.left() + leftSize, rect.top(), rect.width() - leftSize - rightSize, rect.height()), mid);
        }
    } else {
        if (sum <= 0) {
            return;
        }
        leftSize = leftSize * rect.width() / sum;
        rightSize = rect.width() - leftSize;
    }
    if (leftSize > 0) p->drawPixmap(QRect(rect.left(), rect.top(), leftSize, rect.height()), left);
    if (rightSize > 0) p->drawPixmap(QRect(rect.left() + rect.width() - rightSize, rect.top(), rightSize, rect.height()), right);
}

void drawVerticalTiled(QPainter *p, const QRect &rect,
                       const QPixmap &top, const QPixmap &mid, const QPixmap &bottom)
{
    if (rect.isEmpty()) {
        return;
    }
    int topSize = top.height();
    int bottomSize = bottom.height();
    int sum = topSize + mid.height() + bottomSize;
    if (mid.height() > 0) {
        if (rect.height() >= sum) {
            p->drawTiledPixmap(QRect(rect.left(), rect.top() + topSize, rect.width(), rect.height() - topSize - bottomSize), mid);
        } else {
            topSize = topSize * rect.height() / sum;
            bottomSize = bottomSize * rect.height() / sum;
            p->drawPixmap(QRect(rect.left(), rect.top() + topSize, rect.width(), rect.height() - topSize - bottomSize), mid);
        }
    } else {
        if (sum <= 0) {
            return;
        }
        topSize = topSize * rect.height() / sum;
        bottomSize = rect.height() - topSize;
    }
    if (topSize > 0) p->drawPixmap(QRect(rect.left(), rect.top(), rect.width(), topSize), top);
    if (bottomSize > 0) p->drawPixmap(QRect(rect.left(), rect.top() + rect.height() - bottomSize, rect.width(), bottomSize), bottom);
}

//////////////////////////////////////////////////////////////////////////////
// paintEvent()
// ------------
// Repaint the window
void DeKoratorClient::paintEvent( QPaintEvent* )
{
    WindowActivationState waState = isActive() ? WindowActive : WindowInactive;

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
        titleR.getRect( &tx, &ty, &tw, &th );
        QRect rect;

        {
            // topLeftCorner

            if ( leftTitleR.width() > 0 )
            {
                rect.setRect( 0, 0, TOPLEFTCORNERWIDTH, TITLESIZE );
                painter.drawPixmap( rect, decoPixmap[ topLeftCorner ][ waState ] );
            }

            // Space under the left button group
            painter.drawTiledPixmap( leftTitleR.right() + 1, titleR.top(),
                                      ( titleR.left() - 1 ) - leftTitleR.right(), titleR.height(), decoPixmap[ leftButtons ][ waState ] );

            drawHorizontalTiled( &painter, QRect( tx, 0, tw, TITLESIZE ),
                decoPixmap[ leftTitle ][ waState ],
                decoPixmap[ midTitle ][ waState ],
                decoPixmap[ rightTitle ][ waState ] );

            // Space under the right button group
            painter.drawTiledPixmap( titleR.right() + 1, titleR.top(),
                                      ( rightTitleR.left() - 1 ) - titleR.right(), titleR.height(), decoPixmap[ rightButtons ][ waState ] );


            //topRightCorner
            if ( rightTitleR.width() > 0 )
            {
                rect.setRect( widget() ->width() - TOPRIGHTCORNERWIDTH, 0, TOPRIGHTCORNERWIDTH, TITLESIZE );
                painter.drawPixmap( rect, decoPixmap[ topRightCorner ][ waState ] );
            }



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

                painter.drawImage( dx, dy, isActive() ? activeShadowImg_ : inActiveShadowImg_, 0, 0, isActive() ? qMin( activeShadowImg_.width(), titleR.width() ) : qMin( inActiveShadowImg_.width(), titleR.width() ), isActive() ? activeShadowImg_.height() : inActiveShadowImg_.height() );
            }


            //draw titleR text
            painter.setFont( options() ->font( isActive(), false ) );
            painter.setPen( options() ->color( KDecoration::ColorFont, isActive() ) );

            Qt::Alignment titleAlignBak = TITLEALIGN;
            if ( captionWidth > titleR.width() )
                titleAlignBak = Qt::AlignLeft;

            painter.drawText( tx + MARGIN, ty, tw - ( MARGIN * 2 ), th, titleAlignBak | Qt::AlignVCenter, caption() );
        }


        // draw frames
        if ( ! isShade() || SHOWBTMBORDER )
        {

            if ( h - TITLESIZE - BOTTOMFRAMESIZE > 0 )
            {
                int leftRightFramesHeight = h - TITLESIZE - BOTTOMFRAMESIZE ;

                //left frame
                drawVerticalTiled( &painter, QRect(0, TITLESIZE, LEFTFRAMESIZE, leftRightFramesHeight ),
                    decoPixmap[ topLeftFrame ][ waState ],
                    decoPixmap[ midLeftFrame ][ waState ],
                    decoPixmap[ bottomLeftFrame ][ waState ] );

                //rightFrame
                drawVerticalTiled( &painter, QRect( w - RIGHTFRAMESIZE, TITLESIZE, RIGHTFRAMESIZE, leftRightFramesHeight ),
                    decoPixmap[ topRightFrame ][ waState ],
                    decoPixmap[ midRightFrame ][ waState ],
                    decoPixmap[ bottomRightFrame ][ waState ] );
            }

            // bottom frame
            drawHorizontalTiled( &painter, QRect( 0, h - BOTTOMFRAMESIZE, w, BOTTOMFRAMESIZE ),
                decoPixmap[ leftBottomFrame ][ waState ],
                decoPixmap[ midBottomFrame ][ waState ],
                decoPixmap[ rightBottomFrame ][ waState ] );
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
    QPainter painter;
    QString c( caption() );
    QFontMetrics fm( options() ->font( isActive() ) );
    int captionWidth = fm.width( c );
    int captionHeight = fm.height( );

    // prepare the shadow
    QImage textImage( captionWidth + ( MARGIN * 2 ) , captionHeight, QImage::Format_ARGB32 ); // 2*2 px shadow space
    textImage.fill( 0 );
    painter.begin( &textImage );
    painter.setFont( options() ->font( isActive(), false ) );
    painter.setPen( Qt::white );
    painter.drawText( textImage.rect(), Qt::AlignCenter, caption() );
    painter.end();


    ShadowEngine se;
    //if ( isActive() )
    activeShadowImg_ = se.makeShadow( textImage, ACTIVESHADECOLOR );
    //else
    inActiveShadowImg_ = se.makeShadow( textImage, INACTIVESHADECOLOR );

    captionBufferDirty_ = false;
}

//////////////////////////////////////////////////////////////////////////////
// resizeEvent()
// -------------
// Window is being resized
void DeKoratorClient::resizeEvent( QResizeEvent *e )
{
    if ( USEMASKS )
    {

        if ( oldSize_ != e->size() )
        {
            sizeChanged = true;
        }
        else
            sizeChanged = false;

        oldSize_ = e->size();

        doShape();
    }
}

//////////////////////////////////////////////////////////////////////////////
// showEvent()
// -----------
// Window is being shown
void DeKoratorClient::showEvent( QShowEvent * )
{
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
        maximize( button[ ButtonMax ] ->lastMousePress() );
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
        button[ ButtonMenu ] ->setDown( false );
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

    if (maximizeMode() == MaximizeFull && !showMaximizedBorders )
    {
        setMask( mask );
        return;
    }

    if ( sizeChanged )
    {
        DeKoratorFactory *decoFactory_ = static_cast<DeKoratorFactory *>( factory() );
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

            for ( int i = 0 ; i < rep ; ++i )
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
            //bottom left
            m = QRegion( decoFactory_->bottomLeftCornerBitmap_ );
            m.translate( 0, h - BOTTOMLEFTMASKHEIGHT );
            mask -= QRegion( m );

            // bottom mid
            if ( BOTTOMMIDMASKWIDTH >= 1 )
            {
                int pos = BOTTOMLEFTMASKWIDTH;
                int rep = ( w - BOTTOMLEFTMASKWIDTH - BOTTOMRIGHTMASKWIDTH ) / BOTTOMMIDMASKWIDTH;
                int hm = h - BOTTOMMIDMASKHEIGHT;
                m = QRegion( decoFactory_->bottomMidBitmap_ );
                QRegion mBak = m;

                for ( int i = 0 ; i < rep ; ++i )
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

            //bottom right
            m = QRegion( decoFactory_->bottomRightCornerBitmap_ );
            m.translate( width() - BOTTOMRIGHTMASKWIDTH, h - BOTTOMRIGHTMASKHEIGHT );
            mask -= QRegion( m );
        }
        mask_ = mask;
    }

    setMask( mask_ );
}


#include "deKoratorclient.moc"
