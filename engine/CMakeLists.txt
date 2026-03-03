add_library(engine STATIC)

target_sources(engine
  PRIVATE
    src/core/Engine.cpp
    src/core/Log.cpp
    src/core/Time.cpp
    src/platform/Window.cpp
    src/input/Input.cpp
    src/renderer/GLDebug.cpp
    src/renderer/Shader.cpp
    src/renderer/Mesh.cpp
    src/renderer/MeshLibrary.cpp
    src/renderer/Camera.cpp
    src/renderer/RenderSystem.cpp
    src/renderer/DebugDraw.cpp
    src/scene/Scene.cpp
    src/physics/PhysicsSystem.cpp
    src/physics/PlayerController.cpp
)

target_include_directories(engine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/glad/include
  PRIVATE
    ${jolt_SOURCE_DIR}
)

target_compile_features(engine PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(engine PRIVATE /W4 /permissive-)
else()
  target_compile_options(engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_compile_definitions(engine
  PUBLIC
    ENGINE_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets"
    $<$<BOOL:${ENGINE_ENABLE_IMGUI}>:ENGINE_ENABLE_IMGUI=1>
)

add_library(glad STATIC
  ${CMAKE_SOURCE_DIR}/third_party/glad/src/gl.c
)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/third_party/glad/include)

target_link_libraries(engine
  PUBLIC
    glm::glm
    SDL2::SDL2-static
  PRIVATE
    glad
    Jolt
)

if(APPLE)
  find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
  find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
  find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
  find_library(COREVIDEO_FRAMEWORK CoreVideo REQUIRED)
  target_link_libraries(engine PRIVATE
    ${COCOA_FRAMEWORK}
    ${OPENGL_FRAMEWORK}
    ${IOKIT_FRAMEWORK}
    ${COREVIDEO_FRAMEWORK}
  )
endif()
