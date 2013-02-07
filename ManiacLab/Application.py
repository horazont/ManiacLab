# File name: Application.py
# This file is part of: ManiacLab
#
# LICENSE
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# FEEDBACK & QUESTIONS
#
# For feedback and questions about ManiacLab please e-mail one of the
# authors named in the AUTHORS file.
########################################################################
from __future__ import print_function, unicode_literals, division
from our_future import *

import argparse

import _cmaniaclab as CManiacLab

from OpenGL.GL import *

import Engine.Application
import Engine.CEngine.Window as CWindow
import Engine.CEngine.Window.key as key

from Engine.GL.Texture import Texture2D

class ManiacLab(Engine.Application.Application):
    def __init__(self):
        args = self.parse_args()

        super(ManiacLab, self).__init__(CWindow.display, geometry=(500, 500))
        self.level = CManiacLab.Level(50, 50, args.threaded_simulation)

        self.visualization = Texture2D(512, 512, GL_RGBA)

        glClearColor(0., 0., 0., 1.);
        self._window.setTitle("ManiacLab")

        self.running = False
        self.show_thread_regions = False

    def parse_args(self):
        parser = argparse.ArgumentParser()
        parser.add_argument(
            "--no-threaded-simulation",
            dest="threaded_simulation",
            default=True,
            action="store_false",
            help="Disable multithreading for the simulation."
        )

        return parser.parse_args()

    def frameSynced(self):
        if self.running:
            self.level.update()
            self.level.update()

    def frameUnsynced(self, timeDelta):
        self.visualization.bind()
        self.level.physicsToGLTexture(self.show_thread_regions)
        window = self._screens[0][0]
        window.switchTo()

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glLoadIdentity()
        wx, wy, ww, wh = self._primaryWidget.AbsoluteRect.XYWH
        glViewport(0, 0, ww, wh)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        glOrtho(wx, ww, wh, wy, -1., 1.)
        glMatrixMode(GL_MODELVIEW)

        glEnable(GL_TEXTURE_2D)
        glBegin(GL_QUADS)
        glTexCoord2f(0., 0.)
        glVertex2f(0, 0)
        glTexCoord2f(0., 250/512)
        glVertex2f(0, 500)
        glTexCoord2f(250/512, 250/512)
        glVertex2f(500, 500)
        glTexCoord2f(250/512, 0.)
        glVertex2f(500, 0)
        glEnd()

        window.flip()

    def handleKeyDown(self, symbol, modifiers):
        if symbol == key.Return:
            self.running = not self.running
        elif symbol == key.F1:
            self.show_thread_regions = not self.show_thread_regions
        elif symbol == 111:     # o
            self.level.debug_testObject()
        elif symbol == 115:     # s
            self.level.debug_testStamp(44, 24)

    def handleMouseDown(self, x, y, button, modifiers):
        x = x / 800 * 50
        y = y / 800 * 50
        print("{0} {1}".format(x, y))
        self.level.debug_output(x, y)
