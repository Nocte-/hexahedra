[![Hexahedra](http://hexahedra.net/img/banner1.png)](http://hexahedra.net/)

[![Build status](https://api.travis-ci.org/Nocte-/hexahedra.svg)](https://travis-ci.org/Nocte-/hexahedra)

Hexahedra is a C++ framework for blocky/voxel games.  The most notable
features are:

- Multiplayer
- "Infinite" terrain (4 billion blocks in all directions)
- Lua scripting on the server
- Runtime selection of OpenGL 2.1 and 3.3 renderers
- Very flexible [DSL for terrain generation](http://github.com/Nocte-/hexanoise), compiles to OpenCL if available
- Centralized player authentication (optional)
- Server list

Current status
--------------
It's pretty much pre-alpha.  There are some cool features you can check out,
but it's definitely not ready for making actual games yet.  You can follow
its development on the [forum](http://forum.hexahedra.net/), or see it in
action on [Youtube](http://www.youtube.com/user/NocteHexahedra).


Dependencies
------------
This library depends on the following projects:

- The [Simple and Fast Multimedia Library](http://www.sfml-dev.org/), for I/O abstraction and OpenGL initialization.
- Scripting is done through [LuaJIT](http://luajit.org/) and [LuaBind](http://www.rasterbar.com/products/luabind.html).
- The save files are stored using [LevelDB](http://code.google.com/p/leveldb/).
- Uses the [IQM](http://sauerbraten.org/iqm/) format for storing 3D models and animations.
- The network layer is provided by [ENet](http://enet.bespin.org/).
- The [Play](http://www.google.com/fonts/specimen/Play) font is provided as the default.
- The GUI uses [Rhea](http://github.com/Nocte-/rhea) as its layout constraint solver.
- [HexaNoise](http://github.com/Nocte-/hexanoise) (the DSL used for [terrain generation](http://github.com/Nocte-/hexahedra/wiki/HNDL-examples)) is now available as a separate MIT-licensed library.

This project uses [Travis CI](http://travis-ci.org/) to build and run the unit
tests on different compilers automatically.

