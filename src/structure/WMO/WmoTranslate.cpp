// Copyright (C) 2026 WraithEngine
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include "WmoTranslate.hpp"

#include <cmath>
#include <cstring>
#include <string>

// Down-convert a modern WMO into the chunk layout the target client (335) loader expects, working on the
// file bytes in memory. The target loader walks chunks POSITIONALLY (it assumes the classic order and
// advances by each chunk [size]; only a trailing MCVP is magic-checked), so any modern-only chunk left in
// the stream shifts every following read and corrupts the parse. The transform STRIPS modern chunks and
// normalizes modern fields. It is DATA-GATED (keyed off chunk presence / field values), so one path serves
// every modern source (the format only adds data; gating is by presence, not version).
namespace wraith::structure::wmo
{
    namespace
    {
        constexpr uint32_t FourCC(char a, char b, char c, char d)
        {
            return (static_cast<uint32_t>(static_cast<uint8_t>(a)) << 24) |
                   (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 16) |
                   (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 8) |
                    static_cast<uint32_t>(static_cast<uint8_t>(d));
        }

        constexpr uint32_t kMVER = FourCC('M', 'V', 'E', 'R');
        constexpr uint32_t kMOHD = FourCC('M', 'O', 'H', 'D'); // root header (marks a root .wmo)
        constexpr uint32_t kMOGP = FourCC('M', 'O', 'G', 'P'); // group header (marks a group .wmo)
        constexpr uint32_t kMOMT = FourCC('M', 'O', 'M', 'T'); // materials

        // Root chunks the target loader has no slot for; left in the stream they break the positional walk.
        // MCVP is kept (read as an optional trailing chunk once GFID after it is gone).
        constexpr uint32_t kGFID = FourCC('G', 'F', 'I', 'D'); // group FileDataIDs (modern)
        constexpr uint32_t kMOUV = FourCC('M', 'O', 'U', 'V'); // animated UV translations (modern)
        constexpr uint32_t kMODI = FourCC('M', 'O', 'D', 'I'); // doodad FileDataIDs (modern)
        constexpr uint32_t kMOSI = FourCC('M', 'O', 'S', 'I'); // skybox FileDataID (modern)
        constexpr uint32_t kMCVP = FourCC('M', 'C', 'V', 'P'); // convex volume planes (kept, trailing)

        // Group sub-chunks the target group loader knows. Anything else in a group is stripped; a second
        // MOCV or an extra MOTV beyond the first is also stripped (the target keeps one of each).
        constexpr uint32_t kMOPY = FourCC('M', 'O', 'P', 'Y');
        constexpr uint32_t kMOVI = FourCC('M', 'O', 'V', 'I');
        constexpr uint32_t kMOVT = FourCC('M', 'O', 'V', 'T');
        constexpr uint32_t kMONR = FourCC('M', 'O', 'N', 'R');
        constexpr uint32_t kMOTV = FourCC('M', 'O', 'T', 'V');
        constexpr uint32_t kMOBA = FourCC('M', 'O', 'B', 'A');
        constexpr uint32_t kMOLR = FourCC('M', 'O', 'L', 'R');
        constexpr uint32_t kMODR = FourCC('M', 'O', 'D', 'R');
        constexpr uint32_t kMOBN = FourCC('M', 'O', 'B', 'N');
        constexpr uint32_t kMOBR = FourCC('M', 'O', 'B', 'R');
        constexpr uint32_t kMOCV = FourCC('M', 'O', 'C', 'V');
        constexpr uint32_t kMLIQ = FourCC('M', 'L', 'I', 'Q');

        constexpr uint32_t kMOTX = FourCC('M', 'O', 'T', 'X'); // texture name blob (offsets referenced by MOMT)

