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

#ifndef KTHEMESELECTOR_H
#define KTHEMESELECTOR_H

#include <QtGui/QWidget>

class KComponentData;
class KUrl;
class QStyleOptionViewItem;


class KThemeSelector : public QWidget
{
    Q_OBJECT

    public:
        enum ViewMode {
            NamesOnly,
            FullPreview
        };

        enum Property {
            RemoveEnabled,
            ConfigureEnabled
        };

    public:
        explicit KThemeSelector(QWidget *parent = 0);
        explicit KThemeSelector(const KComponentData &componentData, QWidget *parent = 0);
        virtual ~KThemeSelector();

        QStringList installedThemes();

        QString configFileKNS() const;
        void setConfigFileKNS(const QString &configFile);

        bool isConfigureAllowed() const;
        void setConfigureAllowed(bool allowed);

        bool isRemoveAllowed() const;
        void setRemoveAllowed(bool allowed);

        QString selectedTheme() const;
        void setSelectedTheme(const QString &localPath);

        ViewMode viewMode() const;
        void setViewMode(ViewMode viewMode);

        QByteArray saveState() const;
        void restoreState(const QByteArray &state);

        void rescanThemes();

    Q_SIGNALS:
        void themeSelected(const QString &localPath);
        void themeInstalled(const QString &localPath);
        void themeRemoved(const QString &localPath);
        void configureClicked(const QString &localPath);

    public:
        virtual QString installTheme(const KUrl &themeUrl);
        virtual bool removeTheme(const QString &localPath);

    protected:
        virtual QStringList scanInstalledThemes();
        virtual bool supportsViewMode(ViewMode viewMode) const;

        virtual void paintThemeItem(QPainter *painter, const QStyleOptionViewItem *option,
                                    const QString &localPath, ViewMode viewMode) const;
        virtual QSize sizeHintThemeItem(const QStyleOptionViewItem *option,
                                        const QString &localPath, ViewMode viewMode) const;
        virtual QString themeName(const QString &localPath) const;
        virtual bool hasProperty(const QString &localPath, Property property) const;

    private:
        class Private;
        Private * const d;
};


#endif // KTHEMESELECTOR_H

