/*  Revil Format Library
    Copyright(C) 2020-2025 Lukas Cone

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

#include "revil/tex.hpp"
#include "spike/except.hpp"
#include "spike/format/DDS.hpp"
#include "spike/io/binreader_stream.hpp"
#include "spike/type/bitfield.hpp"
#include <map>
#include <vector>

using namespace revil;

enum class TextureType : uint8 {
  None,
  ColorPixel,
  General,
  Cubemap,
  Volume,
};

enum class TextureTypeV2 {
  General = 2,
  Volume = 3,
  Cubemap = 6,
};

enum class GeneralTextureType {
  None,
  IllumMap,  // IM
  ColorMap,  // BM, LM, or SRGB??
  NormalMap, // NM XGXA
};

enum class CubemapTextureType {
  Eye, // LP PC, some struct dump
  Classic,
};

enum class TEXFormat : uint32 {
  DXT1 = CompileFourCC("DXT1"),
  DXT2 = CompileFourCC("DXT2"),
  DXT3 = CompileFourCC("DXT3"),
  DXT5 = CompileFourCC("DXT5"),
  RGBA8_PACKED = 0x15,
  RG8_SNORM = 0x3c,

  /*360
  405274959
  405275014
  438305106
  438305107
  438305108
  438305137
  438305147
  438305148*/
};

enum class TEXFormatV2 : uint8 {
  RGBA16F = 0x2,
  DXT5_YUV = 0xA,
  BC7 = 0x10,
  DXT1 = 0x13,
  DXT3 = 0x15,
  DXT5 = 0x17,
  DXT1_Gray = 0x19, // BC4 for some?
  DXT1_NM = 0x1e,
  DXT5_NM = 0x1f,
  DXT5_LM = 0x20, // rgb lightmap, alpha ambient occlusion?
  DXT5_PM = 0x25, // rgb NM, alpha HM
  DXT5_ID = 0x2a,
  RGBA8 = 0x27,
};

enum class TEXFormatV2PS4 : uint8 {
  R8 = 0x7,
  DXT5_YUV = 0xA,
  BC7 = 0x10,
  DXT1 = 0x13,
  DXT3 = 0x15,
  DXT5 = 0x17,
  DXT1_NM = 0x1e,
  BC5S = 0x1f,
  BC4 = 0x19,
};

enum class TEXFormatA0 : uint8 {
  R8 = 0,
  RGBA8 = 7,
  BC3_YUV = 0xA,
  BC1 = 0x13,
  BC2 = 0x15,
  BC3 = 0x17,
  BC4 = 0x19,
  BC1_NM = 0x1e,
  BC5 = 0x1f,
};

static constexpr uint32 TEXID = CompileFourCC("TEX");
static constexpr uint32 TEXSID = CompileFourCC("TEX ");
static constexpr uint32 XETID = CompileFourCC("\0XET");

struct TEXx70 {
  using TextureType = BitMemberDecl<0, 4>;
  using TextureSubtype = BitMemberDecl<1, 4>;
  using TextureLayout = BitFieldType<uint16, TextureType, TextureSubtype>;
  uint32 id = TEXID;
  uint16 version;
  TextureLayout type;
  uint8 numMips;
  uint8 numFaces = 1; // 6 for cubemap
  uint16 null = 0;
  uint16 width;
  uint16 height;
  uint32 arraySize = 0;
  TEXFormat fourcc;
  Vector4 colorCorrection{1.f, 1.f, 1.f, 0};

  void SwapEndian(bool way) {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(type, way);
    FByteswapper(width);
    FByteswapper(height);
    FByteswapper(arraySize);
    FByteswapper(fourcc);
    FByteswapper(colorCorrection);
  }
};

struct TEXx66 {
  using TextureType = BitMemberDecl<0, 4>;
  using TextureSubtype = BitMemberDecl<1, 4>;
  using TextureLayout = BitFieldType<uint16, TextureType, TextureSubtype>;
  uint32 id = TEXID;
  uint16 version;
  TextureLayout type;
  uint8 numMips;
  uint8 numFaces = 1;
  uint16 width;
  uint16 height;
  uint16 arraySize;
  TEXFormat fourcc;
  Vector4 colorCorrection{1.f, 1.f, 1.f, 0};

