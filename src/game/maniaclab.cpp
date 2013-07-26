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
        PyEngine::EventSinkHandle sink(new Application(*dpy.get()));
        PyEngine::EventLoop loop(dpy, sink);
        loop.run();
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
