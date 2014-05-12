-- Minecraft compatible materials list

function mc(id, meta, info)
    define_material(id * 16 + meta, info)
end

-- ---------------------------------------------------------------------------

-- Special functions for placing fences
local function is_fence(a, b)
  return b >= a and b < (a + 16)
end

local function place_fence(p, id)
  local shape = 0
  local e = p + veci(1, 0, 0)
  local n = p + veci(0, 1, 0)
  local w = p + veci(-1, 0, 0)
  local s = p + veci(0, -1, 0)
  
  if (is_fence(id, get_block(e))) then
    shape = shape + 1
    change_block(e, get_block(e) + 2)
  end
  if (is_fence(id, get_block(w))) then
    shape = shape + 2
    change_block(w, get_block(w) + 1)
  end    
  if (is_fence(id, get_block(n))) then
    shape = shape + 4
    change_block(n, get_block(n) + 8)
  end  
  if (is_fence(id, get_block(s))) then
    shape = shape + 8
    change_block(s, get_block(s) + 4)    
  end
  
  change_block(p, id + shape)
end

local function remove_fence(p, id)
  local e = p + veci(1, 0, 0)
  local n = p + veci(0, 1, 0)
  local w = p + veci(-1, 0, 0)
  local s = p + veci(0, -1, 0)
  
  if (is_fence(id, get_block(e))) then
    change_block(e, get_block(e) - 2)
  end
  if (is_fence(id, get_block(w))) then
    change_block(w, get_block(w) - 1)
  end    
  if (is_fence(id, get_block(n))) then
    change_block(n, get_block(n) - 8)
  end  
  if (is_fence(id, get_block(s))) then
    change_block(s, get_block(s) - 4)    
  end
  
  change_block(p, 0)  
end

-- ---------------------------------------------------------------------------

mc(  1,  0, { name = "stone", texture = {"mc.smoothstone"} } )
mc(  2,  0, { name = "grass", texture = {"grass.side", "grass"} } )
mc(  3,  0, { name = "dirt", texture = {"dirt"} } )
mc(  4,  0, { name = "cobblestone", texture = {"cobblestone"} } )
mc(  5,  0, { name = "planks", texture = {"wood"} } )

-- Saplings use metadata to determine the tree type and growth stage
mc(  6,  0, { name = "saplings (oak 0)",    texture = {"leaves.oak"} } )
mc(  6,  1, { name = "saplings (spruce 0)", texture = {"leaves.spruce"} } )
mc(  6,  2, { name = "saplings (birch 0)",  texture = {"leaves.birch"} } )
mc(  6,  3, { name = "saplings (jungle 0)", texture = {"leaves.oak"} } )
mc(  6,  4, { name = "saplings (oak 1)",    texture = {"leaves.oak"} } )
mc(  6,  5, { name = "saplings (spruce 1)", texture = {"leaves.spruce"} } )
mc(  6,  6, { name = "saplings (birch 1)",  texture = {"leaves.birch"} } )
mc(  6,  7, { name = "saplings (jungle 1)", texture = {"leaves.oak"} } )
mc(  6,  8, { name = "saplings (oak 2)",    texture = {"leaves.oak"} } )
mc(  6,  9, { name = "saplings (spruce 2)", texture = {"leaves.spruce"} } )
mc(  6, 10, { name = "saplings (birch 2)",  texture = {"leaves.birch"} } )
mc(  6, 11, { name = "saplings (jungle 2)", texture = {"leaves.oak"} } )
mc(  6, 12, { name = "saplings (oak 3)",    texture = {"leaves.oak"} } )
mc(  6, 13, { name = "saplings (spruce 3)", texture = {"leaves.spruce"} } )
mc(  6, 14, { name = "saplings (birch 3)",  texture = {"leaves.birch"} } )
mc(  6, 15, { name = "saplings (jungle 3)", texture = {"leaves.oak"} } )

mc(  7,  0, { name = "bedrock", texture = {"bedrock"} } )

