# File name: Mode.py
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

class Mode(object):
    def __init__(self):
        self.windows = []
        self.desktopWidgets = []

    def enable(self, onDesktop):
        windowLayer = onDesktop.RootWidget.WindowLayer
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
