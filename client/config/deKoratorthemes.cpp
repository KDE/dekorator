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
    : KThemeSelector(KComponentData("deKorator"), parent)
{
    setup();
    setCreateAllowed(true);
    setConfigureAllowed(true);
}


/*------------------------------------------------------------------------*/

bool deKoratorThemes::isValidTheme(const QString &localPath) const
{
    QFileInfo info(localPath);

    if (KThemeSelector::isValidTheme(localPath) && info.isDir()) {
        QDir dir(localPath);
        if (dir.exists() && dir.exists("deco") && dir.exists("deco/midTitleBg.png") && dir.exists("buttons")) {
            return true;
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
    return ViewModes;
}


QString deKoratorThemes::viewModeLabel(int viewMode) const
{
    if (viewMode == FullPreviews) {
        return i18n("Show Full Previews");
    } else {
        return i18n("Show Names Only");
    }
}


static void paintThemePreview(QPainter *painter, const QStyleOption *option, const QString &localPath, const QString &title, QColor bgColor)
{
    static const char * const tileNames[] = {
        "topLeftCorner", "leftButtons", "leftTitle", "midTitle",
        "rightTitle", "rightButtons", "topRightCorner",
        "topLeftFrame", "midLeftFrame",
        "topRightFrame", "midRightFrame",
        "topLeftCorner", "topRightCorner",
        "Min", "Max", "Close"
    };
    QImage buttonImage[3];
    int rightButtonsWidth = 0;
    for (int i = 13; i < 16; ++i) {
        QString imagePath = localPath + QLatin1String("/buttons/normal/button") + QLatin1String(tileNames[i]) + QLatin1String(".png");
        buttonImage[i - 13] = QImage(imagePath);
        rightButtonsWidth += buttonImage[i - 13].width();
    }
    QRect rect = option->rect.adjusted(40, 8, -100, -32);
    QString text = option->fontMetrics.elidedText(title, Qt::ElideRight, rect.width());
    int x = option->rect.left() + 5, y = option->rect.top() + 5;
    for (int i = 0; i < 3; ++i) {
        QString imagePath = localPath + QLatin1String("/deco/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        QImage image(imagePath);
        painter->drawImage(x, y, image);
        x += image.width();
    }
    rect.setLeft(x);
    x = option->rect.right() - 5;
    for (int i = 6; i > 3; --i) {
        QString imagePath = localPath + QLatin1String("/deco/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        QImage image(imagePath);
        if (i == 5) {
            x -= rightButtonsWidth;
            painter->drawTiledPixmap(QRect(x, y, rightButtonsWidth, image.height()), QPixmap::fromImage(image));
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
    QString imagePath = localPath + QLatin1String("/deco/") + QLatin1String(tileNames[3]) + QLatin1String("Bg.png");
    QImage image = QImage(imagePath);
    QPixmap pixmap = QPixmap::fromImage(image);
    image = image.scaled(1, 1, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    rect.setTop(y); rect.setHeight(pixmap.height());
    painter->drawTiledPixmap(rect, pixmap);
    y = option->rect.top() + 5 + pixmap.height();
    painter->setClipRect(option->rect.adjusted(0, 0, 0, -5));
    QRect contentRect(5, 5, 10, 10);
    contentRect.setTop(option->rect.top() + 5 + pixmap.height());
    contentRect.setBottom(option->rect.bottom());
    for (int i = 7; i < 9; ++i) {
        QString imagePath = localPath + QLatin1String("/deco/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        if (i == 7) {
            QImage image(imagePath);
            painter->drawImage(option->rect.left() + 5, y, image);
            contentRect.setLeft(option->rect.left() + 5 + image.width());
            y += image.height();
        } else {
            QPixmap pixmap = QPixmap::fromImage(QImage(imagePath));
            painter->drawTiledPixmap(QRect(option->rect.left() + 5, y, pixmap.width(), option->rect.height()), pixmap);
        }
    }
    y = option->rect.top() + 5 + pixmap.height();
    for (int i = 9; i < 11; ++i) {
        QString imagePath = localPath + QLatin1String("/deco/") + QLatin1String(tileNames[i]) + QLatin1String("Bg.png");
        if (i == 9) {
            QImage image(imagePath);
            painter->drawImage(option->rect.right() - 5 - image.width(), y, image);
            contentRect.setRight(option->rect.right() - 5 - image.width() - 1);
            y += image.height();
        } else {
            QPixmap pixmap = QPixmap::fromImage(QImage(imagePath));
            painter->drawTiledPixmap(QRect(option->rect.right() - 5 - pixmap.width(), y, pixmap.width(), option->rect.height()), pixmap);
        }
    }
    painter->fillRect(contentRect, option->palette.color(QPalette::Window));
    painter->setCompositionMode(QPainter::CompositionMode_DestinationOut);
    for (int i = 11; i < 13; ++i) {
        QString imagePath = localPath + QLatin1String("/masks/") + QLatin1String(tileNames[i]) + QLatin1String("Bitmap.png");
        QBitmap bitmap = QBitmap::fromImage(QImage(imagePath));
        painter->setPen(bgColor);
        if (i == 11) {
            painter->drawPixmap(option->rect.left() + 5, option->rect.top() + 5, bitmap);
        } else {
            painter->drawPixmap(option->rect.right() - 5 - bitmap.width(), option->rect.top() + 5, bitmap);
        }
    }
#if 0
    QLinearGradient gradient(option->rect.bottomLeft() - QPoint(0, 15), option->rect.bottomLeft() - QPoint(0, 5));
    gradient.setColorAt(1.0, bgColor);
    bgColor.setAlpha(0);
    gradient.setColorAt(0.0, bgColor);
    painter->fillRect(option->rect.adjusted(4, 4, -4, -4), gradient);
#endif
    painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
    QFont font = painter->font();
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::black);
    if (qGray(image.pixel(0, 0)) < 150) {
        painter->drawText(rect.adjusted(1, 1, 1, 1), Qt::AlignCenter, text);
        painter->setPen(Qt::white);
    }
    painter->drawText(rect, Qt::AlignCenter, text);
}


void deKoratorThemes::paintThemeItem(QPainter *painter, const QStyleOptionViewItem *option,
                                    const QString &localPath, int viewMode) const
{
    if (viewMode == FullPreviews) {
#if 1
        int w = option->rect.width();
        int h = option->rect.height();
        QString cacheKey = QString(QLatin1String("deKoratorPreview-%1-%2-%3")).arg(w).arg(h).arg(localPath);
        QPixmap pixmap;

        if (!QPixmapCache::find(cacheKey, pixmap)) {
            QImage image(w, h, QImage::Format_ARGB32);
            image.fill(0);
            QPainter p(&image);
            QStyleOptionViewItem opt = *option;
            opt.rect = QRect(0, 0, w, h);
            paintThemePreview(&p, &opt, localPath, themeName(localPath), Qt::transparent);
            pixmap = QPixmap::fromImage(image);
            QPixmapCache::insert(cacheKey, pixmap);
        }
        painter->drawPixmap(option->rect.left(), option->rect.top(), pixmap);
#else
        painter->save();
        QColor bgColor = option->palette.color(option->state & QStyle::State_Selected ? QPalette::Highlight : QPalette::Base);
        paintThemePreview(painter, option, localPath, themeName(localPath), bgColor);
        painter->restore();
#endif
    } else {
        KThemeSelector::paintThemeItem(painter, option, localPath, viewMode);
    }
}


QSize deKoratorThemes::sizeHintThemeItem(const QStyleOptionViewItem *option,
                                        const QString &localPath, int viewMode) const
{
    if (viewMode == FullPreviews) {
        return QSize(300, 40 + option->fontMetrics.height());
    } else {
        return KThemeSelector::sizeHintThemeItem(option, localPath, viewMode);
    }
}

