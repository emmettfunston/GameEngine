add_executable(DemoApp
  main.cpp
)

set_target_properties(DemoApp PROPERTIES OUTPUT_NAME "DemoApp")

target_compile_features(DemoApp PRIVATE cxx_std_20)
target_link_libraries(DemoApp PRIVATE engine)

target_compile_definitions(DemoApp PRIVATE $<$<BOOL:${ENGINE_ENABLE_IMGUI}>:ENGINE_ENABLE_IMGUI=1>)

if(ENGINE_ENABLE_IMGUI)
  target_link_libraries(DemoApp PRIVATE imgui)
endif()


