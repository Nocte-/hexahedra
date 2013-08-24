Hexahedra
=========

About
-----
Hexahedra is a C++ framework for voxel sandbox games.


Current status
--------------
It's pretty much pre-alpha.  There are some cool features you can check out,
but it's definitely not ready for making actual games yet.  You can follow
its development on my [blog](http://hexahedra.blogspot.com/), or see it in action on [Youtube](http://www.youtube.com/user/NocteHexahedra).


Dependencies
------------
This library depends on the following projects:

- The [Simple and Fast Multimedia Library](http://www.sfml-dev.org/), for I/O abstraction and OpenGL initialization.
- Scripting is done through [Lua](http://www.lua.org/) and [LuaBind](http://www.rasterbar.com/products/luabind.html).
- The save files are stored using [SQLite](http://www.sqlite.org/).
- Uses the [IQM](http://lee.fov120.com/iqm/) format for storing 3D models and animations.
- The network layer is provided by [ENet](http://enet.bespin.org/).
- The [Jura](http://www.google.com/webfonts/specimen/Jura) font is provided as the default.
- The GUI uses [Rhea](http://github.com/Nocte-/rhea) as its layout constraint solver.

Build status
------------
[![Build status](https://travis-ci.org/Nocte-/hexahedra.png?branch=master)](https://travis-ci.org/Nocte-/hexahedra)

This project uses [Travis CI](http://travis-ci.org/) to build and run the unit
tests on different compilers automatically.

