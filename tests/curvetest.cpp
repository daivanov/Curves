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

#include <QtTest>

#include "curvetest.h"
#include "utils.h"

#define CURVE_LENGTH    80
#define DUMP_FILE       "curves.csv"
#define EPSILON         1e-4

CurveTest::CurveTest(QObject *parent) : QObject(parent), m_fitter(0)
{
}

void CurveTest::initTestCase()
{
    qDebug("Test case initialization");
    m_fitter = new CurveFitter();
}

void CurveTest::testCurve()
{
    /* Original Bezier curve */
    PointArray<256> curve;
    curve << QPointF(0.0, 0.0) << QPointF(-0.25, 1.0)
          << QPointF(1.25, -1.0) << QPointF(1.0, 0.0);
    QCOMPARE(curve.count(), 4);

    /* Original Bezier curve points */
    PointArray<256> points = m_fitter->curve(curve, CURVE_LENGTH);

    /* Computed Bezier curve */
    PointArray<256> curve2;
    qreal err = m_fitter->fit(points, curve2);
    QVERIFY(err < EPSILON);
    qreal stdCurve = 0;
    for (int i = 0; i < curve.count() && i < curve2.count(); ++i) {
        QPointF diff = curve.at(i) - curve2.at(i);
        stdCurve += (diff.x() * diff.x() + diff.y() * diff.y()) / CURVE_LENGTH;
    }
    stdCurve = qSqrt(stdCurve);
    QVERIFY(stdCurve < EPSILON);

    /* Computed Bezier curve points */
    PointArray<256> points2 = m_fitter->curve(curve2, CURVE_LENGTH);
    qreal stdPoints = 0;
    for (int i = 0; i < points.count() && i < points2.count(); ++i) {
        QPointF diff = points.at(i) - points2.at(i);
        stdPoints += (diff.x() * diff.x() + diff.y() * diff.y()) / CURVE_LENGTH;
    }
    stdPoints = qSqrt(stdPoints);
    QVERIFY(stdPoints < EPSILON);
    Utils::saveToFile(DUMP_FILE, points, false);
    Utils::saveToFile(DUMP_FILE, points2, true);
}

void CurveTest::testSplit()
{
    /* Original Bezier curve */
    PointArray<256> curve;
    curve << QPointF(0.0, 0.0) << QPointF(-0.25, 1.0)
          << QPointF(1.25, -1.0) << QPointF(1.0, 0.0);
    PointArray<256> left, right;

    for (int i = 4; i > 1; --i) {
        m_fitter->splitCasteljau(curve, 0.3, left, right);

        QCOMPARE(curve.count(), i);
        QCOMPARE(left.count(), i);
        QCOMPARE(right.count(), i);
        QCOMPARE(curve[0], left[0]);
        QCOMPARE(curve[1], left[1]);
        QCOMPARE(left[i * 2 - 2], right[0]);
        QCOMPARE(left[i * 2 - 1], right[1]);
        QCOMPARE(curve[i * 2 - 2], right[i * 2 - 2]);
        QCOMPARE(curve[i * 2 - 1], right[i * 2 -1]);
        curve.removeLast();
    }
}

qreal CurveTest::func(qreal x, void *data)
{
    CurveTest *ct = static_cast<CurveTest*>(data);
    return ct->m_a * x * x +  ct->m_b *x + ct->m_c;
}

void CurveTest::testGoldenSectionSearch()
{
    m_a = 1.0;
    m_b = 2.0;
    m_c = 3.0;
    qreal epsilon = 0.01;
    qreal result = m_fitter->goldenSectionSearch(func, -5.0, 5.0, epsilon, this);

    QVERIFY((result - (-m_b / 2 * m_a)) < epsilon);
}

void CurveTest::testReparametrization()
{
    /* Original Bezier curve */
    PointArray<256> curve;
    curve << QPointF(0.0, 0.0) << QPointF(-0.25, 1.0)
          << QPointF(1.25, -1.0) << QPointF(1.0, 0.0);
    qreal *curveData = curve.data();
    qreal xy[2];
    qreal t = 0.5, newT = 0.6;
    m_fitter->point(curveData, t, xy);

    m_fitter->reparametrizePoints(curveData, 1, xy, &newT);
    QVERIFY(t - newT < EPSILON);
}

void CurveTest::cleanupTestCase()
{
    qDebug("Test case cleanup");
    delete m_fitter;
}

QTEST_MAIN(CurveTest)
