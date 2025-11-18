/*  TEXDump
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
#include "revil/hashreg.hpp"
#include "spike/io/binreader_stream.hpp"
#include "tex.hpp"
#include <fstream>
#include <mutex>
#include <spanstream>

#include "revil/arc.hpp"

std::string_view filters[]{".tex$", ".arc$"};

struct TEXConvert : ReflectorBase<TEXConvert> {
  std::string title;
  Platform platform = Platform::Auto;
} settings;

REFLECT(CLASS(TEXConvert),
        MEMBER(title, "t", ReflDesc{"Set title for correct archive handling."}),
        MEMBER(platform, "p",
               ReflDesc{"Set platform for correct archive handling."}));

static AppInfo_s appInfo{
    .filteredLoad = true,
    .header =
        TEXDump_DESC " v" TEXDump_VERSION ", " TEXDump_COPYRIGHT "Lukas Cone",
    .settings = reinterpret_cast<ReflectorFriend *>(&settings),
    .filters = filters,
};

AppInfo_s *AppInitModule() { return &appInfo; }

static std::ofstream REPORT("tex_report.csv");

void DumpTexture(BinReaderRef_e rd, std::string_view fileName) {
  static std::mutex mtx;
  auto func = [&](uint32 version, BinReaderRef_e rd, Platform) -> bool {
    switch (version) {
    case 0xA3:
    case 0xA4:
    case 0xA5:
    case 0xA6:
    case 0xA0:
    case 0x9D:
    case 0x99:
    case 0x9A:
    case 0x98:
    case 0x97: {
      TEXx9D header;
      rd.Read(header);
      using t = TEXx9D;
      std::lock_guard lg(mtx);
      // Version,Unk00,Unk01,TextureType,NumMips,Width,Height,NumFaces,TextureFormat,Depth,BigEndian,FilePath
      REPORT << header.tier0.Get<t::Version>() << ','
             << header.tier0.Get<t::Unk00>() << ','
             << header.tier0.Get<t::Unk01>() << ','
             << header.tier0.Get<t::TextureType>() << ','
             << header.tier1.Get<t::NumMips>() << ','
             << header.tier1.Get<t::Width>() << ','
             << header.tier1.Get<t::Height>() << ',' << int(header.numFaces)
             << ',' << int(header.format) << ',' << header.depth << ','
             << (rd.SwappedEndian() ? "true" : "false") << ',' << fileName
             << '\n';
      return true;
    }
    case 0x9: {
      TEXx09 header;
      BinReaderRef(rd).Read(header);
      std::lock_guard lg(mtx);
      // Version,TextureFormat,Unk00,Unk01,TextureType,Width,Height,NumMips,Unk0,Unk1,DataOffset,PVROffset,UnkOffset,DataSize,PVRSize,UnkSize,FilePath
      REPORT << header.version << ','
             << int(header.format) << ','
             << int(header.unk00) << ','
             << int(header.unk01) << ','
             << int(header.type) << ','
             << header.width << ','
             << header.height << ','
             << header.numMips << ','
             << header.unk0 << ','
             << header.unk1 << ','
             << header.dataOffset << ','
             << header.pvrVariantOffset << ','
             << header.unkVariantOffset << ','
             << header.dataSize << ','
             << header.pvrSize << ','
             << header.unkSize << ','
             << fileName
             << '\n';
      return true;
    }
    }

    return false;
  };

  LoadDetectTex(rd, settings.platform, func);
}

struct ExtractContext : ArcExtractContext {
  std::string currentFile;
  void NewFile(const std::string &path) override {
    currentFile = AFileInfo(path).GetFullPathNoExt();
  }
  void SendData(std::string_view data) override {
    std::ispanstream spstr(std::span<const char>(data.data(), data.size()));
    DumpTexture(spstr, currentFile);
  }
};

void AppProcessFile(AppContext *ctx) {
  if (ctx->workingFile.GetExtension() == ".arc") {
    static const std::set<uint32> filter{
        MTHashV1("rTexture"),
        MTHashV2("rTexture"),
    };

    ExtractContext ectx;

    EnumerateArchive(
        ctx->GetStream(), settings.platform, settings.title,
        [&] { return &ectx; }, filter);
    return;
  }

  DumpTexture(ctx->GetStream(), ctx->workingFile.GetFullPathNoExt());
}
