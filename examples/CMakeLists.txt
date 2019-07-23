# macro to get subdirectories
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# Ignore examples that wont work in CI
set(EXAMPLES_TO_IGNORE
    user_flags/main.cpp
    openvr/openvr.cpp
    openvr/openvr-app.cpp
    one-line-of-c/main.cpp
    sound/multiFilePlayer.cpp
    sound/sound_file.cpp
    sound/ambiPlayer.cpp
    graphics/textureImage.cpp
    graphics/font.cpp
    graphics/asset.cpp
    user_flags/main.cpp
    openvr/openvr.cpp
    openvr/openvr-app.cpp
    one-line-of-c/main.cpp
)

#if(AL_WINDOWS EQUAL 1 AND APPVEYOR_BUILD EQUAL 1) 
# Don't build files with dynamic dependencies on Appveyor CI build
#set(EXAMPLES_TO_IGNORE 
#    examples/sound/multiFilePlayer.cpp 
#    examples/sound/sound_file.cpp 
#    examples/sound/ambiPlayer.cpp
#    examples/graphics/textureImage.cpp
#    examples/graphics/font.cpp
#    examples/graphics/asset.cpp
#    examples/user_flags/main.cpp
#    examples/openvr/openvr.cpp
#    examples/openvr/openvr-app.cpp
#    examples/one-line-of-c/main.cpp
#)
#endif()

macro(BuildExample example_src dir)
  get_filename_component(EXAMPLE_NAME ${example_src} NAME_WE) # Get name w/o extension
  get_filename_component(EXAMPLE_DIRECTORY ${example_src} DIRECTORY) # Get name w/o extension

  if ("${dir}" STREQUAL ".")
    set(EXAMPLE_TARGET examples_${EXAMPLE_NAME})
  else()
    set(EXAMPLE_TARGET examples_${dir}_${EXAMPLE_NAME})
  endif()

  add_executable(${EXAMPLE_TARGET} ${example_src})

  set_target_properties(${EXAMPLE_TARGET}
    PROPERTIES
    DEBUG_POSTFIX d
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON

    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${EXAMPLE_DIRECTORY}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${EXAMPLE_DIRECTORY}/bin
    )

#       message("Adding target for example: ${example_src}")
  include_directories(${ALLOCORE_INCLUDE_DIR} ${GAMMA_INCLUDE_DIRS}})
  #    message("Gamma : ${GAMMA_INCLUDE_DIRs}")
  add_dependencies(${EXAMPLE_TARGET} al Gamma)
  target_link_libraries(${EXAMPLE_TARGET} al Gamma ${OPENGL_gl_LIBRARY} ${ADDITIONAL_LIBRARIES} ${EXTERNAL_LIBRARIES})

  get_target_property(DLLS_TO_COPY al AL_DLL_LIBRARIES)
  Copy_dlls("${EXAMPLE_DIRECTORY}/bin" "${EXAMPLE_TARGET}" "${EXTENSIONS_DLLS}")

endmacro()

subdirlist(EXAMPLE_DIRS ${CMAKE_CURRENT_LIST_DIR})
foreach(dir ${EXAMPLE_DIRS})
  file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${dir}/*.cpp)

  foreach(example_src ${EXAMPLE_FILES})
    list (FIND EXAMPLES_TO_IGNORE "${example_src}" _index)
    message("${example_src} -- ${_index}")
    if (${_index} EQUAL -1)
      BuildExample("${example_src}" "${dir}")
    else()
      message("Ignoring example ${EXAMPLE_NAME}")
    endif (${_index} EQUAL -1)
  endforeach(example_src)

endforeach(dir)

foreach(example_src ${EXTENSIONS_EXAMPLES})
#    message("Building extension example: ${example_src}")
  get_filename_component(EXAMPLE_DIR "${example_src}" DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${EXAMPLE_DIR}/bin")
  BuildExample("${example_src}" "extensions" )
endforeach()


