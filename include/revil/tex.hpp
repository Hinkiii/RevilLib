/*  Revil Format Library
    Copyright(C) 2017-2025 Lukas Cone

    This program is free software : you can redistribute it and / or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once
#include "spike/io/bincore_fwd.hpp"
#include "spike/type/vectors_simd.hpp"
#include "spike/app_context.hpp"
#include "platform.hpp"
#include "settings.hpp"
#include <string>

namespace revil {
struct RE_EXTERN TEX {
  NewTexelContextCreate ctx;
  Vector4A16 color;
  std::string buffer;
  std::vector<uint32> offsets;
  float harmonics[27];

  void Load(BinReaderRef_e rd, Platform platform = Platform::Auto);
};
} // namespace revil
