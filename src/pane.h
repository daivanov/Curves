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

#include <QGraphicsView>
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
    QVarLengthArray<qreal,128> direction(const PointArray<256> &points, bool derivative);
    QVarLengthArray<int,128> detectSpikes(const QVarLengthArray<qreal,128> &directions, int tinySegment = 4);
    void analyse();

    QGraphicsScene *m_scene;
    PointArray<256> m_points;
    QVarLengthArray<qreal,128> m_angles;
    bool m_active;
    const qreal tolerance;
};

#endif // PANE_H
