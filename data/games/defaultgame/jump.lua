
function jump (plr, inv, slot, lookat)
    if (plr.impact.z > 0) then
        local v = plr.velocity
        v.z = 7
        plr.velocity = v;
    end
end

on_action(0, jump)