  void SwapEndian(bool way) {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(type, way);
    FByteswapper(width);
    FByteswapper(height);
    FByteswapper(arraySize);
    FByteswapper(fourcc);
    FByteswapper(colorCorrection);
  }
};

struct TEXx56 {
  enum class TextureLayout : uint8 { General, Illum, Corrected = 4 };

  uint32 id;
  uint8 version;
  TextureType type;
  TextureLayout layout;
  uint8 numMips;
  uint32 width;
  uint32 height;
  uint32 arraySize;
  TEXFormat fourcc;

  void SwapEndian() {
    FByteswapper(id);
    FByteswapper(width);
    FByteswapper(height);
    FByteswapper(arraySize);
    FByteswapper(fourcc);
  }
};

struct TEXx87 {
  using TextureType = BitMemberDecl<0, 4>;
  using NumMips = BitMemberDecl<1, 5>;
  using NumFaces = BitMemberDecl<2, 8>;
  using Width = BitMemberDecl<3, 13>;
  using Tier0 = BitFieldType<uint32, TextureType, NumMips, NumFaces, Width>;
  using Height = BitMemberDecl<0, 13>;
  using Depth = BitMemberDecl<1, 13>;
  using Null = BitMemberDecl<2, 6>;
  using Tier1 = BitFieldType<uint32, Height, Depth, Null>;

  uint32 id;
  uint16 version;
  uint16 null;
  Tier0 tier0;
  Tier1 tier1;
  TEXFormatV2 format;

  void SwapEndian(bool way) {
    FByteswapper(id);
    FByteswapper(version);
    FByteswapper(tier0, way);
    FByteswapper(tier1, way);
  }
};

struct TEXx9D {
  using Version = BitMemberDecl<0, 8>;
  using Unk00 = BitMemberDecl<1, 6>;
  using Unk01 = BitMemberDecl<2, 14>;
  using TextureType = BitMemberDecl<3, 4>;
  using Tier0 = BitFieldType<uint32, Version, Unk00, Unk01, TextureType>;

  using NumMips = BitMemberDecl<0, 6>;
  using Width = BitMemberDecl<1, 13>;
  using Height = BitMemberDecl<2, 13>;
  using Tier1 = BitFieldType<uint32, NumMips, Width, Height>;

  using NumFaces = BitMemberDecl<0, 8>;
  using TextureFormat = BitMemberDecl<1, 5>;
  using Flags = BitMemberDecl<2, 3>;
  using Depth = BitMemberDecl<3, 16>;
  using Tier2 = BitFieldType<uint32, NumFaces, TextureFormat, Flags, Depth>;

  uint32 id;
  Tier0 tier0;
  Tier1 tier1;
  Tier2 tier2;

  void SwapEndian(bool) {
    FByteswapper(id);
    FByteswapper(tier0.value);
    FByteswapper(tier1.value);
    FByteswapper(tier2.value);
  }
};

enum class TEXFormatAndr : uint8 {
  RGBA8 = 0x1,
  RGBA4 = 0x7,
  ETC1 = 0x0A,
  PVRTC4 = 0x0D,
};

enum class TEXTypeAndr : uint32 {
  Base = 0x11,
  Normal = 0x21,
  Mask = 0x31,
  CubeMap = 0x61,
};

struct TEXx09 {
  uint32 id;
  uint16 version;
  TEXFormatAndr format;
  uint8 unk; // 4
  TEXTypeAndr type;

  uint32 width : 13;
  uint32 height : 13;
  uint32 numMips : 4;
  uint32 unk0 : 1;
  uint32 unk1 : 1;

  uint32 offset0;
  uint32 pvrtcOffset;
  uint32 offset1;

  uint32 unkSize;
  uint32 pvrtcSize;
  uint32 dxtSize;
};

struct TEXCubemapData {
  float unk[27];
};

TexelInputFormat ConvertTEXFormat(TEXFormat fmt) {
  TexelInputFormat retVal;

  switch (fmt) {
  case TEXFormat::DXT1:
    retVal.type = TexelInputFormatType::BC1;
    break;
  case TEXFormat::DXT2:
    retVal.type = TexelInputFormatType::BC2;
    retVal.premultAlpha = true;
    break;
  case TEXFormat::DXT3:
    retVal.type = TexelInputFormatType::BC2;
    break;
  case TEXFormat::DXT5:
    retVal.type = TexelInputFormatType::BC3;
    break;
  case TEXFormat::RGBA8_PACKED:
    retVal.type = TexelInputFormatType::RGBA8;
    break;
  case TEXFormat::RG8_SNORM:
    retVal.type = TexelInputFormatType::RG8;
    retVal.snorm = true;
    break;

  default:
    throw std::runtime_error("Unknown texture format!");
  }

  return retVal;
}

