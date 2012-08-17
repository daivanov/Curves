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

#define CURVE_LENGTH    80
#define DUMP_FILE       "curves.csv"

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

    /* Original Bezier curve points */
    PointArray<256> points = m_fitter->curve(curve, CURVE_LENGTH);

    /* Computed Bezier curve */
    PointArray<256> curve2;
    qreal err = m_fitter->fit(points, curve2);
    QVERIFY(err < 1e-4);
    qreal stdCurve = 0;
    for (int i = 0; i < curve.count() && i < curve2.count(); ++i) {
        QPointF diff = curve.at(i) - curve2.at(i);
        stdCurve += (diff.x() * diff.x() + diff.y() * diff.y()) / CURVE_LENGTH;
    }
    stdCurve = qSqrt(stdCurve);
    QVERIFY(stdCurve < 1e-4);

    /* Computed Bezier curve points */
    PointArray<256> points2 = m_fitter->curve(curve2, CURVE_LENGTH);
    qreal stdPoints = 0;
    for (int i = 0; i < points.count() && i < points2.count(); ++i) {
        QPointF diff = points.at(i) - points2.at(i);
        stdPoints += (diff.x() * diff.x() + diff.y() * diff.y()) / CURVE_LENGTH;
    }
    stdPoints = qSqrt(stdPoints);
    QVERIFY(stdPoints < 1e-4);
    saveToFile(points, false);
    saveToFile(points2, true);
}

void CurveTest::testSplit()
{
    /* Original Bezier curve */
    PointArray<256> curve;
    curve << QPointF(0.0, 0.0) << QPointF(-0.25, 1.0)
          << QPointF(1.25, -1.0) << QPointF(1.0, 0.0);
    PointArray<256> left, right;

    m_fitter->splitCasteljau(curve, 0.3, left, right);

    QCOMPARE(curve[0], left[0]);
    QCOMPARE(curve[1], left[1]);
    QCOMPARE(left[SPLINE_LEN * 2 - 2], right[0]);
    QCOMPARE(left[SPLINE_LEN * 2 - 1], right[1]);
    QCOMPARE(curve[SPLINE_LEN * 2 - 2], right[SPLINE_LEN * 2 - 2]);
    QCOMPARE(curve[SPLINE_LEN * 2 - 1], right[SPLINE_LEN * 2 -1]);
}

void CurveTest::saveToFile(const PointArray<256> &points, bool append)
{
    QFile file(DUMP_FILE);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text |
                   (append ? QIODevice::Append : QIODevice::NotOpen)))
        return;
    QTextStream in(&file);
    for (int i = 0; i < points.count(); ++i) {
        in << points.at(i).x() << ",";
    }
    in << points.last().x() << "\n";
    for (int i = 0; i < points.count(); ++i) {
        in << points.at(i).y() << ",";
    }
    in << points.last().y() << "\n";
    file.close();
}

void CurveTest::cleanupTestCase()
{
    qDebug("Test case cleanup");
    delete m_fitter;
}

QTEST_MAIN(CurveTest)
