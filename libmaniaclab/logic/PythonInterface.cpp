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
//~ #include "Tileset.hpp"

using namespace boost::python;

BOOST_PYTHON_MODULE(_cmaniaclab)
{
  /*
    class_<TilesetInfo, TilesetInfoHandle>("TilesetInfo")
        .add_property("unique_name", &TilesetInfo::unique_name)
        .add_property("display_name", &TilesetInfo::display_name)
        .add_property("author", &TilesetInfo::author)
        .add_property("description", &TilesetInfo::description)
        ;

    class_<Tileset, TilesetHandle>("Tileset", init<>())
        .add_property("unique_name",
                      make_function(&Tileset::get_unique_name,
                                    return_value_policy<copy_const_reference>()),
                      &Tileset::set_unique_name)
        .add_property("display_name",
                      make_function(&Tileset::get_display_name,
                                    return_value_policy<copy_const_reference>()),
                      &Tileset::set_display_name)
        .add_property("author",
                      make_function(&Tileset::get_author,
                                    return_value_policy<copy_const_reference>()),
                      &Tileset::set_author)
        .add_property("description",
                      make_function(&Tileset::get_description,
                                    return_value_policy<copy_const_reference>()),
                      &Tileset::set_description)
        .def("save_to_stream", &Tileset::save_to_stream)
        ;
    */

    /*class_<Level, LevelHandle>("Level", init<CoordInt, CoordInt, bool>())
        .def("update", &Level::update)
        .def("physics_to_gl_texture", &Level::physics_to_gl_texture)
        .def("debug_test_object", &Level::debug_test_object)
        .def("debug_test_stamp", &Level::debug_test_stamp)
        .def("debug_output", &Level::debug_output);*/

    /*def("read_tileset_info", &TilesetInfo::read_from_stream,
        return_value_policy<manage_new_object>());
    def("load_tileset_from_stream", &Tileset::load_from_stream,
        return_value_policy<manage_new_object>());*/
}

void add_maniac_lab_to_inittab()
{
    PyImport_AppendInittab("_cmaniaclab", &init_cmaniaclab);
}
