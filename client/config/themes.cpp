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


#include "themes.h"

#include <KDE/KApplication>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KProgressDialog>
#include <KDE/KStandardDirs>
#include <KDE/KTar>
#include <KDE/KUrlRequester>
#include <KDE/KUrlRequesterDialog>

#include <KDE/KIO/Job>
#include <KDE/KIO/NetAccess>

#include <KDE/KNS/Engine>

#include <QtCore/QDir>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPushButton>

#include "deKoratorthemes.h"


//////////////////////////////////////////////////////////////////////////////
// IconThemesConfig()
// ----------
//

IconThemesConfig::IconThemesConfig( QWidget *parent, QListWidget *themesView /*, QPushButton *removethemeBtn*/ )
{
    parent_ = parent;
    themesView_ = themesView;
    //removeThemeBtn_ = removethemeBtn;

    loadThemes();
}

//////////////////////////////////////////////////////////////////////////////
// ~IconThemesConfig()
// ----------
//

IconThemesConfig::~IconThemesConfig()
{}

//////////////////////////////////////////////////////////////////////////////
// iconThemeItem()
// ----------
//

QListWidgetItem *IconThemesConfig::iconThemeItem( const QString &name )
{
    QList<QListWidgetItem *> items = themesView_->findItems( name, Qt::MatchExactly );
    if ( !items.isEmpty() ) {
        return items.at( 0 );
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// loadThemes()
// ----------
//


void IconThemesConfig::loadThemes()
{
    themesView_->clear();
    m_themeNames.clear();
    QStringList themelist;
    QString name;
    QString place;
    QStringList::Iterator it, itj;
    QStringList themesDirs;

    themesDirs = KGlobal::dirs() ->findDirs( "data", "deKorator/themes" ) ;

    for ( it = themesDirs.begin(); it != themesDirs.end(); ++it )
    {
        QDir dir = QDir( *it );
        themelist = dir.entryList( QString("*-theme").split(';') );

        for ( itj = themelist.begin(); itj != themelist.end(); ++itj )
        {
            place = *it;

            name = ( QString ) * itj;

            themesView_->addItem( name );

            m_themeNames.insert( name, place );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// getNewThemes()
// ----------
//

void IconThemesConfig::getNewThemes()
{
    KNS::Engine engine(parent_);

    if (engine.init(QLatin1String("deKoratorthemes.knsrc"))) {
        KNS::Entry::List entries = engine.downloadDialogModal(parent_);
        if (entries.size() > 0) {
            loadThemes();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// installNewTheme()
// ----------
//

void IconThemesConfig::installNewTheme()
{
}

QString deKoratorThemes::installTheme(const KUrl &themeURL)
{
    // themeTmpFile contains the name of the downloaded file
    QString themeTmpFile;
    QWidget *parent_ = this;

    if ( !KIO::NetAccess::download( themeURL, themeTmpFile, parent_ ) )
    {
        QString sorryText;
        if ( themeURL.isLocalFile() )
            sorryText = i18n( "Unable to find the deKorator theme archive %1.", themeURL.prettyUrl() );
        else
            sorryText = i18n( "Unable to download deKorator theme archive;\n"
                    "please check that address %1 is correct.", themeURL.prettyUrl() );
        KMessageBox::sorry( parent_, sorryText );
        return QString();
    }

    //
    QStringList themesNames = findThemeDirs( themeTmpFile );
    if ( themesNames.isEmpty() )
    {
        QString invalidArch( i18n( "The file is not a valid deKorator theme archive." ) );
        KMessageBox::error( parent_, invalidArch );

        KIO::NetAccess::removeTempFile( themeTmpFile );
        return QString();
    }

    if ( !installThemes( themesNames, themeTmpFile ) )
    {
        //FIXME: make me able to know what is wrong....
        // QStringList instead of bool?
        QString somethingWrong =
            i18n( "A problem occurred during the installation process; "
                  "however, most of the themes in the archive have been installed" );
        KMessageBox::error( parent_, somethingWrong );
    }

    KIO::NetAccess::removeTempFile( themeTmpFile );

    rescanThemes();

    QString localThemesDir = KStandardDirs::locateLocal("data", "deKorator/themes/");
    return localThemesDir + themesNames.at( 0 );
}

//////////////////////////////////////////////////////////////////////////////
// installThemes()
// ----------
//

bool deKoratorThemes::installThemes( const QStringList &themes, const QString &archiveName )
{
    bool everythingOk = true;
    QString localThemesDir = KStandardDirs::locateLocal("data", "deKorator/themes/");

    if (localThemesDir.isEmpty()) {
        return false;
    }
    //
    KProgressDialog progressDiag( this );//, "themeinstallprogress",
                                /*  i18n( "Installing icon themes" ),
                                  QString::null,
                                  true );*/
    progressDiag.setAutoClose( true );
    progressDiag.progressBar() ->setRange(0, themes.count() );
    progressDiag.show();

    KTar archive( archiveName );
    archive.open( QIODevice::ReadOnly );
    kapp->processEvents();

    const KArchiveDirectory* rootDir = archive.directory();

    KArchiveDirectory* currentTheme;
    for ( QStringList::ConstIterator it = themes.begin(); it != themes.end(); ++it )
    {
        progressDiag.setLabelText(
            i18n( "<qt>Installing <strong>%1</strong> theme</qt>", *it )
            );
        kapp->processEvents();

        if ( progressDiag.wasCancelled() )
            break;

        currentTheme = dynamic_cast<KArchiveDirectory*>(
                           const_cast<KArchiveEntry*>(
                               rootDir->entry( *it ) ) );
        if ( currentTheme == NULL )
        {
            // we tell back that something went wrong, but try to install as much
            // as possible
            everythingOk = false;
            continue;
        }

        currentTheme->copyTo( localThemesDir + *it );
	progressDiag.progressBar() ->setValue(progressDiag.progressBar() ->value() + 1 );
    }

    archive.close();
    return everythingOk;
}

//////////////////////////////////////////////////////////////////////////////
// findThemeDirs()
// ----------
//

QStringList deKoratorThemes::findThemeDirs( const QString &archiveName )
{
    QStringList foundThemes;

    KTar archive( archiveName );
    archive.open( QIODevice::ReadOnly );
    const KArchiveDirectory* themeDir = archive.directory();

    KArchiveEntry* possibleDir = 0L;
    KArchiveDirectory* subDir = 0L;

    // iterate all the dirs looking for an index.theme or index.desktop file
    QStringList entries = themeDir->entries();
    for ( QStringList::Iterator it = entries.begin(); it != entries.end(); ++it )
    {
        QString name = ( QString ) * it;
        QRegExp rxp = QRegExp( "*-theme", Qt::CaseSensitive, QRegExp::Wildcard );
        bool i = name.contains( rxp );
        if ( !i )
        {
        //    continue;
        }

        possibleDir = const_cast<KArchiveEntry*>( themeDir->entry( *it ) );
        if ( possibleDir->isDirectory() )
        {
            subDir = dynamic_cast<KArchiveDirectory*>( possibleDir );
            if ( subDir && ( subDir->entry( "deco" ) != NULL && subDir->entry( "buttons" ) != NULL && subDir->entry( "masks" ) != NULL ) )
            {
                foundThemes.append( subDir->name() );
            }
        }
    }

    archive.close();

    return foundThemes;
}

//////////////////////////////////////////////////////////////////////////////
// removeSelectedTheme()
// ----------
//

void IconThemesConfig::removeSelectedTheme()
{
    QList<QListWidgetItem *> selection = themesView_->selectedItems();
    if ( selection.isEmpty() ) {
        return;
    }
    QListWidgetItem *selected = selection.at( 0 );

    QString question = i18n( "<qt>Are you sure you want to remove the "
            "<strong>%1</strong> theme?<br>"
            "<br>"
            "This will delete the files installed by this theme.</qt>", selected->text() );

    int r = KMessageBox::warningContinueCancel( parent_, question, i18n( "Confirmation" ), KStandardGuiItem::del() );
    if ( r != KMessageBox::Continue ) return ;

    QString delTheme = selected->text();
    QString localThemesDir = KStandardDirs::locateLocal("data", "deKorator/themes/");
    if (localThemesDir.isEmpty()) {
        return ;
    }
    QString deldirStr = localThemesDir + delTheme;
    QDir dir = QDir( deldirStr );
    dir.rename( deldirStr, deldirStr + "del" );

    KIO::NetAccess::del( KUrl( deldirStr + "del" ), parent_ );

    loadThemes();
}

//////////////////////////////////////////////////////////////////////////////
// themeSelected()
// ----------
//

void IconThemesConfig::themeSelected( QListWidgetItem *item, QLabel *previewLabel, QPushButton *btn )
{
    QPixmap topLeftCornerBg, leftButtonsBg, leftTitleBg, midTitleBg, rightTitleBg, rightButtonsBg, topRightCornerBg;
    QPainter painter;
    int w, h, x;
    QString dirName( m_themeNames[ item->text() ] );

    if ( dirName.contains( "home" ) )
    {
        btn->setEnabled( true );
    }
    else
    {
        btn->setEnabled( false );
    }

    QString dirNameStr = dirName + item->text() + "/deco/" ;
    topLeftCornerBg.load( dirNameStr + "topLeftCornerBg.png" );
    leftButtonsBg.load( dirNameStr + "leftButtonsBg" );
    leftTitleBg.load( dirNameStr + "leftTitleBg.png" );
    midTitleBg.load( dirNameStr + "midTitleBg.png" );
    rightTitleBg.load( dirNameStr + "rightTitleBg.png" );
    rightButtonsBg.load( dirNameStr + "rightButtonsBg.png" );
    topRightCornerBg.load( dirNameStr + "topRightCornerBg.png" );

    w = topLeftCornerBg.width() + leftButtonsBg.width() + leftTitleBg.width() + midTitleBg.width() + rightTitleBg.width() + rightButtonsBg.width() + topRightCornerBg.width() + 8;
    h = midTitleBg.height() + 8;
    QPixmap previewPix(w, h);
    painter.begin( &previewPix );

    // paint outer rect
    painter.fillRect( 0, 0, w, h, QColor( 20, 20, 20 ) );

    painter.setPen( QColor( 60, 60, 60 ) );
    painter.drawRect( 0, 0, w , h );

    painter.setPen( QColor( 80, 80, 80 ) );
    painter.drawRect( 1, 1, w - 2 , h - 2 );

    painter.setPen( QColor( 180, 180, 180 ) );
    painter.drawLine( w - 1, 1, w - 1, h - 1 );
    painter.drawLine( 1, h - 1, w - 2, h - 1 );

    painter.setPen( QColor( 150, 150, 150 ) );
    painter.drawLine( w - 2, 2, w - 2, h - 2 );
    painter.drawLine( 2, h - 2, w - 3, h - 2 );


    // paint deco tiles
    x = 4;
    painter.drawPixmap( x, 4, topLeftCornerBg );

    x += topLeftCornerBg.width();
    painter.drawPixmap( x, 4, leftButtonsBg );

    x += leftButtonsBg.width();
    painter.drawPixmap( x , 4, leftTitleBg );

    x += leftTitleBg.width();
    painter.drawPixmap( x, 4, midTitleBg );

    x += midTitleBg.width();
    painter.drawPixmap( x, 4, rightTitleBg );

    x += rightTitleBg.width();
    painter.drawPixmap( x, 4, rightButtonsBg );

    x += rightButtonsBg.width();
    painter.drawPixmap( x, 4, topRightCornerBg );

    painter.setPen( QColor( 0, 0, 0 ) );
    painter.drawLine( 4, h - 5, w - 4 , h - 5 );

    painter.end();

    previewLabel->resize( w, h );
    previewLabel->setPixmap( previewPix );
}

//////////////////////////////////////////////////////////////////////////////
// setTheme()
// ----------
//

void IconThemesConfig::setTheme( KUrlRequester *framesPath, KUrlRequester *buttonsPath, KUrlRequester *masksPath )
{
    QList<QListWidgetItem *> selection = themesView_->selectedItems();
    if ( selection.isEmpty() ) {
        return;
    }
    QListWidgetItem *selected = selection.at( 0 );

    QString dirName( m_themeNames[ selected->text() ] );

    QString setTheme = selected->text();
    QString setThemeStr = dirName + setTheme;

    framesPath->setUrl( setThemeStr + "/deco" );
    buttonsPath->setUrl( setThemeStr + "/buttons" );
    masksPath->setUrl( setThemeStr + "/masks" );
}




#include "themes.moc"
