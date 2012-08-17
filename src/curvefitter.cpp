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

#include <math.h>
#include <levmar.h>
#include <QDebug>

#include "curvefitter.h"

#define SPLINE_SIZE (2 * SPLINE_LEN)
#define MAX_ITER 500

qreal CurveFitter::bins[SPLINE_LEN];
int CurveFitter::refCount = 0;

CurveFitter::CurveFitter()
{
    Q_ASSERT(sizeof(qreal) == sizeof(qreal));
    if(!refCount)
        initBins();
    refCount++;
}

CurveFitter::~CurveFitter()
{
    refCount--;
}

void CurveFitter::initBins()
{
    for (int i = 0; i < SPLINE_LEN; ++i) {
        int a = qMax(i, SPLINE_LEN - 1 - i);
        bins[i] = 1.0;
        for (int j = a + 1; j <= SPLINE_LEN - 1; ++j)
            bins[i] *= (qreal) j / (j - a);
    }
}

void CurveFitter::point(const qreal *pxy, qreal t, qreal *xy)
{
    xy[0] = xy[1] = 0.0;
    int i;
    const qreal *pxyi;
    for (i = 0, pxyi = pxy; i < SPLINE_LEN; ++i, pxyi += 2) {
        qreal multiplier = bins[i] * pow(t, i) *
            pow(1 - t, SPLINE_LEN - 1 - i);
        xy[0] += multiplier * pxyi[0];
        xy[1] += multiplier * pxyi[1];
    }
}

void CurveFitter::splitCasteljau(const PointArray<256> &curve, qreal t,
    PointArray<256> &left, PointArray<256> &right)
{
    left.resize(SPLINE_SIZE);
    right.resize(SPLINE_SIZE);
    splitCasteljau(curve.data(), t, left.data(), right.data());
}

void CurveFitter::splitCasteljau(const qreal *pxy,  qreal t,
    qreal *pxy1, qreal *pxy2)
{
    qreal tmp[SPLINE_SIZE];
    memcpy(tmp, pxy, sizeof(qreal) * SPLINE_SIZE);
    for (int k = 0; k < SPLINE_LEN; ++k) {
        pxy1[0 + 2 * k] = tmp[0];
        pxy1[1 + 2 * k] = tmp[1];
        pxy2[SPLINE_SIZE - 2 - 2 * k] = tmp[SPLINE_SIZE - 2 - 2 * k];
        pxy2[SPLINE_SIZE - 1 - 2 * k] = tmp[SPLINE_SIZE - 1 - 2 * k];
        for (int i = 0; i < SPLINE_SIZE - 2 - 2 * k; ++i) {
            tmp[i] = (1 - t) * tmp[i] + t * tmp[i + 2];
        }
    }
}

PointArray<256> CurveFitter::curve(const PointArray<256> &curvePoints, int count)
{
    PointArray<256> points;
    points.resize(2 * count);

    curve(curvePoints.data(), count, points.data());

    return points;
}

void CurveFitter::curve(const qreal *pxy, int num, qreal *xy)
{
    qreal spline1[SPLINE_SIZE], spline2[SPLINE_SIZE], spline3[SPLINE_SIZE];
    qreal *pleft = spline1, *ptmp = spline2, *pright = spline3;
    memcpy(ptmp, pxy, sizeof(qreal) * SPLINE_SIZE);
    int i; qreal *cxy;
    /* Initial point */
    memcpy(xy, pxy, sizeof(qreal) * 2);
    /* Last point  */
    memcpy(xy + 2 * (num - 1), pxy + 2 * (SPLINE_LEN - 1), sizeof(qreal) * 2);
    for (i = 1, cxy = xy + 2; i < num - 1; ++i, cxy += 2) {
        splitCasteljau(ptmp, 1.0 / (num - i), pleft, pright);
        memcpy(cxy, pright, sizeof(qreal) * 2);
        qSwap(pright, ptmp);
    }
}

void CurveFitter::func(qreal *p, qreal *hx, int m, int n, void *data)
{
    qreal *pxy = (qreal*)data;
    if (pxy + 2 != p)
        memcpy(pxy + 2, p, sizeof(qreal) * m);
    curve(pxy, n / 2, hx);
}

qreal CurveFitter::fit(const PointArray<256> &points, PointArray<256> &curve)
{
    /* Init input data */
    int sz = 2 * points.count();
    qreal *x = const_cast<qreal*>(points.data());

    curve.resize(SPLINE_SIZE);
    qreal *pxy = curve.data();

    qreal segmentLen = (sz / (SPLINE_LEN - 1));
    /* Init middle points of Bezier curve */
    for (int i = 2; i < SPLINE_SIZE - 2; i += 2) {
        int idx = (i / 2) * segmentLen;
        idx -= idx % 2;
        pxy[i] = x[idx];
        pxy[i + 1] = x[idx + 1];
    }

    /* Init first point of Bezier curve */
    pxy[0] = x[0];
    pxy[1] = x[1];

    int idx = (segmentLen - (int)segmentLen % 2);
    pxy[2] = (x[idx] - x[0]) * 2 + x[0];
    pxy[3] = (x[idx + 1] - x[1]) * 2 + x[1];

    pxy[SPLINE_SIZE - 4] = (x[sz - 2 - idx] - x[sz - 2]) * 2 + x[sz - 2];
    pxy[SPLINE_SIZE - 3] = (x[sz - 1 - idx] - x[sz - 1]) * 2 + x[sz - 1];

    /* Init last point of Bezier curve */
    pxy[SPLINE_SIZE - 2] = x[sz - 2];
    pxy[SPLINE_SIZE - 1] = x[sz - 1];

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
    qreal *p = pxy + 2;
    int m = SPLINE_SIZE - 2 * 2;
    int n = sz;
    dlevmar_dif(CurveFitter::func, p, x, m, n, MAX_ITER, NULL, info, NULL,
        NULL, pxy);

    /* Residuals */
    qreal fnorm = info[1] / sz;
    qDebug() << "Termination reason" << info[6]
             << "after" << info[5] << "iterations"
             << "error" << fnorm;

    return fnorm;
}
