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
#include <QVector>
#include <QPointF>

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
    template <class Type>
    QVector<Type> smooth(const QVector<Type> &points, int size = 3);
    QVector<qreal> direction(const QVector<QPointF> &points, bool derivative);
    QVector<int> detectSpikes(const QVector<qreal> &directions, int tinySegment = 4);
    void analyse();

    QGraphicsScene *m_scene;
    QVector<QPointF> m_points;
    QVector<qreal> m_angles;
    bool m_active;
    const qreal tolerance;
};

#endif // PANE_H
