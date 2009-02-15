/*
 * KThemeSelector - select theme from (and manage) list of installed themes
 *
 * Copyright (c) 2008 Christoph Feck <christoph@maxiom.de>
 *
 * Parts of this file are based on theme.cpp from "deKorator" which is
 * Copyright (c) 2005 moty rahamim <moty.rahamim@gmail.com>
 *
 * Based on Example-0.8, some ideas and code have been taken from plastik and KCMicons
 *
 * Example window decoration for KDE
 * Copyright (c) 2003, 2004 David Johnson <david@usermode.org>
 *
 * Plastik KWindowSystem window decoration
 * Copyright (C) 2003-2005 Sandro Giessl <sandro@giessl.com>
 *
 * KCMicons for KDE
 * Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "kthemeselector.h"
#include "kthemeselector_p.h"

#include <KDE/KAboutData>
#include <KDE/KHelpMenu>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KStandardDirs>
#include <KDE/KUrl>
#include <KDE/KUrlRequesterDialog>

#include <KDE/KIO/Job>
#include <KDE/KIO/NetAccess>

#include <KDE/KNS/Engine>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QPainter>
#include <QtGui/QStyle>


/*------------------------------------------------------------------------*/

KThemeSelector::Private::Private(QObject *parent)
    : QObject(parent), m_themesScanned(false), m_viewMode(NamesOnly)
{
    /* */
}


KThemeSelector::Private::~Private()
{
    /* */
}


void KThemeSelector::Private::setup(KThemeSelector *widget)
{
    m_parent = widget;
    setupUi((QWidget *) widget);

    connect(m_editFilter, SIGNAL(textChanged(QString)), SLOT(filterChanged(QString)));
    connect(m_buttonConfigure, SIGNAL(clicked()), SLOT(configureClicked()));
    connect(m_buttonInstall, SIGNAL(clicked()), SLOT(installClicked()));
    connect(m_buttonGetNew, SIGNAL(clicked()), SLOT(getNewClicked()));
    connect(m_buttonRemove, SIGNAL(clicked()), SLOT(removeClicked()));
    connect(m_view, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));

    m_editFilter->setClickMessage(i18n("Filter Themes"));
    m_editFilter->setClearButtonShown(true);
    m_editFilter->setVisible(false);

    m_buttonConfigure->setVisible(false);
    m_buttonConfigure->setEnabled(false);
    m_buttonRemove->setEnabled(false);
    m_buttonGetNew->setVisible(false);
    KHelpMenu *helpMenu = new KHelpMenu(m_buttonAbout, m_componentData.aboutData());
    QMenu *menu = (QMenu *) helpMenu->menu();
    m_buttonAbout->setMenu(menu);
    m_buttonAbout->setText(i18n("About %1", m_componentData.aboutData()->appName()));

    m_parent->rescanThemes();
}


void KThemeSelector::Private::setFilter(const QString &text)
{
    if (text != m_editFilter->text()) {
        m_editFilter->setText(text);
    }
}


void KThemeSelector::Private::filterChanged(const QString &filter)
{
    // TODO
}


void KThemeSelector::Private::configureClicked()
{
    if (!m_selected.isEmpty()) {
        emit m_parent->configureClicked(m_selected);
    }
}


void KThemeSelector::Private::installClicked()
{
    KUrl themeUrl = KUrlRequesterDialog::getUrl(QString(), m_parent, i18n("Drag or Type Theme URL"));

    if (themeUrl.url().isEmpty()) {
        return;
    }
    QString installed = m_parent->installTheme(themeUrl);
    if (!installed.isEmpty()) {
        // select first to allow the slot to select a different theme
        setSelected(installed);
        // TODO m_view.addItem();
        emit m_parent->themeInstalled(installed);
    }
}


