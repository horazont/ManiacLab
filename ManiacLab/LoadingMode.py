# File name: LoadingMode.py
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

import threading

import Mode
from Engine.UI import ParentWidget, Button, AbstractVBox, LabelWidget, \
    Space, HBox

import Engine.UI.CSS.Minilanguage

class Loading(AbstractVBox):
    def __init__(self, parent, **kwargs):
        super(Loading, self).__init__(parent, **kwargs)

        title = LabelWidget(self, text="ManiacLab")
        title.StyleClasses.add("title")

        Space(self).StyleClasses.add("large")
        LabelWidget(self, text="Loading")
        Space(self).StyleClasses.add("small")
        self._curr_task_label = LabelWidget(self, text="...")
        Space(self).StyleClasses.add("large")

        self._curr_task_mutex = threading.Lock()

        self._progress = 0

    def set_current_task(self, text):
        with self._curr_task_mutex:
            self._curr_task_label.Text = text

    def do_align(self):
        with self._curr_task_mutex:
            super(Loading, self).do_align()

    def render(self):
        with self._curr_task_mutex:
            super(Loading, self).render()

class Mode(Mode.Mode):
    def __init__(self):
        super(Mode, self).__init__()
        self._widget = Loading(None)
        self.desktopWidgets.append(
            self._widget
            )

    def set_current_task(self, text):
        self._widget.set_current_task(text)

Engine.UI.CSS.Minilanguage.ElementNames().register_widget_class(Loading)
