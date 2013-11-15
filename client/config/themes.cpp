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


#include "deKoratorthemes.h"

#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KProgressDialog>
#include <KDE/KStandardDirs>
#include <KDE/KTar>

#include <KDE/KIO/NetAccess>

#include <QApplication>
#include <QUrl>


QString deKoratorThemes::installTheme(const QUrl &themeURL)
{
    // themeTmpFile contains the name of the downloaded file
    QString themeTmpFile;
    QWidget *parent_ = this;

    if ( !KIO::NetAccess::download( themeURL, themeTmpFile, parent_ ) )
    {
        QString sorryText;
        if ( themeURL.isLocalFile() )
            sorryText = i18n( "Unable to find the deKorator theme archive %1.", themeURL.toDisplayString() );
        else
            sorryText = i18n( "Unable to download deKorator theme archive;\n"
                    "please check that address %1 is correct.", themeURL.toDisplayString() );
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
    qApp->processEvents();

    const KArchiveDirectory* rootDir = archive.directory();

    KArchiveDirectory* currentTheme;
    for ( QStringList::ConstIterator it = themes.begin(); it != themes.end(); ++it )
    {
        progressDiag.setLabelText(
            i18n( "<qt>Installing <strong>%1</strong> theme</qt>", *it )
            );
        qApp->processEvents();

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

        possibleDir = const_cast<KArchiveEntry*>( themeDir->entry( *it ) );
        if ( possibleDir->isDirectory() )
        {
            subDir = dynamic_cast<KArchiveDirectory*>( possibleDir );
            if ( subDir
                && ( ( subDir->entry( "deco" ) || subDir->entry( "Deco" ) )
                && ( subDir->entry( "buttons" ) || subDir->entry( "Buttons" ) ) ) )
            {
                foundThemes.append( subDir->name() );
            }
        }
    }

    archive.close();

    return foundThemes;
}

