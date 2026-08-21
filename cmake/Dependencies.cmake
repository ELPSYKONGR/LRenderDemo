# Defines third-party targets from pinned Git submodules.
set(LRENDER_EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

foreach(dependency DirectXTK imgui ImGuizmo)
    if(NOT EXISTS "${LRENDER_EXTERNAL_DIR}/${dependency}")
        message(FATAL_ERROR
            "Missing external/${dependency}. Run scripts/bootstrap-and-verify.ps1 "
            "or clone with --recurse-submodules.")
    endif()
endforeach()

set(BUILD_XAUDIO_WIN7 OFF CACHE BOOL "" FORCE)
set(BUILD_XAUDIO_REDIST OFF CACHE BOOL "" FORCE)
set(BUILD_TOOLS OFF CACHE BOOL "Do not build DirectXTK command-line tools" FORCE)
add_subdirectory("${LRENDER_EXTERNAL_DIR}/DirectXTK" EXCLUDE_FROM_ALL)

add_library(imgui STATIC
    "${LRENDER_EXTERNAL_DIR}/imgui/imgui.cpp"
    "${LRENDER_EXTERNAL_DIR}/imgui/imgui_demo.cpp"
    "${LRENDER_EXTERNAL_DIR}/imgui/imgui_draw.cpp"
    "${LRENDER_EXTERNAL_DIR}/imgui/imgui_tables.cpp"
    "${LRENDER_EXTERNAL_DIR}/imgui/imgui_widgets.cpp"
    "${LRENDER_EXTERNAL_DIR}/imgui/backends/imgui_impl_dx11.cpp"
    "${LRENDER_EXTERNAL_DIR}/imgui/backends/imgui_impl_win32.cpp")
target_include_directories(imgui PUBLIC
    "${LRENDER_EXTERNAL_DIR}/imgui"
    "${LRENDER_EXTERNAL_DIR}/imgui/backends")
target_link_libraries(imgui PRIVATE d3d11)
set_target_properties(imgui PROPERTIES FOLDER "External")

add_library(imguizmo STATIC
    "${LRENDER_EXTERNAL_DIR}/ImGuizmo/src/ImGuizmo.cpp")
target_include_directories(imguizmo PUBLIC "${LRENDER_EXTERNAL_DIR}/ImGuizmo/src")
target_link_libraries(imguizmo PUBLIC imgui)
set_target_properties(imguizmo PROPERTIES FOLDER "External")
