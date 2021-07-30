#pragma once
#include <DataCache/IDataCache.h>

namespace mesh::DataCache
{
    class DataCache final : public IDataCache
    {
    public:
        virtual std::string GetVersion(int compatibilityFlag) override;
    };
}