        // Remaining target root chunks consumed by position.
        constexpr uint32_t kMOGN = FourCC('M', 'O', 'G', 'N');
        constexpr uint32_t kMOGI = FourCC('M', 'O', 'G', 'I');
        constexpr uint32_t kMOSB = FourCC('M', 'O', 'S', 'B');
        constexpr uint32_t kMOPV = FourCC('M', 'O', 'P', 'V');
        constexpr uint32_t kMOPT = FourCC('M', 'O', 'P', 'T');
        constexpr uint32_t kMOPR = FourCC('M', 'O', 'P', 'R');
        constexpr uint32_t kMOVV = FourCC('M', 'O', 'V', 'V');
        constexpr uint32_t kMOVB = FourCC('M', 'O', 'V', 'B');
        constexpr uint32_t kMOLT = FourCC('M', 'O', 'L', 'T');
        constexpr uint32_t kMODS = FourCC('M', 'O', 'D', 'S');
        constexpr uint32_t kMODN = FourCC('M', 'O', 'D', 'N');
        constexpr uint32_t kMODD = FourCC('M', 'O', 'D', 'D');
        constexpr uint32_t kMFOG = FourCC('M', 'F', 'O', 'G');

        // Material record: stride 0x40, shader at +0x04. The target valid shader range is 0..6; a modern
        // shader id (7..22) is an unchecked out-of-range index at draw time, collapsed to the nearest family.
        constexpr uint32_t kMomtStride       = 0x40;
        constexpr uint32_t kMomtShaderOffset = 0x04;
        // Texture name offsets into MOTX (target MOMT has two; +0x24 is a float, not a texture). A modern
        // FDID-textured material stores a FileDataID here instead of a MOTX offset.
        constexpr uint32_t kMomtTexOffsets[2] = { 0x0C, 0x18 };
        constexpr uint32_t kMomtTexCount      = 2;
        constexpr uint32_t kMaxNativeShader   = 6;

        // Placeholder name used when a texture FileDataID does not resolve, so the material's texture offset
        // still points at a valid NUL-terminated string in MOTX.
        constexpr const char kFallbackTexture[] = "createcrappygreentexture.blp";

        // The MOGP fixed header is 0x44 in every WMO version; sub-chunks follow it. Group flags at payload+0x08.
        constexpr uint32_t kMogpHeader335   = 0x44;
        constexpr uint32_t kMogpFlagsOffset = 0x08;

        // Batch counts inside the MOGP fixed header (u16 each). Zeroed when an empty MOBA is injected.
        constexpr uint32_t kMogpTransBatchCountOffset = 0x28;
        constexpr uint32_t kMogpIntBatchCountOffset   = 0x2A;
        constexpr uint32_t kMogpExtBatchCountOffset   = 0x2C;

        // Group flag bits the target loader gates optional sub-chunk consumption on (it reads the full u32).
        constexpr uint32_t kGrpFlagBSP  = 0x1;    // MOBN + MOBR
        constexpr uint32_t kGrpFlagMOCV = 0x4;    // vertex colors
        constexpr uint32_t kGrpFlagMOLR = 0x200;  // light refs
        constexpr uint32_t kGrpFlagMODR = 0x800;  // doodad refs
        constexpr uint32_t kGrpFlagMLIQ = 0x1000; // liquid

        // Every chunk-gating bit plus the modern-only high bits, cleared before recomputing the flags.
        constexpr uint32_t kGrpFlagClear =
            0x1u | 0x2u | 0x4u | 0x200u | 0x400u | 0x800u | 0x1000u | 0x20000u |
            0x1000000u | 0x2000000u | 0x4000000u | 0x8000000u | 0x10000000u | 0x20000000u | 0x40000000u | 0x80000000u;

        uint32_t Rd32(const uint8_t* p) { return *reinterpret_cast<const uint32_t*>(p); }
        void     Wr32(uint8_t* p, uint32_t v) { *reinterpret_cast<uint32_t*>(p) = v; }
        void     Wr16(uint8_t* p, int16_t v) { *reinterpret_cast<int16_t*>(p) = v; }
        float    Rdf(const uint8_t* p) { return *reinterpret_cast<const float*>(p); }

