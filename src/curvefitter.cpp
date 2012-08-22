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

#include <QtCore/qmath.h>
#include <levmar.h>
#include <QDebug>

#include "curvefitter.h"
#include "utils.h"

#define SPLINE_SIZE (2 * SPLINE_ORDER)
#define MAX_ITER 500

qreal CurveFitter::bins[SPLINE_ORDER] = { 0 };
qreal CurveFitter::resPhi = 0;

CurveFitter::CurveFitter()
{
    Q_ASSERT(sizeof(qreal) == sizeof(double));
    if(!bins[0])
        initBins();
}

CurveFitter::~CurveFitter()
{
}

void CurveFitter::initBins()
{
    for (int i = 0; i < SPLINE_ORDER; ++i) {
        int a = qMax(i, SPLINE_ORDER - 1 - i);
        bins[i] = 1.0;
        for (int j = a + 1; j <= SPLINE_ORDER - 1; ++j)
            bins[i] *= (qreal) j / (j - a);
    }
    resPhi = 2 - (1 + qSqrt(5)) / 2;
}

void CurveFitter::point(int splineOrder, const qreal *pxy, qreal t, qreal *xy)
{
    qreal *pxy1 = new qreal[2 * splineOrder];
    qreal *pxy2 = new qreal[2 * splineOrder];
    splitCasteljau(splineOrder, pxy, t, pxy1, pxy2);
    memcpy(xy, pxy2, sizeof(qreal) * 2);
    delete [] pxy1;
    delete [] pxy2;
}

void CurveFitter::point(const qreal *pxy, qreal t, qreal *xy)
{
    xy[0] = xy[1] = 0.0;
    int i;
    const qreal *pxyi;
    for (i = 0, pxyi = pxy; i < SPLINE_ORDER; ++i, pxyi += 2) {
        qreal multiplier = bins[i] * qPow(t, i) *
            qPow(1 - t, SPLINE_ORDER - 1 - i);
        xy[0] += multiplier * pxyi[0];
        xy[1] += multiplier * pxyi[1];
    }
}

void CurveFitter::splitCasteljau(const PointArray<256> &curve, qreal t,
    PointArray<256> &left, PointArray<256> &right)
{
    left.resize(curve.count());
    right.resize(curve.count());
    splitCasteljau(curve.count(), curve.data(), t, left.data(), right.data());
}

void CurveFitter::splitCasteljau(int splineOrder, const qreal *pxy,  qreal t,
    qreal *pxy1, qreal *pxy2)
{
    qreal *tmp = new qreal[2 * splineOrder];
    memcpy(tmp, pxy, sizeof(qreal) * 2 * splineOrder);
    for (int k = 0; k < splineOrder; ++k) {
        pxy1[0 + 2 * k] = tmp[0];
        pxy1[1 + 2 * k] = tmp[1];
        pxy2[2 * (splineOrder - k) - 2] = tmp[2 * (splineOrder - k) - 2];
        pxy2[2 * (splineOrder - k) - 1] = tmp[2 * (splineOrder - k) - 1];
        for (int i = 0; i < 2 * (splineOrder - k) - 2; ++i) {
            tmp[i] = (1 - t) * tmp[i] + t * tmp[i + 2];
        }
    }
    delete [] tmp;
}

PointArray<256> CurveFitter::curve(const PointArray<256> &curvePoints, int count)
{
    PointArray<256> points;
    points.resize(count);

    curve(curvePoints.count(), curvePoints.data(), count, points.data());

    return points;
}

void CurveFitter::curve(int splineOrder, const qreal *pxy, int num, qreal *xy,
    const qreal *ts)
{
    qreal *pleft = new qreal[2 * splineOrder];
    qreal *ptmp = new qreal[2 * splineOrder];
    qreal *pright = new qreal[2 * splineOrder];
    memcpy(ptmp, pxy, sizeof(qreal) * 2 * splineOrder);
    int i; qreal *cxy;
    /* Initial point */
    memcpy(xy, pxy, sizeof(qreal) * 2);
    /* Last point */
    memcpy(xy + 2 * (num - 1), pxy + 2 * (splineOrder - 1), sizeof(qreal) * 2);
    /* Other points */
    qreal t;
    for (i = 1, cxy = xy + 2; i < num - 1; ++i, cxy += 2) {
        if (ts) {
            t = (ts[i] - ts[i - 1]) / (1.0 - ts[i - 1]);
        } else
            t = 1.0 / (num - i);
        splitCasteljau(splineOrder, ptmp, t, pleft, pright);
        memcpy(cxy, pright, sizeof(qreal) * 2);
        qSwap(pright, ptmp);
    }
    delete [] pleft;
    delete [] ptmp;
    delete [] pright;
}

void CurveFitter::func(qreal *p, qreal *hx, int m, int n, void *data)
{
    InternalData *iData = (InternalData*)data;
    if (iData->pxy + 2 != p)
        memcpy(iData->pxy + 2, p, sizeof(qreal) * m);
    curve(SPLINE_ORDER, iData->pxy, n / 2, hx, iData->ts);
}

void CurveFitter::chordLengthParam(int len, const qreal *x, qreal *ts,
    bool centripetal)
{
    Q_ASSERT(x);
    Q_ASSERT(ts);

    /* Cumulative chord distances */
    ts[0] = 0.0;
    for (int i = 1; i < len; ++i) {
        qreal dx = x[2 * i] - x[2 * i - 2];
        qreal dy = x[2 * i + 1] - x[2 * i - 1];
        ts[i] = ts[i - 1] + qPow((dx * dx + dy * dy), (centripetal ? .25 : .5));
    }

    /* Normalized cumulative chord distances */
    for (int i = 1; i < len; ++i)
        ts[i] /= ts[len - 1];
}

