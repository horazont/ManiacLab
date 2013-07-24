# encoding=utf-8
# File name: MainMenu.py
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

import Mode
from Engine.UI import ParentWidget, Button, AbstractVBox, LabelWidget, \
    Space, HBox

import Engine.UI.CSS.Minilanguage

class MainMenu(AbstractVBox):
    def __init__(self, parent, **kwargs):
        super(MainMenu, self).__init__(parent, **kwargs)

        title = LabelWidget(self, text="ManiacLab")
        title.StyleClasses.add("title")
        subtitle = LabelWidget(self, text="◀ main menu ▶")
        subtitle.StyleClasses.add("subtitle")


        Space(self),
        Button(self, caption="Select profile"),
        Space(self),
        Button(self, caption="Continue playing"),
        Space(self),
        editor_hbox = HBox(self)
        Button(editor_hbox, caption="Map editor", on_click=self.mapEditor),
        Space(editor_hbox)
        Button(editor_hbox, caption="Tileset editor", on_click=self.tilesetEditor),
        Space(self),
        Button(self, caption="Quit", on_click=self.quit),
        Space(self),

        self.AbsoluteRect.Width = 400
        self.AbsoluteRect.Height = 6 * 30

    def mapEditor(self, sender):
        self.RootWidget.switchMode(self.RootWidget.mapEditorMode)

    def tilesetEditor(self, sender):
        self.RootWidget.switchMode(self.RootWidget.tilesetEditorMode)

    def quit(self, sender):
        self.RootWidget.handleWMQuit()


class Mode(Mode.Mode):
    def __init__(self):
        super(Mode, self).__init__()
        self.desktopWidgets.append(
            MainMenu(None)
            )

Engine.UI.CSS.Minilanguage.ElementNames().register_widget_class(MainMenu)
