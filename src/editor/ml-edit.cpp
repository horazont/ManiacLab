/**********************************************************************
File name: ml-edit.cpp
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
#include <cassert>

#include <gtkmm.h>

#include "RootWindow.hpp"

#include <CEngine/IO/FileStream.hpp>
#include <CEngine/IO/Log.hpp>

#include "io/TilesetData.hpp"

using namespace Gtk;
using namespace Glib;
using namespace PyEngine;

int main(int argc, char *argv[])
{
    LogServer *const log = PyEngine::log;

    StreamHandle xmlfile(new FileStream("ml-edit.log.xml", OM_WRITE, WM_OVERWRITE));
    log->addSink(LogStdOutSink(All));
    log->addSink(LogSinkHandle(
        new LogXMLSink(All & (~Debug), xmlfile, "log.xsl", "ManiacLab Editor")
    ));

    Main app(argc, argv);

    RefPtr<Builder> builder = Builder::create_from_file("data/ui/editor.glade");

    RootWindow *window;

    builder->get_widget_derived("root", window);
    assert(window);

    app.run(*window);
    delete window;
    return 0;
}
