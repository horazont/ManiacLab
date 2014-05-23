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
#include <CEngine/WindowInterface/X11/X11Display.hpp>
#include <CEngine/WindowInterface/EventLoop.hpp>
#include <CEngine/IO/FileStream.hpp>

#include "Application.hpp"

using namespace PyEngine;

int main(int argc, char** argv)
{
    LogServer *const log = PyEngine::log;

    StreamHandle xmlfile(new FileStream("log.xml", OM_WRITE, WM_OVERWRITE));
    log->addSink(LogStdOutSink(All));
    log->logf(Debug, "Set up stdout sink");

    log->addSink(LogSinkHandle(
        new LogXMLSink(All & (~Debug), xmlfile, "log.xsl", "ManiacLab")
    ));
    log->logf(Debug, "Set up xml sink");
    log->logf(Information, "Log system started up successfully.");

    try {
        PyEngine::DisplayHandle dpy(new PyEngine::X11Display());
        boost::shared_ptr<Application> sink(new Application(*dpy.get()));
        PyEngine::EventLoop loop(dpy, sink);
        sink->run(&loop);
    }
    catch (const Exception& err)
    {
        log->logException(err, Panic);
        return 1;
    }
    catch (const std::exception& err)
    {
        log->log(Error) << "STL exception: " << err.what() << std::endl;
        return 1;
    }

    return 0;
}
