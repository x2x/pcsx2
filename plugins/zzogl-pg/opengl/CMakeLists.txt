# Check that people use the good file
if(NOT TOP_CMAKE_WAS_SOURCED)
    message(FATAL_ERROR "
    You did not 'cmake' the good CMakeLists.txt file. Use the one in the top dir.
    It is advice to delete all wrongly generated cmake stuff => CMakeFiles & CMakeCache.txt")
endif(NOT TOP_CMAKE_WAS_SOURCED)

# Allow to build the shader files
if(EXISTS "${CMAKE_SOURCE_DIR}/plugins/zzogl-pg/opengl/ZeroGSShaders" AND REBUILD_SHADER AND NOT GLSL_API)
    add_subdirectory(ZeroGSShaders)
endif(EXISTS "${CMAKE_SOURCE_DIR}/plugins/zzogl-pg/opengl/ZeroGSShaders" AND REBUILD_SHADER AND NOT GLSL_API)

# plugin name
set(Output zzogl-0.4.0)

set(CommonFlags
    -pthread
    -DZEROGS_SSE2
    -fno-regmove
    -fno-strict-aliasing
    -Wstrict-aliasing # Allow to track strict aliasing issue.
    -Wunused-variable
    )

set(OptimizationFlags
    -O2
    -DNDEBUG
    )

# Debug - Build
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    # add defines
	add_definitions(${CommonFlags} -g -Wall -D_DEBUG)
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

# Devel - Build
if(CMAKE_BUILD_TYPE STREQUAL Devel)
    # add defines
    add_definitions(${CommonFlags} ${OptimizationFlags} -g -W -DZEROGS_DEVBUILD)
endif(CMAKE_BUILD_TYPE STREQUAL Devel)

# Release - Build
if(CMAKE_BUILD_TYPE STREQUAL Release)
    # add defines
    add_definitions(${CommonFlags} ${OptimizationFlags} -W)
endif(CMAKE_BUILD_TYPE STREQUAL Release)

# Select the shader API 
if(GLSL_API)
    add_definitions(-DGLSL4_API -DOGL4_LOG)
    #add_definitions(-DGLSL_API)
else(GLSL_API)
	add_definitions(-DNVIDIA_CG_API)
endif(GLSL_API)

# Select the EGL API
if(EGL_API AND EGL_FOUND)
    if (EGL_GL_CONTEXT_SUPPORT)
        add_definitions(-DEGL_API)
    else()
        message(WARNING "Current EGL implementation doesn't support openGL context. Fallback to standard GLX.")
    endif()
endif()


# zzogl sources
set(zzoglSources
    GifTransfer.cpp
    GLWin32.cpp
    GLWinX11.cpp
    GSDump.cpp
    GSmain.cpp
    HostMemory.cpp
    Mem.cpp
    Mem_Swizzle.cpp
    Mem_Tables.cpp
    Profile.cpp
    rasterfont.cpp
    Regs.cpp
    targets.cpp
    zerogs.cpp
    zpipe.cpp
    ZZDepthTargets.cpp
    ZZMemoryTargets.cpp
    ZZRenderTargets.cpp
    ZZClut.cpp
    ZZHacks.cpp
    ZZKeyboard.cpp
    ZZoglDrawing.cpp
    ZZLog.cpp
    ZZoglCreate.cpp
    ZZoglCRTC.cpp
    ZZoglFlush.cpp
    ZZoglFlushHack.cpp
    ZZoglMem.cpp
    ZZoglSave.cpp
    ZZoglShaders.cpp
    ZZoglShadersGLSL.cpp
    ZZoglShadersGLSL4.cpp
    ZZoglShoots.cpp
    ZZoglVB.cpp
    )

# zzogl headers
set(zzoglHeaders
    common.h
    CRC.h
    GifTransfer.h
    # glprocs.h
    GS.h
    GSDump.h
    HostMemory.h
    ps2hw_gl4.h
    Mem.h
    Mem_Swizzle.h
    Mem_Transmit.h
    Profile.h
    rasterfont.h
    Regs.h
    targets.h
    Util.h
    x86.h
    zerogs.h
    zpipe.h
    ZZClut.h
    ZZoglFlush.h
    ZZGl.h
    ZZHacks.h
    ZZoglDrawing.h
    ZZLog.h
    ZZoglCRTC.h
    ZZoglMath.h
    ZZoglMem.h
    ZZoglShaders.h
    ZZoglShoots.h
    ZZoglVB.h
    )

# zzogl shader sources
set(zzoglShaderSources
    ctx0/ps2hw_ctx.fx
    ctx1/ps2hw_ctx.fx)

# zzogl Linux sources
set(zzoglLinuxSources
    Linux/Conf.cpp
    Linux/Linux.cpp)

# zzogl Linux headers
set(zzoglLinuxHeaders
    Linux/Linux.h)

# add additional include directories
include_directories(.
    Linux)

# Generate Glsl header file. Protect with REBUILD_SHADER to avoid build-dependency on PERL
if (REBUILD_SHADER)
    add_custom_command(OUTPUT ps2hw_gl4.h COMMAND perl ${CMAKE_SOURCE_DIR}/linux_various/glsl2h.pl)
endif()

# add library
add_library(${Output} SHARED
    ${zzoglSources}
    ${zzoglHeaders}
    ${zzoglShaderSources}
    ${zzoglLinuxSources}
    ${zzoglLinuxHeaders})

# Trick that allow to compile zzogl with GSOPEN2 and the replayer with GSOPEN
set_target_properties(${Output} PROPERTIES COMPILE_DEFINITIONS USE_GSOPEN2)

# link target with project internal libraries
target_link_libraries(${Output} Utilities)

# link target with the various opengl flavor
target_link_libraries(${Output} ${OPENGL_LIBRARIES})
if(EGL_API AND EGL_FOUND AND EGL_GL_CONTEXT_SUPPORT)
    target_link_libraries(${Output} ${EGL_LIBRARIES})
endif()
if(NOT GLSL_API)
	target_link_libraries(${Output} ${CG_LIBRARIES})
endif(NOT GLSL_API)

target_link_libraries(${Output} ${GLEW_LIBRARY})

target_link_libraries(${Output} ${X11_LIBRARIES})
target_link_libraries(${Output} ${JPEG_LIBRARIES})

if(Linux)
	target_link_libraries(${Output} ${GTK2_LIBRARIES})
endif(Linux)

target_link_libraries(${Output} ${ZLIB_LIBRARIES})

# User flags options
if(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
    target_link_libraries(${Output} "${USER_CMAKE_LD_FLAGS}")
endif(NOT USER_CMAKE_LD_FLAGS STREQUAL "")

if(PACKAGE_MODE)
    install(TARGETS ${Output} DESTINATION ${PLUGIN_DIR})
else()
    install(TARGETS ${Output} DESTINATION ${CMAKE_SOURCE_DIR}/bin/plugins)
endif()

if(NOT GLSL_API AND NOT REBUILD_SHADER)
    if(PACKAGE_MODE)
        install(FILES ${CMAKE_SOURCE_DIR}/plugins/zzogl-pg/opengl/ps2hw.dat DESTINATION ${PLUGIN_DIR})
    else()
        install(FILES ${CMAKE_SOURCE_DIR}/plugins/zzogl-pg/opengl/ps2hw.dat DESTINATION ${CMAKE_SOURCE_DIR}/bin/plugins)
    endif()
endif()

################################### Replay Loader
if(BUILD_REPLAY_LOADERS)
	set(Replay pcsx2_ZZReplayLoader)
	set(Static zzogl-static)

	add_library(${Static} STATIC
	    ${zzoglSources}
	    ${zzoglHeaders}
	    ${zzoglSSources}
	    ${zzoglShaderSources}
	    ${zzoglLinuxSources}
	    ${zzoglLinuxHeaders})

	target_link_libraries(${Static} Utilities)
	if(NOT GLSL_API)
		target_link_libraries(${Static} ${CG_LIBRARIES})
	endif(NOT GLSL_API)
	target_link_libraries(${Static} ${GLEW_LIBRARY})
	target_link_libraries(${Static} ${OPENGL_LIBRARIES})
if(EGL_API AND EGL_FOUND AND EGL_GL_CONTEXT_SUPPORT)
	target_link_libraries(${Static} ${EGL_LIBRARIES})
endif()
	target_link_libraries(${Static} ${X11_LIBRARIES})
	target_link_libraries(${Static} ${JPEG_LIBRARIES})

	add_executable(${Replay} linux_replay.cpp)

	target_link_libraries(${Replay} ${Static})
	target_link_libraries(${Replay} Utilities)
	if(NOT GLSL_API)
		target_link_libraries(${Replay} ${CG_LIBRARIES})
	endif(NOT GLSL_API)
	target_link_libraries(${Replay} ${GLEW_LIBRARY})
	target_link_libraries(${Replay} ${OPENGL_LIBRARIES})
	target_link_libraries(${Replay} ${X11_LIBRARIES})
	target_link_libraries(${Replay} ${JPEG_LIBRARIES})

	target_link_libraries(${Replay} ${GTK2_LIBRARIES})
	target_link_libraries(${Replay} ${ZLIB_LIBRARIES})

	if(NOT USER_CMAKE_LD_FLAGS STREQUAL "")
	    target_link_libraries(${Replay} "${USER_CMAKE_LD_FLAGS}")
	endif(NOT USER_CMAKE_LD_FLAGS STREQUAL "")

	if(PACKAGE_MODE)
	    install(TARGETS ${Replay} DESTINATION bin)
	else(PACKAGE_MODE)
	    install(TARGETS ${Replay} DESTINATION ${CMAKE_SOURCE_DIR}/bin)
	endif(PACKAGE_MODE)
endif(BUILD_REPLAY_LOADERS)
