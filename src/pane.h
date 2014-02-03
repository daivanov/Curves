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

#ifndef PANE_H
#define PANE_H

#include <QFile>
#include <QGraphicsView>
#include <QTextStream>
#include <QVarLengthArray>

#include "pointarray.h"

class QPointF;
class QGraphicsScene;

class Pane : public QGraphicsView
{
    Q_OBJECT

public:
    Pane(QWidget *parent = 0);
    ~Pane();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void addEllipse(const QPointF &point, const QColor &color);
    void addLine(const QPointF &point0, const QPointF &point1, const QColor &color);

    QVarLengthArray<qreal,128> direction(const PointArray<256> &points, bool derivative);

    QVarLengthArray<qreal,128> length(const PointArray<256> &points);
    PointArray<256> derivative(const PointArray<256> &points);

    QVarLengthArray<int,128> detectOutliers(const QVarLengthArray<qreal,128> &outliers,
        qreal multiplier, int tinySegment = 4);
    void analyse();

    QFile *m_file;
    QTextStream m_out;
    QGraphicsScene *m_scene;
    PointArray<256> m_points;
    QVarLengthArray<qreal,128> m_angles;
    bool m_active;
    const qreal tolerance;
};

#endif // PANE_H