-- Water uses the bottom 3 bits for its level, and the top bit for falling water
mc(  8,  0, { name = "water (level 0)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  1, { name = "water (level 1)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  2, { name = "water (level 2)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  3, { name = "water (level 3)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  4, { name = "water (level 4)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  5, { name = "water (level 5)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  6, { name = "water (level 6)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  7, { name = "water", texture = {"water"}, transparency = 0.9 } )
mc(  8,  8, { name = "water (level 0, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  8,  9, { name = "water (level 1, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  8, 10, { name = "water (level 2, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  8, 11, { name = "water (level 3, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  8, 12, { name = "water (level 4, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  8, 13, { name = "water (level 5, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  8, 14, { name = "water (level 6, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  8, 15, { name = "water", texture = {"water"}, transparency = 0.9 } )
mc(  9,  0, { name = "stationary water (level 0)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  1, { name = "stationary water (level 1)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  2, { name = "stationary water (level 2)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  3, { name = "stationary water (level 3)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  4, { name = "stationary water (level 4)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  5, { name = "stationary water (level 5)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  6, { name = "stationary water (level 6)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  7, { name = "stationary water", texture = {"water"}, transparency = 0.9 } )
mc(  9,  8, { name = "stationary water (level 0, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  9,  9, { name = "stationary water (level 1, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  9, 10, { name = "stationary water (level 2, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  9, 11, { name = "stationary water (level 3, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  9, 12, { name = "stationary water (level 4, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  9, 13, { name = "stationary water (level 5, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  9, 14, { name = "stationary water (level 6, falling)", texture = {"water"}, transparency = 0.9 } )
mc(  9, 15, { name = "stationary water", texture = {"water"}, transparency = 0.7 } )

mc( 10,  0, { name = "lava (level 0)", texture = {"lava"}, emit_light = 0.4 } )
mc( 10,  2, { name = "lava (level 1)", texture = {"lava"}, emit_light = 0.4 } )
mc( 10,  4, { name = "lava (level 2)", texture = {"lava"}, emit_light = 0.4 } )
mc( 10,  6, { name = "lava (level 3)", texture = {"lava"}, emit_light = 0.4 } )
mc( 10,  8, { name = "lava (level 0, falling)", texture = {"lava"}, emit_light = 0.4 } )
mc( 10, 10, { name = "lava (level 1, falling)", texture = {"lava"}, emit_light = 0.4 } )
mc( 10, 12, { name = "lava (level 2, falling)", texture = {"lava"}, emit_light = 0.4 } )
mc( 10, 14, { name = "lava (level 3, falling)", texture = {"lava"}, emit_light = 0.4 } )
mc( 11,  0, { name = "stationary lava (level 0)", texture = {"lava"}, emit_light = 0.4 } )
mc( 11,  2, { name = "stationary lava (level 1)", texture = {"lava"}, emit_light = 0.4 } )
mc( 11,  4, { name = "stationary lava (level 2)", texture = {"lava"}, emit_light = 0.4 } )
mc( 11,  6, { name = "stationary lava (level 3)", texture = {"lava"} } )
mc( 11,  8, { name = "stationary lava (level 0, falling)", texture = {"lava"}, emit_light = 0.4 } )
mc( 11, 10, { name = "stationary lava (level 1, falling)", texture = {"lava"}, emit_light = 0.4 } )
mc( 11, 12, { name = "stationary lava (level 2, falling)", texture = {"lava"}, emit_light = 0.4 } )
mc( 11, 14, { name = "stationary lava (level 3, falling)", texture = {"lava"}, emit_light = 0.4 } )

mc( 12,  0, { name = "sand", texture = {"sand"} } )
mc( 13,  0, { name = "gravel", texture = {"tiles"} } )
mc( 14,  0, { name = "blue window", texture = {"window.1"} } )
mc( 15,  0, { name = "red window", texture = {"window.3"} } )
mc( 16,  0, { name = "coal ore", texture = {"window.4"} } )

-- 
mc( 17,  0, { name = "wood (oak)", texture = {"bark.oak", "log_top"} } )
mc( 17,  1, { name = "wood (pine)", texture = {"bark.pine", "log_top"} } )
mc( 17,  2, { name = "wood (birch)", texture = {"bark.birch", "log_top"} } )
mc( 17,  3, { name = "wood (jungle)", texture = {"bark.jungle", "log_top"} } )

-- Leaves use bit 0x4 for permanency, and 0x8 for decay checks
mc( 18,  0, { name = "leaves (oak)", texture = {"leaves.oak"}, transparency = 0.9 } )
mc( 18,  1, { name = "leaves (pine)", texture = {"leaves.pine"}, transparency = 0.9 } )
mc( 18,  2, { name = "leaves (birch)", texture = {"leaves.birch"}, transparency = 0.9 } )
mc( 18,  3, { name = "leaves (jungle)", texture = {"leaves.jungle"}, transparency = 0.7 } )
mc( 18,  4, { name = "leaves (permanent, oak)", texture = {"leaves.oak"}, transparency = 0.9 } )
mc( 18,  5, { name = "leaves (permanent, pine)", texture = {"leaves.pine"}, transparency = 0.9 } )
mc( 18,  6, { name = "leaves (permanent, birch)", texture = {"leaves.birch"}, transparency = 0.9 } )
mc( 18,  7, { name = "leaves (permanent, jungle)", texture = {"leaves.jungle"}, transparency = 0.7 } )
mc( 18,  8, { name = "leaves (decaying, oak)", texture = {"leaves.oak"}, transparency = 0.9 } )
mc( 18,  9, { name = "leaves (decaying, pine)", texture = {"leaves.pine"}, transparency = 0.9 } )
mc( 18, 10, { name = "leaves (decaying, birch)", texture = {"leaves.birch"}, transparency = 0.9 } )
mc( 18, 11, { name = "leaves (decaying, jungle)", texture = {"leaves.jungle"}, transparency = 0.7 } )
mc( 18, 12, { name = "leaves (permanent, oak)", texture = {"leaves.oak"}, transparency = 0.9 } )
mc( 18, 13, { name = "leaves (permanent, pine)", texture = {"leaves.pine"}, transparency = 0.9 } )
mc( 18, 14, { name = "leaves (permanent, birch)", texture = {"leaves.birch"}, transparency = 0.9 } )
mc( 18, 15, { name = "leaves (permanent, jungle)", texture = {"leaves.jungle"}, transparency = 0.7 } )

mc( 19,  0, { name = "sponge", texture = {"sponge"} } )
mc( 20,  0, { name = "glass", texture = {"glass"}, transparency = 0.97 } )
mc( 20,  1, { name = "glass.black", texture = {"glass.black"}, transparency = 0.8 } )
mc( 20,  2, { name = "glass.blue", texture = {"glass.blue"}, transparency = 0.8 } )
mc( 20,  3, { name = "glass.white", texture = {"glass.white"}, transparency = 0.8 } )
mc( 21,  0, { name = "lapiz ore", texture = {"lapiz"} } )
mc( 22,  0, { name = "lapiz", texture = {"lapiz"} } )

mc( 23,  2, { name = "dispenser", texture = {"machine"} } )
mc( 23,  3, { name = "dispenser", texture = {"machine"} } )
mc( 23,  4, { name = "dispenser", texture = {"machine"} } )
mc( 23,  5, { name = "dispenser", texture = {"machine"} } )

mc( 24,  0, { name = "sandstone", texture = {"sandstone"} } )
mc( 25,  0, { name = "note block", texture = {"note"} } )

mc( 26,  0, { name = "bed", texture = {"bed"} } )
mc( 26,  1, { name = "bed", texture = {"bed"} } )
mc( 26,  2, { name = "bed", texture = {"bed"} } )
mc( 26,  3, { name = "bed", texture = {"bed"} } )
mc( 26,  4, { name = "bed (occupied)", texture = {"bed"} } )
mc( 26,  5, { name = "bed (occupied)", texture = {"bed"} } )
mc( 26,  6, { name = "bed (occupied)", texture = {"bed"} } )
mc( 26,  7, { name = "bed (occupied)", texture = {"bed"} } )
mc( 26,  8, { name = "bed", texture = {"bed.head"} } )
mc( 26,  9, { name = "bed", texture = {"bed.head"} } )
mc( 26, 10, { name = "bed", texture = {"bed.head"} } )
mc( 26, 11, { name = "bed", texture = {"bed.head"} } )
mc( 26, 12, { name = "bed (occupied)", texture = {"bed.head"} } )
mc( 26, 13, { name = "bed (occupied)", texture = {"bed.head"} } )
mc( 26, 14, { name = "bed (occupied)", texture = {"bed.head"} } )
mc( 26, 15, { name = "bed (occupied)", texture = {"bed.head"} } )

mc( 27,  0, { name = "powered rail (flat)", texture = {"rail.ns"}, transparency = 0.5 } )
mc( 27,  1, { name = "powered rail (flat)", texture = {"rail.ew"}, transparency = 0.5 } )
mc( 27,  2, { name = "powered rail (ascending e)", texture = {"rail"}, transparency = 0.5 } )
mc( 27,  3, { name = "powered rail (ascending w)", texture = {"rail"}, transparency = 0.5 } )
mc( 27,  4, { name = "powered rail (ascending n)", texture = {"rail"}, transparency = 0.5 } )
mc( 27,  5, { name = "powered rail (ascending s)", texture = {"rail"}, transparency = 0.5 } )

mc( 28,  0, { name = "detector rail (flat)", texture = {"rail.ns"}, transparency = 0.5 } )
mc( 28,  1, { name = "detector rail (flat)", texture = {"rail.ew"}, transparency = 0.5 } )
mc( 28,  2, { name = "detector rail (ascending e)", texture = {"rail"}, transparency = 0.5 } )
mc( 28,  3, { name = "detector rail (ascending w)", texture = {"rail"}, transparency = 0.5 } )
mc( 28,  4, { name = "detector rail (ascending n)", texture = {"rail"}, transparency = 0.5 } )
mc( 28,  5, { name = "detector rail (ascending s)", texture = {"rail"}, transparency = 0.5 } )

-- The 0x8 bit signals if a piston is extended
mc( 29,  0, { name = "sticky piston (down)", texture = {"piston"} } )
mc( 29,  1, { name = "sticky piston (up)", texture = {"piston"} } )
mc( 29,  2, { name = "sticky piston (north)", texture = {"piston"} } )
mc( 29,  3, { name = "sticky piston (south)", texture = {"piston"} } )
mc( 29,  4, { name = "sticky piston (west)", texture = {"piston"} } )
mc( 29,  5, { name = "sticky piston (east)", texture = {"piston"} } )
mc( 29,  8, { name = "extended sticky piston (down)", texture = {"piston"} } )
mc( 29,  9, { name = "extended sticky piston (up)", texture = {"piston"} } )
mc( 29, 10, { name = "extended sticky piston (north)", texture = {"piston"} } )
mc( 29, 11, { name = "extended sticky piston (south)", texture = {"piston"} } )
mc( 29, 12, { name = "extended sticky piston (west)", texture = {"piston"} } )
mc( 29, 13, { name = "extended sticky piston (east)", texture = {"piston"} } )

mc( 30,  0, { name = "cobweb", texture = {"cobweb"} } )

--
mc( 31,  0, { name = "dead shrub", texture = {"deadshrub"} } )
mc( 31,  1, { name = "tall grass", texture = {"tallgrass"} } )
mc( 31,  2, { name = "fern", texture = {"fern"} } )

mc( 32,  0, { name = "dead bush", texture = {"deadbush"} } )

--
mc( 33,  0, { name = "piston (down)", texture = {"piston"} } )
mc( 33,  1, { name = "piston (up)", texture = {"piston"} } )
mc( 33,  2, { name = "piston (north)", texture = {"piston"} } )
mc( 33,  3, { name = "piston (south)", texture = {"piston"} } )
mc( 33,  4, { name = "piston (west)", texture = {"piston"} } )
mc( 33,  5, { name = "piston (east)", texture = {"piston"} } )
mc( 33,  8, { name = "extended piston (down)", texture = {"piston"} } )
mc( 33,  9, { name = "extended piston (up)", texture = {"piston"} } )
mc( 33, 10, { name = "extended piston (north)", texture = {"piston"} } )
mc( 33, 11, { name = "extended piston (south)", texture = {"piston"} } )
mc( 33, 12, { name = "extended piston (west)", texture = {"piston"} } )
mc( 33, 13, { name = "extended piston (east)", texture = {"piston"} } )

--
mc( 34,  0, { name = "piston extension (down)", texture = {"piston"} } )
mc( 34,  1, { name = "piston extension (up)", texture = {"piston"} } )
mc( 34,  2, { name = "piston extension (north)", texture = {"piston"} } )
mc( 34,  3, { name = "piston extension (south)", texture = {"piston"} } )
mc( 34,  4, { name = "piston extension (west)", texture = {"piston"} } )
mc( 34,  5, { name = "piston extension (east)", texture = {"piston"} } )
mc( 34,  8, { name = "sticky piston extension (down)", texture = {"piston"} } )
mc( 34,  9, { name = "sticky piston extension (up)", texture = {"piston"} } )
mc( 34, 10, { name = "sticky piston extension (north)", texture = {"piston"} } )
mc( 34, 11, { name = "sticky piston extension (south)", texture = {"piston"} } )
mc( 34, 12, { name = "sticky piston extension (west)", texture = {"piston"} } )
mc( 34, 13, { name = "sticky piston extension (east)", texture = {"piston"} } )

--
mc( 35,  0, { name = "wool", texture = {"plain.white"} } )
mc( 35,  1, { name = "wool", texture = {"shingles"} } )
mc( 35,  2, { name = "wool", texture = {"marble"} } )
mc( 35,  3, { name = "wool", texture = {"bedrock"} } )
mc( 35,  4, { name = "wool", texture = {"bookcase"} } )
mc( 35,  5, { name = "wool", texture = {"glass.2"}, transparency = 0.96  } )
mc( 35,  6, { name = "wool", texture = {"leather.red"} } )
mc( 35,  7, { name = "wool", texture = {"fireplace.off"} } )
mc( 35,  8, { name = "wool", texture = {"plain.gray"} } )
mc( 35,  9, { name = "wool", texture = {"bookshelf"} } )
mc( 35, 10, { name = "wool", texture = {"plain.red.dark"} } )
mc( 35, 11, { name = "wool", texture = {"leather.blue"} } )
mc( 35, 12, { name = "wool", texture = {"marble"} } )
mc( 35, 13, { name = "wool", texture = {"snow"} } )
mc( 35, 14, { name = "wool", texture = {"fireplace.on"} } )
mc( 35, 15, { name = "wool", texture = {"plain.black"} } )

mc( 36,  0, { name = "block moved by piston", texture = {"plain.black"} } )
mc( 37,  0, { name = "dandelion", texture = {"flower.yellow"}, transparency = 1.0 } )
mc( 38,  0, { name = "rose", texture = {"flower.red"}, transparency = 1.0 } )
mc( 39,  0, { name = "brown mushroom", texture = {"mushroom.brown"} } )
mc( 40,  0, { name = "red mushroom", texture = {"mushroom.red"} } )
mc( 41,  0, { name = "gold block", texture = {"plain.gold"} } )
mc( 42,  0, { name = "iron block", texture = {"plain.iron"} } )

--
mc( 43,  0, { name = "stone double slabs", texture = {"mc.doubleslab.stone", "mc.smoothstone"} } )
mc( 43,  1, { name = "sandstone double slabs", texture = {"mc.doubleslab.sandstone"} } )
mc( 43,  2, { name = "wooden double slabs", texture = {"mc.doubleslab.wood"} } )
mc( 43,  3, { name = "cobblestone double slabs", texture = {"mc.doubleslab.cobblestone"} } )
mc( 43,  4, { name = "brick double slabs", texture = {"mc.doubleslab.brick.red"} } )
mc( 43,  5, { name = "stone brick double slabs", texture = {"mc.doubleslab.brick.grey"} } )

mc( 44,  0, { name = "stone slabs", texture = {"rock"} } )
mc( 44,  1, { name = "sandstone slabs", texture = {"sandstone"} } )
mc( 44,  2, { name = "wooden slabs", texture = {"wood"} } )
mc( 44,  3, { name = "cobblestone slabs", texture = {"mc.slab"} } )
mc( 44,  4, { name = "brick slabs", texture = {"brick.red"} } )
mc( 44,  5, { name = "stone brick slabs", texture = {"brick.grey"} } )

mc( 45,  0, { name = "bricks", texture = {"brick.red"} } )
mc( 46,  0, { name = "tnt", texture = {"tnt_side", "tnt_top", "tnt_bottom"} } )
mc( 47,  0, { name = "bookshelf", texture = {"bookshelf", "wood"} } )
mc( 48,  0, { name = "moss stone", texture = {"stone.moss"} } )
mc( 49,  0, { name = "obsidian", texture = {"obsidian"} } )

mc( 50,  1, { name = "torch (east)", texture = {"lamp"}, emit_light = 0.7, 
              custom_model = { { 7, 7, 3, 9, 9, 10, "wood" }, 
                               { 8, 0, 8, 8, 6, 8, "plain.red.dark" } }  
            } )
mc( 50,  2, { name = "torch (west)", texture = {"lamp"}, emit_light = 0.7 } )
mc( 50,  3, { name = "torch (south)", texture = {"lamp"}, emit_light = 0.7 } )
mc( 50,  4, { name = "torch (north)", texture = {"lamp"}, emit_light = 0.7 } )
mc( 50,  5, { name = "torch (floor)", texture = {"lamp"}, emit_light = 0.7 } )

-- Fire uses a tick counter
for i=0,15 do
    mc( 51,  i, { name = "fire", texture = {"fire"} } )
end

mc( 52,  0, { name = "monster spawner", texture = {"spawner"} } )

mc( 53,  0, { name = "wooden stairs (e)", texture = {"wood"} } )
mc( 53,  1, { name = "wooden stairs (w)", texture = {"wood"} } )
mc( 53,  2, { name = "wooden stairs (n)", texture = {"wood"} } )
mc( 53,  3, { name = "wooden stairs (s)", texture = {"wood"} } )
mc( 53,  4, { name = "upside down wooden stairs (e)", texture = {"wood"} } )
mc( 53,  5, { name = "upside down wooden stairs (w)", texture = {"wood"} } )
mc( 53,  6, { name = "upside down wooden stairs (n)", texture = {"wood"} } )
mc( 53,  7, { name = "upside down wooden stairs (s)", texture = {"wood"} } )

mc( 54,  2, { name = "chest (n)", texture = {"chest_side", "chest_side", "chest_front", "chest_side", "chest_top", "chest_top" } } )
mc( 54,  3, { name = "chest (s)", texture = {"chest_side", "chest_side", "chest_side", "chest_front", "chest_top", "chest_top" } } )
mc( 54,  4, { name = "chest (w)", texture = {"chest_side", "chest_front", "chest_side", "chest_side", "chest_top", "chest_top" } } )
mc( 54,  5, { name = "chest (e)", texture = {"chest_front", "chest_side", "chest_side", "chest_side", "chest_top", "chest_top" } } )

-- Redstone wire stores the distance from the power source
mc( 55,  0, { name = "redstone wire (unpowered)", texture = {"redstone"} } )
for i=1,15 do
    mc( 55,  i, { name = "redstone wire", texture = {"redstone"} } )
end

mc( 56,  0, { name = "diamond ore", texture = {"ore.diamond"} } )
mc( 57,  0, { name = "diamond block", texture = {"plain.diamond"} } )
mc( 58,  0, { name = "crafting table", texture = {"workbench_side", "workbench_top"} } )

-- Wheat has 8 stages of growth
for i=0,7 do
    mc( 59,  i, { name = "wheat seeds", texture = {"wheat."..i} } )
end 

-- Farmland has 9 stages of irrigation
for i=0,8 do
    mc( 60,  i, { name = "farmland", texture = {"farmland"} } )
end

--
mc( 61,  2, { name = "furnace.n", texture = {"furnace_side", "furnace_side", "furnace_front", "furnace_side", "cobblestone", "cobblestone"} } )
mc( 61,  3, { name = "furnace.e", texture = {"furnace_side", "furnace_side", "furnace_side", "furnace_front", "cobblestone", "cobblestone"} } )
mc( 61,  4, { name = "furnace.s", texture = {"furnace_side", "furnace_front", "furnace_side", "furnace_side", "cobblestone", "cobblestone"} } )
mc( 61,  5, { name = "furnace.w", texture = {"furnace_front", "furnace_side", "furnace_side", "furnace_side", "cobblestone", "cobblestone"} } )

mc( 62,  2, { name = "burning furnace", texture = {"furnace_side", "furnace_side", "furnace_front.burning", "furnace_side", "cobblestone", "cobblestone"} } )
mc( 62,  3, { name = "burning furnace", texture = {"furnace_side", "furnace_side", "furnace_side", "furnace_front.burning", "cobblestone", "cobblestone"} } )
mc( 62,  4, { name = "burning furnace", texture = {"furnace_side", "furnace_front.burning", "furnace_side", "furnace_side", "cobblestone", "cobblestone"} } )
mc( 62,  5, { name = "burning furnace", texture = {"furnace_front.burning", "furnace_side", "furnace_side", "furnace_side", "cobblestone", "cobblestone"} } )

-- Sign posts use the data field as a clockwise rotation (0 = south, 4 = west)
for i=0,15 do
    mc( 63,  i, { name = "sign post", texture = {"sign"} } )
end

-- Doors are impossible to convert directly since 1.2, so here we use our
-- own encoding.  east-west-north-south is 0-3.  Bit 0x4 is clear for the
-- bottom part and set for the top part.  Bit 0x8 is set if the handle is
-- on the right, clear for left.
for i=0,15 do
    mc( 64,  0, { name = "wooden door", texture = {"door.wood"} } )
end

--
mc( 65,  2, { name = "ladder", texture = {"ladder"}, transparency = 0.9 } )
mc( 65,  3, { name = "ladder", texture = {"ladder"}, transparency = 0.9 } )
mc( 65,  4, { name = "ladder", texture = {"ladder"}, transparency = 0.9 } )
mc( 65,  5, { name = "ladder", texture = {"ladder"}, transparency = 0.9 } )

--
mc( 66,  0, { name = "rail (flat)", texture = {"rail.ns"}, transparency = 0.7 } )
mc( 66,  1, { name = "rail (flat)", texture = {"rail.ew"}, transparency = 0.7 } )
mc( 66,  2, { name = "rail (ascending e)", texture = {"rail"}, transparency = 0.7 } )
mc( 66,  3, { name = "rail (ascending w)", texture = {"rail"}, transparency = 0.7 } )
mc( 66,  4, { name = "rail (ascending n)", texture = {"rail"}, transparency = 0.7 } )
mc( 66,  5, { name = "rail (ascending s)", texture = {"rail"}, transparency = 0.7 } )
mc( 66,  6, { name = "rail (nw corner)", texture = {"rail"}, transparency = 0.7 } )
mc( 66,  7, { name = "rail (ne corner)", texture = {"rail"}, transparency = 0.7 } )
mc( 66,  8, { name = "rail (se corner)", texture = {"rail"}, transparency = 0.7 } )
mc( 66,  9, { name = "rail (sw corner)", texture = {"rail"}, transparency = 0.7 } )

--
mc( 67,  0, { name = "cobblestone stairs (e)", texture = {"cobblestone.stair"} } )
mc( 67,  1, { name = "cobblestone stairs (w)", texture = {"cobblestone.stair"} } )
mc( 67,  2, { name = "cobblestone stairs (n)", texture = {"cobblestone.stair"} } )
mc( 67,  3, { name = "cobblestone stairs (s)", texture = {"cobblestone.stair"} } )
mc( 67,  4, { name = "upside down cobblestone stairs (e)", texture = {"cobblestone.stair"} } )
mc( 67,  5, { name = "upside down cobblestone stairs (w)", texture = {"cobblestone.stair"} } )
mc( 67,  6, { name = "upside down cobblestone stairs (n)", texture = {"cobblestone.stair"} } )
mc( 67,  7, { name = "upside down cobblestone stairs (s)", texture = {"cobblestone"} } )

--
mc( 68,  2, { name = "wall sign", texture = {"wood"} } )
mc( 68,  3, { name = "wall sign", texture = {"wood"} } )
mc( 68,  4, { name = "wall sign", texture = {"wood"} } )
mc( 68,  5, { name = "wall sign", texture = {"wood"} } )

-- Lever orientation: 1 east, 2 west, 3 south, 4 north, 5 ground n-s, 6 ground e-w. 
-- Bit 0x8 for activated levers
mc( 69,  1, { name = "lever", texture = {"lever"} } )
mc( 69,  2, { name = "lever", texture = {"lever"} } )
mc( 69,  3, { name = "lever", texture = {"lever"} } )
mc( 69,  4, { name = "lever", texture = {"lever"} } )
mc( 69,  5, { name = "lever", texture = {"lever"} } )
mc( 69,  6, { name = "lever", texture = {"lever"} } )
mc( 69,  9, { name = "lever", texture = {"lever"} } )
mc( 69, 10, { name = "lever", texture = {"lever"} } )
mc( 69, 11, { name = "lever", texture = {"lever"} } )
mc( 69, 12, { name = "lever", texture = {"lever"} } )
mc( 69, 13, { name = "lever", texture = {"lever"} } )
mc( 69, 14, { name = "lever", texture = {"lever"} } )

mc( 70,  0, { name = "stone pressure plate", texture = {"stone.plate"} } )
mc( 70,  1, { name = "stone pressure plate (pressed)", texture = {"stone.plate"} } )

for i=0,15 do
    mc( 71,  i, { name = "iron door", texture = {"door.iron"} } )
end

mc( 72,  0, { name = "wooden pressure plate", texture = {"wood.plate"} } )
mc( 72,  1, { name = "wooden pressure plate (pressed)", texture = {"wood.plate"} } )

mc( 73,  0, { name = "redstone ore", texture = {"ore.redstone"} } )
mc( 74,  0, { name = "glowing redstone ore", texture = {"ore.redstone"} } )

mc( 75,  1, { name = "redstone torch off (east)", texture = {"torch"} } )
mc( 75,  2, { name = "redstone torch off (west)", texture = {"torch"} } )
mc( 75,  3, { name = "redstone torch off (south)", texture = {"torch"} } )
mc( 75,  4, { name = "redstone torch off (north)", texture = {"torch"} } )
mc( 75,  5, { name = "redstone torch off (floor)", texture = {"torch"} } )

mc( 76,  1, { name = "redstone torch on (east)", texture = {"torch"}, emit_light = 0.6 } )
mc( 76,  2, { name = "redstone torch on (west)", texture = {"torch"}, emit_light = 0.6 } )
mc( 76,  3, { name = "redstone torch on (south)", texture = {"torch"}, emit_light = 0.6 } )
mc( 76,  4, { name = "redstone torch on (north)", texture = {"torch"}, emit_light = 0.6 } )
mc( 76,  5, { name = "redstone torch on (floor)", texture = {"torch"}, emit_light = 0.6 } )

-- Button orientation: 1 facing east, 2 west, 3 south, 4 north
-- Bit 0x8 is set if it is pressed
mc( 77,  1, { name = "stone button", texture = {"button.stone"} } )
mc( 77,  2, { name = "stone button", texture = {"button.stone"} } )
mc( 77,  3, { name = "stone button", texture = {"button.stone"} } )
mc( 77,  4, { name = "stone button", texture = {"button.stone"} } )
mc( 77,  9, { name = "stone button (pressed)", texture = {"button.stone"} } )
mc( 77, 10, { name = "stone button (pressed)", texture = {"button.stone"} } )
mc( 77, 11, { name = "stone button (pressed)", texture = {"button.stone"} } )
mc( 77, 12, { name = "stone button (pressed)", texture = {"button.stone"} } )

-- Snow has 7 levels, repeated over 8-15
for i=0,7 do
    mc( 78,  i    , { name = "snow", texture = {"snow."..i} } )
    mc( 78,  i + 8, { name = "snow", texture = {"snow."..i} } )
end

mc( 79,  0, { name = "ice", texture = {"ice"} } )
mc( 80,  0, { name = "snow block", texture = {"snow"} } )

-- Cacti have a growth counter
for i=0,15 do
    mc( 81,  i, { name = "cactus", texture = {"cactus"} } )
end

mc( 82,  0, { name = "clay", texture = {"clay"} } )

-- Sugar cane has a growth counter
for i=0,15 do
    mc( 83,  i, { name = "sugar cane", texture = {"sugarcane"} } )
end

-- Jukeboxes can hold different kinds of records (0 = empty)
for i=0,15 do
    mc( 84,  i, { name = "jukebox", texture = {"jukebox"} } )
end

-- We reserve 16 different kinds of fence blocks, even though MC does not
-- assign any metadata to them.  Hexahedra uses this value to pick the
-- correct 3-D custom block module.

mc(85, 0, { name = "fence.0000", texture = { "fence" }, transparency = 0.9,
            on_place = place_fence, on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" } } })

mc(85, 1, { name = "fence.000E", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 10, 8, 10, 15, 8, 12, "bark" } },
            collision_boxes = { { 7, 7, 0, 15, 9, 15 } } })

mc(85, 2, { name = "fence.00W0", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 0, 8, 10,   6, 8, 12, "bark" } },
            collision_boxes = { { 0, 7, 0, 9, 9, 15 } } })

mc(85, 3, { name = "fence.00WE", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 0, 8, 10,  15, 8, 12, "bark" } },
            collision_boxes = { { 0, 7, 0, 15, 9, 15 } } })

mc(85, 4, { name = "fence.0N00", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8, 10, 10,  8, 15, 12, "bark" } },
            collision_boxes = { { 7, 7, 0, 9, 15, 15 } } })

mc(85, 5, { name = "fence.0N0E", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8, 10, 10,  8, 15, 12, "bark" }, { 10, 8, 10, 15, 8, 12, "bark" } },
            collision_boxes = { { 7, 7, 0, 9, 15, 15 }, { 7, 7, 0, 15, 9, 15 } } })

mc(85, 6, { name = "fence.0NW0", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8, 10, 10,  8, 15, 12, "bark" }, { 0, 8, 10,   7, 8, 12, "bark" } },
            collision_boxes = { { 7, 7, 0, 9, 15, 15 }, { 0, 7, 0, 9, 9, 15 } } })

mc(85, 7, { name = "fence.0NWE", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8, 10, 10,  8, 15, 12, "bark" }, { 0, 8, 10,  15, 8, 12, "bark" } },
            collision_boxes = { { 7, 7, 0, 9, 15, 15 }, { 0, 7, 0, 15, 9, 15 } } })

mc(85, 8, { name = "fence.S000", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 6, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 9, 15 } } })

mc(85, 9, { name = "fence.S00E", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 6, 12, "bark" }, { 10, 8, 10, 15, 8, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 9, 15 }, { 7, 7, 0, 15, 9, 15 } } })

mc(85, 10, { name = "fence.S0W0", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 6, 12, "bark" }, { 0, 8, 10,   6, 8, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 9, 15 }, { 0, 7, 0, 9, 9, 15 } } })

mc(85, 11, { name = "fence.S0WE", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 6, 12, "bark" }, { 0, 8, 10,  15, 8, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 9, 15 }, { 0, 7, 0, 15, 9, 15 } } })

mc(85, 12, { name = "fence.SN00", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 15, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 15, 15 } } })

mc(85, 13, { name = "fence.SN0E", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 15, 12, "bark" }, { 10, 8, 10, 15, 8, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 15, 15 }, { 7, 7, 0, 15, 9, 15 } } })

