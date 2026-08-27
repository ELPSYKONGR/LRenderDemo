set(LRENDER_CORE_HEADERS
    commands/CommandHistory.h
    commands/CreateEntityCommand.h
    commands/CreateModelCommand.h
    commands/ICommand.h
    commands/MaterialCommand.h
    commands/SolidGeometryCommand.h
    commands/TransformCommand.h
    core/Camera.h
    core/EntityMaterial.h
    core/Scene.h
    core/SolidGeometry.h
    core/Transform.h
    utils/Logger.h)

add_library(LRenderCore STATIC
    ${LRENDER_CORE_HEADERS}
    core/Camera.cpp
    core/Scene.cpp
    core/SolidGeometry.cpp
    commands/CommandHistory.cpp
    commands/CreateEntityCommand.cpp
    commands/CreateModelCommand.cpp
    commands/MaterialCommand.cpp
    commands/SolidGeometryCommand.cpp
    commands/TransformCommand.cpp
    utils/Logger.cpp)
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Header Files"
    FILES ${LRENDER_CORE_HEADERS})
target_include_directories(LRenderCore PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LRenderCore PUBLIC DirectXTK)
lrender_set_warnings(LRenderCore)

set(LRENDER_PERSISTENCE_HEADERS
    persistence/SceneSerializer.h)

add_library(LRenderPersistence STATIC
    ${LRENDER_PERSISTENCE_HEADERS}
    persistence/SceneSerializer.cpp)
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Header Files"
    FILES ${LRENDER_PERSISTENCE_HEADERS})
target_include_directories(LRenderPersistence PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LRenderPersistence PUBLIC LRenderCore PRIVATE nlohmann_json)
lrender_set_warnings(LRenderPersistence)

set(LRENDER_ASSET_HEADERS
    render/GltfLoader.h
    render/IModelImporter.h
    render/Material.h
    render/Mesh.h
    render/MeshAsset.h
    render/MeshImportUtils.h
    render/ModelLoader.h
    render/ObjLoader.h
    render/ResourceCache.h
    render/SamplerState.h
    render/Texture2D.h)

add_library(LRenderAssets STATIC
    ${LRENDER_ASSET_HEADERS}
    render/GltfLoader.cpp
    render/Mesh.cpp
    render/MeshImportUtils.cpp
    render/ModelLoader.cpp
    render/ObjLoader.cpp
    render/ResourceCache.cpp
    render/SamplerState.cpp
    render/Texture2D.cpp)
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Header Files"
    FILES ${LRENDER_ASSET_HEADERS})
target_include_directories(LRenderAssets PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LRenderAssets
    PUBLIC LRenderCore DirectXTK d3d11
    PRIVATE cgltf tinyobjloader)
lrender_set_warnings(LRenderAssets)

set(LRENDER_DEMO_HEADERS
    app/Application.h
    editor/EditorLayer.h
    platform/Window.h
    render/BasicMeshConstants.h
    render/BasicMeshEffect.h
    render/Dx11ConstantBuffer.h
    render/Dx11Renderer.h
    render/EffectContext.h
    render/IRenderEffect.h
    render/Lighting.h
    render/PrimitiveFactory.h
    render/RenderTarget.h)
list(APPEND LRENDER_DEMO_HEADERS render/SolidMeshCache.h)

add_executable(LRenderDemo WIN32
    ${LRENDER_DEMO_HEADERS}
    app/Main.cpp
    app/Application.cpp
    platform/Window.cpp
    render/BasicMeshEffect.cpp
    render/Dx11Renderer.cpp
    render/EffectContext.cpp
    render/PrimitiveFactory.cpp
    render/RenderTarget.cpp
    render/SolidMeshCache.cpp
    editor/EditorAssets.cpp
    editor/EditorCamera.cpp
    editor/EditorHierarchy.cpp
    editor/EditorMaterial.cpp
    editor/EditorScene.cpp
    editor/EditorSolid.cpp
    editor/EditorLayer.cpp)
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Header Files"
    FILES ${LRENDER_DEMO_HEADERS})

set(LRENDER_SHADER_OUTPUT_ROOT "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(LRENDER_SHADER_BUILD_OUTPUT_DIR "${LRENDER_SHADER_OUTPUT_ROOT}/$(Configuration)")
set(LRENDER_SHADER_RUNTIME_OUTPUT_DIR "${LRENDER_SHADER_OUTPUT_ROOT}/$<CONFIG>")
set(LRENDER_SHADER_SOURCES
    shaders/BasicMeshVS.hlsl
    shaders/BasicMeshPS.hlsl)
set(LRENDER_SHADER_HEADERS
    shaders/BasicMeshConstants.hlsli)
set_source_files_properties(${LRENDER_SHADER_HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)

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

target_sources(LRenderDemo PRIVATE ${LRENDER_SHADER_SOURCES} ${LRENDER_SHADER_HEADERS})
source_group("Shaders" FILES ${LRENDER_SHADER_SOURCES} ${LRENDER_SHADER_HEADERS})
add_custom_command(TARGET LRenderDemo PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${LRENDER_SHADER_RUNTIME_OUTPUT_DIR}")
target_compile_definitions(LRenderDemo PRIVATE
    LRENDER_SHADER_OUTPUT_DIR="${LRENDER_SHADER_RUNTIME_OUTPUT_DIR}")
target_include_directories(LRenderDemo PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(LRenderDemo PRIVATE
    LRenderCore
    LRenderAssets
    LRenderPersistence
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
set_target_properties(LRenderAssets PROPERTIES FOLDER "LRenderDemo")
set_target_properties(LRenderPersistence PROPERTIES FOLDER "LRenderDemo")