qreal CurveFitter::func3(qreal t, void *data)
{
    SectionData *sData = (SectionData*)data;
    qreal hx[2];

    point(sData->pxy, t, hx);
    qreal dx = hx[0] - sData->x[0];
    qreal dy = hx[1] - sData->x[1];

    /* Compute squared error */
    return dx * dx + dy * dy;
}

qreal CurveFitter::goldenSectionSearch(qreal (*func)(qreal x, void *data),
    qreal a, qreal b, qreal epsilon, void *data)
{
    qreal x1 = a + resPhi * (b - a);
    qreal x2 = b - resPhi * (b - a);
    qreal f1 = func(x1, data);
    qreal f2 = func(x2, data);

    do {
        if (f1 < f2) {
            b = x2;
            x2 = x1;
            f2 = f1;
            x1 = a + resPhi * (b - a);
            f1 = func(x1, data);
        } else {
            a = x1;
            x1 = x2;
            f1 = f2;
            x2 = b - resPhi * (b - a);
            f2 = func(x2, data);
        }
    } while (qAbs(b - a) > epsilon);

    return (a + b) / 2;
}

qreal CurveFitter::fit(const PointArray<256> &points, PointArray<256> &curve)
{
    /* Init input data */
    int sz = 2 * points.count();
    qreal *x = const_cast<qreal*>(points.data());

    InternalData data(sz / 2);
    curve.resize(SPLINE_ORDER);
    data.pxy = curve.data();
    chordLengthParam(sz / 2, x, data.ts, false);

    qreal segmentLen = (sz / (SPLINE_ORDER - 1));
    /* Init middle points of Bezier curve */
    for (int i = 2; i < SPLINE_SIZE - 2; i += 2) {
        int idx = (i / 2) * segmentLen;
        idx -= idx % 2;
        data.pxy[i] = x[idx];
        data.pxy[i + 1] = x[idx + 1];
    }

    /* Init first point of Bezier curve */
    data.pxy[0] = x[0];
    data.pxy[1] = x[1];

    int idx = (segmentLen - (int)segmentLen % 2);
    data.pxy[2] = (x[idx] - x[0]) * 2 + x[0];
    data.pxy[3] = (x[idx + 1] - x[1]) * 2 + x[1];

    data.pxy[SPLINE_SIZE - 4] = (x[sz - 2 - idx] - x[sz - 2]) * 2 + x[sz - 2];
    data.pxy[SPLINE_SIZE - 3] = (x[sz - 1 - idx] - x[sz - 1]) * 2 + x[sz - 1];

    /* Init last point of Bezier curve */
    data.pxy[SPLINE_SIZE - 2] = x[sz - 2];
    data.pxy[SPLINE_SIZE - 1] = x[sz - 1];

    /* info[0]= ||e||_2 at initial p.
     * info[1-4]=[ ||e||_2, ||J^T e||_inf,  ||Dp||_2, \mu/max[J^T J]_ii ], all computed at estimated p.
     * info[5]= # iterations,
     * info[6]=reason for terminating: 1 - stopped by small gradient J^T e
     *                                 2 - stopped by small Dp
     *                                 3 - stopped by itmax
     *                                 4 - singular matrix. Restart from current p with increased \mu
     *                                 5 - no further error reduction is possible. Restart with increased mu
     *                                 6 - stopped by small ||e||_2
     *                                 7 - stopped by invalid (i.e. NaN or Inf) "func" values; a user error
     * info[7]= # function evaluations
     * info[8]= # Jacobian evaluations
     * info[9]= # linear systems solved, i.e. # attempts for reducing error
     */
    qreal info[LM_INFO_SZ];
    qreal *p = data.pxy + 2;
    int m = SPLINE_SIZE - 2 * 2;
    int n = sz;
    qreal fnorm = INT_MAX, fnormPrev;

    do {
        /* Optimize spline shape */
        dlevmar_dif(CurveFitter::func, p, x, m, n, MAX_ITER, NULL, info, NULL,
            NULL, &data);

        /* Residuals */
        fnormPrev = fnorm;
        fnorm = info[1] / sz;
        qDebug() << "Termination reason" << info[6]
                 << "after" << info[5] << "iterations"
                 << "error" << fnorm;

        /* Quit, when improvement is less than 1% */
        if ((fnormPrev - fnorm) / fnormPrev < 0.01)
            break;

        /* Optimize point parameters */
        qreal diff = 0;
        SectionData sData;
        sData.pxy = data.pxy;
        for (int j = 1; j < sz / 2 - 1; ++j) {
            data.ts[j] += diff;
            diff = data.ts[j]; /* Saving old value */
            qreal a = (j < 1) ? 0.0 : data.ts[j - 1];
            qreal b = (j > sz / 2 - 2) ? 1.0 : data.ts[j + 1] + diff;
            qreal epsilon = (b - a) * 0.01; /* 1% of interval */
            sData.x = x + 2 * j;
            data.ts[j] = goldenSectionSearch(CurveFitter::func3, a, b,
                epsilon, &sData);
            diff = data.ts[j] - diff; /* Change between new and old */
        }
    } while (true);

    return fnorm;
}
