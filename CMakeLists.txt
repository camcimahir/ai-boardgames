cmake_minimum_required(VERSION 3.11.0)
project(ai-boardgames VERSION 0.1.0 LANGUAGES C CXX)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOS TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX TRUE)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(LINUX OR WINDOWS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-ignored-qualifiers")
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)

if(MACOS)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
    find_package(glfw3 REQUIRED)
    include_directories(${GLFW_INCLUDE_DIRS})
endif()

include(CTest)
enable_testing()

if(MACOS)
    set(MAIN_FILE "main_macos.cpp")
    set(IMPL_FILE "imgui/imgui_impl_glfw.cpp")
    set(BCKD_FILE "imgui/imgui_impl_opengl3.cpp")
elseif(WINDOWS)
    set(MAIN_FILE "main_win32.cpp")
    set(IMPL_FILE "imgui/imgui_impl_win32.cpp")
    set(BCKD_FILE "imgui/imgui_impl_dx11.cpp")
else() # Linux
    set(MAIN_FILE "main_macos.cpp")
    set(IMPL_FILE "imgui/imgui_impl_glfw.cpp")
    set(BCKD_FILE "imgui/imgui_impl_opengl3.cpp")
endif()

add_executable(demo
    Application.cpp
    ${MAIN_FILE}
    ${IMPL_FILE}
    ${BCKD_FILE}

    # Dear ImGui core
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp

    # Shared engine
    classes/Bit.cpp
    classes/BitHolder.cpp
    classes/ChessSquare.cpp
    classes/Game.cpp
    classes/Grid.cpp
    classes/Sprite.cpp
    classes/Square.cpp

    # Tic-Tac-Toe
    classes/TicTacToe.cpp

    # Connect Four
    classes/ConnectFour.cpp

    # Chess (also includes Bitboard.h, MagicBitboards.h, Evaluate.h headers)
    classes/Chess.cpp

    # AstroBots / Robots
    classes/AstroBots.cpp
    classes/AstroArena.cpp
    classes/AstroCollision.cpp
)

if(EMSCRIPTEN)
    # Browser build: GLFW + WebGL come from Emscripten ports, not system packages.
    # Resources are baked into the virtual filesystem under /resources via --preload-file,
    # so the same "resources/<file>" paths used by the desktop build keep working.
    target_link_options(demo PRIVATE
        "-sUSE_GLFW=3"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sNO_EXIT_RUNTIME=1"
        "--preload-file" "${CMAKE_SOURCE_DIR}/resources@/resources"
        "--shell-file" "${CMAKE_SOURCE_DIR}/web/shell.html"
    )
    # Emit index.html (+ index.js / index.wasm / index.data) for GitHub Pages.
    set_target_properties(demo PROPERTIES OUTPUT_NAME "index" SUFFIX ".html")
elseif(MACOS OR LINUX)
    target_link_libraries(demo ${OPENGL_gl_LIBRARY} glfw)
elseif(WINDOWS)
    target_link_libraries(demo
        d3d11.lib
        d3dcompiler.lib
        dxgi.lib
        user32.lib
        gdi32.lib
        winmm.lib
    )
endif()

# Native builds run from a folder, so copy resources next to the executable.
# The Emscripten build bundles them with --preload-file instead.
if(NOT EMSCRIPTEN)
    add_custom_command(
        TARGET demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_SOURCE_DIR}/resources"
                "$<TARGET_FILE_DIR:demo>/resources"
        COMMENT "Copying resources to runtime output dir"
    )
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
