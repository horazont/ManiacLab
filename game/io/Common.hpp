/**********************************************************************
File name: Common.hpp
This file is part of: ManiacLab

LICENSE

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program.  If not, see <http://www.gnu.org/licenses/>.

FEEDBACK & QUESTIONS

For feedback and questions about ManiacLab please e-mail one of the
authors named in the AUTHORS file.
**********************************************************************/
#ifndef _ML_IO_COMMON_H
#define _ML_IO_COMMON_H

#include <stdexcept>

enum TileVisualFormat {
    TVF_BGRA
};

enum PhysicsInitialAttribute {
    PHYATTR_AIR_PRESSURE = 0,
    PHYATTR_TEMPERATURE = 1,
    PHYATTR_FOG_DENSITY = 2
};

enum TileLayer {
    TILELAYER_AFFECTOR = -1,
    TILELAYER_DEFAULT = 0
};

enum IOQuality {
    /* No errors or warnings occurred while loading the data. */
    IOQ_PERFECT = 0,

    /* Some data was ignored (e.g. out-of-bounds values) */
    IOQ_DEGRADED = 1,

    /* An error occured and the import failed fatally */
    IOQ_ERRORNOUS = 2
};

class LevelIOError: public std::runtime_error {
public:
    LevelIOError(const std::string &what);
    LevelIOError(const char *what);

};

#endif
