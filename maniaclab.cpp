/**********************************************************************
File name: pyuni-client.cpp
This file is part of: Pythonic Universe

LICENSE

The contents of this file are subject to the Mozilla Public License
Version 1.1 (the "License"); you may not use this file except in
compliance with the License. You may obtain a copy of the License at
http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS"
basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
License for the specific language governing rights and limitations under
the License.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public license (the  "GPL License"), in which case  the
provisions of GPL License are applicable instead of those above.

FEEDBACK & QUESTIONS

For feedback and questions about pyuni please e-mail one of the authors
named in the AUTHORS file.
**********************************************************************/

#include <iostream>
#include <fstream>
#include <cassert>
#include <boost/python.hpp>

#include "IO/Log.hpp"
#include "IO/FileStream.hpp"
#include "IO/StdIOStream.hpp"

#include "WindowInterface/Display.hpp"
#include "WindowInterface/Window.hpp"
#include "WindowInterface/X11/X11Display.hpp"

#include "PythonInterface/Package.hpp"
#include "PythonInterface/CairoHelpers.hpp"

PyEngine::Display *disp = 0;

int main(int argc, char** argv) {
    PyEngine::StreamHandle xmlFile(new PyEngine::FileStream("log.xml", PyEngine::OM_WRITE, PyEngine::WM_OVERWRITE));
    PyEngine::log->addSink(PyEngine::LogSinkHandle(new PyEngine::LogStreamSink(PyEngine::All, PyEngine::stdout)));
    PyEngine::log->logf(PyEngine::Debug, "Set up stdout sink");
    PyEngine::log->addSink(PyEngine::LogSinkHandle(new PyEngine::LogXMLSink(PyEngine::All & (~PyEngine::Debug), xmlFile, "log.xsl")));
    PyEngine::log->logf(PyEngine::Debug, "Set up xml sink");
    PyEngine::log->logf(PyEngine::Information, "Log system started up successfully.");
    try
    {
        PyEngine::addCUniToInittab();
        Py_Initialize();
        PySys_SetArgv(argc, argv);
        // this must happen after python was initialized. We're loading
        // a module here ;)
        PyEngine::setupCairoHelpers();

        boost::python::object cuni_window_namespace = boost::python::import("_cuni_window").attr("__dict__");

        boost::python::object cuni_log_namespace = boost::python::import("_cuni_log").attr("__dict__");

        boost::python::object main_namespace = boost::python::import("__main__").attr("__dict__");

        // FIXME: Is this possible without explizit reference to the
        // platform?
        // Boost needs the explicit type for casting, but it would be
        // nice to force it somehow to do the right thing.
        PyEngine::X11Display *x11 = new PyEngine::X11Display();
        disp = x11;

        cuni_window_namespace["display"] = x11;
        cuni_log_namespace["server"] = PyEngine::logHandle;

        std::string str;
        {
            std::stringstream s;
            std::ifstream main("py-universe.py");
            s << main.rdbuf();
            main.close();
            str = std::string(s.str());
        }
        exec(str.c_str(), main_namespace);
        delete PyEngine::log;
    }
    catch (boost::python::error_already_set const&)
    {
        delete PyEngine::log;
        PyErr_Print();
        return 1;
    }
    return 0;
}

// Local Variables:
// c-file-style: "k&r"
// c-basic-offset: 4
// End:

