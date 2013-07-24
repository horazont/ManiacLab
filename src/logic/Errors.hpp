/**********************************************************************
File name: Errors.hpp
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
#ifndef _ML_ERRORS_H
#define _ML_ERRORS_H

#include "CEngine/Misc/Exception.hpp"

class ProgrammingError: public PyEngine::Exception {
public:
    ProgrammingError(const std::string &message): PyEngine::Exception(message) {};
    ProgrammingError(const char *message): PyEngine::Exception(message) {};
};

#endif