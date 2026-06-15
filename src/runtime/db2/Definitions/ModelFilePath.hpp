// ModelFilePath data table: FileDataID -> model-asset path.
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

#pragma once

#include "DB2File.hpp"
#include "DB2Mgr.hpp"

// ModelFilePath: FileDataID -> model-asset path. Covers model/skin/anim/physics/skeleton siblings.
// Inline id, one string field.
namespace wraith::features::db2::defs
{
    struct ModelFilePathRow
    {
        uint32_t id;     // FileDataID
        int32_t  path;   // string offset
    };

    class ModelFilePath : public DB2Table<ModelFilePathRow>
    {
    public:
        static ModelFilePath& Get()
        {
            static ModelFilePath instance;
            if (!instance.IsLoaded())
            {
                instance.Load("ModelFilePath.db2");
                Track(&instance, "ModelFilePath.db2");
            }
            return instance;
        }

        // Resolve a FileDataID to its path, or nullptr if absent.
        const char* Path(uint32_t fileDataId)
        {
            const auto* r = Find(static_cast<int32_t>(fileDataId));
            return r ? Str(static_cast<uint32_t>(r->path)) : nullptr;
        }
    };
}
