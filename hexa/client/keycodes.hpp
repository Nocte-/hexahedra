//---------------------------------------------------------------------------
/// \file	client/keycodes.hpp
/// \brief  Enumeration of key codes
//
// This file is part of Hexahedra.
//
// Hexahedra is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

namespace hexa
{

/** An enumeration of key codes.
 *  This is not just used by the library's own classes, but also as an
 *  intermediate between event producers (such as SFML or OIS) and
 *  consumers (such as CEGUI). */
#if (defined _MSC_VER) && _MSC_VER <= 1600 // :'(
#pragma warning(disable : 4482)
enum key
#else
enum class key : uint32_t
#endif
{ a,
  b,
  c,
  d,
  e,
  f,
  g,
  h,
  i,
  j,
  k,
  l,
  m,
  n,
  o,
  p,
  q,
  r,
  s,
  t,
  u,
  v,
  w,
  x,
  y,
  z,
  num0,
  num1,
  num2,
  num3,
  num4,
  num5,
  num6,
  num7,
  num8,
  num9,
  esc,
  l_control,
  l_shift,
  l_alt,
  l_system,
  r_control,
  r_shift,
  r_alt,
  r_system,
  menu,
  l_bracket,
  r_bracket,
  semicolon,
  comma,
  period,
  quote,
  slash,
  backslash,
  tilde,
  equal,
  dash,
  space,
  enter,
  backspace,
  tab,
  pg_up,
  pg_down,
  end,
  home,
  insert,
  del,
  add,
  subtract,
  multiply,
  divide,
  left,
  right,
  up,
  down,
  numpad0,
  numpad1,
  numpad2,
  numpad3,
  numpad4,
  numpad5,
  numpad6,
  numpad7,
  numpad8,
  numpad9,
  f1,
  f2,
  f3,
  f4,
  f5,
  f6,
  f7,
  f8,
  f9,
  f10,
  f11,
  f12,
  f13,
  f14,
  f15,
  pause,

  numpad_enter,
  numpad_period,
  grave,
  num_lock,
  scroll_lock,
  print_screen,

  prev_track,
  next_track,
  mute,
  calculator,
  play_pause,
  stop,
  volume_down,
  volume_up,
  web_home,
  power,
  sleep,
  wake,
  web_search,
  web_favorites,
  web_refresh,
  web_stop,
  web_forward,
  web_back,
  my_computer,
  mail,
  media,

  count // special value
};

} // namespace hexa
