# encoding=utf-8
from __future__ import print_function, unicode_literals, division
from our_future import *

import Mode
from Engine.UI import ParentWidget, ButtonWidget, \
    AbstractVBox, LabelWidget, Space, HBox, \
    MenuBar, Menu

import Engine.UI.CSS.Minilanguage

class MapEditor(AbstractVBox):
    def __init__(self, parent, **kwargs):
        super(MapEditor, self).__init__(parent, **kwargs)

        titleBox = HBox(self)
        titleBox.StyleClasses.add("title")

        menuBar = MenuBar(titleBox)

        _, menu_map = menuBar.addButtonAndMenu("Map")
        menu_map.addButton("New map", hotkeyString="Ctrl+N")
        menu_map.addButton("Open…", hotkeyString="Ctrl+O")
        menu_map.addSeparator()
        menu_map.addButton("Save", hotkeyString="Ctrl+S")
        menu_map.addButton("Save as…", hotkeyString="Shift+Ctrl+S")
        menu_map.addSeparator()
        menu_map.addButton("Back to main menu",
                           onclick=self.backToMainMenu,
                           hotkeyString="Ctrl+Q")

        _, menu_edit = menuBar.addButtonAndMenu("Edit")
        menu_edit.addButton("Undo", hotkeyString="Ctrl+Z")
        menu_edit.addButton("Redo", hotkeyString="Ctrl+Y")
        menu_edit.addSeparator()
        menu_edit.addButton("Cut", hotkeyString="Ctrl+X")
        menu_edit.addButton("Copy", hotkeyString="Ctrl+C")
        menu_edit.addButton("Paste", hotkeyString="Ctrl+V")
        menu_edit.addButton("Delete", hotkeyString="Del")

        _, menu_tools = menuBar.addButtonAndMenu("Tools")
        menu_tools.addButton("Tile editor…")
        menu_tools.addButton("Map settings…")

    def backToMainMenu(self, sender):
        self.RootWidget.switchMode(self.RootWidget.mainMenuMode)

class Mode(Mode.Mode):
    def __init__(self):
        super(Mode, self).__init__()
        self.desktopWidgets.append(
            MapEditor(None)
            )


Engine.UI.CSS.Minilanguage.ElementNames().registerWidgetClass(MapEditor)
