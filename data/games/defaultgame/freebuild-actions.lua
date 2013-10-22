-- Left click places a block of the player's color, right click digs it.


function place_block (plr, inv, look_at, pos)
    local blk = raycast(pos + vecf(0,0,1.7), look_at, 9)
    local actions = { 
      "planks",
      "fence.0000", 
      "water",
      "bookshelf",
      "obsidian",
      "torch (floor)",
      "glass.red",
      "glass.green",
      "glass.white",
      "glowstone"
    }

    if (blk[1] ~= blk[2]) then 
        local rel_pos = blk[1] - pos
        local blk_bbox = block_boundingbox(rel_pos)
        local player_bbox = boundingbox_from_size(plr.bounding_box_size)

        if (are_overlapping(player_bbox, blk_bbox)) then
            -- The player is trying to place a block that overlaps with his
            -- or her own bounding box.
            return
        end

        local mat_id = material_id(actions[inv + 1])
        local info = material_definition(mat_id)
        if (info and info.on_place) then
          info.on_place(blk[1], mat_id)
        else     
          change_block(blk[1], actions[inv + 1])
        end
    end
end

function remove_block (plr, inv, look_at, pos)
    local blk = raycast(pos + vecf(0,0,1.7), look_at, 9)
    if (blk[1] ~= blk[2]) then
        local mat_id = get_block(blk[2])
        local info = material_definition(mat_id)
        if (info and info.on_remove) then
          info.on_remove(blk[2], mat_id)
        else     
          change_block(blk[2], 0)
        end
    end
end

on_action(1, place_block)
on_action(2, remove_block)

