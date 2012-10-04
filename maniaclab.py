#!/usr/bin/python2
# encoding=utf8
# File name: maniaclab.py
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
