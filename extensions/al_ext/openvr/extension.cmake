

find_library(OPENVR_LIB openvr_api
  "${CMAKE_CURRENT_LIST_DIR}/../../../../openvr/lib/win64"
  "${CMAKE_CURRENT_LIST_DIR}/../../../../../openvr/lib/win64"
  )


if (NOT OPENVR_LIB)
  message("OpenVR not found. Not building al_openvr extension.")
else()

  get_filename_component(OPENVR_ROOT ${OPENVR_LIB} DIRECTORY)
  set(OPENVR_ROOT "${OPENVR_ROOT}/../../")
  get_filename_component(OPENVR_ROOT ${OPENVR_ROOT} REALPATH)

  message("Using OpenVR at: ${OPENVR_ROOT}")
  set(OPENVR_INCLUDE_DIR "${OPENVR_ROOT}/headers")

  set(THIS_EXTENSION_SRC
    "${CMAKE_CURRENT_LIST_DIR}/src/al_OpenVRWrapper.cpp"
  )

  set(THIS_EXTENSION_HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/al_VRApp.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/al_OpenVRWrapper.hpp"
  )
  set(THIS_EXTENSION_LIBRARY_NAME al_openvr)

  add_library(${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_SRC} ${THIS_EXTENSION_HEADERS})

  # c++14 It's very important to set this for the extension as not
  # setting it will cause targets upstream to not use c++14
  set_target_properties(${THIS_EXTENSION_LIBRARY_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    )

  set(THIS_EXTENSION_LIBRARIES ${OPENVR_LIB})

  target_link_libraries(${THIS_EXTENSION_LIBRARY_NAME} al ${THIS_EXTENSION_LIBRARIES})

  # Connections to outside
  set(CURRENT_EXTENSION_LIBRARIES ${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_LIBRARIES})
  set(CURRENT_EXTENSION_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR} ${OPENVR_INCLUDE_DIR})
  set(CURRENT_EXTENSION_DEFINITIONS -DAL_EXT_OPENVR)
  set(CURRENT_EXTENSION_DLLS "${OPENVR_ROOT}/bin/win64/openvr_api.dll")

#  message("libs: ${CURRENT_EXTENSION_LIBRARIES}")

#  # unit tests
#  add_executable(soundfileBufferedRecordTests ${CMAKE_CURRENT_LIST_DIR}/unitTests/utSoundfileBufferedRecord.cpp)
#  target_link_libraries(soundfileBufferedRecordTests al ${SPATIALAUDIO_LINK_LIBRARIES} )
#  target_include_directories(soundfileBufferedRecordTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/catch")
#  set_target_properties(soundfileBufferedRecordTests PROPERTIES
#    CXX_STANDARD 14
#    CXX_STANDARD_REQUIRED ON
#    )
#  add_test(NAME soundfileBufferedRecordTests
#    COMMAND $<TARGET_FILE:soundfileBufferedRecordTests> ${TEST_ARGS})

endif(NOT OPENVR_LIB)
