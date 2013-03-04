from __future__ import print_function, unicode_literals, division
from our_future import *

class Mode(object):
    def __init__(self):
        self.windows = []
        self.desktopWidgets = []

    def enable(self, onDesktop):
        windowLayer = onDesktop.getRootWidget().WindowLayer
        for window in self.windows:
            windowLayer.add(window)
        for desktopW in self.desktopWidgets:
            onDesktop.add(desktopW)

    def disable(self):
        for window in self.windows:
            window.Parent.remove(window)
        for desktopW in self.desktopWidgets:
            desktopW.Parent.remove(desktopW)

    def frame_synced(self):
        pass

    def frame_unsynced(self, deltaT):
        pass
