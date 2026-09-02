/**
 * @file Runtime path discovery independent of the process working directory.
 */
#pragma once

#include <filesystem>

namespace lrender {

class RuntimePaths final {
public:
    [[nodiscard]] static std::filesystem::path ExecutablePath();
    [[nodiscard]] static std::filesystem::path ExecutableDirectory();
};

} // namespace lrender
