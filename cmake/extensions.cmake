message("** Processing extensions **")

macro(SUBDIRLIST result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

subdirlist(SUBDIRS "${al_path}/extensions/al_ext")

#clear all extension variables in case they are not set
set(EXTENSIONS_INCLUDE_DIRS "${al_path}/extensions")


foreach(subdir ${SUBDIRS})
  if (EXISTS "${al_path}/extensions/al_ext/${subdir}/extension.cmake")
    message(" * Extension: ${subdir}")

    set(CURRENT_EXTENSION_LIBRARIES)
    set(CURRENT_EXTENSION_LINK_DIRS)
    set(CURRENT_EXTENSION_INCLUDE_DIRS)
    set(CURRENT_EXTENSION_DEFINITIONS)
    set(CURRENT_EXTENSION_EXAMPLES)
    set(CURRENT_EXTENSION_DLLS) # Only used on Windows currently


    include("${al_path}/extensions/al_ext/${subdir}/extension.cmake")
    list(APPEND EXTENSIONS_LIBRARIES ${CURRENT_EXTENSION_LIBRARIES})
    list(APPEND EXTENSIONS_LINK_DIRS ${CURRENT_EXTENSION_LINK_DIRS})
    list(APPEND EXTENSIONS_INCLUDE_DIRS ${CURRENT_EXTENSION_INCLUDE_DIRS})
    list(APPEND EXTENSIONS_DEFINITIONS ${CURRENT_EXTENSION_DEFINITIONS})
    list(APPEND EXTENSIONS_EXAMPLES ${CURRENT_EXTENSION_EXAMPLES})
    list(APPEND EXTENSIONS_DLLS ${CURRENT_EXTENSION_DLLS})
#    message("--**-- ${subdir} : ${CURRENT_EXTENSION_LIBRARIES}")
#    message("---- ${CURRENT_EXTENSION_EXAMPLES}")
  endif()
endforeach()
message("** DONE Processing extensions **")