mc(85, 14, { name = "fence.SNW0", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 15, 12, "bark" }, { 0, 8, 10,   6, 8, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 15, 15 }, { 0, 7, 0, 9, 9, 15 } } })

mc(85, 15, { name = "fence.SNWE", texture = { "fence" }, transparency = 0.9,
            on_remove = remove_fence,
            custom_model = { { 7, 7, 0, 9, 9, 15, "fence" }, { 8,  0, 10,  8, 15, 12, "bark" }, { 0, 8, 10,   15, 8, 12, "bark" } },
            collision_boxes = { { 7, 0, 0, 9, 15, 15 }, { 0, 7, 0, 15, 9, 15 } } })

      
-- Pumpkins, facing south, west, north, east
mc( 86,  0, { name = "pumpkin", texture = {"pumpkin"} } )
mc( 86,  1, { name = "pumpkin", texture = {"pumpkin"} } )
mc( 86,  2, { name = "pumpkin", texture = {"pumpkin"} } )
mc( 86,  3, { name = "pumpkin", texture = {"pumpkin"} } )

mc( 87,  0, { name = "netherrack", texture = {"mc.netherrack"} } )
mc( 88,  0, { name = "soul sand", texture = {"mc.soulsand"} } )
mc( 89,  0, { name = "glowstone", texture = {"lamp"}, emit_light = 0.8 } )
mc( 90,  0, { name = "portal", texture = {"mc.portal"} } )

