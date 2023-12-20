cmake_minimum_required(VERSION 3.0.0)

project(Qbox VERSION 0.1.0)

find_package(Freetype REQUIRED)
find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB ImGui_Src libs/imgui/*.cpp libs/imgui/backends/imgui_impl_glfw.cpp libs/imgui/backends/imgui_impl_opengl3.cpp libs/ImGuiFileDialog/ImGuiFileDialog.cpp)
file(GLOB Glad_Src libs/glad/gl.c)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/window.cpp
    src/utils.cpp
    src/controller.cpp
    src/renderer.cpp
    src/slide.cpp
    src/gl.cpp
    ${ImGui_Src}
    ${Glad_Src}
    )

target_include_directories(${PROJECT_NAME} PRIVATE "libs/imgui")
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE "libs")
target_include_directories(${PROJECT_NAME} PRIVATE ${FREETYPE_INCLUDE_DIRS})

add_subdirectory(libs/glm)
target_link_libraries(${PROJECT_NAME} glm::glm)

target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS} pthread)
