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
#include <unistd.h>
#include <thread>

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

