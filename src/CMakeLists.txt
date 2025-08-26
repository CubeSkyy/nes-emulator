# Collect all source and header files
file(GLOB_RECURSE CPU_SOURCES "cpu/*.cpp")
file(GLOB_RECURSE CPU_HEADERS "cpu/*.h")
file(GLOB_RECURSE MAPPER_SOURCES "cartridge/mappers/*.cpp")
file(GLOB_RECURSE MAPPER_HEADERS "cartridge/mappers/*.h")


# Shared library for common code
add_library(nes_core STATIC ${CPU_SOURCES} ${CPU_HEADERS} ${MAPPER_SOURCES} ${MAPPER_HEADERS}
        bus.cpp
        bus.h
        log/logging.cpp
        log/logging.h
        ppu/ppu.cpp
        ppu/ppu.h
        cartridge/cartridge.cpp
        cartridge/cartridge.h
)
target_include_directories(nes_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cpu ${CMAKE_CURRENT_SOURCE_DIR}/ppu ${CMAKE_SOURCE_DIR}/external/imgui)
target_link_libraries(nes_core PUBLIC Threads::Threads)

# Graphics Debugger executable
add_executable(NESGraphicsDebug
        ppu/graphics_debug.cpp
        ppu/graphics_wrapper.cpp
        ppu/ppu.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdl2.cpp
        ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdlrenderer2.cpp
)
target_link_options(NESGraphicsDebug PRIVATE -mconsole)
set_target_properties(NESGraphicsDebug PROPERTIES WIN32_EXECUTABLE OFF)
target_include_directories(NESGraphicsDebug PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/external/imgui)
target_link_libraries(NESGraphicsDebug PRIVATE Threads::Threads nes_core SDL2 imgui)

# Automatically copy SDL2.dll after build
add_custom_command(TARGET NESGraphicsDebug POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/SDL2-2.32.6/x86_64-w64-mingw32/bin/SDL2.dll"
        $<TARGET_FILE_DIR:NESGraphicsDebug>
)
