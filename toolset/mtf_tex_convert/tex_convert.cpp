/*  MTFTEXConvert
    Copyright(C) 2021-2025 Lukas Cone

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

#include "project.h"
#include "re_common.hpp"
#include "revil/tex.hpp"
#include "spike/io/binreader_stream.hpp"

std::string_view filters[]{
    ".tex$",
};

struct TEXConvert : ReflectorBase<TEXConvert> {
  Platform platformOverride = Platform::Auto;
} settings;

REFLECT(CLASS(TEXConvert),
        MEMBERNAME(platformOverride, "platform", "p",
                   ReflDesc{"Set platform for correct texture handling."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = MTFTEXConvert_DESC " v" MTFTEXConvert_VERSION
                                 ", " MTFTEXConvert_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

void AppProcessFile(AppContext *ctx) {
  TEX tex;
  tex.Load(ctx->GetStream(), settings.platformOverride);

  auto tctx = ctx->NewImage(tex.ctx);

  if (tex.ctx.numFaces > 0) {
    for (uint16 f = 0; f < tex.ctx.numFaces; f++) {
      for (uint8 m = 0; m < tex.ctx.numMipmaps; m++) {
        TexelInputLayout layout{
            .mipMap = m,
            .face = CubemapFace(f + 1),
        };
        tctx->SendRasterData(
            tex.buffer.data() + tex.offsets.at(m) + f * tex.faceSize, layout);
      }
    }
  } else {
    for (uint16 d = 0; d < tex.ctx.depth; d++) {
      for (uint8 m = 0; m < tex.ctx.numMipmaps; m++) {
        TexelInputLayout layout{
            .mipMap = m,
            .layer = d,
        };
        tctx->SendRasterData(tex.buffer.data() +
                                 tex.offsets.at(d * tex.ctx.numMipmaps + m),
                             layout);
      }
    }
  }
}
