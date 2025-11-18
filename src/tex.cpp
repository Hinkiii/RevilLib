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

#include "tex.hpp"
#include "hfs.hpp"
#include "revil/tex.hpp"
#include "spike/except.hpp"
#include "spike/format/DDS.hpp"
#include "spike/io/binreader_stream.hpp"
#include <map>

using namespace revil;

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
    throw es::RuntimeError("Unknown texture format!");
  }

  return retVal;
}

TexelInputFormat ConvertTEXFormat(TEXFormatV2 fmt, Platform platform) {
  TexelInputFormat retVal;

  switch (fmt) {
  case TEXFormatV2::BC1:
  case TEXFormatV2::COMPRESSED_NORMAL_MAP:
  case TEXFormatV2::BC1A:
    retVal.type = TexelInputFormatType::BC1;
    break;
  case TEXFormatV2::COMPRESSED_GRAYSCALE:
    retVal.type = platform == Platform::PS4 || platform == Platform::NSW
                      ? TexelInputFormatType::BC4
                      : TexelInputFormatType::BC1;
    break;
  case TEXFormatV2::BC2:
  case TEXFormatV2::BC2_PA:
    retVal.type = TexelInputFormatType::BC2;
    break;
  case TEXFormatV2::BC3:
  case TEXFormatV2::BC3_LM:
  case TEXFormatV2::BC3_PM:
  case TEXFormatV2::BC3_YUV:
  case TEXFormatV2::BC3_CM:
  case TEXFormatV2::BC3_PA:
  case TEXFormatV2::BC3_YUV_PA:
  case TEXFormatV2::BC3_EFF:
    retVal.type = TexelInputFormatType::BC3;
    break;

  case TEXFormatV2::COMPRESSED_DERIVED_NORMAL_MAP:
    retVal.type = platform == Platform::PS4 || platform == Platform::NSW
                      ? TexelInputFormatType::BC5
                      : TexelInputFormatType::BC3;
    break;
  case TEXFormatV2::RGBA16F:
    retVal.type = TexelInputFormatType::RGBA16;
    break;
  case TEXFormatV2::RGBA8:
  case TEXFormatV2::RGBA8_PA:
  case TEXFormatV2::RGBA8_LP4:
    retVal.type = TexelInputFormatType::RGBA8;
    break;
  case TEXFormatV2::R8:
    retVal.type = TexelInputFormatType::R8;
    break;
  case TEXFormatV2::BC7_PA:
  case TEXFormatV2::BC7:
    retVal.type = TexelInputFormatType::BC7;
    break;
  case TEXFormatV2::RGB10A2:
    retVal.type = TexelInputFormatType::RGB10A2;
    break;
  default:
    throw es::RuntimeError("Unknown texture format!");
  }

  return retVal;
}

TexelInputFormat ConvertTEXFormat(TEXFormat3DS fmt) {
  TexelInputFormat retVal;

  switch (fmt) {
  case TEXFormat3DS::IA8:
    retVal.type = TexelInputFormatType::RG8;
    retVal.swizzle.r = TexelSwizzleType::Red;
    retVal.swizzle.g = TexelSwizzleType::Red;
    retVal.swizzle.b = TexelSwizzleType::Red;
    retVal.swizzle.a = TexelSwizzleType::Green;
    break;
  case TEXFormat3DS::ETC1:
    retVal.type = TexelInputFormatType::ETC1;
    break;
  case TEXFormat3DS::ETC1A4:
    retVal.type = TexelInputFormatType::ETC1A4;
    break;
  case TEXFormat3DS::A4:
  case TEXFormat3DS::L4:
    retVal.type = TexelInputFormatType::R4;
    break;
  case TEXFormat3DS::RGB8:
    retVal.type = TexelInputFormatType::RGB8;
    break;
  case TEXFormat3DS::RGBA8:
    retVal.type = TexelInputFormatType::RGBA8;
    retVal.swizzle.r = TexelSwizzleType::Alpha;
    retVal.swizzle.g = TexelSwizzleType::Red;
    retVal.swizzle.b = TexelSwizzleType::Green;
    retVal.swizzle.a = TexelSwizzleType::Blue;
    break;
  case TEXFormat3DS::RGBA4:
    retVal.type = TexelInputFormatType::RGBA4;
    break;
  case TEXFormat3DS::R5G6B5:
    retVal.type = TexelInputFormatType::R5G6B5;
    break;
  case TEXFormat3DS::R8:
    retVal.type = TexelInputFormatType::R8;
    break;
  case TEXFormat3DS::RG4:
    retVal.type = TexelInputFormatType::RG4;
    break;
  case TEXFormat3DS::AL4:
    retVal.type = TexelInputFormatType::RG4;
    retVal.swizzle.r = TexelSwizzleType::Red;
    retVal.swizzle.g = TexelSwizzleType::Red;
    retVal.swizzle.b = TexelSwizzleType::Red;
    retVal.swizzle.a = TexelSwizzleType::Green;
    break;

  default:
    throw es::RuntimeError("Unknown texture format!");
  }

  return retVal;
}

