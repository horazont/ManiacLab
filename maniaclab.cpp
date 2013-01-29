/**********************************************************************
File name: maniaclab.cpp
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

#include <iostream>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <cfenv>

#include <boost/python.hpp>

#include "CEngine/IO/Log.hpp"
#include "CEngine/IO/FileStream.hpp"
#include "CEngine/IO/StdIOStream.hpp"
#include "CEngine/IO/Thread.hpp"

#include "CEngine/WindowInterface/Display.hpp"
#include "CEngine/WindowInterface/Window.hpp"
#include "CEngine/WindowInterface/X11/X11Display.hpp"

#include "CEngine/PythonInterface/Package.hpp"
#include "CEngine/PythonInterface/CairoHelpers.hpp"

#include "CManiacLab/PythonInterface.hpp"

using namespace boost::python;
using namespace PyEngine;

PyEngine::Display *disp = 0;

int main(int argc, char** argv) {
    int exitCode = 0;

    // to avoid name conflict with log from cmath
    LogServer *const log = PyEngine::log;

    StreamHandle xmlFile(new FileStream("log.xml", OM_WRITE, WM_OVERWRITE));
    log->addSink(LogStdOutSink(All & (~Debug)));
    log->logf(Debug, "Set up stdout sink");

    log->addSink(LogSinkHandle(
        new LogXMLSink(All & (~Debug), xmlFile, "log.xsl", "ManiacLab")
    ));
    log->logf(Debug, "Set up xml sink");
    log->logf(Information, "Log system started up successfully.");
    try
    {
        log->logf(Information, "Setting up boost/python environment");
        addCUniToInittab();
        addManiacLabToInittab();
        log->logf(Information, "Initializing python");
        Py_Initialize();
        PySys_SetArgv(argc, argv);

        // this must happen after python was initialized. We're loading
        // a module here ;)
        log->logf(Information, "Load some cairo helpers");
        setupCairoHelpers();

        log->logf(Information, "Setting up python runtime");
        object cuni_window_namespace = import("_cuni_window").attr("__dict__");
        object cuni_log_namespace = import("_cuni_log").attr("__dict__");
        object main_namespace = import("__main__").attr("__dict__");

        // FIXME: Is this possible without explicit reference to the
        // platform?
        // Boost needs the explicit type for casting, but it would be
        // nice to force it somehow to do the right thing.
        log->getChannel("platform")->logf(Information, "Setting up X11 environment");
        X11Display *x11 = new X11Display();
        disp = x11;

        cuni_window_namespace["display"] = x11;
        cuni_log_namespace["server"] = logHandle;

        log->logf(Information, "Loading python main module");
        std::string str;
        {
            std::stringstream s;
            std::ifstream main("maniaclab.py");
            if (!main.good()) {
                log->logf(Panic, "Could not find python main file. Terminating.");
                exitCode = 2;
                goto error;
            }
            s << main.rdbuf();
            main.close();
            str = std::string(s.str());
        }
        log->logf(Information, "Handing over control to python");
        exec(str.c_str(), main_namespace);
    }
    catch (boost::python::error_already_set const&)
    {
        PyErr_Print();
        exitCode = 1;
        goto error;
    }
    catch (Exception const& err)
    {
        log->logException(err, Panic);
        exitCode = 1;
        goto noPythonError;
    }

    error:
    // XXX: This is neccessary as python won't free it soon enough for the
    // finalizing writes to happen.
    delete log;
    noPythonError:
    return exitCode;
}

// Local Variables:
// c-file-style: "k&r"
// c-basic-offset: 4
// End:

