/*
 * Copyright (C) 2021 zkyd
 *
 * Author:     zkyd <zkyd@zjide.org>
 *
 * Maintainer: zkyd <zkyd@zjide.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <wglobal.h>

#include <QtCore/qobjectdefs.h>
#include <QtCore/qmetaobject.h>

WAYLIB_SERVER_BEGIN_NAMESPACE

class WLR {
public:
    enum Transform {
        Normal = 0,
        R90 = 1,
        R180 = 2,
        R270 = 3,
        Flipped = 4,
        Flipped90 = 5,
        Flipped180 = 6,
        Flipped270 = 7
    };
};

WAYLIB_SERVER_END_NAMESPACE
