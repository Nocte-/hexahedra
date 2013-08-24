-- Alright, popen isn't bidirectional, so we can't talk GTP to an engine.
-- Probably the easiest and most portable solution would be to implement
-- this in C++, and to have it turned off by default.

gnugo = assert(io.popen('gnugo --mode gtp', 'rw'))

on_login(function(plr)
   gnugo:write('boardsize 19\n');
end)