        bool IsKnownGroupChunk(uint32_t magic)
        {
            switch (magic)
            {
                case kMOPY: case kMOVI: case kMOVT: case kMONR: case kMOTV: case kMOBA:
                case kMOLR: case kMODR: case kMOBN: case kMOBR: case kMOCV: case kMLIQ:
                    return true;
                default:
                    return false;
            }
        }

        bool IsRootKeepChunk(uint32_t magic)
        {
            switch (magic)
            {
                case kMVER: case kMOHD: case kMOTX: case kMOMT: case kMOGN: case kMOGI:
                case kMOSB: case kMOPV: case kMOPT: case kMOPR: case kMOVV: case kMOVB:
                case kMOLT: case kMODS: case kMODN: case kMODD: case kMFOG: case kMCVP:
                    return true;
                default:
                    return false;
            }
        }

        // Modern WMO shader id (0..22) -> nearest target 0..6 family. Anything past the table degrades to 0.
        uint32_t RemapShader(uint32_t shader)
        {
            static const uint8_t kMap[23] = {
                0, 1, 2, 3, 4, 5, 6, // identity for the target range
                5,                   // 7  TwoLayerEnvMetal     -> EnvMetal
                6,                   // 8  TwoLayerTerrain      -> TwoLayerDiffuse
                0,                   // 9  DiffuseEmissive      -> Diffuse
                0, 0, 0, 0, 0,       // 10..14 unused           -> Diffuse
                6,                   // 15 TwoLayerDiffuseEmiss -> TwoLayerDiffuse
                0,                   // 16 Diffuse alias        -> Diffuse
                5,                   // 17 AdditiveMaskedEnvMet -> EnvMetal
                6,                   // 18 TwoLayerDiffuseMod2x -> TwoLayerDiffuse
                6,                   // 19 ..Mod2xNA            -> TwoLayerDiffuse
                6,                   // 20 ..DiffuseAlpha       -> TwoLayerDiffuse
                4,                   // 21 Lod impostor         -> Opaque
                0,                   // 22 Parallax             -> Diffuse
            };
            return (shader < 23) ? kMap[shader] : 0;
        }

        // Growable MOTX string blob. Appends NUL-terminated texture names and dedups identical paths so the
        // same texture referenced by several materials shares one entry. Returns the byte offset of the name.
        struct MotxBuilder
        {
            std::vector<uint8_t> data;

            void Seed(const uint8_t* blob, uint32_t len)
            {
                data.assign(blob, blob + len);
            }

            uint32_t Find(const char* path) const
            {
                const size_t pathLen = strlen(path);
                uint32_t i = 0;
                while (i < data.size())
                {
                    const char* s = reinterpret_cast<const char*>(data.data() + i);
                    const size_t sLen = strlen(s);
                    if (sLen == pathLen && memcmp(s, path, pathLen) == 0)
                        return i;
                    i += static_cast<uint32_t>(sLen) + 1;
                }
                return 0xFFFFFFFFu;
            }

            uint32_t Append(const char* path)
            {
                const uint32_t found = Find(path);
                if (found != 0xFFFFFFFFu)
                    return found;
                const uint32_t off = static_cast<uint32_t>(data.size());
                data.insert(data.end(), path, path + strlen(path) + 1);
                return off;
            }
        };

        // Append one chunk (8-byte header [magic][size] then payload) to out.
        void EmitChunk(std::vector<uint8_t>& out, uint32_t magic, const uint8_t* payload, uint32_t payloadLen)
        {
            uint8_t hdr[8];
            Wr32(hdr + 0, magic);
            Wr32(hdr + 4, payloadLen);
            out.insert(out.end(), hdr, hdr + 8);
            if (payloadLen)
                out.insert(out.end(), payload, payload + payloadLen);
        }

