
list(APPEND CMAKE_MODULE_PATH
${CMAKE_CURRENT_LIST_DIR}
)

# TODO check for openvr module

#if (NOT ASSIMP_LIBRARY)
#  message("OpenVR not found. Not building al_openvr extension.")
#else()
#  list(APPEND ADDITIONAL_INCLUDE_DIRS ${ASSIMP_INCLUDE_DIR})
#  list(APPEND ADDITIONAL_LIBRARIES ${ASSIMP_LIBRARY})
  
  set(THIS_EXTENSION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/src/al_App.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/al_ComputationDomain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/al_GraphicsDomain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/al_AudioDomain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/al_OSCDomain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/al_OpenVRDomain.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/al_StateDistributionDomain.cpp"
  )

  set(THIS_EXTENSION_HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/al_App.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_ComputationDomain.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_GraphicsDomain.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_AudioDomain.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_OSCDomain.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_OpenVRDomain.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_StateDistributionDomain.hpp"
  )

  set(THIS_EXTENSION_LIBRARIES
  )

  set(THIS_EXTENSION_LIBRARY_NAME al_distributed)

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
  set(CURRENT_EXTENSION_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})
  set(CURRENT_EXTENSION_DLLS "${OPENVR_ROOT}/bin/win64/openvr_api.dll")

  set(CURRENT_EXTENSION_EXAMPLES
    "${CMAKE_CURRENT_LIST_DIR}/examples/app-domains.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/examples/app-vr.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/examples/distributedapp.cpp"
    )

  message("libs: ${CURRENT_EXTENSION_LIBRARIES}")

#endif()
