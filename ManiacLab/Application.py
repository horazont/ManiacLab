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

        super(ManiacLab, self).__init__(CWindow.display, geometry=(800, 800))
        self.level = CManiacLab.Level(100, 100, args.threaded_simulation)

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
        glTexCoord2f(0., 400/512)
        glVertex2f(0, 800)
        glTexCoord2f(400/512, 400/512)
        glVertex2f(800, 800)
        glTexCoord2f(400/512, 0.)
        glVertex2f(800, 0)
        glEnd()

        window.flip()

    def handleKeyDown(self, symbol, modifiers):
        if symbol == key.Return:
            self.running = True
        elif symbol == key.F1:
            self.show_thread_regions = not self.show_thread_regions
        elif symbol == 98:      # b
            self.level.debug_testBlockStamp()
        elif symbol == 116:     # t
            self.level.debug_testUnblockStamp()