TexelInputFormat ConvertTEXFormat(TEXFormatV2 fmt) {
  TexelInputFormat retVal;

  switch (fmt) {
  case TEXFormatV2::DXT1:
  case TEXFormatV2::DXT1_Gray:
  case TEXFormatV2::DXT1_NM:
    retVal.type = TexelInputFormatType::BC1;
    break;
  case TEXFormatV2::DXT3:
    retVal.type = TexelInputFormatType::BC2;
    break;
  case TEXFormatV2::DXT5:
  case TEXFormatV2::DXT5_NM:
  case TEXFormatV2::DXT5_LM:
  case TEXFormatV2::DXT5_PM:
  case TEXFormatV2::DXT5_ID:
  case TEXFormatV2::DXT5_YUV:
    retVal.type = TexelInputFormatType::BC3;
    break;
  case TEXFormatV2::RGBA16F:
    retVal.type = TexelInputFormatType::RGBA16;
    break;
  case TEXFormatV2::RGBA8:
    retVal.type = TexelInputFormatType::RGBA8;
    break;

  default:
    throw std::runtime_error("Unknown texture format!");
  }

  return retVal;
}

TexelInputFormat ConvertTEXFormat(TEXFormatV2PS4 fmt) {
  TexelInputFormat retVal;

  switch (fmt) {
  case TEXFormatV2PS4::DXT1:
  case TEXFormatV2PS4::DXT1_NM:
    retVal.type = TexelInputFormatType::BC1;
    break;
  case TEXFormatV2PS4::DXT3:
    retVal.type = TexelInputFormatType::BC2;
    break;
  case TEXFormatV2PS4::DXT5:
  case TEXFormatV2PS4::DXT5_YUV:
    retVal.type = TexelInputFormatType::BC3;
    break;
  case TEXFormatV2PS4::BC4:
    retVal.type = TexelInputFormatType::BC4;
    break;
  case TEXFormatV2PS4::BC5S:
    retVal.type = TexelInputFormatType::BC5;
    retVal.snorm = true;
    break;
  case TEXFormatV2PS4::BC7:
    retVal.type = TexelInputFormatType::BC7;
    break;
  case TEXFormatV2PS4::R8:
    retVal.type = TexelInputFormatType::R8;
    break;
  default:
    throw std::runtime_error("Unknown texture format!");
  }

  return retVal;
}

TexelInputFormat ConvertTEXFormat(TEXFormatA0 fmt) {
  TexelInputFormat retVal;

  switch (fmt) {
  case TEXFormatA0::BC1:
  case TEXFormatA0::BC1_NM:
    retVal.type = TexelInputFormatType::BC1;
    break;
  case TEXFormatA0::BC2:
    retVal.type = TexelInputFormatType::BC2;
    break;
  case TEXFormatA0::BC3:
  case TEXFormatA0::BC3_YUV:
    retVal.type = TexelInputFormatType::BC3;
    break;
  case TEXFormatA0::BC4:
    retVal.type = TexelInputFormatType::BC4;
    break;
  case TEXFormatA0::BC5:
    retVal.type = TexelInputFormatType::BC5;
    retVal.swizzle.g = TexelSwizzleType::DeriveZ;
    break;
  case TEXFormatA0::RGBA8:
    retVal.type = TexelInputFormatType::RGBA8;
    break;
  case TEXFormatA0::R8:
    retVal.type = TexelInputFormatType::R8;
    break;

  default:
    throw std::runtime_error("Unknown texture format!");
  }

  return retVal;
}

void ApplyModifications(NewTexelContextCreate &ctx, Platform platform) {
  if (ctx.baseFormat.type == TexelInputFormatType::RGBA8 &&
      platform == Platform::PS3 && IsPow2(ctx.width) && IsPow2(ctx.height)) {
    ctx.baseFormat.tile = TexelTile::Morton;
  } else if (platform == Platform::PS4) {
    ctx.baseFormat.tile = TexelTile::MortonForcePow2;
  } else if (platform == Platform::NSW) {
    ctx.baseFormat.tile = TexelTile::NX;
  }
}

