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

#ifndef POINT_ARRAY_H
#define POINT_ARRAY_H

#include <QVarLengthArray>
#include <QPointF>

template <int Prealloc>
class PointArray : public QVarLengthArray<qreal, Prealloc>
{
public:
    inline explicit PointArray(int size = 0) :
        QVarLengthArray<qreal,Prealloc>(size) {};
    inline ~PointArray() {};

    inline QPointF at(int i) const {
        return QPointF(QVarLengthArray<qreal,Prealloc>::at(2 * i),
                       QVarLengthArray<qreal,Prealloc>::at(2 * i + 1));
    };
    inline QPointF first(void) const { return at(0); };
    inline QPointF last(void) const  { return at(count() - 1);
    };

    inline void removeLast(void) {
        QVarLengthArray<qreal, Prealloc>::removeLast();
        QVarLengthArray<qreal, Prealloc>::removeLast();
    };
    inline void resize(int size) {
        QVarLengthArray<qreal, Prealloc>::resize(2 * size);
    };
    inline int count(void) const {
        return QVarLengthArray<qreal,Prealloc>::count() / 2;
    };

    inline PointArray<Prealloc> &operator<< (const QPointF &value) {
        QVarLengthArray<qreal, Prealloc>::append(value.x());
        QVarLengthArray<qreal, Prealloc>::append(value.y());
        return *this;
    };
};

#endif // POINT_ARRAY_H
