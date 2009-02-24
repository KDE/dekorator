/*
 * deKoratorthemes.cpp - select theme from (and manage) list of installed deKorator themes
 *
 * Copyright (c) 2009 Christoph Feck <christoph@maxiom.de>
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

#include "deKoratorthemes.h"

#include <KDE/KAboutData>
#include <KDE/KComponentData>
#include <KDE/KLocale>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QBitmap>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtGui/QStyleOptionViewItem>


/*------------------------------------------------------------------------*/

deKoratorThemes::deKoratorThemes(QWidget *parent)
    : KThemeSelector(parent)
{
    aboutData = new KAboutData("deKorator", 0, ki18n("deKorator"), "0.5.0",
        ki18n("Themable window decorator for KDE"),
        KAboutData::License_GPL,
        ki18n("(c) 2005, Moty Rahamim"), KLocalizedString(),
        "http://www.kde-look.org/content/show.php/?content=87921");
    aboutData->addAuthor(ki18n("Moty Rahamim"), ki18n("Developer"), "moty.rahamim@gmail.com");
    aboutData->addAuthor(ki18n("Christoph Feck"), ki18n("KDE 4 port"), "christoph@maxiom.de");
    aboutData->addCredit(ki18n("Jon 'ArbitraryReason' Clarke"), ki18n("Art Designer"), "jjclarke2003@yahoo.com");
    aboutData->addCredit(ki18n("Zoran 'the-error' Karavla"), ki18n("Theme contributor"), "webmaster@the-error.net");
    aboutData->addCredit(ki18n("David Johnson"), ki18n("Window decoration example"), "david@usermode.org");
    aboutData->addCredit(ki18n("Sandro Giessl"), ki18n("Plastik window decoration"), "sandro@giessl.com");
    aboutData->addCredit(ki18n("Antonio Larrosa"), ki18n("Icon theme KCM"), "larrosa@kde.org");
    componentData = new KComponentData(aboutData);
    setup(*componentData);
    setConfigFileKNS("deKoratorthemes.knsrc");
    setCreateAllowed(true);
    setConfigureAllowed(true);
}


deKoratorThemes::~deKoratorThemes()
{
    delete componentData;
    delete aboutData;
}


/*------------------------------------------------------------------------*/

static QString themePath(const QString &localPath, const QString &dirName)
{
    QDir dir(localPath);
    if (dir.exists(dirName)) {
        return (localPath + QLatin1String("/") + dirName);
    }
    QString lowerName = dirName.toLower();
    if (dir.exists(lowerName)) {
        return (localPath + QLatin1String("/") + lowerName);
    }
    return QString();
}


bool deKoratorThemes::isValidTheme(const QString &localPath) const
{
    QFileInfo info(localPath);

    if (KThemeSelector::isValidTheme(localPath) && info.isDir()) {
        QDir dir(localPath);
        if (dir.exists()) {
            QString decoPath = themePath(localPath, QLatin1String("Deco"));
            if (!decoPath.isEmpty() && dir.exists(decoPath + QLatin1String("/midTitleBg.png"))) {
                QString buttonsPath = themePath(localPath, QLatin1String("Buttons"));
                if (!buttonsPath.isEmpty()) {
                    return true;
                }
            }
        }
    }
    return false;
}


QString deKoratorThemes::themeName(const QString &localPath) const
{
    QString name = KThemeSelector::themeName(localPath);

    if (name.endsWith(QLatin1String("-theme"))) {
        name.chop(6);
    }
    return (name);
}


/*------------------------------------------------------------------------*/

int deKoratorThemes::viewModes() const
{
    return 0;//ViewModes;
}


QString deKoratorThemes::viewModeLabel(int viewMode) const
{
    if (viewMode == FullPreviews) {
        return i18n("Show Full Previews");
    } else {
        return i18n("Show Names Only");
    }
}


