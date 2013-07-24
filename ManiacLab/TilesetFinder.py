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
