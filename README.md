ManiacLab
=========

*ManiacLab* is (or wants to become) a game, based on the good old [Boulder Dash][0]
and its remake, [Safrosoft RoX][1] (link is to an inofficial website). Its goal
is to be more action loaded, **with lasers, smoke and explosions**!

*ManiacLab* will have more awesome physics, including but not limited to:

* Ray casting for laser installments
* Thermodynamics simulation, simulating heat, air and smoke flow through air and
  matter.
* Ray casting for line of sight

Physics
-------

For *ManiacLab* to be awesome, a decent physics simulation is implemented. It
divides the game cells (the original Boulder Dash cells) into subdivisions,
currently set to 5×5 subdivided cells per game cell. Each of the cells has a set
of values, like air pressure, temperature, fog density and the likes.

During each simulation frame, the interaction of the cells is simulated using a
simple double-buffered integration scheme.

This provides a vast amount of options for interaction between game objects and
the physics simulation. Planned interactions include:

* Objects being shifted due to pressure differences
* Objects catching fire (or exploding) due to overheat
* Player dying due to too low or too high air pressure, overheat, underheat and
  similar misconditions
* Emitting and absorbing air and fog
* Heating or cooling

There are some [videos][2] showing the simulation in action, most notably the
[last one of the series][3], providing a pretty complete summary of the current
simulation capabilities. In that video, we see a level where in both halves,
objects (red blobs) are placed. In the left half, the visualization shows air
pressure, in the right half it shows fog density. Throughout the video, objets
get spawned and we can see the interaction with respect to air pressure and
fog. All objects are spawned with high heat, which can be observed at the start
of the video where inside the left tower, air rises (and larger pressure is
generated at the top) due to convection.

Dependencies
------------

After cloning and initializing the submodules, there are some dependencies which
must be satisfied to build ManiacLab.

* Python 2 (≥ 2.7)
* boost
* cairo and pycairo
* pango
* libsigc++
* gtk3 (for the editor)

Building
--------

If everything is fine, a simple ``cmake`` and ``make`` workflow *should* build
ManiacLab. If you’re experiencing problems on any unix-ish platform with X11
support, you can open an issue. Other platforms are currently not supported.

   [0]: https://en.wikipedia.org/wiki/Boulder_Dash
   [1]: http://www.autofish.net/shrines/rox/
   [2]: http://sotecware.net/files/maniaclab/
   [3]: http://sotecware.net/files/maniaclab/phy-simulation-01-mostly-complete.ogv
