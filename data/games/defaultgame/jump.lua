
function jump (plr, inv, slot, lookat)
    if (plr.impact.z > 0) then
        local v = plr.velocity
        v.z = v.z + 7;
    end
end

on_action(0, jump)