void KThemeSelector::Private::removeClicked()
{
    if (!m_selected.isEmpty()) {
        QString removed = m_selected;
        QString question = i18n("<qt>Are you sure you want to remove the "
                "<strong>%1</strong> theme?<br>"
                "<br>"
                "This will delete the files installed by this theme.</qt>",
                m_parent->themeName(removed));

        int r = KMessageBox::warningContinueCancel(m_parent, question, i18n("Confirmation"),
                KStandardGuiItem::del());
        if (r != KMessageBox::Continue) {
            return;
        }

        if (m_parent->removeTheme(removed)) {
            // unselect first to allow the slot to select a different theme
            setSelected(QString());
            QList<QListWidgetItem *> items = m_view->findItems(removed, Qt::MatchExactly);
            if (!items.isEmpty()) {
                m_view->takeItem(m_view->row(items.at(0)));
            }
            emit m_parent->themeRemoved(removed);
        }
    }
}


void KThemeSelector::Private::getNewClicked()
{
    KNS::Engine engine(m_parent);

    if (engine.init(m_configFileKNS)) {
        KNS::Entry::List entries = engine.downloadDialogModal(m_parent);
        if (entries.size() > 0) {
            m_parent->rescanThemes();
        }
    }
}


void KThemeSelector::Private::setSelected(const QString &localPath)
{
    if (m_selected != localPath) {
        m_selected = localPath;
        m_buttonRemove->setEnabled(!localPath.isEmpty() && m_parent->hasProperty(localPath, RemoveEnabled));
        m_buttonConfigure->setEnabled(!localPath.isEmpty() && m_parent->hasProperty(localPath, ConfigureEnabled));
        QList<QListWidgetItem *> items = m_view->findItems(localPath, Qt::MatchExactly);
        if (!items.isEmpty()) {
            m_view->setCurrentItem(items.at(0), QItemSelectionModel::SelectCurrent);
        } else {
            m_view->clearSelection();
        }
        emit m_parent->themeSelected(localPath);
    }
}


void KThemeSelector::Private::selectionChanged()
{
    QList<QListWidgetItem *> selection = m_view->selectedItems();
    setSelected(selection.isEmpty() ? QString() : selection.at(0)->text());
}


/*------------------------------------------------------------------------*/

KThemeSelector::KThemeSelector(QWidget *parent)
    : QWidget(parent), d(new Private)
{
    d->m_componentData = KGlobal::mainComponent();
    d->setup(this);
}


KThemeSelector::KThemeSelector(const KComponentData &componentData, QWidget *parent)
    : QWidget(parent), d(new Private)
{
    d->m_componentData = componentData;
    d->setup(this);
}


KThemeSelector::~KThemeSelector()
{
    delete d;
}


QStringList KThemeSelector::installedThemes()
{
    if (!d->m_themesScanned) {
        d->m_themes = scanInstalledThemes();
        d->m_themesScanned = true;
    }
    return d->m_themes;
}


QString KThemeSelector::configFileKNS() const
{
    return d->m_configFileKNS;
}


void KThemeSelector::setConfigFileKNS(const QString &configFile)
{
    if (configFile != d->m_configFileKNS) {
        d->m_configFileKNS = configFile;
        d->m_buttonGetNew->setVisible(!d->m_configFileKNS.isEmpty());
    }
}


bool KThemeSelector::isConfigureAllowed() const
{
    return !d->m_buttonConfigure->isHidden();
}


void KThemeSelector::setConfigureAllowed(bool allowed)
{
    if (allowed != !d->m_buttonConfigure->isHidden()) {
        d->m_buttonConfigure->setVisible(allowed);
        if (allowed && !d->m_selected.isEmpty()) {
            d->m_buttonConfigure->setEnabled(hasProperty(d->m_selected, ConfigureEnabled));
        }
    }
}


bool KThemeSelector::isRemoveAllowed() const
{
    return !d->m_buttonRemove->isHidden();
}


