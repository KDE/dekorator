/*
 * KThemeSelector - select theme from (and manage) list of installed themes
 *
 * Copyright (c) 2008 Christoph Feck <christoph@maxiom.de>
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

#ifndef KTHEMESELECTOR_PRIVATE_H
#define KTHEMESELECTOR_PRIVATE_H

#include "kthemeselector.h"
#include "ui_kthemeselectorwidgets.h"
#include <KDE/KComponentData>
#include <QtGui/QStyledItemDelegate>


class KThemeSelectorDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        KThemeSelectorDelegate(QObject *parent = 0);

    public:
        virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    public:
        KThemeSelector *selector;
};


class KThemeSelector::Private : public QObject, public Ui::KThemeSelectorWidgets
{
    Q_OBJECT

    public:
        Private(QObject *parent = 0);
        virtual ~Private();

        KThemeSelector *m_parent;
        KComponentData m_componentData;
        QString m_configFileKNS;

        QStringList m_themes;
        bool m_themesScanned;
        QString m_selected;

        int m_viewMode;
        KThemeSelectorDelegate *m_delegate;

    public Q_SLOTS:
        void removeClicked();
        void configureClicked();
        void installClicked();
        void getNewClicked();
        void selectionChanged();
        void filterChanged(const QString &filter);
        void setViewMode(int viewMode);

    public:
        void setup(KThemeSelector *widget);
        void setFilter(const QString &text);
        void setSelected(const QString &localPath);
};

#endif


