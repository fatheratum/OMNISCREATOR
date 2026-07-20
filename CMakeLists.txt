cmake_minimum_required(VERSION 3.20)
project(OMNISCREATOR VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================================================================
# COMPILER FLAGS
# ============================================================================
if(MSVC)
    add_compile_options(
        /W4
        /WX-
        /permissive-
        /fp:fast
        /arch:AVX2
        /O2
        /Ob2
        /GL                          # Whole-program optimisation
        /MP                          # Parallel compilation
    )
    add_compile_definitions(
        _UNICODE UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0A00          # Windows 10+
    )
endif()

# ============================================================================
# PATHS
# ============================================================================
set(OMNI_INCLUDE_DIR   ${CMAKE_SOURCE_DIR}/include)
set(OMNI_SRC_DIR       ${CMAKE_SOURCE_DIR}/src)
set(OMNI_SHADER_DIR    ${CMAKE_SOURCE_DIR}/shaders)

# ============================================================================
# EXTERNAL DEPS
# ============================================================================

# Windows SDK (DX12)
# Usually found automatically via MSVC toolset — confirm paths if needed:
# set(WIN_SDK_DIR "C:/Program Files (x86)/Windows Kits/10")

# Vulkan SDK
find_package(Vulkan QUIET)

# Dear ImGui — expected at ../imgui relative to project or via IMGUI_DIR
if(NOT DEFINED IMGUI_DIR)
    set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/../imgui)
endif()

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx12.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
)

# ============================================================================
# OMNIBRAIN CORE (header-only — just include dirs needed)
# ============================================================================
add_library(OmniBrainCore INTERFACE)
target_include_directories(OmniBrainCore INTERFACE ${OMNI_INCLUDE_DIR})

