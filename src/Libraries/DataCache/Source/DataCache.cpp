#include <DataCache/DataCache.h>

using namespace mesh::DataCache;

std::string DataCache::GetVersion(int compatibilityFlag)
{
    if (compatibilityFlag > 5)
        return "1.0.1-extreme";

    return "0.0.1-alpha";
}