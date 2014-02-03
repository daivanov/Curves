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

#include <QFile>
#include <QDateTime>
#include <QtCore/qmath.h>
#include <QPointF>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>

#include "pane.h"
#include "curvefitter.h"

Pane::Pane(QWidget *parent)
    : QGraphicsView(parent),
    m_scene(new QGraphicsScene(this)),
    m_active(true),
    tolerance(2.0)
{
    m_scene->setBackgroundBrush(Qt::black);
    m_scene->installEventFilter(this);

    setRenderHints(renderHints() | QPainter::Antialiasing);
    setWindowState(windowState() ^ Qt::WindowMaximized);
    setFrameShape(QFrame::NoFrame);
    setScene(m_scene);

    m_file = new QFile("curve.csv", this);
    if (m_file->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        m_out.setDevice(m_file);
    } else {
        qWarning("Failed to open file for writing");
    }
}

Pane::~Pane()
{
    m_file->close();
}

bool Pane::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::WindowActivate: {
        QRectF newSceneRect(QPointF(0.0, 0.0), maximumViewportSize());
        m_scene->setSceneRect(newSceneRect);
    }
        break;
    case QEvent::GraphicsSceneMousePress: {
        if (!m_active) {
            /* Clear everything */
            m_points.clear();
            m_angles.clear();
            QList<QGraphicsItem*> items = m_scene->items();
            foreach (QGraphicsItem *item, items) {
                m_scene->removeItem(item);
                delete item;
            }
            m_active = true;
        }
        QGraphicsSceneMouseEvent *mouseEvent =
            static_cast<QGraphicsSceneMouseEvent*>(event);

        Q_ASSERT(mouseEvent);

        QPointF point = mouseEvent->scenePos();
        m_out << point.x() << ","
              << point.y() << ","
              << QDateTime::currentMSecsSinceEpoch() << "\n";

        m_points << point;
    }
        break;
    case QEvent::GraphicsSceneMouseMove: {
        QGraphicsSceneMouseEvent *mouseEvent =
            static_cast<QGraphicsSceneMouseEvent*>(event);

        Q_ASSERT(mouseEvent);

        QPointF point = mouseEvent->scenePos();
        m_out << point.x() << ","
              << point.y() << ","
              << QDateTime::currentMSecsSinceEpoch() << "\n";

        QPointF distance = m_points.last() - point;
        /* Too close point breaks angle computation, so we just skip them */
        if (distance.x() * distance.x() + distance.y() * distance.y() >=
                tolerance * tolerance) {
            addLine(m_points.last(), point, Qt::yellow);
            m_points << point;
        }
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

QVarLengthArray<qreal,128> Pane::direction(const PointArray<256> &points, bool derivative)
{
    QVarLengthArray<qreal,128> angles;

    QPointF p0 = (points.count() > 0 ? points.first() : QPointF(0.0, 0.0));
    qreal angle0 = 0.0;
    qreal len0 = 1.0;
    for (int i = 1; i < points.count(); ++i) {
        QPointF p1(points.at(i));
        QPointF df = p1 - p0;
        qreal len = qSqrt(df.x() * df.x() + df.y() * df.y());
        qreal sinA = df.y() / len;

        qreal angle1 = qAsin(sinA); /* [-M_PI/2 ; M_PI/2] */
        if (df.x() < 0) {
            angle1 = M_PI - angle1; /* [-M_PI/2 ; 3*M_PI/2] */
        }
        if (angle1 - angle0 > M_PI) {
            angle1 = angle1 - 2 * M_PI;
        } else if(angle0 - angle1 > M_PI) {
            angle1 = angle1 + 2 * M_PI;
        }

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

QVarLengthArray<qreal,128> Pane::length(const PointArray<256> &points)
{
    QVarLengthArray<qreal,128> lengths;

    if (points.count() > 1) {
        QPointF prev = points.at(0);
        for (int i = 1; i < points.count(); ++i) {
            QPointF curr = points.at(i);
            QPointF vector = curr - prev;
            lengths << qSqrt(vector.x() * vector.x() + vector.y() * vector.y());
            prev = curr;
        }
    }

    return lengths;
}

PointArray<256> Pane::derivative(const PointArray<256> &points)
{
    PointArray<256> dpoints;

    if (points.count() > 1) {
        QPointF prev = points.at(0);
        for (int i = 1; i < points.count(); ++i) {
            QPointF curr = points.at(i);
            dpoints << (curr - prev);
            prev = curr;
        }
    }

    return dpoints;
}

QVarLengthArray<int,128> Pane::detectOutliers(const QVarLengthArray<qreal,128> &values,
    qreal multiplier, int tinySegment)
{
    int cnt = values.count();

    qreal mean = 0;
    foreach (qreal value, values)
        mean += value / cnt;

    qreal stddev = 0;
    foreach (qreal value, values)
        stddev += (value - mean) * (value - mean) / (cnt - 1);
    stddev = qSqrt(stddev);

    QVarLengthArray<int,128> outliers;
    outliers << -1;
    for (int i = 0; i < values.count(); ++i) {
        if (qAbs(values.at(i) - mean) > multiplier * stddev) {
            if (i - outliers.at(outliers.count() - 1) > tinySegment)
                outliers << i;
        }
    }

    if ((values.count() - 1) - outliers.at(outliers.count() - 1) < tinySegment)
        outliers.remove(outliers.count() - 1);
    outliers << values.count() - 1;

    return outliers;
}

void Pane::addEllipse(const QPointF &point, const QColor &color)
{
    QGraphicsEllipseItem *ellipse =
        new QGraphicsEllipseItem(point.x() - 3.0, point.y() - 3.0, 6, 6);
    ellipse->setPen(QPen(color));
    m_scene->addItem(ellipse);
}

void Pane::addLine(const QPointF &point0, const QPointF &point1, const QColor &color)
{
    QLineF line(point0, point1);
    m_scene->addLine(line, QPen(color));
}

void Pane::analyse()
{
    PointArray<256> points1 = derivative(m_points);
    PointArray<256> points2 = derivative(points1);
    QVarLengthArray<qreal,128> stats = length(points2);
    QVarLengthArray<int,128> outliers = detectOutliers(stats, 1.5);

    /* Compensate double derivation */
    for (int i = 1; i < outliers.count(); ++i) {
        outliers[i] += 2;
    }

    CurveFitter fitter;
    PointArray<256> segment;
    int k = 1;
    for (int i = 0; i < m_points.count(); ++i) {
        segment << m_points.at(i);

        /* End of segment reached */
        if (outliers[k] + 1 == i) {
            PointArray<256> curve;
            fitter.fit(segment, curve, CurveFitter::AFFINE);

            for (int j = 0; j < curve.count(); j += 2) {
                addLine(curve.at(j), curve.at(j + 1), Qt::blue);
            }

            PointArray<256> points = fitter.curve(curve, segment.count());
            for (int j = 0; j < points.count() - 1; ++j) {
                addLine(points.at(j), points.at(j + 1), Qt::white);
            }

            for (int j = 1; j < curve.count() - 1; ++j) {
                addEllipse(curve.at(j), Qt::red);
            }

            k++;
            segment.clear();
            segment << m_points.at(i);
        }
    }

    foreach (int outlier, outliers) {
        QPointF point = m_points.at(outlier + 1);
        addEllipse(point, Qt::red);
    }
}
