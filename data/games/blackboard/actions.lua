-- A simple script to let users "paint" on an infinite blackboard.
-- Left click sets a block to the player's color, right click erases it.


function paint_block (plr, inv, slot, lookat)
    local hg  = vecf(0, 0, 1.7)
    local blk = raycast(plr.position + hg, lookat, 9)

    if (blk[1] ~= blk[2]) then
        change_block(blk[2], 3) -- (plr.ip_address % 10) + 2)
    end
end

function erase_block (plr, inv, slot, lookat)
    local hg  = vecf(0, 0, 1.7)
    local blk = raycast(plr.position + hg, lookat, 9)

    if (blk[1] ~= blk[2]) then
        change_block(blk[2], 1)
    end
end

on_action(1, paint_block)
on_action(2, erase_block)