-- Jack'o'lantern, facing south, west, north, east
mc( 91,  0, { name = "jack-o-lantern", texture = {"mc.jackolantern"} } )
mc( 91,  1, { name = "jack-o-lantern", texture = {"mc.jackolantern"} } )
mc( 91,  2, { name = "jack-o-lantern", texture = {"mc.jackolantern"} } )
mc( 91,  3, { name = "jack-o-lantern", texture = {"mc.jackolantern"} } )

-- Cake, in six stages of nom.
for i=0,5 do
    mc( 92,  i, { name = "cake", texture = {"mc.cake"} } )
end

-- Redstone repeaters have a direction in the low crumb, the number of
-- ticks in the high crumb.
for i=0,3 do
    for j=0,3 do
        mc( 93, j + i * 4, { name = "redstone repeater off ("..i.." tick)", texture = {"mc.redstone"} } )
        mc( 94, j + i * 4, { name = "redstone repeater on ("..i.." tick)", texture = {"mc.redstone"} } )
    end
end

mc( 95,  0, { name = "locked chest", texture = {"chest"} } )

-- Trapdoors, hinged at the south, north, east, and west wall
mc( 96,  0, { name = "trapdoor", texture = {"window.2"} } )
mc( 96,  1, { name = "trapdoor", texture = {"window.2"} } )
mc( 96,  2, { name = "trapdoor", texture = {"window.2"} } )
mc( 96,  3, { name = "trapdoor", texture = {"window.2"} } )