void ApplyModifications(NewTexelContextCreate &ctx, Platform platform) {
  if (ctx.baseFormat.type == TexelInputFormatType::RGBA8 &&
      platform == Platform::PS3 && IsPow2(ctx.width) && IsPow2(ctx.height)) {
    ctx.baseFormat.tile = TexelTile::Morton;
  } else if (platform == Platform::PS4) {
    ctx.baseFormat.tile = TexelTile::PS4;
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
      throw es::RuntimeError("Unknown texture format!");
    }
  } else if (header.type == TextureType::Cubemap) {
    throw es::RuntimeError("Cubemaps are not supported.");
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
  main.ctx.depth = std::max(1, int(header.arraySize));
  main.ctx.numMipmaps = header.numMips;
  main.ctx.baseFormat = ConvertTEXFormat(header.fourcc);
  main.color = Vector4A16(header.colorCorrection);

  TextureType type = static_cast<TextureType>(
      header.type.template Get<typename header_type::TextureType>());

  if (type == TextureType::Cubemap) {
    main.ctx.numFaces = 6;
    rd.Read(main.harmonics);
  }

  rd.ReadContainer(main.offsets, header.numFaces * header.numMips);
  uint32 bufferBegin = rd.Tell();

  for (uint32 &o : main.offsets) {
    o -= bufferBegin;
  }

  size_t bufferSize = rd.GetSize() - bufferBegin;

  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, platform);

  if (rd.SwappedEndian() &&
      main.ctx.baseFormat.type == TexelInputFormatType::RGBA8) {
    main.ctx.baseFormat.swapPacked = true;
  }

  return main;
}

TEX LoadTEXx87(BinReaderRef_e rd, Platform platform) {
  TEX main;
  TEXx87 header;
  rd.Read(header);
  using t = TEXx87;

  main.ctx.width = header.tier0.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.tier1.Get<t::Depth>();
  main.ctx.numMipmaps = header.tier0.Get<t::NumMips>();
  main.ctx.baseFormat = ConvertTEXFormat(header.format, platform);

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    main.ctx.numFaces = 6;
    rd.Read(main.harmonics);
  }

  uint32 numOffsets =
      std::max(int8(1), main.ctx.numFaces) * main.ctx.numMipmaps;
  rd.ReadContainer(main.offsets, numOffsets);

  uint32 bufferBegin = rd.Tell();

  for (uint32 &o : main.offsets) {
    o -= bufferBegin;
  }

  size_t bufferSize = rd.GetSize() - bufferBegin;
  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, platform);

  if (rd.SwappedEndian() &&
      main.ctx.baseFormat.type == TexelInputFormatType::RGBA8) {
    main.ctx.baseFormat.swapPacked = true;
  }

  return main;
}

