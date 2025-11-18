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

#pragma once
#include "revil/platform.hpp"
#include "revil/settings.hpp"
#include "spike/io/bincore_fwd.hpp"
#include "spike/type/bitfield.hpp"
#include "spike/type/vectors.hpp"
#include <functional>

namespace revil {

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
// 9 14 55
enum class TEXFormatV2 : uint8 {
  RGBA16F = 0x2,
  R8 = 0x7,
  RGBA8_LP4 = 0x9,
  RGB10A2 = 0xE, // vertex field (VTF)
  BC1 = 0x13,
  BC1A = 0x14,
  BC2 = 0x15,
  BC2_PA = 0x16, // alpha premult?
  BC3 = 0x17,
  BC3_PA = 0x18,                        // alpha premult?
  COMPRESSED_GRAYSCALE = 0x19,          // BC4, BC1
  COMPRESSED_NORMAL_MAP = 0x1e,         // BC1
  COMPRESSED_DERIVED_NORMAL_MAP = 0x1f, // BC3, BC5, BC5S
  BC3_LM = 0x20,                        // alpha premult?
  BC3_CM = 0x23,                        // ddon defaultcube only, alpha premult?
  BC3_PM = 0x25,                        // rgb NM, alpha HM
  RGBA8 = 0x27,
  RGBA8_PA = 0x28,   // alpha premult?
  BC3_YUV = 0x2A,    // red Y, green A, blue U, alpha V
  BC3_YUV_PA = 0x2B, // alpha premult?
  BC3_EFF = 0x2F,    // effect related (DD)
  BC7 = 0x30,
  BC7_PA = 0x37, // premult aplha or srgb
};

enum class TEXFormat3DS : uint8 {
  RGBA4 = 1,
  RGBA8 = 3,
  R5G6B5 = 4,
  R8 = 5,
  RG4 = 6,
  IA8 = 7,
  ETC1 = 0xB,
  ETC1A4 = 0xC,
  A4 = 0xE,
  L4 = 0xF,
  AL4 = 0x10,
  RGB8 = 0x11,
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

  uint32 id;
  Tier0 tier0;
  Tier1 tier1;
  uint8 numFaces;
  TEXFormatV2 format;
  uint16 depth;

  void SwapEndian(bool) {
    FByteswapper(id);
    FByteswapper(tier0.value);
    FByteswapper(tier1.value);
    FByteswapper(reinterpret_cast<uint32 &>(numFaces));
  }
};

enum class TEXFormatAndr : uint8 {
  RGBA8 = 0x1,
  R5G6B5 = 0x6,
  RGBA4 = 0x7,
  ETC1 = 0xA,
  BC3 = 0xC,
  PVRTC4 = 0xD,
};

enum class TEXTypeAndr : uint32 {
  Common,       // GSM, ID, CMM, fallback?
  BaseMap,      // BM
  MaskMap,      // MM
  AlphaMap = 5, // AM
  Cubemap,      // CM
  LP4,          // event slides?
  Nuki,         // NUKI, extract in jp
};

struct TEXx09 {
  uint32 id;
  uint16 version;
  TEXFormatAndr format;
  uint8 unk00;
  uint32 unk01 : 4;
  TEXTypeAndr type : 28;

  uint32 width : 13;
  uint32 height : 13;
  uint32 numMips : 4;
  uint32 unk0 : 1;
  uint32 unk1 : 1;

  uint32 dataOffset;
  uint32 pvrVariantOffset;
  uint32 unkVariantOffset;

  uint32 dataSize;
  uint32 pvrSize;
  uint32 unkSize;
};

using TextureVersion =
    std::function<bool(uint32 version, BinReaderRef_e rd, Platform platform)>;
void RE_EXTERN LoadDetectTex(BinReaderRef_e rd, Platform platform,
                             TextureVersion loadFunc);

} // namespace revil