mc( 97,  0, { name = "hidden silverfish (stone)", texture = {"mc.smoothstone"} } )
mc( 97,  1, { name = "hidden silverfish (cobblestone)", texture = {"cobblestone"} } )
mc( 97,  2, { name = "hidden silverfish (stone brick)", texture = {"brick.red"} } )

mc( 98,  0, { name = "stone bricks", texture = {"brick.grey"} } )
mc( 98,  1, { name = "stone bricks", texture = {"brick.grey.mossy"} } )
mc( 98,  2, { name = "stone bricks", texture = {"brick.grey.cracked"} } )
mc( 98,  3, { name = "stone bricks", texture = {"brick.grey.circle"} } )

-- TODO
mc( 99,  0, { name = "huge brown mushroom", texture = {"mushroom.brown"} } )
mc(100,  0, { name = "huge red mushroom", texture = {"mushroom.red"} } )


mc(101,  0, { name = "iron bars", texture = {"bars.iron"} } )
mc(102,  0, { name = "glass pane", texture = {"glass"}, transparency = 1.0 } )
mc(103,  0, { name = "melon", texture = {"melon"} } )

-- Pumpkin and melon stems grow in 8 stages
for i=0,7 do
    mc(104,  i, { name = "pumpkin stem", texture = {"stem"} } )
    mc(105,  i, { name = "melon stem", texture = {"stem"} } )
