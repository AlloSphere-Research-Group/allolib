
list(APPEND CMAKE_MODULE_PATH
${CMAKE_CURRENT_LIST_DIR}
)

find_package(Assimp)

if (NOT ASSIMP_LIBRARY)
  message("Assimp not found. Not building al_assets3d extension.")
else()
#  list(APPEND ADDITIONAL_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
#  list(APPEND ADDITIONAL_LIBRARIES ${ASSIMP_LIBRARY})
  
  set(THIS_EXTENSION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/src/al_Asset.cpp"
  )
  set(THIS_EXTENSION_HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/al_Asset.hpp"
  )

  set(THIS_EXTENSION_LIBRARIES
    ${ASSIMP_LIBRARY}
  )

  set(THIS_EXTENSION_LIBRARY_NAME al_assets3d)

  add_library(${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_SRC} ${THIS_EXTENSION_HEADERS})

  # c++14 It's very important to set this for the extension as not
  # setting it will cause targets upstream to not use c++14
  set_target_properties(${THIS_EXTENSION_LIBRARY_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    )

  target_link_libraries(${THIS_EXTENSION_LIBRARY_NAME} al ${THIS_EXTENSION_LIBRARIES})

  # Connections to outside
  set(CURRENT_EXTENSION_LIBRARIES ${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_LIBRARIES})
  set(CURRENT_EXTENSION_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR} ${ASSIMP_INCLUDE_DIR})

  set(CURRENT_EXTENSION_DEFINITIONS -DAL_EXT_ASSETS3D)
  get_filename_component(ASSIMP_ROOT_DIR "${ASSIMP_LIBRARY}" DIRECTORY)
  set(CURRENT_EXTENSION_DLLS "${ASSIMP_ROOT_DIR}/../../bin/x64/assimp-vc140-mt.dll")

#  message("libs: ${CURRENT_EXTENSION_LIBRARIES}")

endif()
