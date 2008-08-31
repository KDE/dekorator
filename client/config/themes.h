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

#ifndef ICONTHEMES_H
#define ICONTHEMES_H

//#include <kcmodule.h>
#include <qmap.h>
//Added by qt3to4:
#include <QLabel>
#include <k3listview.h>

class QPushButton;
class DeviceManager;
class QCheckBox;
class QStringList;



class IconThemesConfig : public QObject
{
    Q_OBJECT

public:
    IconThemesConfig( QWidget *parent, K3ListView *themesView/*, QPushButton *removethemeBtn*/ );
    virtual ~IconThemesConfig();

    void loadThemes();
    bool installThemes( const QStringList &themes, const QString &archiveName );
    QStringList findThemeDirs( const QString &archiveName );
    void themeSelected( Q3ListViewItem *item, QLabel *preview, QPushButton *btn );
    void installNewTheme();
    void removeSelectedTheme();
    void setTheme( KUrlRequester *framesPath, KUrlRequester *buttonsPath, KUrlRequester *masksPath );

private:
    Q3ListViewItem *iconThemeItem( const QString &name );
    K3ListView *themesView_;
    QWidget *parent_;
    //QPushButton *removeThemeBtn_;
    Q3ListViewItem *m_defaultTheme;
    QMap <QString, QString>m_themeNames;
};

#endif // ICONTHEMES_H

