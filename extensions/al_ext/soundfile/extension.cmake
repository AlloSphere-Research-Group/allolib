
#find_library(SNDFILE_LIBRARY
#  NAMES sndfile)

if (NOT SNDFILE_LIBRARY)
  message("libsndfile not found. Not building soundfile extension.")
else()
  message(STATUS "Using libsndfile: ${SNDFILE_LIBRARY}")

  set(THIS_EXTENSION_SRC
    ${CMAKE_CURRENT_LIST_DIR}/src/al_SoundfileBuffered.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/al_SoundfileBufferedRecord.cpp
  #  ${CMAKE_CURRENT_LIST_DIR}/src/al_AmbiFilePlayer.cpp
  )

  set(THIS_EXTENSION_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/al_SoundfileBuffered.hpp
    ${CMAKE_CURRENT_LIST_DIR}/al_SoundfileBufferedRecord.hpp
    ${CMAKE_CURRENT_LIST_DIR}/al_OutputRecorder.hpp
  #  ${CMAKE_CURRENT_LIST_DIR}/al_AmbiFilePlayer.hpp
  )
  set(THIS_EXTENSION_LIBRARY_NAME al_soundfile)

  add_library(${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_SRC} ${THIS_EXTENSION_HEADERS})

  # c++14 It's very important to set this for the extension as not
  # setting it will cause targets upstream to not use c++14
  set_target_properties(${THIS_EXTENSION_LIBRARY_NAME} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    )

  set(THIS_EXTENSION_LIBRARIES ${SNDFILE_LIBRARY} ${GAMMA_LIBRARY})

  target_link_libraries(${THIS_EXTENSION_LIBRARY_NAME} al ${THIS_EXTENSION_LIBRARIES})

  # Connections to outside
  set(CURRENT_EXTENSION_LIBRARIES ${THIS_EXTENSION_LIBRARY_NAME} ${THIS_EXTENSION_LIBRARIES})
  set(CURRENT_EXTENSION_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR})

  message("libs: ${CURRENT_EXTENSION_LIBRARIES}")

  # unit tests
  add_executable(soundfileBufferedRecordTests ${CMAKE_CURRENT_LIST_DIR}/unitTests/utSoundfileBufferedRecord.cpp)
  target_link_libraries(soundfileBufferedRecordTests al ${SPATIALAUDIO_LINK_LIBRARIES} )
  target_include_directories(soundfileBufferedRecordTests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/external/catch")
  set_target_properties(soundfileBufferedRecordTests PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    )
  add_test(NAME soundfileBufferedRecordTests
    COMMAND $<TARGET_FILE:soundfileBufferedRecordTests> ${TEST_ARGS})

endif(NOT SNDFILE_LIBRARY)
