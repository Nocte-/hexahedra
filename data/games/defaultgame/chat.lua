
on_console(function(plr, text)
    broadcast_console_message("{\"type\":\"chat\",\"name\":\""..plr.name.."\",\"message\":\""..text.."\"}")
end)

