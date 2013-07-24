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
