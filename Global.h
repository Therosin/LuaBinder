// Copyright (C) 2023 Theros < MisModding | SvalTek >
// 
// This file is part of LuaBinder.
// 
// LuaBinder is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// LuaBinder is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with LuaBinder.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#include <string>
#include <iostream>
#include "LuaScript.h"

/** format text like s_format
 * @param format format string
 * @param ... format string arguments
 */
std::string formatText(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return std::string(buffer);
}