TEX LoadTEXx56(BinReaderRef_e rd) {
  TEX main;
  TEXx56 header;
  rd.Read(header);

  if (header.layout == TEXx56::TextureLayout::Corrected) {
    rd.Read(main.color);
  }

  if (header.type == TextureType::Volume) {
    BinReaderRef rdn(rd);
    DDS_Header ddsHdr;
    DDS_PixelFormat ddsPf;
    DDS_HeaderEnd ddsFt;
    rdn.Read(ddsHdr);
    rdn.Read(ddsPf);
    rdn.Read(ddsFt);

    main.ctx.width = ddsHdr.width;
    main.ctx.height = ddsHdr.height;
    main.ctx.depth = ddsHdr.depth;
    main.ctx.numMipmaps = ddsHdr.mipMapCount;

    if (ddsPf == DDSFormat_DXT5) {
      main.ctx.baseFormat.type = TexelInputFormatType::BC3;
    } else {
      throw std::runtime_error("Unknown texture format!");
    }
  } else if (header.type == TextureType::Cubemap) {
    throw std::runtime_error("Cubemaps are not supported.");
  } else {
    main.ctx.width = header.width;
    main.ctx.height = header.height;
    main.ctx.depth = header.arraySize;
    main.ctx.numMipmaps = header.numMips;
    main.ctx.baseFormat = ConvertTEXFormat(header.fourcc);
  }

  size_t bufferSize = rd.GetSize() - rd.Tell();
  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, Platform::Win32);

  return main;
}

template <class header_type>
TEX LoadTEXx66(BinReaderRef_e rd, Platform platform) {
  TEX main;
  header_type header;
  rd.Read(header);

  main.ctx.width = header.width;
  main.ctx.height = header.height;
  main.ctx.depth = header.arraySize;
  main.ctx.numMipmaps = header.numMips;
  main.ctx.baseFormat = ConvertTEXFormat(header.fourcc);
  main.color = Vector4A16(header.colorCorrection);

  TextureType type = static_cast<TextureType>(
      header.type.template Get<typename header_type::TextureType>());

  if (type == TextureType::Cubemap) {
    throw std::runtime_error("Cubemaps are not supported.");
  }

  rd.ReadContainer(main.offsets, header.numFaces * header.numMips);

  size_t bufferSize = rd.GetSize() - rd.Tell();

  if (header.arraySize) {
    bufferSize *= header.arraySize;
  }

  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, platform);

  return main;
}

TEX LoadTEXx87(BinReaderRef_e rd, Platform) {
  TEX main;
  TEXx87 header;
  rd.Read(header);
  using t = TEXx87;

  main.ctx.width = header.tier0.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.tier1.Get<t::Depth>();
  main.ctx.numMipmaps = header.tier0.Get<t::NumMips>();
  main.ctx.baseFormat = ConvertTEXFormat(header.format);

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    throw std::runtime_error("Cubemaps are not supported.");
  }

  uint32 numOffsets = main.ctx.depth * main.ctx.numMipmaps;
  rd.ReadContainer(main.offsets, numOffsets);

  size_t bufferSize = rd.GetSize() - rd.Tell();

  if (main.ctx.depth) {
    bufferSize *= main.ctx.depth;
  }

  rd.ReadContainer(main.buffer, bufferSize);

  return main;
}

TEX LoadTEXx9D(BinReaderRef_e rd, Platform platform) {
  TEX main;
  TEXx9D header;
  rd.Read(header);
  using t = TEXx9D;

  main.ctx.width = header.tier1.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.tier2.Get<t::Depth>();
  main.ctx.numMipmaps = header.tier1.Get<t::NumMips>();

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    throw std::runtime_error("Cubemaps are not supported.");
  }

  uint32 numOffsets = main.ctx.depth * main.ctx.numMipmaps;

  auto fallback = [&] {
    rd.ReadContainer(main.offsets, numOffsets);
    main.ctx.baseFormat =
        ConvertTEXFormat((TEXFormatV2)header.tier2.Get<t::TextureFormat>());
  };

  if (!rd.SwappedEndian()) {
    uint32 offset0;
    rd.Push();
    rd.Read(offset0);
    rd.Pop();
    uint32 dataBeginPredict = (numOffsets * sizeof(uint32)) + rd.Tell();

    if (offset0 == dataBeginPredict) {
      fallback();
    } else {
      std::vector<uint64> offsets;
      rd.ReadContainer(offsets, numOffsets);
      main.offsets.assign(offsets.begin(), offsets.end());
      platform = Platform::PS4;
      main.ctx.baseFormat = ConvertTEXFormat(
          (TEXFormatV2PS4)header.tier2.Get<t::TextureFormat>());
    }
  } else {
    fallback();
  }

  size_t bufferSize = rd.GetSize() - rd.Tell();

  if (main.ctx.depth) {
    bufferSize *= main.ctx.depth;
  }

  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, platform);

  return main;
}

