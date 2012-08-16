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
    Q_ASSERT(sizeof(qreal) == sizeof(double));
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
            bins[i] *= (double) j / (j - a);
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

QVector<QPointF> CurveFitter::curve(const QVector<QPointF> &curvePoints, int count)
{
    qreal *pxy = new qreal[SPLINE_SIZE];

    for (int i = 0; i < SPLINE_SIZE; i += 2) {
        const QPointF &point = curvePoints.at(i / 2);
        pxy[i] = point.x();
        pxy[i + 1] = point.y();
    }

    qreal *x = new qreal[2 * count];

    curve(pxy, count, x);

    QVector<QPointF> points;
    for (int i = 0; i < 2 * count; i += 2) {
        points << QPointF(x[i], x[i + 1]);
    }

    delete [] pxy;
    delete [] x;

    return points;
}

void CurveFitter::curve(const qreal *pxy, int num, qreal *xy)
{
    int i; qreal *cxy; qreal t;
    for (i = 0, cxy = xy, t = 0.0; i < num; ++i, cxy += 2, t += 1.0/(num - 1))
        point(pxy, t, cxy);
}

void CurveFitter::func(double *p, double *hx, int m, int n, void *data)
{
    qreal *pxy = (qreal*)data;
    if (pxy + 2 != p)
        memcpy(pxy + 2, p, sizeof(qreal) * m);
    curve(pxy, n / 2, hx);
}

qreal CurveFitter::fit(const QVector<QPointF> &points, QVector<QPointF> &curve)
{
    int sz = 2 * points.count();

    /* Init input data */
    qreal *x = (qreal*)calloc(sz, sizeof(qreal));
    for (int i = 0; i < points.count(); ++i) {
        const QPointF &point = points.at(i);
        x[2 * i] = point.x();
        x[2 * i + 1] = point.y();
    }

    qreal *pxy = (qreal*)calloc(SPLINE_SIZE, sizeof(qreal));

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
    double *p = pxy + 2;
    int m = SPLINE_SIZE - 2 * 2;
    int n = sz;
    dlevmar_dif(CurveFitter::func, p, x, m, n, MAX_ITER, NULL, info, NULL,
        NULL, pxy);

    curve.clear();
    for (int i = 0; i < SPLINE_SIZE; i += 2)
        curve << QPointF(pxy[i], pxy[i + 1]);

    free(x);
    free(pxy);

    qDebug() << "Termination reason" << info[6] << "after" << info[5] << "iterations";
    /* Residuals */
    qreal fnorm = info[1] / sz;

    return fnorm;
}
