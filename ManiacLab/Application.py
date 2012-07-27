from __future__ import print_function, unicode_literals, division
from our_future import *

import _cmaniaclab as CManiacLab

from OpenGL.GL import *

import Engine.Application
import Engine.CEngine.Window as CWindow
import Engine.CEngine.Window.key as key

from Engine.GL.Texture import Texture2D

class ManiacLab(Engine.Application.Application):
    def __init__(self):
        super(ManiacLab, self).__init__(CWindow.display, geometry=(800, 800))
        self.level = CManiacLab.Level(100, 100, True)

        self.visualization = Texture2D(512, 512, GL_RGBA)

        glClearColor(0., 0., 0., 1.);
        self._window.setTitle("ManiacLab")

        self.running = False

    def frameSynced(self):
        if self.running:
                self.level.update()

    def frameUnsynced(self, timeDelta):
        self.visualization.bind()
        self.level.physicsToGLTexture()
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
