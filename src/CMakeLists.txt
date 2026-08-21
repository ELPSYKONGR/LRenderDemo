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
    render/GltfLoader.cpp
    render/Mesh.cpp
    render/ResourceCache.cpp
    render/SamplerState.cpp
    render/PrimitiveFactory.cpp
    render/RenderTarget.cpp
    render/Texture2D.cpp
    editor/EditorAssets.cpp
    editor/EditorLayer.cpp)

set(LRENDER_SHADER_OUTPUT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(LRENDER_SHADER_BUILD_OUTPUT_DIR "${LRENDER_SHADER_OUTPUT_ROOT}/$(Configuration)")
set(LRENDER_SHADER_RUNTIME_OUTPUT_DIR "${LRENDER_SHADER_OUTPUT_ROOT}/$<CONFIG>")
set(LRENDER_SHADER_SOURCES
    shaders/BasicMeshVS.hlsl
    shaders/BasicMeshPS.hlsl)

set_source_files_properties(shaders/BasicMeshVS.hlsl PROPERTIES
    VS_SHADER_TYPE Vertex
    VS_SHADER_MODEL 5.0
    VS_SHADER_ENTRYPOINT VSMain
    VS_SHADER_OBJECT_FILE_NAME "${LRENDER_SHADER_BUILD_OUTPUT_DIR}/BasicMeshVS.cso"
    VS_SHADER_DISABLE_OPTIMIZATIONS "$<$<CONFIG:Debug>:true>"
    VS_SHADER_ENABLE_DEBUG "$<$<CONFIG:Debug>:true>")
set_source_files_properties(shaders/BasicMeshPS.hlsl PROPERTIES
    VS_SHADER_TYPE Pixel
    VS_SHADER_MODEL 5.0
    VS_SHADER_ENTRYPOINT PSMain
    VS_SHADER_OBJECT_FILE_NAME "${LRENDER_SHADER_BUILD_OUTPUT_DIR}/BasicMeshPS.cso"
    VS_SHADER_DISABLE_OPTIMIZATIONS "$<$<CONFIG:Debug>:true>"
    VS_SHADER_ENABLE_DEBUG "$<$<CONFIG:Debug>:true>")

target_sources(LRenderDemo PRIVATE ${LRENDER_SHADER_SOURCES})
source_group("Shaders" FILES ${LRENDER_SHADER_SOURCES})
add_custom_command(TARGET LRenderDemo PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LRENDER_SHADER_RUNTIME_OUTPUT_DIR}")
target_compile_definitions(LRenderDemo PRIVATE
    LRENDER_SHADER_OUTPUT_DIR="${LRENDER_SHADER_RUNTIME_OUTPUT_DIR}")
target_include_directories(LRenderDemo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LRenderDemo PRIVATE
    LRenderCore
    cgltf
    imgui
    imguizmo
    d3d11
    d3dcompiler
    comdlg32
    dxgi
    dxguid)
lrender_set_warnings(LRenderDemo)

set_target_properties(LRenderDemo PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    FOLDER "LRenderDemo")
set_target_properties(LRenderCore PROPERTIES FOLDER "LRenderDemo")
