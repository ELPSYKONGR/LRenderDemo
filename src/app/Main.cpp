/**
 * @file Win32 GUI entry point with fatal-error logging and user feedback.
 * @author Codex
 * @created 2026-08-20
 * @depends app/Application.h, utils/Logger.h
 */
#include "app/Application.h"
#include "utils/Logger.h"

#include <exception>
#include <filesystem>
#include <string>
#include <windows.h>

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    try
    {
        lrender::Logger::Instance().Initialize(std::filesystem::current_path());
        lrender::Logger::Instance().Info("main", "LRenderDemo starting");
        lrender::Application application(instance);
        return application.Run();
    }
    catch (const std::exception& error)
    {
        try
        {
            lrender::Logger::Instance().Error("main", error.what());
        }
        catch (...)
        {
            // The fatal dialog remains available even when the log destination is unavailable.
        }
        const std::string message = std::string("LRenderDemo failed:\n") + error.what();
        MessageBoxA(nullptr, message.c_str(), "LRenderDemo Error", MB_OK | MB_ICONERROR);
        return 1;
    }
}
