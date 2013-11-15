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

#include <QWidget>

class KAboutData;
class QStyleOptionViewItem;


class KThemeSelector : public QWidget
{
    Q_OBJECT

    public:
        enum Property {
            Removable,
            Configurable
        };

    public:
        explicit KThemeSelector(QWidget *parent = 0);
        virtual ~KThemeSelector();

        QStringList installedThemes();

        QString configFileKNS() const;
        void setConfigFileKNS(const QString &configFile);

        bool isConfigureAllowed() const;
        void setConfigureAllowed(bool allowed);

        bool isCreateAllowed() const;
        void setCreateAllowed(bool allowed);

        bool isRemoveAllowed() const;
        void setRemoveAllowed(bool allowed);

        QString selectedTheme() const;
        void setSelectedTheme(const QString &localPath);

        QByteArray saveState() const;
        void restoreState(const QByteArray &state);

        void rescanThemes();

    Q_SIGNALS:
        void themeSelected(const QString &localPath);
        void themeInstalled(const QString &localPath);
        void themeRemoved(const QString &localPath);
        void configureClicked(const QString &localPath);
        void createClicked();

    public:
        virtual QString installTheme(const QUrl &themeUrl);
        virtual void addTheme(const QString &localPath);
        virtual bool removeTheme(const QString &localPath);

    protected:
        virtual QStringList scanInstalledThemes();
        virtual bool isValidTheme(const QString &localPath) const;
        virtual QString themeName(const QString &localPath) const;
        virtual bool hasProperty(const QString &localPath, Property property) const;

        friend class KThemeSelectorDelegate;
        virtual int viewModes() const;
        virtual QString viewModeLabel(int viewMode) const;
        virtual void paintThemeItem(QPainter *painter, const QStyleOptionViewItem *option,
                                    const QString &localPath, int viewMode) const;
        virtual QSize sizeHintThemeItem(const QStyleOptionViewItem *option,
                                        const QString &localPath, int viewMode) const;
        int viewMode() const;
        void setup(KAboutData *data);

    private:
        class Private;
        Private * const d;
};


#endif // KTHEMESELECTOR_H

