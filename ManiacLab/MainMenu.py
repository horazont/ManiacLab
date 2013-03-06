# encoding=utf-8
from __future__ import print_function, unicode_literals, division
from our_future import *

import Mode
from Engine.UI import ParentWidget, ButtonWidget, AbstractVBox, LabelWidget, Space

import Engine.UI.CSS.Minilanguage

class MainMenu(AbstractVBox):
    def __init__(self, parent, **kwargs):
        super(MainMenu, self).__init__(parent, **kwargs)

        title = LabelWidget(self, text="ManiacLab")
        title.StyleClasses.add("title")
        subtitle = LabelWidget(self, text="◀ main menu ▶")
        subtitle.StyleClasses.add("subtitle")

        buttons = [
            Space(self),
            ButtonWidget(self, caption="Select profile"),
            Space(self),
            ButtonWidget(self, caption="Continue playing"),
            Space(self),
            ButtonWidget(self, caption="Map editor"),
            Space(self),
            ButtonWidget(self, caption="Quit", onclick=self.quit),
            Space(self),
            ]

        self.AbsoluteRect.Width = 400
        self.AbsoluteRect.Height = len(buttons) * 30

    def quit(self, sender):
        self.RootWidget.handleWMQuit()


class Mode(Mode.Mode):
    def __init__(self):
        super(Mode, self).__init__()
        self.desktopWidgets.append(
            MainMenu(None)
            )

Engine.UI.CSS.Minilanguage.ElementNames().registerWidgetClass(MainMenu)
