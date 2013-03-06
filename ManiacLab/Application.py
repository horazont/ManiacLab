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
import os
import sys
import copy

import _cmaniaclab as CManiacLab

from OpenGL.GL import *

import Engine.Application
import Engine.CEngine.Window as CWindow
import Engine.CEngine.Window.key as key
import Engine.CEngine.GL as CGL

from Engine.GL.Texture import Texture2D
from Engine.VFS.FileSystem import XDGFileSystem, MountPriority
from Engine.VFS.Mounts import MountDirectory
from Engine.Resources.Manager import ResourceManager
import Engine.Resources.PNGTextureLoader
import Engine.Resources.ModelLoader
import Engine.Resources.CSSLoader
import Engine.Resources.MaterialLoader
import Engine.Resources.ShaderLoader

from Engine.GL import makePOT

from Engine.UI import SceneWidget, VBox, HBox, LabelWidget, WindowWidget
from Engine.UI.Theme import Theme
import Engine.GL.Base as GL
from Engine.UI.CSS.Rect import Rect

import MainMenu

class ManiacLab(Engine.Application.Application):
    def __init__(self, args):
        display = CWindow.display
        modes = display.DisplayModes
        modes.sort(reverse=True)
        displayMode = modes[0]
        candidate = CWindow.DisplayMode(displayMode)
        candidate.samples = 0

        if candidate in modes:
            displayMode = candidate

        candidate = CWindow.DisplayMode(displayMode)
        candidate.stencilBits = 0

        if candidate in modes:
            displayMode = candidate

        super(ManiacLab, self).__init__(CWindow.display,
                                        geometry=(1024,768),
                                        displayMode=displayMode)
        del displayMode
        del candidate
        vfs = XDGFileSystem('maniaclab')
        vfs.mount('/data', MountDirectory(os.path.join(os.getcwd(), "data")), MountPriority.FileSystem)
        ResourceManager(vfs)

        glClearColor(0., 0.05, 0.1, 1.);
        self._window.setTitle("ManiacLab")

        theme = Theme()
        theme.addRules(ResourceManager().require("ui.css"))
        self.Theme = theme

        self.show_thread_regions = False

        self.currentMode = None
        self.mainMenuMode = MainMenu.Mode()

        self.switchMode(self.mainMenuMode)

    def _recreateCairoContext(self, width, height):
        super(ManiacLab, self)._recreateCairoContext(width, height)
        potW, potH = makePOT(width), makePOT(height)

        self.cairoTexCoords = (width / potW, height / potH)

        self.cairoTex = Texture2D(
            potW, potH, format=GL_RGBA,
            data=(GL_RGBA, GL_UNSIGNED_BYTE, None))

    def switchMode(self, mode):
        if self.currentMode is not None:
            self.currentMode.disable()
        self.currentMode = mode
        self.currentMode.enable(self.mainScreen)
        self.invalidate()

    def frameSynced(self):
        pass

    def frameUnsynced(self, timeDelta):
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

        self.render()

        self.cairoTex.bind()

        if self.surfaceDirty or True:
            # only re-transfer texture if something changed
            CGL.glTexCairoSurfaceSubImage2D(GL_TEXTURE_2D, 0, 0, 0, self._cairoSurface)
        s, t = self.cairoTexCoords
        glEnable(GL_TEXTURE_2D)
        glEnable(GL_BLEND)
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)
        glBegin(GL_QUADS)
        glTexCoord2f(0, 0)
        glVertex2f(0, 0)
        glTexCoord2f(0, t)
        glVertex2f(0, wh)
        glTexCoord2f(s, t)
        glVertex2f(ww, wh)
        glTexCoord2f(s, 0)
        glVertex2f(ww, 0)
        glEnd()
        Texture2D.unbind()

        window.flip()
