// Client-side data-table load path: reads a .db2 from the mounted archives through the engine
// file-I/O primitives, then decodes it through the shared LoadBytes path.
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

#include "DB2File.hpp"

#include "Io.hpp"
#include "Logger.hpp"

#include <cstdint>
#include <vector>

// Client-only: DB2File::Load reads the table from the mounted archives via the engine file-I/O
// primitives. The host builds tables with LoadBytes after a plain disk read, so this TU is excluded
// from the host build.
namespace io = wraith::offsets::engine::io;

namespace
{
    // Read the whole "DBFilesClient\<name>" file from the mounted archives into dst. Returns true on
    // success; dst is left empty on failure.
    bool ReadWholeFile(const char* dbFilesClientName, std::vector<uint8_t>& dst)
    {
        dst.clear();

        char path[260];
        const char* prefix = "DBFilesClient\\";
        size_t p = 0;
        for (const char* s = prefix; *s && p < sizeof(path) - 1; ++s) path[p++] = *s;
        for (const char* s = dbFilesClientName; *s && p < sizeof(path) - 1; ++s) path[p++] = *s;
        path[p] = '\0';

        auto open  = reinterpret_cast<io::Storage_FileOpenFn>(io::kFileOpen);
        auto size  = reinterpret_cast<io::Storage_FileSizeFn>(io::kFileSize);
        auto read  = reinterpret_cast<io::Storage_FileReadFn>(io::kFileRead);
        auto close = reinterpret_cast<io::Storage_FileCloseFn>(io::kFileClose);

        void* handle = nullptr;
        if (open(nullptr, path, io::kOpenWholeFile, &handle) == 0 || !handle)
            return false;

        uint32_t sizeHigh = 0;
        uint32_t len = size(handle, &sizeHigh);
        if (len == 0)
        {
            close(handle);
            return false;
        }

        dst.resize(len);
        bool ok = read(handle, dst.data(), len, nullptr, nullptr, 0) != 0;
        close(handle);
        if (!ok) { dst.clear(); return false; }
        return true;
    }
}

namespace wraith::features::db2
{
    bool DB2File::Load(const char* fileName)
    {
        if (m_loaded) return true;

        std::vector<uint8_t> buf;
        if (!ReadWholeFile(fileName, buf))
        {
            WLOG_WARN("DB2: cannot read %s", fileName);
            return false;
        }
        return LoadBytes(buf.data(), static_cast<uint32_t>(buf.size()), fileName);
    }
}
