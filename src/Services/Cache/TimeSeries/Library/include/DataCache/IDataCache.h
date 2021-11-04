#pragma once
#include <string>

namespace mesh::DataCache
{
    class IDataCache
    {
    public:
        // compatibilityFlag param has no any meaning and is for unit test usage only (to present some ideas in unit tests)
        virtual std::string GetVersion(int compatibilityFlag) = 0;
    };
}