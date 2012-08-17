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

#ifndef CURVETEST_H
#define CURVETEST_H

#include "curvefitter.h"
#include <QObject>

class CurveTest : public QObject
{
    Q_OBJECT

public:
    CurveTest(QObject *parent = 0);

protected:
    void saveToFile(const PointArray<256> &points, bool append);

private slots:
    void initTestCase();
    void testCurve();
    void testSplit();
    void cleanupTestCase();

private:
    CurveFitter *m_fitter;
};

#endif // CURVETEST_H