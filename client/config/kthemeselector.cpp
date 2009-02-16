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

KThemeSelectorDelegate::KThemeSelectorDelegate(QObject *parent)
    : QStyledItemDelegate(parent), selector(0)
{
    /* */
}


void KThemeSelectorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    // paint styled background
    QStyledItemDelegate::paint(painter, option, QModelIndex());
    QString localPath = index.data().toString();

    if (selector) {
        selector->paintThemeItem(painter, &option, localPath, selector->viewMode());
    } else {
        painter->setPen(option.palette.color(option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
        painter->drawText(option.rect, Qt::AlignCenter, localPath);
    }
    painter->restore();
}


QSize KThemeSelectorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (selector) {
        QString localPath = index.data().toString();
        return selector->sizeHintThemeItem(&option, localPath, selector->viewMode());
    } else {
        return QSize(-1, option.fontMetrics.height());
    }
}


/*------------------------------------------------------------------------*/

KThemeSelector::Private::Private(QObject *parent)
    : QObject(parent), m_themesScanned(false), m_viewMode(0), m_delegate(new KThemeSelectorDelegate(parent))
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
    m_delegate->selector = widget;

    setupUi((QWidget *) widget);

    int modes = widget->viewModes();
    if (modes > 0) {
        for (int i = 0; i < modes; ++i) {
            m_viewModeChooser->addItem(widget->viewModeLabel(i));
        }
    } else {
        m_viewModeChooser->setVisible(false);
    }

    connect(m_editFilter, SIGNAL(textChanged(QString)), SLOT(filterChanged(QString)));
    connect(m_buttonConfigure, SIGNAL(clicked()), SLOT(configureClicked()));
    connect(m_buttonInstall, SIGNAL(clicked()), SLOT(installClicked()));
    connect(m_buttonGetNew, SIGNAL(clicked()), SLOT(getNewClicked()));
    connect(m_buttonRemove, SIGNAL(clicked()), SLOT(removeClicked()));
    connect(m_buttonCreate, SIGNAL(clicked()), widget, SIGNAL(createClicked()));
    connect(m_view, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));
    connect(m_viewModeChooser, SIGNAL(currentIndexChanged(int)), SLOT(setViewMode(int)));

    m_view->setUniformItemSizes(true);
    m_view->setItemDelegate(m_delegate);

    m_editFilter->setClickMessage(i18n("Filter Themes"));
    m_editFilter->setClearButtonShown(true);

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


void KThemeSelector::Private::setViewMode(int viewMode)
{
    if (viewMode != m_viewMode) {
        m_viewMode = viewMode;
        m_view->reset();
        if (!m_selected.isEmpty()) {
            QList<QListWidgetItem *> items = m_view->findItems(m_selected, Qt::MatchExactly);
            if (!items.isEmpty()) {
                m_view->scrollToItem(items.at(0));
            }
        }
    }
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
        m_buttonRemove->setEnabled(!localPath.isEmpty() && m_parent->hasProperty(localPath, Removable));
        m_buttonConfigure->setEnabled(!localPath.isEmpty() && m_parent->hasProperty(localPath, Configurable));
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
}


KThemeSelector::KThemeSelector(const KComponentData &componentData, QWidget *parent)
    : QWidget(parent), d(new Private)
{
    d->m_componentData = componentData;
}


KThemeSelector::~KThemeSelector()
{
    delete d;
}


void KThemeSelector::setup()
{
    d->setup(this);
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
            d->m_buttonConfigure->setEnabled(hasProperty(d->m_selected, Configurable));
        }
    }
}


bool KThemeSelector::isCreateAllowed() const
{
    return !d->m_buttonCreate->isHidden();
}


void KThemeSelector::setCreateAllowed(bool allowed)
{
    if (allowed != !d->m_buttonCreate->isHidden()) {
        d->m_buttonCreate->setVisible(allowed);
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
            d->m_buttonRemove->setEnabled(hasProperty(d->m_selected, Removable));
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
        d->setViewMode(state.at(1));
        d->setFilter(QString::fromUtf8(state.mid(2)));
    }
}


void KThemeSelector::rescanThemes()
{
    d->m_themesScanned = false;
    QStringList oldThemes = d->m_themes;
    QStringList newThemes = installedThemes();
    if (!newThemes.contains(d->m_selected)) {
        d->setSelected(QString());
    }
    foreach (const QString &theme, newThemes) {
        if (!oldThemes.contains(theme)) {
            d->m_view->addItem(theme);
            emit themeInstalled(theme);
            d->setSelected(theme);
        }
    }
    foreach (const QString &theme, oldThemes) {
        if (!newThemes.contains(theme)) {
            QList<QListWidgetItem *> items = d->m_view->findItems(theme, Qt::MatchExactly);
            if (!items.isEmpty()) {
                d->m_view->takeItem(d->m_view->row(items.at(0)));
                emit themeRemoved(theme);
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


void KThemeSelector::addTheme(const QString &localPath)
{
    // TODO
}


QStringList KThemeSelector::scanInstalledThemes()
{
    QStringList directories = d->m_componentData.dirs()->findDirs("appdata", "themes");
    QStringList themes;

    foreach (const QString &directory, directories) {
        QDir dir(directory);
        QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
        foreach (QString entry, entries) {
            QString localPath = dir.absoluteFilePath(entry);
            if (isValidTheme(localPath)) {
                themes.append(localPath);
            }
        }
    }
    return themes;
}


bool KThemeSelector::isValidTheme(const QString &localPath) const
{
    return true;
}


QString KThemeSelector::themeName(const QString &localPath) const
{
    QFileInfo info(localPath);
    return (info.fileName());
}


bool KThemeSelector::hasProperty(const QString &localPath, Property property) const
{
    switch (property) {
        case Configurable:
            return true;
        case Removable:
            // TODO check actual permissions...
            return localPath.contains(QString::fromUtf8("/home/"));
        // no default; warn when new properties are added
    }
    return false;
}


int KThemeSelector::viewModes() const
{
    return 1;
}


int KThemeSelector::viewMode() const
{
    return d->m_viewMode;
}


QString KThemeSelector::viewModeLabel(int viewMode) const
{
    return i18n("Themes");
}


void KThemeSelector::paintThemeItem(QPainter *painter, const QStyleOptionViewItem *option,
                                    const QString &localPath, int viewMode) const
{
    Q_UNUSED(viewMode);

    painter->save();
    QRect rect = option->rect.adjusted(3, 0, -3, 0);
    QString text = option->fontMetrics.elidedText(themeName(localPath), Qt::ElideRight, rect.width());
    painter->setPen(option->palette.color(option->state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
    painter->drawText(rect, Qt::AlignVCenter, text);
    painter->restore();
}


QSize KThemeSelector::sizeHintThemeItem(const QStyleOptionViewItem *option,
                                        const QString &localPath, int viewMode) const
{
    Q_UNUSED(localPath); Q_UNUSED(viewMode);

    return QSize(-1, option->fontMetrics.height());
}


