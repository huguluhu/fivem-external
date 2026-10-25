cmake_minimum_required(VERSION 3.20)
project(FiveMExternal LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_compile_definitions(
    WIN32_LEAN_AND_MEAN NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    IMGUI_DEFINE_MATH_OPERATORS
)

set(IMGUI ${CMAKE_SOURCE_DIR}/vendor/imgui)
set(LUA   ${CMAKE_SOURCE_DIR}/vendor/lua)

set(SRC
    src/main.cpp
    src/core/memory.cpp
    src/game/sdk.cpp
    src/features/aimbot.cpp
    src/features/esp.cpp
    src/features/misc.cpp
    src/ui/overlay.cpp
    src/ui/render.cpp
    src/ui/theme.cpp
    src/ui/menu.cpp
    src/ui/notify.cpp
    src/config/config.cpp
    src/scripting/lua_engine.cpp
)

set(IMGUI_SRC
    ${IMGUI}/imgui.cpp ${IMGUI}/imgui_draw.cpp
    ${IMGUI}/imgui_tables.cpp ${IMGUI}/imgui_widgets.cpp
    ${IMGUI}/backends/imgui_impl_dx11.cpp
    ${IMGUI}/backends/imgui_impl_win32.cpp
)

file(GLOB LUA_SRC ${LUA}/src/*.c)
list(REMOVE_ITEM LUA_SRC ${LUA}/src/lua.c ${LUA}/src/luac.c ${LUA}/src/onelua.c)

add_executable(FiveMExternal WIN32 ${SRC} ${IMGUI_SRC} ${LUA_SRC})

target_include_directories(FiveMExternal PRIVATE
    src ${IMGUI} ${IMGUI}/backends ${LUA}/src)

target_link_libraries(FiveMExternal PRIVATE
    d3d11 dxgi d3dcompiler dwmapi winmm psapi gdi32 user32 ole32 shell32)

target_compile_options(FiveMExternal PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/MP /permissive->)

set_target_properties(FiveMExternal PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:WinMainCRTStartup")