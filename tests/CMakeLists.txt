add_executable(LRenderCoreTests CoreTests.cpp)
target_link_libraries(LRenderCoreTests PRIVATE LRenderCore)
lrender_set_warnings(LRenderCoreTests)
set_target_properties(LRenderCoreTests PROPERTIES FOLDER "Tests")

add_test(NAME LRenderCoreTests COMMAND LRenderCoreTests)
