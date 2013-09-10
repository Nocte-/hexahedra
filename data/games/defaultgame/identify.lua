
function show_name (plr, inv, look_at, pos)
    local blk = raycast(pos, look_at, 40);

    if (blk[1] ~= blk[2]) then
        print(get_block(blk[2]))
        print(material(get_block(blk[2])).name)
    end
end

on_action(3, show_name)

