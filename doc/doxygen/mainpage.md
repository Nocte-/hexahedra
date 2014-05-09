Hexahedra documentation {#mainpage}
=======================

Hexahedra is a C++ framework for creating "blocky sandbox" or "sandvox" games.
The distinctive feature of such games is that the world is basically a grid of
cubes, or voxels.  Often, the player(s) can place or destroy cubes freely.


Definitions and conventions
---------------------------


- Block

  A block is an atomic element in the game world.  In memory, a block is just
  a 16-bit number in an array.  This number determines the material type.

- Material

  Determines what a block looks like and how it behaves.  Material zero is
  empty air, by definition.  Most materials consists of six textures (one for
  each block face), but could also have extra attrbutes (durability,
  transparency, what kind of item it drops), extra storage (for chests or
  cabinets), or even a custom 3-D model.

- Chunk

  A chunk is a 16x16x16 cube of blocks.  For efficiency reasons, all game world
  data is stored, transmitted, and processed in chunks.

- Game world

  The game world is virtually infinite (2^32 blocks in every direction).  In a
  particular game, the size of the world could be limited for gameplay
  reasons (e.g. an arena for a first person shooter, or a last man standing
  survival game).

- Direction

  Hexahedra uses a right-hand coordinate system:

  Direction | Axis | Value | Bitmask
  :---------|:----:|:-----:|:------:
  East      | +X   | 0     | 0x00
  West      | -X   | 1     | 0x01
  North     | +Y   | 2     | 0x02
  South     | -Y   | 3     | 0x04
  Up        | +Z   | 4     | 0x08
  Down      | -Z   | 5     | 0x10

- Surface

  The visible faces of a chunk.  Instead of sending complete chunks to the
  client, the server only sends the surfaces. 


