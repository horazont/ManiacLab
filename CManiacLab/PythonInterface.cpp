/**********************************************************************
File name: PythonInterface.cpp
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
#include "PythonInterface.hpp"

#include <boost/python.hpp>

#include "Level.hpp"
#include "GameObject.hpp"

using namespace boost::python;

BOOST_PYTHON_MODULE(_cmaniaclab)
{
    class_<Level, LevelHandle>("Level", init<CoordInt, CoordInt, bool>())
        .def("update", &Level::update)
        .def("physicsToGLTexture", &Level::physicsToGLTexture)
        .def("debug_testBlockStamp", &Level::debug_testBlockStamp)
        .def("debug_testUnblockStamp", &Level::debug_testUnblockStamp);
}

void addManiacLabToInittab()
{
    PyImport_AppendInittab("_cmaniaclab", &init_cmaniaclab);
}
