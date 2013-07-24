# encoding=utf-8
# File name: TilesetEditor.py
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

import _cmaniaclab as CManiacLab
import Engine.CEngine as CEngine

import copy

import Mode
from Engine.CEngine import key, motion
from Engine.UI import ParentWidget, Button, \
    AbstractVBox, LabelWidget, Space, HBox, \
    MenuBar, Menu, Edit, ScrollBar, ScrollMode, VBox, Window, \
    List, TextListItem, Widget, ParentWidget, Grid, FilePicker

import Engine.UI.CSS.Minilanguage

import Engine.UI.CSS.Constants as Constants
import Engine.UI.CSS.Rect as Rect

class TilesetSelector(Window):
    def __init__(self, parent, **kwargs):
        super(TilesetSelector, self).__init__(parent, **kwargs)

        self.Caption = "Open tileset…"
        self.AbsoluteRect.Width = 512
        self.AbsoluteRect.Height = 384

        self.tileset_list = List(self,
                                 on_dbl_click=self._list_dbl_click)

        button_bar = HBox(self)
        button_bar.StyleClasses.add("buttons")
        Space(button_bar)
        self._cancel = Button(button_bar,
                              caption="Cancel",
                              on_click=self._do_cancel)
        self._load = Button(button_bar,
                            caption="Load",
                            on_click=self._do_load)

    def on_show_modal(self):
        self.tileset_list.clear()
        tilesets = self.RootWidget.tilesets
        for info, vfs_path in tilesets.viewvalues():
            TextListItem(self.tileset_list,
                         info.display_name).info = (info, vfs_path)

    def on_key_up(self, symbol, modifiers):
        if symbol == key.Escape:
            self.close()

    def _do_cancel(self, sender):
        self.ModalResult = False

    def _do_load(self, sender):
        if not self.tileset_list.SelectedItem:
            return
        self.ModalResult = self.tileset_list.SelectedItem.info

    def _list_dbl_click(self, sender):
        self._do_load(self)

    def _parent_changed(self):
        super(TilesetSelector, self)._parent_changed()
        if self.Parent:
            parent_rect = self.Parent.AbsoluteRect
            pw, ph = parent_rect.Width, parent_rect.Height
            w, h = self.AbsoluteRect.Width, self.AbsoluteRect.Height
            x = round(parent_rect.Left + pw / 2 - w / 2)
            y = round(parent_rect.Top + ph / 2 - h / 2)
            self.AbsoluteRect = Rect.Rect(
                x, y,
                x + w, y + h)

class BoolStamp(Widget):
    def __init__(self, parent, **kwargs):
        super(BoolStamp, self).__init__(parent, **kwargs)
        self._flags = set()

    def on_mouse_move(self, x, y, dx, dy, buttons, modifiers):
        if buttons & (1 << 8) != 0:
            self.IsActive = True
            return True
        elif buttons & (1 << 10) != 0:
            self.IsActive = False
            return True

    def on_mouse_down(self, x, y, button, modifiers):
        if button == 1:
            self.IsActive = True
            return True
        elif button == 3:
            self.IsActive = False
            return True

class TileEditor(Grid):
    def __init__(self, parent, **kwargs):
        super(TileEditor, self).__init__(parent, **kwargs)
        self.Rows = 5
        self.Columns = 5

        for i in range(5*5):
            BoolStamp(self)

class CenterPanel(ParentWidget):
    def __init__(self, parent, **kwargs):
        super(CenterPanel, self).__init__(parent, **kwargs)

    def do_align(self):
        style = self.ComputedStyle
        rect = self.AbsoluteRect
        rect.shrink(style.Padding)
        rect.shrink(style.Border.get_box())
        for child in self:
            child_style = child.ComputedStyle
            w, h = child.get_dimensions()
            child_margin = copy.copy(child_style.Margin)

            if w is None:
                w = rect.Width
            if h is None:
                h = rect.Height
            child_rect = Rect.Rect(0, 0, w, h)
            child_margin.deautoify(child_rect, rect)
            child_rect = copy.copy(rect)
            child_rect.shrink(child_margin)

            child.AbsoluteRect = child_rect

class TilesetEditor(AbstractVBox):
    def __init__(self, parent, **kwargs):
        super(TilesetEditor, self).__init__(parent, **kwargs)

        top_panel = HBox(self)
        top_panel.StyleClasses.add("top")

        menu = MenuBar(top_panel)
        _, submenu = menu.add_button_and_menu("Tileset")
        submenu.add_button("New...",
                           on_click=self.new_tileset)
        submenu.add_button("Open...",
                           on_click=self.open_tileset)
        submenu.add_button("Save",
                           on_click=self.save_tileset)
        submenu.add_button("Save as...",
                           on_click=self.save_tileset_as)
        submenu.add_button("Quit tileset editor",
                           on_click=self.quit)

        _, submenu = menu.add_button_and_menu("Tile")
        submenu.add_button("New...",
                           on_click=self.new_tile)
        submenu.add_button("Import...",
                           on_click=self.import_tile)

        main_hbox = HBox(self)

        editor_box = CenterPanel(main_hbox)
        editor_box.StyleClasses.add("editor")
        tile_editor = TileEditor(editor_box)

        right_panel = VBox(main_hbox)
        right_panel.StyleClasses.add("right")

        LabelWidget(right_panel, text="Current tileset").StyleClasses.add("title")

        LabelWidget(right_panel, text="Unique name")
        self._unique_name = Edit(right_panel,
                                 on_apply=self.change_unique_name)
        LabelWidget(right_panel, text="Display name")
        self._display_name = Edit(right_panel,
                                  on_apply=self.change_display_name)

        LabelWidget(right_panel, text="Tiles").StyleClasses.add("title")

        tilelist = List(right_panel)

        self.new_tileset(self)

    def new_tileset(self, sender):
        self._current_tileset = CManiacLab.Tileset()
        self._update_tileset()

    def open_tileset(self, sender):
        self.RootWidget.show_modal_window(TilesetSelector(
                None,
                on_modal_close=self._open_tileset
                ))

    def _open_tileset(self, sender):
        result = sender.ModalResult
        if not result:
            return
        tileset_info, vfs_path = result
        tileset = CManiacLab.load_tileset_from_stream(
            CEngine.Stream(self.RootWidget.vfs.open(vfs_path, "r"))
            )
        print(tileset.unique_name)
        self._current_tileset = tileset
        self._update_tileset()

    def save_tileset(self, sender):
        pass

    def save_tileset_as(self, sender):
        FilePicker.pick_file(self.RootWidget, self._file_picked, start_at=".")

    def _file_picked(self, path):
        print(path)

    def new_tile(self, sender):
        pass

    def import_tile(self, sender):
        pass

    def quit(self, sender):
        self.RootWidget.switchMode(self.RootWidget.mainMenuMode)

    def change_unique_name(self, sender, value):
        self._current_tileset.unique_name = value
        self._update_tileset()

    def change_display_name(self, sender, value):
        self._current_tileset.display_name = value
        self._update_tileset()

    def _update_tileset(self):
        tileset = self._current_tileset
        self._unique_name.Text = tileset.unique_name
        self._display_name.Text = tileset.display_name


class Mode(Mode.Mode):
    def __init__(self):
        super(Mode, self).__init__()
        self.desktopWidgets.append(
            TilesetEditor(None)
            )


Engine.UI.CSS.Minilanguage.ElementNames().register_widget_class(TilesetEditor)
Engine.UI.CSS.Minilanguage.ElementNames().register_widget_class(TileEditor)
Engine.UI.CSS.Minilanguage.ElementNames().register_widget_class(BoolStamp)
