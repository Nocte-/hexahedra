-- Place a Go stone

on_action(0, function (plr, inv)
    local blk = raycast(plr.position, plr.aiming_at, 9);

    if (blk[1] ~= blk[2]) then
        change_block(blk[2], 3)
    end
end)

