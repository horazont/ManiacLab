#!/usr/bin/python2
# encoding=utf8

from __future__ import unicode_literals, print_function, division
from our_future import *

# global PyOpenGL flags MUST ONLY be set here.
import OpenGL
OpenGL.ERROR_ON_COPY = True

if __name__ == '__main__':
    import argparse
    import sys, os
    sys.path.append(os.path.join(os.getcwd(), "PyEngine"))
    import Engine.CEngine.Window as CWindow
    from Engine.CEngine.Log import server as log, Severity

    log.log(Severity.Information, "Python is ready to rumble!")

    from ManiacLab.Application import ManiacLab
    app = ManiacLab()
    app.run()