        // MOBA render-batch layout. Modern builds relocate the material id: when the flag byte at +0x16 has
        // bit 0x2 set, the real material id sits at +0x0A and the leading bytes overlap what the target reads
        // as the i16 bounding box. The target reads the material id at +0x17, so an unfixed modern batch
        // yields a garbage out-of-range material index that faults at render. The fix moves the id to +0x17,
        // clears the flag, and rebuilds the i16 bounding box from the group's vertices.
        constexpr uint32_t kMobaEntryStride  = 0x18;
        constexpr uint32_t kMobaBBoxOffset   = 0x00; // 6 i16: min x,y,z then max x,y,z
        constexpr uint32_t kMobaMinIndexOff  = 0x12; // u16 first vertex index in this batch
        constexpr uint32_t kMobaMaxIndexOff  = 0x14; // u16 last vertex index in this batch
        constexpr uint32_t kMobaModernMatOff = 0x0A; // u8 modern material id
        constexpr uint32_t kMobaFlagOffset   = 0x16; // u8 relocation flag (bit 0x2)
        constexpr uint32_t kMobaMatIdOffset  = 0x17; // u8 target material id
        constexpr uint8_t  kMobaRelocFlag    = 0x02;
        constexpr uint32_t kMovtStride       = 0x0C; // C3Vector per vertex

        // Recompute the i16 bounding box of one MOBA entry over its vertex index range [start..end], reading
        // C3Vector vertices from the MOVT payload. Indices out of range are skipped so reads stay in bounds.
        void FixMobaBox(uint8_t* entry, const uint8_t* movtData, uint32_t movtVerts, uint16_t start, uint16_t end)
        {
            float mn[3] = { 3.4e38f, 3.4e38f, 3.4e38f };
            float mx[3] = { -3.4e38f, -3.4e38f, -3.4e38f };
            bool any = false;

            for (uint32_t i = start; i <= end; ++i)
            {
                if (i >= movtVerts)
                    break;
                const uint8_t* v = movtData + kMovtStride * i;
                for (int k = 0; k < 3; ++k)
                {
                    const float c = Rdf(v + 4 * k);
                    if (c < mn[k]) mn[k] = c;
                    if (c > mx[k]) mx[k] = c;
                }
                any = true;
            }

            if (!any)
                return; // no valid vertices: leave the box bytes as-is

            for (int k = 0; k < 3; ++k)
            {
                Wr16(entry + kMobaBBoxOffset + 2 * k,       static_cast<int16_t>(std::floor(mn[k])));
                Wr16(entry + kMobaBBoxOffset + 2 * (k + 3), static_cast<int16_t>(std::ceil(mx[k])));
            }
        }

        // Walk a MOBA payload (entries of 0x18) and apply the material-id relocation + bbox rebuild to each
        // flagged entry. Returns the number of entries relocated.
        uint32_t FixMobaChunk(uint8_t* mobaData, uint32_t mobaLen, const uint8_t* movtData, uint32_t movtLen)
        {
            const uint32_t n         = mobaLen / kMobaEntryStride;
            const uint32_t movtVerts = movtLen / kMovtStride;
            uint32_t relocated = 0;

            for (uint32_t i = 0; i < n; ++i)
            {
                uint8_t* e = mobaData + i * kMobaEntryStride;
                if ((e[kMobaFlagOffset] & kMobaRelocFlag) == 0)
                    continue;

                e[kMobaMatIdOffset] = e[kMobaModernMatOff];
                e[kMobaFlagOffset]  = 0;

                const uint16_t start = *reinterpret_cast<const uint16_t*>(e + kMobaMinIndexOff);
                const uint16_t end   = *reinterpret_cast<const uint16_t*>(e + kMobaMaxIndexOff);
                FixMobaBox(e, movtData, movtVerts, start, end);
                ++relocated;
            }
            return relocated;
        }
    }

