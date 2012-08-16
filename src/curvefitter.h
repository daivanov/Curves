/*
 * Copyright 2012 Daniil Ivanov <daniil.ivanov@gmail.com>
 *
 * This file is part of Curves.
 *
 * Curves is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Curves is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with Curves. If not, see http://www.gnu.org/licenses/.
 */

#ifndef CURVEFITTER_H
#define CURVEFITTER_H

#include <QtGlobal>
#include <QVector>
#include <QPointF>

#define SPLINE_LEN 4

class CurveFitter
{
public:
    CurveFitter();
    ~CurveFitter();

    qreal fit(const QVector<QPointF> &points, QVector<QPointF> &curve);
    QVector<QPointF> curve(const QVector<QPointF> &curve, int count);

private:
    static qreal bins[SPLINE_LEN];
    static int refCount;

    void initBins();
    static void point(const qreal *pxy, qreal t, qreal *xy);
    static void curve(const qreal *pxy, int num, qreal *xy);
    static void func(double *p, double *hx, int m, int n, void *data);
};

#endif
