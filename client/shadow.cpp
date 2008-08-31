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

#include "shadow.h"
#include <qcolor.h>
//Added by qt3to4:
#include <QPixmap>

ShadowEngine::ShadowEngine()
{
    thickness_ = 1;
    multiplicationFactor_ = 10.0;
}

ShadowEngine::~ShadowEngine()
{
}

QImage ShadowEngine::makeShadow(const QPixmap& textPixmap, const QColor &bgColor)
{
  // create a new image for for the shaddow
  int w = textPixmap.width();
  int h = textPixmap.height();

  QImage result(w, h, QImage::Format_ARGB32);

  // avoid calling these methods for every pixel
  int bgRed = bgColor.red();
  int bgGreen = bgColor.green();
  int bgBlue = bgColor.blue();

  double alphaShadow;

  /*
   *	This is the source pixmap
   */
  QImage img = textPixmap.toImage();

  result.fill(0); // transparent

  for (int i = thickness_; i < w - thickness_; i++)
  {
    for (int j = thickness_; j < h - thickness_; j++)
    {
        alphaShadow = decay(img, i, j);
        alphaShadow = (alphaShadow > 180.0) ? 180.0 : alphaShadow;
      // update the shadow's i,j pixel.
      result.setPixel(i,j, qRgba(bgRed, bgGreen , bgBlue, (int) alphaShadow));
    }
  }
  return result;
}

double ShadowEngine::decay(QImage& source, int i, int j)
{
  // create a new image for the shadow
  int w = source.width();
  int h = source.height();
  int sx, sy;

  double alphaShadow = 0;
  double opacity = 0;
  for (int k = 1; k <= thickness_; k++) {
    /* Generate a shadow thickness_ pixels thicker
     * on either side than the text image. Ensure
     * that i +/- k and j +/- k are within the
     * bounds of the text pixmap.
     */
    opacity = 0;
    for (int l = -k; l <= k; l++) {
      if (i < k)
	sx = 0;
      else if (i >= w - k)
	sx = w - 1;
      else
	sx = i + l;

      for (int m = -k; m <= k; m++) {
	if (j < k)
	  sy = 0;
	else if (j >= h - k)
	  sy = h - 1;
	else
	  sy = j + m;

	opacity += qGray(source.pixel(sx, sy));
      }
    }
    alphaShadow += opacity / multiplicationFactor_;
  }
  return alphaShadow;
}
