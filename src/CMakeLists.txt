add_library(LRenderCore STATIC
    core/Camera.cpp
    core/Scene.cpp
    commands/CommandHistory.cpp
    commands/CreateEntityCommand.cpp
    commands/TransformCommand.cpp
    utils/Logger.cpp)
target_include_directories(LRenderCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LRenderCore PUBLIC DirectXTK)
lrender_set_warnings(LRenderCore)

add_executable(LRenderDemo WIN32
    app/Main.cpp
    app/Application.cpp
    platform/Window.cpp
    render/BasicMeshEffect.cpp
    render/Dx11Renderer.cpp
    render/Mesh.cpp
    render/PrimitiveFactory.cpp
    render/RenderTarget.cpp
    editor/EditorLayer.cpp)
target_include_directories(LRenderDemo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LRenderDemo PRIVATE
    LRenderCore
    imgui
    imguizmo
    d3d11
    dxgi
    dxguid)
lrender_set_warnings(LRenderDemo)

set_target_properties(LRenderDemo PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    FOLDER "LRenderDemo")
set_target_properties(LRenderCore PROPERTIES FOLDER "LRenderDemo")
