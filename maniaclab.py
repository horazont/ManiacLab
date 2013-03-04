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

    parser = argparse.ArgumentParser()

    parser.add_argument(
        "-p", "--profile",
        dest="profile",
        nargs="?",
        const=True,
        default=False,
        metavar="FILE",
        help="Run in cProfile mode. If FILE is given, the cProfile\
 output is sent to that file. If FILE is omitted and --profile-shell is\
 set, a default of /tmp/pyuniverse-profile is assumed."
        )

    parser.add_argument(
        "-s", "--profile-shell",
        dest="profileShell",
        action="store_true",
        help="In profiling mode, drop into a shell after the\
 application terminated. In that shell, the profiling data will be\
 available in a pstats variable called p."
        )

    parser.add_argument(
        "--no-threaded-simulation",
        dest="threaded_simulation",
        default=True,
        action="store_false",
        help="Disable multithreading for the simulation."
        )
    args = parser.parse_args(sys.argv[1:])

    from ManiacLab.Application import ManiacLab
    log.log(Severity.Information, "Python is ready to rumble!")

    app = ManiacLab(args)
    if args.profile:
        import cProfile
        log.log(Severity.Warning, "Running in cProfile mode")
        if args.profileShell and args.profile is True:
            log.log(Severity.Warning, "No output file defined but shell requested. Defaulting to /tmp/pyuniverse-profile")
            args.profile = "/tmp/pyuniverse-profile"
        # if args.profileFrames:
        #     if args.profileFrames < 0:
        #         raise ValueError("Nice try.")
        #     app._eventLoop.setFrameCount(args.profileFrames)
        #     log.log(Severity.Information, "Will terminate after {0} frames.".format(args.profileFrames))
        if args.profile is not True:
            log.log(Severity.Information, "cProfile output is going to file: {0}".format(args.profile))
            cProfile.run("app.run()", filename=args.profile)
        else:
            cProfile.run("app.run()")
        if args.profileShell:
            import code
            import pstats
            import readline
            p = pstats.Stats(args.profile)
            def topTimes(n=10):
                p.sort_stats("cum").print_stats(n)
            namespace = {}
            namespace["p"]          = p
            namespace["app"]        = app
            namespace["topTimes"]   = topTimes
            code.InteractiveConsole(namespace).interact("Profiling shell. Stats of the current run are available in the p object. Application state is available in the app object.")
    else:
        app.run()
