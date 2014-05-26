
function chat (plr, text)
    print("Chat:")
    print(text)
    print(plr.position.x)
    print(plr.name)
    broadcast_console_message("{\"type\":\"chat\",\"name\":\""..plr.name.."\",\"message\":\""..text.."\"}")
end

on_console(chat)

