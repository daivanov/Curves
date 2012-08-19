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

#include <QDebug>

#include "utils.h"

void Utils::saveToFile(const QString &fileName, const PointArray<256> &points,
    bool append)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text |
        (append ? QIODevice::Append : QIODevice::NotOpen))) {
        qCritical() << "Cannot open" << fileName << "for writing";
        return;
    }

    QTextStream in(&file);
    for (int i = 0; i < points.count() - 1; ++i)
        in << points.at(i).x() << ",";
    in << points.last().x() << "\n";

    for (int i = 0; i < points.count() - 1; ++i)
        in << points.at(i).y() << ",";
    in << points.last().y() << "\n";

    file.close();
}
