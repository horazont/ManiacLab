from __future__ import print_function, unicode_literals, division
from our_future import *

import Engine.Application
import Engine.CEngine.Window as CWindow

class ManiacLab(Engine.Application.Application):
    def __init__(self):
        super(ManiacLab, self).__init__(CWindow.display)

    def handleMouseDown(self, x, y, button, modifiers):
        print(x, y)
