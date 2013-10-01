 
function set_hotbar(plr)
    plr.hotbar_size = 6
    plr:hotbar_set(0, hotbar_slot(1, "planks"))
    plr:hotbar_set(1, hotbar_slot(1, "fence.0000"))
    plr:hotbar_set(2, hotbar_slot(1, "rock"))
    plr:hotbar_set(3, hotbar_slot(1, "cobblestone"))
    plr:hotbar_set(4, hotbar_slot(1, "chest (n)"))
end


on_login(set_hotbar)
