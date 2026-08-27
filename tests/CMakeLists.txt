add_executable(LRenderCoreTests CoreTests.cpp)
target_link_libraries(LRenderCoreTests PRIVATE LRenderCore)
lrender_set_warnings(LRenderCoreTests)
set_target_properties(LRenderCoreTests PROPERTIES FOLDER "Tests")

add_test(NAME LRenderCoreTests COMMAND LRenderCoreTests)

add_executable(LRenderImportTests ImportTests.cpp)
target_link_libraries(LRenderImportTests PRIVATE LRenderAssets d3d11)
target_compile_definitions(LRenderImportTests PRIVATE
    LRENDER_TEST_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
lrender_set_warnings(LRenderImportTests)
set_target_properties(LRenderImportTests PROPERTIES FOLDER "Tests")

add_test(NAME LRenderImportTests COMMAND LRenderImportTests)

add_executable(LRenderSolidTests
    SolidTests.cpp
    ../src/render/PrimitiveFactory.cpp
    ../src/render/SolidMeshCache.cpp)
target_link_libraries(LRenderSolidTests PRIVATE LRenderAssets d3d11)
lrender_set_warnings(LRenderSolidTests)
set_target_properties(LRenderSolidTests PROPERTIES FOLDER "Tests")
add_test(NAME LRenderSolidTests COMMAND LRenderSolidTests)

add_executable(LRenderPersistenceTests PersistenceTests.cpp)
target_link_libraries(LRenderPersistenceTests PRIVATE LRenderPersistence)
lrender_set_warnings(LRenderPersistenceTests)
set_target_properties(LRenderPersistenceTests PROPERTIES FOLDER "Tests")
add_test(NAME LRenderPersistenceTests COMMAND LRenderPersistenceTests)