end

-- Vines use a bitmask for their directions (1 = south, 2 = west, 4 = north, 8 = east)
for i=0,7 do
    mc(106,  i, { name = "vines", texture = {"vines"} } )
end 

for i=0,3 do
    mc(107,  i    , { name = "fence gate", texture = {"fencegate"} } )
    mc(107,  i + 4, { name = "fence gate (open)", texture = {"fencegate"} } )
end

mc(108,  0, { name = "brick stairs", texture = {"stairs.brick.red"} } )
mc(109,  0, { name = "stone brick stairs", texture = {"stairs.brick.grey"} } )
mc(110,  0, { name = "mycelium", texture = {"mc.mycelium"} } )
mc(111,  0, { name = "lily pad", texture = {"lilypad"} } )
mc(112,  0, { name = "nether brick", texture = {"brick.nether"} } )
mc(113,  0, { name = "nether brick fence", texture = {"fence.nether"} } )
mc(114,  0, { name = "nether brick stairs", texture = {"stairs.brick.nether"} } )

for i=0,3 do
    mc(115,  0, { name = "nether wart", texture = {"mc.netherwart"} } )
end

mc(116,  0, { name = "enchantment table", texture = {"mc.enchantmenttable"} } )

-- Brewing stands have 3 slots for bottles.  The lowest 3 bits flag which slots are filled.
for i=0,7 do
    mc(117,  i, { name = "brewing stand", texture = {"mc.brewingstand"} } )
end

-- Cauldrons have 4 levels
for i=0,3 do
    mc(118,  i, { name = "cauldron", texture = {"mc.cauldron"} } )
end

mc(119,  0, { name = "end portal", texture = {"mc.portal.end"} } )

mc(120,  0, { name = "end portal frame", texture = {"mc.portal.end.frame"} } )
mc(120,  4, { name = "end portal frame (filled)", texture = {"mc.portal.end.frame"} } )

mc(121,  0, { name = "end stone", texture = {"stone.end"} } )
mc(122,  0, { name = "dragon egg", texture = {"mc.dragonegg"} } )
mc(123,  0, { name = "redstone lamp off", texture = {"mc.redstonelamp.off"} } )
mc(124,  0, { name = "redstone lamp on", texture = {"mc.redstonelamp"} } )


