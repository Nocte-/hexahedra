
function jump (plr, inv, slot, lookat)
    if (plr.impact.z > 0) then
        plr.velocity.z = plr.velocity.z + 5;
    end
end

on_action(0, jump)

