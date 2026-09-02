/**
 * @file Win32 executable and content-root path discovery.
 */
#include "platform/RuntimePaths.h"

#include <stdexcept>
#include <string>
#include <windows.h>

namespace lrender
{

std::filesystem::path RuntimePaths::ExecutablePath()
{
    std::wstring buffer(512, L'\0');
    for (;;)
    {
        const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length == 0)
        {
            throw std::runtime_error("Failed to locate LRenderDemo executable");
        }
        if (length < buffer.size() - 1)
        {
            buffer.resize(length);
            return std::filesystem::path(buffer);
        }
        buffer.resize(buffer.size() * 2);
    }
}

std::filesystem::path RuntimePaths::ExecutableDirectory()
{
    return ExecutablePath().parent_path();
}

} // namespace lrender
