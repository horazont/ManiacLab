# File name: TilesetFinder.py
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
from __future__ import unicode_literals

import threading

import _cmaniaclab as CManiacLab
import Engine.CEngine as CEngine

import Engine.VFS.Utils as VFSUtils

def _find_tilesets(vfs, base_path):
    files = list(vfs.listdir(base_path))
    files.sort()
    tilesets = {}
    for filename in files:
        print("tilset file: {}".format(filename))
        vfs_path = VFSUtils.join(base_path, filename)
        try:
            info = CManiacLab.read_tileset_info(
                CEngine.Stream(vfs.open(vfs_path, "r"))
                )
        except RuntimeError as err:
            # the exception doesn't get converted properly yet :)
            print("   error: {}".format(err))
            info = None
        if info is not None:
            print("   unique name: {}".format(info.unique_name))
            print("   display name: {}".format(info.display_name))
            tilesets[info.unique_name] = (info, vfs_path)
    return tilesets


def find_tilesets(vfs, base_path,
                  finish_callback=None):
    result = _find_tilesets(vfs, base_path)
    finish_callback(result)
