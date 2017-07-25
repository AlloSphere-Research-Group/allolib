# sets:
#   RTAUDIO_INCLUDE_DIR
#   RTAUDIO_SRC
#   RTAUDIO_LIBRARIES
#   RTAUDIO_DEFINITIONS


if(IS_DIRECTORY "${al_path}/external/rtaudio")
  message("Using RtAudio")
  add_definitions(-DAL_AUDIO_RTAUDIO)

  set(RTAUDIO_INCLUDE_DIR ${al_path}/external/rtaudio)
  set(RTAUDIO_SRC external/rtaudio/RtAudio.cpp)

  if(LINUX)
    # add_definitions(-D__LINUX_PULSE__)
    set(RTAUDIO_DEFINITIONS -D__LINUX_PULSE__)
    list(APPEND RTAUDIO_LIBRARIES
      pulse pulse-simple pthread
    )
  #  add_definitions(-D__LINUX_ALSA__)
  #  list(APPEND RTAUDIO_LIBRARIES
  #    asound pthread
  #  )
  #  g++ -Wall -D__LINUX_OSS__ -o audioprobe audioprobe.cpp RtAudio.cpp -lpthread
  #  g++ -Wall -D__UNIX_JACK__ -o audioprobe audioprobe.cpp RtAudio.cpp $(pkg-config –cflags –libs jack) -lpthread
  endif(LINUX)

  if(MACOS)
    # add_definitions(-D__MACOSX_CORE__)
    set(RTAUDIO_DEFINITIONS -D__MACOSX_CORE__)
    find_library(COREAUDIO_LIB CoreAudio)
    find_library(COREFOUNDATION_LIB CoreFoundation)
    list(APPEND RTAUDIO_LIBRARIES
      ${COREAUDIO_LIB}
      ${COREFOUNDATION_LIB}
      pthread
    )
  #  g++ -Wall -D__MACOSX_CORE__ -o audioprobe audioprobe.cpp RtAudio.cpp -framework CoreAudio -lpthread
  endif(MACOS)

  if(WINDOWS)
    # add_definitions(-D__WINDOWS_WASAPI__)
    set(RTAUDIO_DEFINITIONS -D__WINDOWS_WASAPI__)
    list(APPEND RTAUDIO_LIBRARIES
      ole32
      winmm
      ksuser
      uuid
    )
  #  g++ -Wall -D__WINDOWS_DS__ -o audioprobe audioprobe.cpp RtAudio.cpp -lole32 -lwinmm -ldsound
  #  g++ -Wall -D__WINDOWS_ASIO__ -Iinclude -o audioprobe audioprobe.cpp RtAudio.cpp asio.cpp asiolist.cpp asiodrivers.cpp iasiothiscallresolver.cpp -lole32
  #  g++ -Wall -D__WINDOWS_WASAPI__ -Iinclude -o audioprobe audioprobe.cpp RtAudio.cpp -lole32 -lwinmm -lksuser -luuid
  endif(WINDOWS)
endif() # RTAUDIO
