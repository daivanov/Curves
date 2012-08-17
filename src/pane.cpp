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
#include <QPointF>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include "pane.h"

Pane::Pane(QWidget *parent)
    : QGraphicsView(parent),
    m_scene(new QGraphicsScene(this)),
    m_active(true),
    tolerance(1e-5)
{
    m_scene->setBackgroundBrush(Qt::black);
    m_scene->installEventFilter(this);

    setRenderHints(renderHints() | QPainter::Antialiasing);
    setWindowState(windowState() ^ Qt::WindowMaximized);
    setFrameShape(QFrame::NoFrame);
    setScene(m_scene);
}

Pane::~Pane()
{
}

bool Pane::eventFilter(QObject *obj, QEvent *event)
{
    if (!m_active)
        return QObject::eventFilter(obj, event);

    switch (event->type()) {
    case QEvent::WindowActivate: {
        QRectF newSceneRect(QPointF(0.0, 0.0), maximumViewportSize());
        m_scene->setSceneRect(newSceneRect);
    }
        break;
    case QEvent::GraphicsSceneMousePress: {
        QGraphicsSceneMouseEvent *mouseEvent =
            static_cast<QGraphicsSceneMouseEvent*>(event);

        Q_ASSERT(mouseEvent);

        m_points << mouseEvent->scenePos();
    }
        break;
    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent *mouseEvent =
            static_cast<QGraphicsSceneMouseEvent*>(event);

        Q_ASSERT(mouseEvent);

        QLineF line(m_points.last(), mouseEvent->scenePos());
        m_scene->addLine(line, QPen(Qt::yellow));
        m_points << mouseEvent->scenePos();
    }
        break;
    case QEvent::GraphicsSceneMouseRelease: {
        /* Proceed with analysis */
        m_active = false;
        analyse();
    }
        break;
    default:
        break;
    }
    // standard event processing
    return QObject::eventFilter(obj, event);
}

QVector<qreal> Pane::direction(const QVector<QPointF> &points, bool derivative)
{
    QVector<qreal> angles;

    QPointF p0 = (points.count() > 0 ? points.first() : QPointF(0.0, 0.0));
    qreal angle0 = 0.0;
    qreal len0 = 1.0;
    for (int i = 1; i < points.count(); ++i) {
        QPointF p1(points.at(i));
        QPointF df = p1 - p0;
        qreal len = qSqrt(df.x() * df.x() + df.y() * df.y());
        if (len < tolerance)
            continue;
        qreal sinA = df.y() / len;

        qreal angle1 = qAsin(sinA); /* [-M_PI/2 ; M_PI/2] */
        if (df.x() < 0)
            angle1 = (angle1 > 0 ? M_PI : -M_PI) - angle1; /* [-M_PI ; M_PI] */

        if (derivative) {
            qreal da = angle1 - angle0; /* [-2*M_PI ; 2*M_PI] */
            if (da > M_PI)
                da -= M_PI;             /* [-2*M_PI ;   M_PI] */
            else if (da < -M_PI)
                da += M_PI;             /* [  -M_PI ;   M_PI] */

            da /= 0.5 * (len + len0);
            angles << da;
        } else {
            angles << angle1;
        }

        p0 = p1;
        angle0 = angle1;
        len0 = len;
    }
    return angles;
}

QVector<int> Pane::detectSpikes(const QVector<qreal> &directions,
    int tinySegment)
{
    int cnt = directions.count();

    qreal mean = 0;
    foreach (qreal direction, directions)
        mean += direction / cnt;

    qreal stddev = 0;
    foreach (qreal direction, directions)
        stddev += (direction - mean) * (direction - mean) / cnt;
    stddev = qSqrt(stddev);

    QVector<int> spikes;
    spikes << 0;
    for (int i = 0; i < directions.count(); ++i) {
        if (qAbs(directions.at(i) - mean) > stddev) {
            if (i - spikes.last() > tinySegment)
                spikes << i;
        }
    }

    if ((directions.count() - 1) - spikes.last() < tinySegment)
        spikes.remove(spikes.count() - 1);
    spikes << directions.count() - 1;

    return spikes;
}

void Pane::analyse()
{
    QVector<qreal> angles = direction(m_points, true);
    QVector<int> spikes = detectSpikes(angles);

    foreach (int spike, spikes) {
        QPointF point = m_points.at(spike + 1);
        QGraphicsEllipseItem *ellipse =
            new QGraphicsEllipseItem(point.x() - 5.0, point.y() - 5.0, 10, 10);
        ellipse->setBrush(Qt::red);
        m_scene->addItem(ellipse);
    }
}