TEX LoadTEXx09(BinReaderRef_e rd_, Platform platform) {
  BinReaderRef rd(rd_);
  TEX main;
  TEXx09 header;
  rd.Read(header);

  main.ctx.width = header.width;
  main.ctx.height = header.height;
  main.ctx.numMipmaps = header.numMips;

  switch (header.format) {
  case TEXFormatAndr::PVRTC4:
    main.ctx.baseFormat.type = TexelInputFormatType::PVRTC4;
    break;
  case TEXFormatAndr::ETC1:
    main.ctx.baseFormat.type = TexelInputFormatType::ETC1;
    break;
  case TEXFormatAndr::RGBA4:
    main.ctx.baseFormat.type = TexelInputFormatType::RGBA4;
    break;
  case TEXFormatAndr::RGBA8:
    main.ctx.baseFormat.type = TexelInputFormatType::RGBA8;
    break;
  default:
    throw std::runtime_error("Unknown texture format!");
  }

  ApplyModifications(main.ctx, platform);

  return main;
}

TEX LoadTEXxA0(BinReaderRef_e rd, Platform platform) {
  TEX main;
  TEXx9D header;
  rd.Read(header);
  using t = TEXx9D;

  main.ctx.width = header.tier1.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.tier2.Get<t::Depth>();
  main.ctx.numMipmaps = header.tier1.Get<t::NumMips>();

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    main.ctx.numFaces = 6;
    rd.Read(main.harmonics);
  }

  uint32 numOffsets = main.ctx.depth * main.ctx.numMipmaps;

  uint32 bufferSize;
  rd.Read(bufferSize);
  rd.ReadContainer(main.offsets, numOffsets);
  main.ctx.baseFormat =
      ConvertTEXFormat((TEXFormatA0)header.tier2.Get<t::TextureFormat>());

  if (type == TextureTypeV2::Cubemap) {
    rd.Read(main.faceSize);
  }

  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, platform);

  return main;
}

static const std::map<uint16, TEX (*)(BinReaderRef_e, Platform)> texLoaders{
    {0x66, LoadTEXx66<TEXx66>}, {0x70, LoadTEXx66<TEXx70>}, {0x87, LoadTEXx87},
    {0x9D, LoadTEXx9D},         {0x09, LoadTEXx09},         {0xA0, LoadTEXxA0},
};

void TEX::Load(BinReaderRef_e rd, Platform platform) {
  struct {
    uint32 id;
    union {
      uint8 versionV10;
      uint16 versionV11;
      uint32 versionV20;
    };

    void NoSwap();
  } header{};

  rd.Read(header);
  rd.Seek(0);

  if (header.id == XETID) {
    rd.SwapEndian(true);
  } else if (header.id != TEXID && header.id != TEXSID) {
    throw es::InvalidHeaderError(header.id);
  }

  if (header.versionV10 == 0x56) {
    if (rd.SwappedEndian()) {
      throw std::runtime_error("X360 texture format is unsupported.");
    }

    *this = LoadTEXx56(rd);
    return;
  } else if (rd.SwappedEndian()) {
    FByteswapper(header.versionV11);
  }

  if (platform == Platform::Auto) {
    platform = rd.SwappedEndian() ? Platform::PS3 : Platform::Win32;
  }

  auto found = texLoaders.find(header.versionV11);

  if (!es::IsEnd(texLoaders, found)) {
    *this = found->second(rd, platform);
    return;
  }

  if (rd.SwappedEndian()) {
    FByteswapper(header.versionV20);
  }

  found = texLoaders.find(header.versionV10);

  if (es::IsEnd(texLoaders, found)) {
    throw es::InvalidVersionError();
  }

  *this = found->second(rd, platform);
  return;
}
