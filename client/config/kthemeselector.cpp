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
#include <KDE/KLocalizedString>
#include <KDE/KMessageBox>
#include <KDE/KUrlRequesterDialog>

#include <KDE/KIO/Job>
#include <KDE/KIO/NetAccess>

#include <KDE/KNS3/DownloadDialog>

#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QStandardPaths>
#include <QStyle>


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
    QStyle *style = QApplication::style();
    if (option.version >= 3) {
        const QWidget *widget = qobject_cast<const QWidget *>(((const QStyleOptionViewItemV3 *) &option)->widget);
        if (widget) {
            style = widget->style();
        }
    }
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, 0);
    QString localPath = index.data().toString();

    if (selector) {
        selector->paintThemeItem(painter, &option, localPath, selector->viewMode());
    } else {
        QPalette::ColorGroup cg;
        if (!(option.state & QStyle::State_Enabled)) {
            cg = QPalette::Disabled;
        } else if (option.state & QStyle::State_Active) {
            cg = QPalette::Active;
        } else {
            cg = QPalette::Inactive;
        }
        painter->setPen(option.palette.color(cg, option.state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
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
    : QObject(parent), m_aboutData(nullptr), m_themesScanned(false), m_viewMode(0), m_delegate(new KThemeSelectorDelegate(parent))
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
    if (modes > 1) {
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
    m_view->setMinimumWidth(330);

    m_editFilter->setClickMessage(i18n("Filter Themes"));
    m_editFilter->setClearButtonShown(true);
    m_editFilter->setVisible(false);

    m_buttonConfigure->setVisible(false);
    m_buttonConfigure->setEnabled(false);
    m_buttonRemove->setEnabled(false);
    m_buttonGetNew->setVisible(false);
    KHelpMenu *helpMenu = new KHelpMenu(m_buttonAbout, *m_aboutData);
    QMenu *menu = (QMenu *) helpMenu->menu();
    m_buttonAbout->setMenu(menu);
    m_buttonAbout->setText(i18n("About %1", m_aboutData->displayName()));

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
    Q_UNUSED(filter);
}


void KThemeSelector::Private::setViewMode(int viewMode)
{
    if (viewMode != m_viewMode) {
        m_viewMode = viewMode;
        m_viewModeChooser->setCurrentIndex(viewMode);
        QString selected = m_selected;
        setSelected(QString());
        m_view->reset();
        setSelected(selected);
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
    QUrl themeUrl = KUrlRequesterDialog::getUrl(QUrl(), m_parent, i18n("Drag or Type Theme URL"));

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


void KThemeSelector::Private::postInstallFiles(const QStringList &files)
{
    foreach (const QString &file, files) {
        if (file.endsWith(QLatin1String(".tar.bz2"), Qt::CaseInsensitive)
            || file.endsWith(QLatin1String(".tar.gz"), Qt::CaseInsensitive)
            || file.endsWith(QLatin1String(".tar.xz"), Qt::CaseInsensitive)) {
            QString installed = m_parent->installTheme(QUrl::fromLocalFile(file));
            if (!installed.isEmpty()) {
                setSelected(installed);
            }
        }
    }
}


void KThemeSelector::Private::getNewClicked()
{
    QPointer<KNS3::DownloadDialog> dialog = new KNS3::DownloadDialog(m_configFileKNS, m_parent);

    if (dialog->exec()) {
        KNS3::Entry::List entries = dialog->changedEntries();
        if (entries.size() > 0) {
            KNS3::Entry::List entries = dialog->installedEntries();
            if (entries.size() > 0) {
                foreach (const KNS3::Entry &entry, entries) {
                    postInstallFiles(entry.installedFiles());
                }
            }
            m_parent->rescanThemes();
        }
    }
    delete dialog;
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
            m_view->scrollToItem(items.at(0));
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
}


KThemeSelector::~KThemeSelector()
{
    delete d;
}


void KThemeSelector::setup(KAboutData *data)
{
    d->m_aboutData = data;
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
        if (allowed) {
            d->m_buttonConfigure->setEnabled(!d->m_selected.isEmpty() && hasProperty(d->m_selected, Configurable));
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
        d->m_buttonInstall->setVisible(allowed);
        if (allowed) {
            d->m_buttonRemove->setEnabled(!d->m_selected.isEmpty() && hasProperty(d->m_selected, Removable));
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
    if (!state.isEmpty()) {
        int version = state.at(0);

        if (version == 1) {
            d->setViewMode(state.at(1));
            d->setFilter(QString::fromUtf8(state.mid(2)));
        }
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
    if (d->m_selected.isEmpty()) {
        if (!newThemes.isEmpty()) {
            d->setSelected(newThemes.at(0));
        }
    }
}


/*------------------------------------------------------------------------*/
/*
 * The following virtual functions implement the actual theme handling
 *
 */

QString KThemeSelector::installTheme(const QUrl &themeUrl)
{
    // TODO
    return themeUrl.url();
}


bool KThemeSelector::removeTheme(const QString &localPath)
{
    KIO::NetAccess::del(QUrl::fromLocalFile(localPath), this);
    return true;
}


void KThemeSelector::addTheme(const QString &localPath)
{
    // TODO
}


QStringList KThemeSelector::scanInstalledThemes()
{
    QStringList directories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                                        QStringLiteral("deKorator/themes"),
                                                        QStandardPaths::LocateDirectory);
    QStringList themes;

    foreach (const QString &directory, directories) {
        QDir dir(directory);
        QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
        foreach (const QString &entry, entries) {
            QString localPath = dir.absoluteFilePath(entry);
            if (isValidTheme(localPath)) {
                themes.append(localPath);
            }
            if (true) {
                QDir dir(localPath);
                QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::Readable | QDir::NoDotAndDotDot);
                foreach (const QString &entry, entries) {
                    QString localPath = dir.absoluteFilePath(entry);
                    if (isValidTheme(localPath)) {
                        themes.append(localPath);
                    }
                }
            }
        }
    }
    return themes;
}


bool KThemeSelector::isValidTheme(const QString &localPath) const
{
    Q_UNUSED(localPath);
    return true;
}


QString KThemeSelector::themeName(const QString &localPath) const
{
    QFileInfo info(localPath);
    QString name = info.fileName().replace(QChar('_', 0), QChar(' ', 0));
    name[0] = name[0].toUpper();
    return name;
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
    Q_UNUSED(viewMode);
    return i18n("Themes");
}


void KThemeSelector::paintThemeItem(QPainter *painter, const QStyleOptionViewItem *option,
                                    const QString &localPath, int viewMode) const
{
    Q_UNUSED(viewMode);
    painter->save();
    QRect rect = option->rect.adjusted(3, 0, -3, 0);
    QString text = option->fontMetrics.elidedText(themeName(localPath), Qt::ElideRight, rect.width());
    QPalette::ColorGroup cg;
    if (!(option->state & QStyle::State_Enabled)) {
        cg = QPalette::Disabled;
    } else if (option->state & QStyle::State_Active) {
        cg = QPalette::Active;
    } else {
        cg = QPalette::Inactive;
    }
    painter->setPen(option->palette.color(cg, option->state & QStyle::State_Selected ? QPalette::HighlightedText : QPalette::Text));
    painter->drawText(rect, Qt::AlignVCenter, text);
    painter->restore();
}


QSize KThemeSelector::sizeHintThemeItem(const QStyleOptionViewItem *option,
                                        const QString &localPath, int viewMode) const
{
    Q_UNUSED(localPath); Q_UNUSED(viewMode);

    return QSize(-1, option->fontMetrics.height());
}


