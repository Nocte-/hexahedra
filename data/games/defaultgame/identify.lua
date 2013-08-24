
function show_name (plr, inv)
    --print "Position "..plr.position
    local blk = raycast(plr.position, plr.aiming_at, 9);

    if (blk[1] ~= blk[2]) then
        print(material(get_block(blk[2])).name)
    end
end

on_action(2, show_name)

