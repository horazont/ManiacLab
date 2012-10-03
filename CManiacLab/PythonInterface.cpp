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