void KThemeSelector::setRemoveAllowed(bool allowed)
{
    if (allowed != !d->m_buttonRemove->isHidden()) {
        d->m_buttonRemove->setVisible(allowed);
        if (allowed && !d->m_selected.isEmpty()) {
            d->m_buttonRemove->setEnabled(hasProperty(d->m_selected, RemoveEnabled));
        }
    }
}


QString KThemeSelector::selectedTheme() const
{
    return d->m_selected;
}


void KThemeSelector::setSelectedTheme(const QString &localPath)
{
    d->setSelected(localPath);
}


KThemeSelector::ViewMode KThemeSelector::viewMode() const
{
    return d->m_viewMode;
}


void KThemeSelector::setViewMode(ViewMode viewMode)
{
    if (viewMode != d->m_viewMode && supportsViewMode(viewMode)) {
        d->m_viewMode = viewMode;
        if (d->m_view->isVisible()) {
            d->m_view->update();
        }
    }
}


QByteArray KThemeSelector::saveState() const
{
    const int version = 1;
    QByteArray state;

    state.append(version);
    state.append(d->m_viewMode);
    state.append(d->m_editFilter->text().toUtf8());
    return state;
}


void KThemeSelector::restoreState(const QByteArray &state)
{
    int version = state.at(0);

    if (version == 1) {
        setViewMode((ViewMode) state.at(1));
        d->setFilter(QString::fromUtf8(state.mid(2)));
    }
}


void KThemeSelector::rescanThemes()
{
    d->m_themesScanned = false;
    QStringList oldThemes = d->m_themes;
    if (oldThemes != installedThemes()) {
        d->m_view->clear();
        d->m_view->addItems(d->m_themes);
        if (!installedThemes().contains(d->m_selected)) {
            d->setSelected(QString());
        } else {
            if (d->m_view->isVisible()) {
                d->m_view->update();
            }
        }
    }
}


/*------------------------------------------------------------------------*/
/*
 * The following virtual functions implement the actual theme handling
 *
 */

QString KThemeSelector::installTheme(const KUrl &themeUrl)
{
    // TODO
    return themeUrl.url();
}


bool KThemeSelector::removeTheme(const QString &localPath)
{
    KIO::NetAccess::del(KUrl(localPath), this);
    return true;
}


QStringList KThemeSelector::scanInstalledThemes()
{
    QStringList directories = d->m_componentData.dirs()->findDirs("appdata", "themes");
    QStringList themes;

    foreach (const QString &directory, directories) {
        QDir dir(directory);
        QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
        foreach (QString entry, entries) {
            themes.append(dir.absoluteFilePath(entry));
        }
    }
    return themes;
}


bool KThemeSelector::supportsViewMode(ViewMode viewMode) const
{
    if (viewMode == NamesOnly) {
        return true;
    }
    return false;
}


void KThemeSelector::paintThemeItem(QPainter *painter, const QStyleOptionViewItem *option,
                                    const QString &localPath, ViewMode viewMode) const
{
    Q_UNUSED(localPath); Q_UNUSED(viewMode);

    style()->drawPrimitive(QStyle::PE_PanelItemViewItem, option, painter, d->m_view);
    // TODO
    //style()->drawItemText(themeName(localPath));
}


QSize KThemeSelector::sizeHintThemeItem(const QStyleOptionViewItem *option,
                                        const QString &localPath, ViewMode viewMode) const
{
    Q_UNUSED(localPath);

    if (viewMode == NamesOnly) {
        return QSize(-1, option->fontMetrics.height());
    } else {
        return QSize(-1, 16 + 2 * option->fontMetrics.height());
    }
}


QString KThemeSelector::themeName(const QString &localPath) const
{
    QFileInfo info(localPath);
    return (info.fileName());
}


bool KThemeSelector::hasProperty(const QString &localPath, Property property) const
{
    switch (property) {
        case ConfigureEnabled:
            return true;
        case RemoveEnabled:
            // TODO check actual permissions...
            return localPath.contains(QString::fromUtf8("/home/"));
        // no default; warn when new properties are added
    }
    return false;
}