    bool TranslateWmoRoot(std::span<const uint8_t> in,
                          const ResolveCtx& rc, std::vector<uint8_t>& out)
    {
        const uint8_t* buf = in.data();
        const uint32_t size = static_cast<uint32_t>(in.size());

        // Pass 1: locate MOTX and MOMT, and detect modern markers. MOTX precedes MOMT in file order, but the
        // blob can still grow because of MOMT, so the rebuild emits MOTX with its FINAL (grown) bytes.
        uint32_t motxPos = 0xFFFFFFFFu, motxLen = 0;
        uint32_t momtPos = 0xFFFFFFFFu, momtLen = 0;
        bool modern = false;
        bool strippedUnknown = false; // source carries any chunk outside the keep-list

        uint32_t pos = 0;
        while (pos + 8 <= size)
        {
            const uint32_t magic = Rd32(buf + pos);
            const uint32_t clen  = 8 + Rd32(buf + pos + 4);
            if (clen < 8 || pos + clen > size)
                break; // malformed / truncated: do not touch it

            if (magic == kGFID || magic == kMOUV || magic == kMODI || magic == kMOSI)
                modern = true;
            else if (magic == kMOTX) { motxPos = pos; motxLen = clen - 8; }
            else if (magic == kMOMT) { momtPos = pos; momtLen = clen - 8; }

            if (!IsRootKeepChunk(magic))
                strippedUnknown = true;

            pos += clen;
        }

        // Build the final MOTX blob and resolve any FileDataID texture references in MOMT against it.
        MotxBuilder motx;
        if (motxPos != 0xFFFFFFFFu)
            motx.Seed(buf + motxPos + 8, motxLen);

        // A copy of the material data we can rewrite (offsets and shader) before emitting.
        std::vector<uint8_t> mats;
        uint32_t matCount = 0, fdidResolved = 0, fdidFallback = 0;
        if (momtPos != 0xFFFFFFFFu)
        {
            mats.assign(buf + momtPos + 8, buf + momtPos + 8 + momtLen);
            matCount = momtLen / kMomtStride;

            // The first material's resolved tex1 path doubles as a per-WMO fallback for unresolved FDIDs.
            uint32_t fallbackOff = 0xFFFFFFFFu;

            for (uint32_t i = 0; i < matCount; ++i)
            {
                uint8_t* m = mats.data() + i * kMomtStride;
                const uint32_t shader = Rd32(m + kMomtShaderOffset);
                if (shader > kMaxNativeShader)
                    Wr32(m + kMomtShaderOffset, RemapShader(shader));

                for (uint32_t t = 0; t < kMomtTexCount; ++t)
                {
                    const uint32_t off = Rd32(m + kMomtTexOffsets[t]);
                    if (off < motx.data.size())
                        continue; // genuine in-blob offset, leave it

                    // Out of MOTX bounds -> treat as a FileDataID and resolve it to a real path.
                    std::string path;
                    const bool resolved = rc.resolve && rc.resolve(rc.user, off, path) && !path.empty();
                    uint32_t newOff;
                    if (resolved)
                    {
                        newOff = motx.Append(path.c_str());
                        ++fdidResolved;
                    }
                    else
                    {
                        // No resolution: reuse the per-WMO fallback (first appended path) or a placeholder.
                        if (fallbackOff == 0xFFFFFFFFu)
                            fallbackOff = motx.Append(kFallbackTexture);
                        newOff = fallbackOff;
                        ++fdidFallback;
                    }
                    Wr32(m + kMomtTexOffsets[t], newOff);
                }

                // Remember the first material's tex1 as the fallback for later materials.
                if (fallbackOff == 0xFFFFFFFFu)
                {
                    const uint32_t tex1 = Rd32(m + kMomtTexOffsets[0]);
                    if (tex1 < motx.data.size())
                        fallbackOff = tex1;
                }
            }
        }

        // Guarantee a MOTX always exists so the loader's MOTX base pointer is never wild. An empty blob still
        // needs one byte (a single NUL) so any 0 offset dereferences to an empty string, not past the buffer.
        if (motx.data.empty())
            motx.data.push_back(0);

        // Leave the bytes as-is for a pure target-shaped root: nothing modern, no FDID textures, no MOTX had
        // to be created, and no chunk outside the keep-list to strip. Anything else needs a rebuild.
        const bool createdMotx = (motxPos == 0xFFFFFFFFu);
        if (!strippedUnknown && !modern && fdidResolved == 0 && fdidFallback == 0 && !createdMotx)
            return false;

        // Pass 2: rebuild the root as a WHITELIST. Emit ONLY the keep-list chunks, in the exact canonical
        // order the positional parser expects, pulling each from the source by magic (first occurrence) and
        // synthesizing an empty header for any mandatory chunk the source lacks. MOTX and MOMT keep their
        // special payloads (grown blob / rewritten materials). MCVP is emitted last only if present. Every
        // chunk outside the keep-list (GFID/MOUV/MODI/MOSI and any unknown modern chunk) is dropped.
        struct SrcChunk { uint32_t pos; uint32_t len; }; // pos/len of payload (past the 8-byte header)
        auto findSrc = [&](uint32_t wantMagic, SrcChunk& dst) -> bool
        {
            uint32_t p = 0;
            while (p + 8 <= size)
            {
                const uint32_t magic = Rd32(buf + p);
                const uint32_t clen  = 8 + Rd32(buf + p + 4);
                if (clen < 8 || p + clen > size)
                    break;
                if (magic == wantMagic) { dst = { p + 8, clen - 8 }; return true; }
                p += clen;
            }
            return false;
        };

        out.clear();
        out.reserve(size + motx.data.size() + 16);

        // The 16 mandatory-by-position chunks in canonical order. MOTX and MOMT are handled specially below.
        static const uint32_t kCanonical[] = {
            kMVER, kMOHD, kMOTX, kMOMT, kMOGN, kMOGI, kMOSB, kMOPV,
            kMOPT, kMOPR, kMOVV, kMOVB, kMOLT, kMODS, kMODN, kMODD, kMFOG,
        };

        for (uint32_t magic : kCanonical)
        {
            if (magic == kMOTX)
            {
                EmitChunk(out, kMOTX, motx.data.data(), static_cast<uint32_t>(motx.data.size()));
                continue;
            }
            if (magic == kMOMT)
            {
                EmitChunk(out, kMOMT, mats.data(), static_cast<uint32_t>(mats.size()));
                continue;
            }
            SrcChunk c{0, 0};
            if (findSrc(magic, c))
                EmitChunk(out, magic, buf + c.pos, c.len);
            else
                EmitChunk(out, magic, nullptr, 0); // synthesize empty to keep the positional walk aligned
        }

        // MCVP: the only optional/trailing chunk and the only one the parser magic-checks. Emit it last.
        {
            SrcChunk c{0, 0};
            if (findSrc(kMCVP, c))
                EmitChunk(out, kMCVP, buf + c.pos, c.len);
        }

        return true;
    }

