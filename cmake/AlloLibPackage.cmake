# Install AlloLib as a relocatable CMake package.
#
# Two export sets (each target appears in only one):
#   AlloLibBundledTargets — vendored third-party targets (no namespace)
#   AlloLibTargets        — al::* modules (NAMESPACE al::)
#
# This bundled layout is for usability with in-tree submodules. A future
# vcpkg/Conan layout would drop AlloLibBundledTargets and resolve those deps
# via find_dependency() in AlloLibDependencies.cmake instead.

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

set(ALLOLIB_CMAKE_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/AlloLib)

# Temporary completeness layer: FILE_SET lists are still historically incomplete
# (same gap as main). Install every module public header tree so installed
# consumers can compile. Refine FILE_SET inventories later; duplicates are fine.
file(GLOB _al_module_include_roots
  LIST_DIRECTORIES true
  "${CMAKE_CURRENT_SOURCE_DIR}/modules/*/include"
)
foreach(_inc IN LISTS _al_module_include_roots)
  if(EXISTS "${_inc}/al")
    install(
      DIRECTORY "${_inc}/al"
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING
        PATTERN "*.hpp"
        PATTERN "*.h"
    )
  endif()
endforeach()
unset(_al_module_include_roots)
unset(_inc)

# --- Umbrella convenience target -------------------------------------------
set_target_properties(al_all PROPERTIES EXPORT_NAME all)
install(TARGETS al_all EXPORT AlloLibTargets)

if(TARGET al_audio_device)
  set_target_properties(al_audio_device PROPERTIES EXPORT_NAME audio_device)
  install(TARGETS al_audio_device EXPORT AlloLibTargets)
endif()

# --- Bundled third-party targets -------------------------------------------
set(_al_bundled_targets
  Gamma
  glfw
  glad
  stb
  dr_libs
  rtaudio
  rtmidi
  oscpack
  imgui
  serial
  cpptoml
)

set(_al_bundled_present)
foreach(_dep IN LISTS _al_bundled_targets)
  if(TARGET ${_dep})
    list(APPEND _al_bundled_present ${_dep})
  endif()
endforeach()

if(_al_bundled_present)
  install(
    TARGETS ${_al_bundled_present}
    EXPORT AlloLibBundledTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  )

  # Headers for libraries that do not install their own include trees here.
  if(TARGET stb)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/graphics/external/stb/stb/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stb
      FILES_MATCHING PATTERN "*.h"
    )
  endif()
  if(TARGET dr_libs)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio/external/dr_libs/dr_libs/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dr_libs
      FILES_MATCHING PATTERN "*.h"
    )
  endif()
  if(TARGET glad)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/graphics/external/glad/include/
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
  if(TARGET glfw)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/graphics/external/glfw/include/GLFW
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
  if(TARGET Gamma)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/external/Gamma/Gamma
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
  if(TARGET rtaudio)
    install(
      FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio-backend-rtaudio/external/rtaudio/RtAudio.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/audio-backend-rtaudio/external/rtaudio/rtaudio_c.h
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtaudio
    )
  endif()
  if(TARGET rtmidi)
    install(
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/modules/midi/external/rtmidi/RtMidi.h
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rtmidi
    )
  endif()
  if(TARGET oscpack)
    install(
      DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/protocol/external/oscpack/ip
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/protocol/external/oscpack/osc
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
      FILES_MATCHING PATTERN "*.h"
    )
  endif()
  if(TARGET imgui)
    install(
      FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui/external/imgui/imgui.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui/external/imgui/imconfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui/external/imgui/imgui_internal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui/external/imgui/imstb_rectpack.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui/external/imgui/imstb_textedit.h
        ${CMAKE_CURRENT_SOURCE_DIR}/modules/imgui/external/imgui/imstb_truetype.h
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
  if(TARGET serial)
    install(
      DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/modules/serial/external/serial/include/serial
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()
  if(TARGET cpptoml)
    install(
      FILES ${CMAKE_CURRENT_SOURCE_DIR}/modules/config/external/cpptoml/include/cpptoml.h
      DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
  endif()

  install(
    EXPORT AlloLibBundledTargets
    FILE AlloLibBundledTargets.cmake
    DESTINATION ${ALLOLIB_CMAKE_INSTALL_DIR}
  )
endif()

# --- AlloLib module targets (NAMESPACE al::) -------------------------------
install(
  EXPORT AlloLibTargets
  FILE AlloLibTargets.cmake
  NAMESPACE al::
  DESTINATION ${ALLOLIB_CMAKE_INSTALL_DIR}
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AlloLibConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/AlloLibConfig.cmake
  INSTALL_DESTINATION ${ALLOLIB_CMAKE_INSTALL_DIR}
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/AlloLibConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/AlloLibConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/AlloLibConfigVersion.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/AlloLibDependencies.cmake
  DESTINATION ${ALLOLIB_CMAKE_INSTALL_DIR}
)
