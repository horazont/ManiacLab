# encoding=utf-8
# File name: MapEditor.py
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
from Engine.UI import ParentWidget, ButtonWidget, \
    AbstractVBox, LabelWidget, Space, HBox, \
    MenuBar, Menu, Edit, ScrollBar, ScrollMode, VBox, Window

import Engine.UI.CSS.Minilanguage

class MapEditor(AbstractVBox):
    def __init__(self, parent, **kwargs):
        super(MapEditor, self).__init__(parent, **kwargs)

        titleBox = HBox(self)
        titleBox.StyleClasses.add("title")

        # edit = Edit(titleBox)

        menuBar = MenuBar(titleBox)

        _, menu_map = menuBar.add_button_and_menu("Map")
        menu_map.add_button("New map",
                            hotkey_string="Ctrl+N",
                            disabled=True)
        menu_map.add_button("Open…",
                            hotkey_string="Ctrl+O",
                            disabled=True)
        menu_map.add_separator()
        menu_map.add_button("Save",
                            hotkey_string="Ctrl+S",
                            disabled=True)
        menu_map.add_button("Save as…",
                            hotkey_string="Shift+Ctrl+S",
                            disabled=True)
        menu_map.add_separator()
        menu_map.add_button("Back to main menu",
                           on_click=self.backToMainMenu,
                           hotkey_string="Ctrl+Q")

        _, menu_edit = menuBar.add_button_and_menu("Edit")
        menu_edit.add_button("Undo",
                             hotkey_string="Ctrl+Z",
                             disabled=True)
        menu_edit.add_button("Redo",
                             hotkey_string="Ctrl+Y",
                             disabled=True)
        menu_edit.add_separator()
        menu_edit.add_button("Cut",
                             hotkey_string="Ctrl+X",
                             disabled=True)
        menu_edit.add_button("Copy",
                             hotkey_string="Ctrl+C",
                             disabled=True)
        menu_edit.add_button("Paste",
                             hotkey_string="Ctrl+V",
                             disabled=True)
        menu_edit.add_button("Delete",
                             hotkey_string="Del",
                             disabled=True)

        _, menu_tools = menuBar.add_button_and_menu("Tools")
        menu_tools.add_button("Tile editor…",
                              disabled=True)
        menu_tools.add_button("Map settings…",
                              disabled=True)

        self._info_label = LabelWidget(titleBox)

        anonbox = VBox(self)

        barhbox = HBox(anonbox)
        barhbox.StyleClasses.add("scroll")
        Space(barhbox)

        barvbox = VBox(anonbox)
        barvbox.StyleClasses.add("scroll")

        scrollbar_v = ScrollBar(barhbox,
                                ScrollMode.VERTICAL,
                                on_scroll=self._scrollbar_scroll)
        scrollbar_h = ScrollBar(barvbox,
                                ScrollMode.HORIZONTAL,
                                on_scroll=self._scrollbar_scroll)

        self._scrollbar_v = scrollbar_v
        self._scrollbar_h = scrollbar_h

        for scrollbar in [scrollbar_h, scrollbar_v]:
            scrollbar.Range = (0, 500)
            scrollbar.Position = 40
            scrollbar.Page = 10

    def backToMainMenu(self, sender):
        self.RootWidget.switchMode(self.RootWidget.mainMenuMode)

    def _scrollbar_scroll(self, old_position, new_position):
        self._info_label.Text = "v: {:4d}; h: {:4d}".format(
            self._scrollbar_v.Position,
            self._scrollbar_h.Position
            )

class Mode(Mode.Mode):
    def __init__(self):
        super(Mode, self).__init__()
        self.desktopWidgets.append(
            MapEditor(None)
            )


Engine.UI.CSS.Minilanguage.ElementNames().register_widget_class(MapEditor)
