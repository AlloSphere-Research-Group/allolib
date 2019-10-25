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

macro(BuildExample example_src dir)
  get_filename_component(EXAMPLE_NAME ${example_src} NAME_WE) # Get name w/o extension
  set(example_directory ${CMAKE_CURRENT_LIST_DIR})

  if ("${dir}" STREQUAL ".")
    set(EXAMPLE_TARGET example_${EXAMPLE_NAME})
  else()
    set(EXAMPLE_TARGET example_${dir}_${EXAMPLE_NAME})
  endif()

  add_executable(${EXAMPLE_TARGET} ${example_src})

  set_target_properties(${EXAMPLE_TARGET}
    PROPERTIES
    DEBUG_POSTFIX d
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON

    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${example_directory}/${dir}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${example_directory}/${dir}/bin
    )

  target_link_libraries(${EXAMPLE_TARGET} al)

  #get_target_property(DLLS_TO_COPY al AL_DLL_LIBRARIES)
  #Copy_dlls("${EXAMPLE_DIRECTORY}/bin" "${EXAMPLE_TARGET}" "${EXTENSIONS_DLLS}")

endmacro()

subdirlist(EXAMPLE_DIRS ${CMAKE_CURRENT_LIST_DIR})

foreach(dir ${EXAMPLE_DIRS})
  file(GLOB EXAMPLE_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${dir}/*.cpp)

  foreach(example_src ${EXAMPLE_FILES})
      BuildExample("${example_src}" "${dir}")
  endforeach(example_src)

endforeach(dir)
