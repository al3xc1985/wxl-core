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

#include "Db2Store.hpp"

#include "Logger.hpp"

#include "DB2File.hpp"

#include <fstream>
#include <iterator>

using wraith::features::db2::DB2File;

namespace
{
    bool LoadDisk(DB2File& table, const std::string& path)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f) { wraith::core::log::Printf("db2: cannot open %s", path.c_str()); return false; }
        std::vector<uint8_t> buf((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        return table.LoadBytes(buf.data(), static_cast<uint32_t>(buf.size()), path.c_str());
    }

    // Resolve a db2 by name under the client root.
    std::string FindDb2(const std::string& root, const std::string& name)
    {
        std::string cands[] = {
            root + "\\Data\\Patch-4.MPQ\\DBFilesClient\\" + name,
            root + "\\" + name,
        };
        for (const std::string& p : cands)
        {
            std::ifstream f(p, std::ios::binary);
            if (f) return p;
        }
        return cands[0];
    }
}

namespace wraith::host::db2
{
    bool Db2Store::Load(std::string_view dataDir)
    {
        std::string root(dataDir);
        if (!root.empty() && (root.back() == '\\' || root.back() == '/')) root.pop_back();

        bool ok = true;
        ok &= LoadDisk(m_tex,     FindDb2(root, "TextureFilePath.db2"));
        ok &= LoadDisk(m_model,   FindDb2(root, "ModelFilePath.db2"));
        ok &= LoadDisk(m_texData, FindDb2(root, "texturefiledata.db2"));

        for (uint32_t i = 0; i < m_texData.RowCount(); ++i)
        {
            const TexDataRow* r = m_texData.At(i);
            if (r) m_mridIndex[r->materialResId].push_back({ r->textureType, r->fileDataId });
        }

        wraith::core::log::Printf("db2: loaded texpath=%u model=%u texdata=%u (MRID index=%zu)",
            m_tex.RowCount(), m_model.RowCount(), m_texData.RowCount(), m_mridIndex.size());
        return ok;
    }

    bool Db2Store::ResolveFile(uint32_t fileDataId, std::string& outPath) const
    {
        const PathRow* r = m_tex.Find(static_cast<int32_t>(fileDataId));
        const char* path = r ? m_tex.Str(static_cast<uint32_t>(r->path)) : nullptr;
        if (!path)
        {
            r = m_model.Find(static_cast<int32_t>(fileDataId));
            path = r ? m_model.Str(static_cast<uint32_t>(r->path)) : nullptr;
        }
        if (!path) return false;
        outPath = path;
        return true;
    }

    uint32_t Db2Store::MridToFdid(uint32_t mrid, uint32_t want) const
    {
        auto it = m_mridIndex.find(mrid);
        if (it == m_mridIndex.end()) return 0;
        for (uint32_t target : { want, 2u })
            for (const auto& c : it->second)
                if (c.first == target) return c.second;
        return it->second[0].second;
    }

    bool Db2Store::ResolveMaterial(uint32_t mrid, uint32_t typeHint, std::string& outPath) const
    {
        uint32_t fdid = MridToFdid(mrid, typeHint);
        if (!fdid) return false;
        const PathRow* r = m_tex.Find(static_cast<int32_t>(fdid));
        const char* path = r ? m_tex.Str(static_cast<uint32_t>(r->path)) : nullptr;
        if (!path) return false;
        outPath = path;
        return true;
    }
}
