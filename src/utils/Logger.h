/**
 * @file Small file logger used by startup and fatal error paths.
 * @author Codex
 * @created 2026-08-20
 */
#pragma once

#include <filesystem>
#include <mutex>
#include <string_view>

namespace lrender {

class Logger final {
public:
    static Logger& Instance();
    void Initialize(const std::filesystem::path& rootDirectory);
    void Info(std::string_view module, std::string_view message);
    void Error(std::string_view module, std::string_view message);

private:
    void Write(std::string_view level, std::string_view module, std::string_view message);

    std::filesystem::path logFile_;
    std::mutex mutex_;
};

} // namespace lrender
