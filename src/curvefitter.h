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

#include "pointarray.h"

#define SPLINE_LEN 4

class CurveFitter
{
public:
    CurveFitter();
    ~CurveFitter();

    qreal fit(const PointArray<256> &points, PointArray<256> &curve);
    PointArray<256> curve(const PointArray<256> &curve, int count);
    void splitCasteljau(const PointArray<256> &curve, qreal t,
        PointArray<256> &left, PointArray<256> &right);

private:
    class InternalData
    {
    public:
        InternalData(int size = 0) : pxy(0), ts(0) {
            if (size > 0)
                ts = new qreal[size];
        }
        ~InternalData() {
            if (ts)
                delete [] ts;
        }

        qreal *pxy; /* Bezier spline of size SPLINE_SIZE */
        qreal *ts;  /* Sample points of size n */
    };

    class SectionData
    {
    public:
        qreal *pxy; /* Bezier spline of size SPLINE_SIZE */
        qreal *x;  /* Sample point of size 2 */
    };

    static qreal bins[SPLINE_LEN];
    static qreal resPhi;

    static qreal func3(double t, void *data);
    qreal goldenSectionSearch(qreal (*func)(qreal x, void *data),
        qreal a, qreal b, qreal epsilon, void *data);

    void initBins();
    static void point(const qreal *pxy, qreal t, qreal *xy);
    static void curve(const qreal *pxy, int num, qreal *xy, const qreal *ts = 0);
    static void func(double *p, double *hx, int m, int n, void *data);
    static void func2(double *t, double *hx, int m, int n, void *data);

    static void splitCasteljau(const qreal *pxy,  qreal t,
        qreal *pxy1, qreal *pxy2);

    void chordLengthParam(int len, const qreal *x, qreal *ts, bool centripetal);
};

#endif