TEX LoadTEXx9D(BinReaderRef_e rd, Platform platform) {
  TEX main;
  TEXx9D header;
  rd.Read(header);
  using t = TEXx9D;

  main.ctx.width = header.tier1.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.depth;
  main.ctx.numMipmaps = header.tier1.Get<t::NumMips>();

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    main.ctx.numFaces = 6;
    rd.Read(main.harmonics);
  }

  uint32 numOffsets =
      std::max(int8(1), main.ctx.numFaces) * main.ctx.numMipmaps;

  auto fallback = [&] {
    rd.ReadContainer(main.offsets, numOffsets);
    main.ctx.baseFormat = ConvertTEXFormat(header.format, platform);
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
      main.ctx.baseFormat = ConvertTEXFormat(header.format, platform);
    }
  } else {
    fallback();
  }

  uint32 bufferBegin = rd.Tell();

  for (uint32 &o : main.offsets) {
    o -= bufferBegin;
  }

  size_t bufferSize = rd.GetSize() - bufferBegin;

  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, platform);

  return main;
}

TEX LoadTEXx09(BinReaderRef_e rd_, Platform) {
  BinReaderRef rd(rd_);
  TEX main;
  TEXx09 header;
  rd.Read(header);

  main.ctx.width = header.width;
  main.ctx.height = header.height;
  main.ctx.numMipmaps = 1;

  if (header.type == TEXTypeAndr::Cubemap) {
    throw es::RuntimeError("Cubemaps are not supported.");
  }

  main.offsets.emplace_back(0);

  switch (header.format) {
  case TEXFormatAndr::PVRTC4:
    main.ctx.baseFormat.type = TexelInputFormatType::PVRTC4;
    main.ctx.baseFormat.swizzle.r = TexelSwizzleType::Blue;
    main.ctx.baseFormat.swizzle.b = TexelSwizzleType::Red;
    main.offsets.back() = header.pvrVariantOffset - header.dataOffset;
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
  case TEXFormatAndr::BC3:
    main.ctx.baseFormat.type = TexelInputFormatType::BC3;
    break;
  case TEXFormatAndr::R5G6B5:
    main.ctx.baseFormat.type = TexelInputFormatType::R5G6B5;
    break;
  default:
    throw es::RuntimeError("Unknown texture format!");
  }

  rd.Seek(header.dataOffset);
  size_t bufferSize = rd.GetSize() - header.dataOffset;

  rd.ReadContainer(main.buffer, bufferSize);

  return main;
}

TEX LoadTEXxA0(BinReaderRef_e rd, Platform platform) {
  TEX main;
  TEXx9D header;
  rd.Read(header);
  using t = TEXx9D;

  main.ctx.width = header.tier1.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.depth;
  main.ctx.numMipmaps = header.tier1.Get<t::NumMips>();

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    main.ctx.numFaces = 6;
    rd.Read(main.harmonics);
  }

  uint32 numOffsets = main.ctx.numMipmaps;

  uint32 bufferSize;
  rd.Read(bufferSize);
  rd.ReadContainer(main.offsets, numOffsets);
  main.ctx.baseFormat = ConvertTEXFormat(header.format, platform);

  if (type == TextureTypeV2::Cubemap) {
    uint32 faceSize;
    rd.Read(faceSize);
    auto offsets = main.offsets;

    for (uint32 f = 0; f < 5; f++) {
      for (auto &o : offsets) {
        o += faceSize;
      }

      main.offsets.insert(main.offsets.end(), offsets.begin(), offsets.end());
    }
  }

  rd.ReadContainer(main.buffer, bufferSize);
  ApplyModifications(main.ctx, platform);

  return main;
}

TEX LoadTEXxA6(BinReaderRef_e rd, Platform) {
  TEX main;
  TEXx9D header;
  rd.Read(header);
  using t = TEXx9D;

  main.ctx.width = header.tier1.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.depth;
  main.ctx.numMipmaps = header.tier1.Get<t::NumMips>();

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    main.ctx.numFaces = 6;
    rd.Read(main.harmonics);
  }

  uint32 numOffsets =
      main.ctx.numMipmaps * std::max(int8(1), main.ctx.numFaces);
  rd.ReadContainer(main.offsets, numOffsets);
  main.ctx.baseFormat = ConvertTEXFormat(TEXFormat3DS(header.format));

  size_t bufferSize = rd.GetSize() - rd.Tell();

  rd.ReadContainer(main.buffer, bufferSize);
  main.ctx.baseFormat.tile = TexelTile::N3DS;

  return main;
}

