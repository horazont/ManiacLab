# encoding=utf-8
from __future__ import print_function, unicode_literals, division
from our_future import *

import Mode
from Engine.UI import ParentWidget, ButtonWidget, VBox, LabelWidget

import Engine.UI.CSS.Minilanguage

class MainMenu(VBox):
    def __init__(self, parent, **kwargs):
        super(MainMenu, self).__init__(parent, **kwargs)

        title = LabelWidget(self, text="ManiacLab")
        title.StyleClasses.add("title")
        subtitle = LabelWidget(self, text="◀ main menu ▶")
        subtitle.StyleClasses.add("subtitle")

        buttons = [
            ButtonWidget(self, caption="Select profile"),
            ButtonWidget(self, caption="Continue playing"),
            ButtonWidget(self, caption="Map editor"),
            ButtonWidget(self, caption="Quit")
            ]

        self.AbsoluteRect.Width = 400
        self.AbsoluteRect.Height = len(buttons) * 30


class Mode(Mode.Mode):
    def __init__(self):
        super(Mode, self).__init__()
        self.desktopWidgets.append(
            MainMenu(None)
            )

Engine.UI.CSS.Minilanguage.ElementNames().registerWidgetClass(MainMenu)