    bool TranslateWmoGroup(std::span<const uint8_t> in, std::vector<uint8_t>& out)
    {
        const uint8_t* buf = in.data();
        const uint32_t size = static_cast<uint32_t>(in.size());

        if (!buf || size < 12 || Rd32(buf) != kMVER)
            return false;

        const uint32_t mverLen   = 8 + Rd32(buf + 4);
        const uint32_t afterMver = mverLen;
        if (afterMver + 8 > size || Rd32(buf + afterMver) != kMOGP)
            return false;

        const uint32_t mogpHdr  = afterMver;
        const uint32_t mogpData = mogpHdr + 8;
        const uint32_t mogpSize = Rd32(buf + mogpHdr + 4);
        const uint32_t mogpEnd  = mogpData + mogpSize;
        if (mogpEnd > size || mogpData + kMogpHeader335 + 8 > mogpEnd)
            return false;

        // Sub-chunks begin immediately after the fixed 0x44 header.
        const uint32_t subStart = mogpData + kMogpHeader335;
        if (!IsKnownGroupChunk(Rd32(buf + subStart)))
            return false; // sub-chunks not at +0x44: leave as-is

        // Locate every sub-chunk once. Record pos+len for the mandatory six (first occurrence) and remember
        // the optional chunks (in source order) for the kept set. MOTV beyond the mandatory one and a second
        // MOCV are dropped; everything unknown is dropped.
        struct ChunkRef { uint32_t pos; uint32_t clen; };
        ChunkRef mopy{0,0}, movi{0,0}, movt{0,0}, monr{0,0}, motv{0,0}, moba{0,0};

        std::vector<ChunkRef> kept;
        bool hasMOBN = false, hasMOCV = false, hasMOLR = false, hasMODR = false, hasMLIQ = false;
        bool seenMOTV = false, seenMOCV = false;

        uint32_t sub = subStart;
        while (sub + 8 <= mogpEnd)
        {
            const uint32_t magic = Rd32(buf + sub);
            const uint32_t clen  = 8 + Rd32(buf + sub + 4);
            if (clen < 8 || sub + clen > mogpEnd) break;

            switch (magic)
            {
                case kMOPY: if (!mopy.clen) mopy = {sub, clen}; break;
                case kMOVI: if (!movi.clen) movi = {sub, clen}; break;
                case kMOVT: if (!movt.clen) movt = {sub, clen}; break;
                case kMONR: if (!monr.clen) monr = {sub, clen}; break;
                case kMOBA: if (!moba.clen) moba = {sub, clen}; break;
                case kMOTV:
                    if (!seenMOTV) { motv = {sub, clen}; seenMOTV = true; } // first MOTV is the mandatory one
                    break;
                case kMOCV:
                    if (!seenMOCV) { seenMOCV = true; hasMOCV = true; kept.push_back({sub, clen}); }
                    break;
                case kMOLR: hasMOLR = true; kept.push_back({sub, clen}); break;
                case kMODR: hasMODR = true; kept.push_back({sub, clen}); break;
                case kMOBN: hasMOBN = true; kept.push_back({sub, clen}); break;
                case kMOBR: kept.push_back({sub, clen}); break;
                case kMLIQ: hasMLIQ = true; kept.push_back({sub, clen}); break;
                default: break; // unknown / stripped
            }
            sub += clen;
        }

        // Build the rebuilt sub-chunk region: the six mandatory chunks (source bytes if present, else an empty
        // size-0 chunk), then the kept optional chunks in source order.
        std::vector<uint8_t> subRegion;
        subRegion.reserve(mogpSize);

        const bool injMOPY = mopy.clen == 0, injMOVI = movi.clen == 0, injMOVT = movt.clen == 0;
        const bool injMONR = monr.clen == 0, injMOTV = motv.clen == 0, injMOBA = moba.clen == 0;

        // Track where MOVT and MOBA land inside subRegion so the MOBA material-id fix can locate both payloads
        // after the region is assembled.
        uint32_t movtPayloadOff = 0, movtPayloadLen = 0;
        uint32_t mobaPayloadOff = 0, mobaPayloadLen = 0;

        auto emitMandatory = [&](const ChunkRef& c, uint32_t magic, uint32_t* outPayloadOff, uint32_t* outPayloadLen)
        {
            const uint32_t payloadOff = static_cast<uint32_t>(subRegion.size()) + 8;
            const uint32_t payloadLen = c.clen ? c.clen - 8 : 0;
            if (c.clen)
                subRegion.insert(subRegion.end(), buf + c.pos, buf + c.pos + c.clen);
            else
                EmitChunk(subRegion, magic, nullptr, 0); // empty 8-byte header
            if (outPayloadOff) *outPayloadOff = payloadOff;
            if (outPayloadLen) *outPayloadLen = payloadLen;
        };

        emitMandatory(mopy, kMOPY, nullptr, nullptr);
        emitMandatory(movi, kMOVI, nullptr, nullptr);
        emitMandatory(movt, kMOVT, &movtPayloadOff, &movtPayloadLen);
        emitMandatory(monr, kMONR, nullptr, nullptr);
        emitMandatory(motv, kMOTV, nullptr, nullptr);
        emitMandatory(moba, kMOBA, &mobaPayloadOff, &mobaPayloadLen);

        for (const ChunkRef& c : kept)
            subRegion.insert(subRegion.end(), buf + c.pos, buf + c.pos + c.clen);

        // MOBA material-id relocation fix. Operates on the freshly emitted MOBA payload using the freshly
        // emitted MOVT payload for the bounding-box rebuild. An injected empty MOBA has no entries.
        if (mobaPayloadLen != 0)
            FixMobaChunk(subRegion.data() + mobaPayloadOff, mobaPayloadLen,
                         subRegion.data() + movtPayloadOff, movtPayloadLen);

        // Assemble the output: MVER verbatim, MOGP wrapper + fixed 0x44 header verbatim (with patched flags /
        // batch counts), then the rebuilt sub-chunk region.
        out.clear();
        out.reserve(mverLen + 8 + kMogpHeader335 + subRegion.size());

        // MVER (whole chunk, header + payload).
        out.insert(out.end(), buf, buf + mverLen);

        // MOGP 8-byte header + 0x44 fixed header, copied verbatim then patched.
        const size_t mogpHeaderStart = out.size();
        out.insert(out.end(), buf + mogpHdr, buf + mogpData + kMogpHeader335);

        uint8_t* outMogpData = out.data() + mogpHeaderStart + 8;

        // Recompute group flags: clear gating bits, re-set only for the chunks actually emitted.
        const uint32_t oldFlags = Rd32(outMogpData + kMogpFlagsOffset);
        uint32_t newFlags = oldFlags & ~kGrpFlagClear;
        if (hasMOBN) newFlags |= kGrpFlagBSP;
        if (hasMOCV) newFlags |= kGrpFlagMOCV;
        if (hasMOLR) newFlags |= kGrpFlagMOLR;
        if (hasMODR) newFlags |= kGrpFlagMODR;
        if (hasMLIQ) newFlags |= kGrpFlagMLIQ;
        Wr32(outMogpData + kMogpFlagsOffset, newFlags);

        // An injected empty MOBA means zero render batches; zero the header batch counts to match.
        if (injMOBA)
        {
            *reinterpret_cast<uint16_t*>(outMogpData + kMogpTransBatchCountOffset) = 0;
            *reinterpret_cast<uint16_t*>(outMogpData + kMogpIntBatchCountOffset)   = 0;
            *reinterpret_cast<uint16_t*>(outMogpData + kMogpExtBatchCountOffset)   = 0;
        }

        // Patch the MOGP chunk size = fixed header + rebuilt sub-chunk region.
        const uint32_t newMogpSize = kMogpHeader335 + static_cast<uint32_t>(subRegion.size());
        Wr32(out.data() + mogpHeaderStart + 4, newMogpSize);

        // Append the rebuilt sub-chunk region.
        out.insert(out.end(), subRegion.begin(), subRegion.end());

        // No-op detection: if nothing was injected, no flag change, and the rebuilt region matches the
        // original sub-chunk span, the file is already target-shaped and need not be replaced.
        const bool injected = injMOPY || injMOVI || injMOVT || injMONR || injMOTV || injMOBA;
        const uint32_t origSubLen = mogpEnd - subStart;
        if (!injected && newFlags == oldFlags && subRegion.size() == origSubLen &&
            memcmp(subRegion.data(), buf + subStart, origSubLen) == 0)
            return false;

        return true;
    }
}
