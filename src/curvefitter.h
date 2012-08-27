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

#define SPLINE_ORDER 4

class CurveFitter
{
public:
    CurveFitter();
    ~CurveFitter();

    enum Transformation { EUCLIDEAN, AFFINE };
    qreal fit(const PointArray<256> &points, PointArray<256> &curve,
        Transformation transformation);
    PointArray<256> curve(const PointArray<256> &curve, int count);
    void splitCasteljau(const PointArray<256> &curve, qreal t,
        PointArray<256> &left, PointArray<256> &right);

    qreal goldenSectionSearch(qreal (*func)(qreal x, void *data),
        qreal a, qreal b, qreal epsilon, void *data);

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

    static qreal bins[SPLINE_ORDER];
    static qreal resPhi;

    static qreal func3(double t, void *data);

    void initBins();
    static void point(const qreal *pxy, qreal t, qreal *xy);
    static void point(int splineOrder, const qreal *pxy, qreal t, qreal *xy);
    static void curve(int splineOrder, const qreal *pxy, int num, qreal *xy, const qreal *ts = 0);
    static void func(double *p, double *hx, int m, int n, void *data);

    static void splitCasteljau(int splineOrder, const qreal *pxy,
        qreal t, qreal *pxy1, qreal *pxy2);

    enum Parametrization { CHORD_LENGTH, CENTRIPETAL };
    void chordLengthParam(int len, const qreal *x, qreal *ts,
        Parametrization parametrization);

    qreal reparametrize(const qreal *pxy, const qreal *pxy1, const qreal *pxy2,
        const qreal *x, qreal t);
    void reparametrizePoints(const qreal *pxy, int num, const qreal *x,
        qreal *ts);

    friend class CurveTest;
};

#endif