# ============================================================================
# OMNIDIRECTXINFINITY TARGET (DirectX 12 + OMNIBRAIN)
# ============================================================================
add_executable(OMNIDIRECTXINFINITY WIN32
    ${OMNI_SRC_DIR}/main.cpp
    ${OMNI_SRC_DIR}/OMNIDirectXInfinity.cpp
    ${OMNI_SRC_DIR}/OMNISystem.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(OMNIDIRECTXINFINITY PRIVATE
    ${OMNI_INCLUDE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(OMNIDIRECTXINFINITY PRIVATE
    OmniBrainCore
    d3d12.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
)

# DXC for DXIL shader compilation (raytracing pipelines)
find_file(DXC_EXECUTABLE dxc.exe
    HINTS
        "$ENV{VULKAN_SDK}/Bin"
        "C:/Program Files (x86)/Windows Kits/10/bin/10.0.22621.0/x64"
)
if(DXC_EXECUTABLE)
    target_compile_definitions(OMNIDIRECTXINFINITY PRIVATE HAS_DXC=1)
    message(STATUS "DXC found: ${DXC_EXECUTABLE}")
else()
    message(WARNING "DXC not found — DXIL shaders will be compiled at runtime via D3DCompiler fallback")
endif()

# ============================================================================
# HLSL SHADER COMPILATION (custom target — runs dxc for each .hlsl)
# ============================================================================
file(GLOB_RECURSE HLSL_FILES "${OMNI_SHADER_DIR}/hlsl/*.hlsl")

set(COMPILED_SHADERS "")
foreach(HLSL ${HLSL_FILES})
    get_filename_component(SHADER_NAME ${HLSL} NAME_WE)
    set(CSO_OUT "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.cso")

    # Detect shader type from filename conventions
    if(SHADER_NAME MATCHES ".*Compute.*" OR SHADER_NAME MATCHES ".*_CS.*")
        set(PROFILE "cs_6_5")
        set(ENTRY "main")
    elseif(SHADER_NAME MATCHES ".*_VS.*")
        set(PROFILE "vs_6_5")
        set(ENTRY "main")
    elseif(SHADER_NAME MATCHES ".*_PS.*")
        set(PROFILE "ps_6_5")
        set(ENTRY "main")
    else()
        # Multi-entry file — compile all entries
        set(PROFILE "lib_6_5")
        set(ENTRY "")
    endif()

    if(DXC_EXECUTABLE)
        add_custom_command(
            OUTPUT ${CSO_OUT}
            COMMAND ${DXC_EXECUTABLE}
                -T ${PROFILE}
                $<$<NOT:$<STREQUAL:${ENTRY},>>:-E ${ENTRY}>
                -Fo ${CSO_OUT}
                -Zi                    # Debug info
                -Od                    # Disable optimisations in debug
                $<$<CONFIG:Release>:-O3>
                ${HLSL}
            DEPENDS ${HLSL}
            COMMENT "Compiling HLSL: ${SHADER_NAME}"
        )
        list(APPEND COMPILED_SHADERS ${CSO_OUT})
    endif()
endforeach()

if(COMPILED_SHADERS)
    add_custom_target(CompileShaders ALL DEPENDS ${COMPILED_SHADERS})
    add_dependencies(OMNIDIRECTXINFINITY CompileShaders)
endif()

# ============================================================================
# OMNIVALKIN TARGET (Vulkan + OMNIBRAIN)
# ============================================================================
if(Vulkan_FOUND)
    add_executable(OMNIVALKIN WIN32
        ${OMNI_SRC_DIR}/main.cpp              # Shared main with compile switch
        ${OMNI_SRC_DIR}/OMNIDirectXInfinity.cpp
        ${OMNI_SRC_DIR}/OMNISystem.cpp
        ${IMGUI_SOURCES}
    )

    target_include_directories(OMNIVALKIN PRIVATE
        ${OMNI_INCLUDE_DIR}
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${Vulkan_INCLUDE_DIRS}
    )

    target_link_libraries(OMNIVALKIN PRIVATE
        OmniBrainCore
        Vulkan::Vulkan
    )

    target_compile_definitions(OMNIVALKIN PRIVATE
        OMNIVALKIN_BUILD=1
    )

    # GLSL → SPIR-V compilation
    find_program(GLSLC glslc
        HINTS "$ENV{VULKAN_SDK}/Bin"
    )
    file(GLOB_RECURSE GLSL_FILES
        "${OMNI_SHADER_DIR}/glsl/*.vert"
        "${OMNI_SHADER_DIR}/glsl/*.frag"
        "${OMNI_SHADER_DIR}/glsl/*.comp"
    )

    set(SPIRV_FILES "")
    foreach(GLSL ${GLSL_FILES})
        get_filename_component(SHADER_NAME ${GLSL} NAME)
        set(SPV_OUT "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
        if(GLSLC)
            add_custom_command(
                OUTPUT ${SPV_OUT}
                COMMAND ${GLSLC} -O ${GLSL} -o ${SPV_OUT}
                DEPENDS ${GLSL}
                COMMENT "Compiling GLSL: ${SHADER_NAME}"
            )
            list(APPEND SPIRV_FILES ${SPV_OUT})
        endif()
    endforeach()

    if(SPIRV_FILES)
        add_custom_target(CompileGLSL ALL DEPENDS ${SPIRV_FILES})
        add_dependencies(OMNIVALKIN CompileGLSL)
    endif()

    message(STATUS "OMNIVALKIN (Vulkan) target configured.")
else()
    message(STATUS "Vulkan SDK not found — OMNIVALKIN target skipped.")
endif()

# ============================================================================
# POST-BUILD — copy shaders and config to output directory
# ============================================================================
add_custom_command(TARGET OMNIDIRECTXINFINITY POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:OMNIDIRECTXINFINITY>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config"
        "$<TARGET_FILE_DIR:OMNIDIRECTXINFINITY>/config"
    COMMENT "Copying shaders and config to output"
)

# ============================================================================
# INSTALL
# ============================================================================
install(TARGETS OMNIDIRECTXINFINITY
    RUNTIME DESTINATION bin
)
install(DIRECTORY ${CMAKE_BINARY_DIR}/shaders
    DESTINATION bin
)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/config
    DESTINATION bin
)

message(STATUS "")
message(STATUS "============================================================")
message(STATUS "  OMNISCREATOR BUILD CONFIGURED")
message(STATUS "  DX12  Target : OMNIDIRECTXINFINITY")
message(STATUS "  Vulkan Target: OMNIVALKIN")
message(STATUS "  C++ Standard : C++17")
message(STATUS "  IMGUI_DIR    : ${IMGUI_DIR}")
message(STATUS "============================================================")
message(STATUS "")
