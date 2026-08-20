/**
 * @file Thread-safe daily log writer implementation.
 * @author Codex
 * @created 2026-08-20
 * @depends utils/Logger.h
 */
#include "utils/Logger.h"

#include <chrono>
#include <format>
#include <fstream>
#include <stdexcept>

namespace lrender {

Logger& Logger::Instance() {
    static Logger logger;
    return logger;
}

void Logger::Initialize(const std::filesystem::path& rootDirectory) {
    if (rootDirectory.empty()) {
        throw std::invalid_argument("Log root directory must not be empty");
    }
    const auto logDirectory = rootDirectory / "logs";
    std::filesystem::create_directories(logDirectory);
    const auto now = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    logFile_ = logDirectory / std::format("{:%F}.log", now);
}

void Logger::Info(std::string_view module, std::string_view message) {
    Write("INFO", module, message);
}

void Logger::Error(std::string_view module, std::string_view message) {
    Write("ERROR", module, message);
}

void Logger::Write(std::string_view level, std::string_view module, std::string_view message) {
    std::scoped_lock lock(mutex_);
    if (logFile_.empty()) {
        return;
    }
    std::ofstream stream(logFile_, std::ios::app);
    if (!stream) {
        throw std::runtime_error("Unable to open application log file");
    }
    const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    stream << std::format("[{:%F %T}] [{}] [{}] {}\n", now, level, module, message);
}

} // namespace lrender