TEX LoadTEXxA4(BinReaderRef_e rd, Platform) {
  TEX main;
  TEXx9D header;
  rd.Read(header);
  using t = TEXx9D;

  main.ctx.width = header.tier1.Get<t::Width>();
  main.ctx.height = header.tier1.Get<t::Height>();
  main.ctx.depth = header.depth;
  main.ctx.numMipmaps = header.tier1.Get<t::NumMips>();

  TextureTypeV2 type = (TextureTypeV2)header.tier0.Get<t::TextureType>();

  if (type == TextureTypeV2::Cubemap) {
    main.ctx.numFaces = 6;
    rd.Read(main.harmonics);
    throw es::RuntimeError("Cubemaps are not supported.");
  }

  main.offsets.emplace_back(0);
  main.ctx.baseFormat = ConvertTEXFormat(TEXFormat3DS(header.format));

  size_t bufferSize = rd.GetSize() - rd.Tell();

  rd.ReadContainer(main.buffer, bufferSize);
  main.ctx.baseFormat.tile = TexelTile::N3DS;

  return main;
}

static const std::map<uint16, TEX (*)(BinReaderRef_e, Platform)> texLoaders{
    {0x66, LoadTEXx66<TEXx66>}, {0x70, LoadTEXx66<TEXx70>}, {0x87, LoadTEXx87},
    {0x97, LoadTEXx9D},         {0x98, LoadTEXx9D},         {0x99, LoadTEXx9D},
    {0x9A, LoadTEXx9D},         {0x9D, LoadTEXx9D},         {0x09, LoadTEXx09},
    {0xA0, LoadTEXxA0},         {0xA3, LoadTEXxA0},         {0xA6, LoadTEXxA6},
    {0xA5, LoadTEXxA6},         {0xA4, LoadTEXxA4},
};

void revil::LoadDetectTex(BinReaderRef_e rd, Platform platform,
                          TextureVersion loadFunc) {
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

  std::stringstream backup;
  if (header.id == SFHID) {
    backup = ProcessHFS(rd);
    rd = BinReaderRef_e(backup);
    rd.Push();
    rd.Read(header);
    rd.Pop();
  }

  if (header.id == XETID) {
    rd.SwapEndian(true);
  } else if (header.id != TEXID && header.id != TEXSID) {
    throw es::InvalidHeaderError(header.id);
  }

  if (header.versionV10 == 0x56) {
    loadFunc(0x56, rd, platform);
    return;
  } else if (rd.SwappedEndian()) {
    FByteswapper(header.versionV11);
  }

  if (platform == Platform::Auto) {
    platform = rd.SwappedEndian() ? Platform::PS3 : Platform::Win32;
  }

  if (loadFunc(header.versionV11, rd, platform)) {
    return;
  }

  if (rd.SwappedEndian()) {
    FByteswapper(header.versionV20);
  }

  if (loadFunc(header.versionV10, rd, platform)) {
    return;
  }

  throw es::InvalidVersionError();
}

void TEX::Load(BinReaderRef_e rd, Platform platform) {
  auto func = [this](uint32 version, BinReaderRef_e rd, Platform platform) {
    if (version == 0x56) {
      if (rd.SwappedEndian()) {
        throw es::RuntimeError("X360 texture format is unsupported.");
      }

      *this = LoadTEXx56(rd);
      return true;
    }

    auto found = texLoaders.find(version);
    if (!es::IsEnd(texLoaders, found)) {
      *this = found->second(rd, platform);
      return true;
    };

    return false;
  };

  LoadDetectTex(rd, platform, func);
}
