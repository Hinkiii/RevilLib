/*  ARCConvert
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

#include "arc_conv.hpp"
#include "hfs.hpp"
#include "project.h"
#include "revil/arc.hpp"

static struct ARCExtract : ReflectorBase<ARCExtract> {
  std::string title;
  Platform platform = Platform::Auto;
  std::string classWhitelist;
  std::set<uint32> classWhitelist_;
} settings;

REFLECT(CLASS(ARCExtract),
        MEMBER(title, "t", ReflDesc{"Set title for correct archive handling."}),
        MEMBER(platform, "p",
               ReflDesc{"Set platform for correct archive handling."}),
        MEMBERNAME(classWhitelist, "class-whitelist",
                   ReflDesc{"Extract only specified (comma separated) classes. "
                            "Extract all if empty."}));

std::string_view filters[]{
    ".arc$",
};

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header = ARCConvert_DESC " v" ARCConvert_VERSION ", " ARCConvert_COPYRIGHT
                              "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

bool AppInitContext(const std::string &) {
  std::string_view sv(settings.classWhitelist);
  size_t lastPost = 0;
  auto found = sv.find(',');

  while (found != sv.npos) {
    auto sub = sv.substr(lastPost, found - lastPost);
    settings.classWhitelist_.insert(revil::MTHashV1(es::TrimWhitespace(sub)));
    settings.classWhitelist_.insert(revil::MTHashV2(es::TrimWhitespace(sub)));
    lastPost = ++found;
    found = sv.find(',', lastPost);
  }

  if (lastPost < sv.size()) {
    std::string_view sub = sv.substr(lastPost);
    settings.classWhitelist_.insert(revil::MTHashV1(es::TrimWhitespace(sub)));
    settings.classWhitelist_.insert(revil::MTHashV2(es::TrimWhitespace(sub)));
  }

  return true;
}

void AppProcessFile(AppContext *ctx) {
  revil::EnumerateArchive(
      ctx->GetStream(), settings.platform, settings.title,
      [ctx] { return ctx->ExtractContext(); }, settings.classWhitelist_);
}

size_t AppExtractStat(request_chunk requester) {
  auto data = requester(0, 32);
  HFS *hfs = reinterpret_cast<HFS *>(data.data());
  ARCBase *arcHdr = nullptr;

  if (hfs->id == SFHID) {
    arcHdr = reinterpret_cast<ARCBase *>(hfs + 1);
  } else {
    arcHdr = reinterpret_cast<ARCBase *>(hfs);
  }

  if (arcHdr->id == CRAID) {
    arcHdr->SwapEndian();
  } else if (arcHdr->id != ARCID && arcHdr->id != ARCCID) {
    return 0;
  }

  return arcHdr->numFiles;
}