static void drawTiledImage(QPainter *painter, const QRect &rect, const QImage &image)
{
    if (!image.size().isEmpty()) {
        for (int x = rect.left(); x <= rect.right(); x += image.width()) {
            int w = qMin(image.width(), rect.right() + 1 - x);
            for (int y = rect.top(); y <= rect.bottom(); y += image.height()) {
                int h = qMin(image.height(), rect.bottom() + 1 - y);
                painter->drawImage(x, y, image, 0, 0, w, h);
            }
        }
    }
}


static void paintThemePreview(QPainter *painter, const QStyleOption *option, const QString &localPath, const QString &title, QColor bgColor)
{
    QString decoPath = themePath(localPath, QLatin1String("Deco"));
    QString masksPath = themePath(localPath, QLatin1String("Masks"));
    QString buttonsPath = themePath(localPath, QLatin1String("Buttons/normal"));
    if (buttonsPath.isEmpty()) {
        buttonsPath = themePath(localPath, QLatin1String("Buttons"));
    }

    static const char * const tileNames[] = {
        "topLeftCorner", "leftButtons", "leftTitle", "midTitle",
        "rightTitle", "rightButtons", "topRightCorner",
        "topLeftFrame", "midLeftFrame",
        "topRightFrame", "midRightFrame",
        "topLeftCorner", "topRightCorner",
        "buttonMin", "buttonMax", "buttonClose"
    };
    QImage buttonImage[3];
    int rightButtonsWidth = 0;
    for (int i = 13; i < 16; ++i) {
        QString imagePath = buttonsPath + QLatin1String("/") + QLatin1String(tileNames[i]) + QLatin1String(".png");
        buttonImage[i - 13] = QImage(imagePath);
        rightButtonsWidth += buttonImage[i - 13].width();
    }
    QRect rect = option->rect.adjusted(40, 8, -100, -32);
    QString text = option->fontMetrics.elidedText(title, Qt::ElideRight, rect.width());
    int x = option->rect.left() + 5, y = option->rect.top() + 5;
    for (int i = 0; i < 3; ++i) {
        QString imagePath = decoPath + QLatin1String("/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        QImage image(imagePath);
        painter->drawImage(x, y, image);
        x += image.width();
    }
    rect.setLeft(x);
    x = option->rect.right() - 5;
    for (int i = 6; i > 3; --i) {
        QString imagePath = decoPath + QLatin1String("/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        QImage image(imagePath);
        if (i == 5) {
            x -= rightButtonsWidth;
            drawTiledImage(painter, QRect(x, y, rightButtonsWidth, image.height()), image);
            int bx = x;
            for (int j = 0; j < 3; ++j) {
                painter->drawImage(bx, y + ((image.height() - buttonImage[j].height()) >> 1), buttonImage[j]);
                bx += buttonImage[j].width();
            }
        } else {
            x -= image.width();
            painter->drawImage(x, y, image);
        }
    }
    rect.setRight(x);
    QString imagePath = decoPath + QLatin1String("/") + QLatin1String(tileNames[3]) + QLatin1String("Bg.png");
    QImage image = QImage(imagePath);
    rect.setTop(y); rect.setHeight(image.height());
    drawTiledImage(painter, rect, image);
    y = option->rect.top() + 5 + image.height();
    painter->setClipRect(option->rect.adjusted(0, 0, 0, -5));
    QRect contentRect(5, 5, 10, 10);
    contentRect.setTop(option->rect.top() + 5 + image.height());
    contentRect.setBottom(option->rect.bottom());
    for (int i = 7; i < 9; ++i) {
        QString imagePath = decoPath + QLatin1String("/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        QImage image(imagePath);
        if (i == 7) {
            painter->drawImage(option->rect.left() + 5, y, image);
            y += image.height();
        } else {
            if (image.isNull()) {
                QString imagePath = decoPath + QLatin1String("/") + QLatin1String("leftFrame") + QLatin1String("Bg.png");
                image = QImage(imagePath);
            }
            contentRect.setLeft(option->rect.left() + 5 + image.width());
            drawTiledImage(painter, QRect(option->rect.left() + 5, y, image.width(), option->rect.height()), image);
        }
    }
    y = option->rect.top() + 5 + image.height();
    for (int i = 9; i < 11; ++i) {
        QString imagePath = decoPath + QLatin1String("/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        QImage image(imagePath);
        if (i == 9) {
            painter->drawImage(option->rect.right() - 5 - image.width(), y, image);
            y += image.height();
        } else {
            if (image.isNull()) {
                QString imagePath = decoPath + QLatin1String("/") + QLatin1String("rightFrame") + QLatin1String("Bg.png");
                image = QImage(imagePath);
            }
            contentRect.setRight(option->rect.right() - 5 - image.width() - 1);
            drawTiledImage(painter, QRect(option->rect.right() - 5 - image.width(), y, image.width(), option->rect.height()), image);
        }
    }
    painter->fillRect(contentRect, option->palette.color(QPalette::Window));
    if (!masksPath.isEmpty()) {
        painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
        for (int i = 11; i < 13; ++i) {
            QString imagePath = masksPath + QLatin1String("/") + QLatin1String(tileNames[i]) + QLatin1String("Bitmap.png");
            QBitmap bitmap = QBitmap::fromImage(QImage(imagePath));
            painter->setPen(Qt::black/*bgColor*/);
            if (i == 11) {
                painter->drawPixmap(option->rect.left() + 5, option->rect.top() + 5, bitmap);
            } else {
                painter->drawPixmap(option->rect.right() - 5 - bitmap.width(), option->rect.top() + 5, bitmap);
            }
        }
        painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    }
    painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
    QLinearGradient gradient(option->rect.bottomLeft() - QPoint(0, 10), option->rect.bottomLeft() - QPoint(0, 5));
    gradient.setColorAt(0.0, QColor(255, 255, 255, 0));
    gradient.setColorAt(1.0, QColor(255, 255, 255, 255));
    painter->fillRect(option->rect.adjusted(4, option->rect.height() - 12, -4, -4), gradient);
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::black);
    image = image.scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    if (qGray(image.pixel(0, 0)) < 150) {
        painter->drawText(rect.adjusted(1, 1, 1, 1), Qt::AlignCenter, text);
        painter->setPen(Qt::white);
    }
    painter->drawText(rect, Qt::AlignCenter, text);
}


void deKoratorThemes::paintThemeItem(QPainter *painter, const QStyleOptionViewItem *option,
                                    const QString &localPath, int viewMode) const
{
    if (true /*viewMode == FullPreviews*/) {
        int w = option->rect.width();
        int h = option->rect.height();
        QString cacheKey = QString(QLatin1String("deKoratorPreview-%1-%2-%3")).arg(w).arg(h).arg(localPath);
        QPixmap pixmap;

        if (!QPixmapCache::find(cacheKey, pixmap)) {
            QPixmap image(w, h);
            image.fill(Qt::transparent);
            QPainter p(&image);
            QStyleOptionViewItem opt = *option;
            opt.rect = QRect(0, 0, w, h);
            paintThemePreview(&p, &opt, localPath, themeName(localPath), Qt::transparent);
            p.end();
            pixmap = image;
            QPixmapCache::insert(cacheKey, pixmap);
        }
        painter->drawPixmap(option->rect.left(), option->rect.top(), pixmap);
    } else {
        KThemeSelector::paintThemeItem(painter, option, localPath, viewMode);
    }
}


QSize deKoratorThemes::sizeHintThemeItem(const QStyleOptionViewItem *option,
                                        const QString &localPath, int viewMode) const
{
    if (true /*viewMode == FullPreviews*/) {
        return QSize(300, 40 + option->fontMetrics.height());
    } else {
        return KThemeSelector::sizeHintThemeItem(option, localPath, viewMode);
    }
}


