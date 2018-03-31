# sets:
#   RTAUDIO_INCLUDE_DIR
#   RTAUDIO_SRC
#   RTAUDIO_LIBRARIES
#   RTAUDIO_DEFINITIONS

set(RtAudioLinuxAPI "alsa" CACHE STRING "API For Linux when using RtAudio")
set_property(CACHE RtAudioLinuxAPI PROPERTY STRINGS alsa pulse jack)


if(IS_DIRECTORY "${al_path}/external/rtaudio")
  if (AL_VERBOSE_OUTPUT)
    message("Using RtAudio")
  endif()

  add_definitions(-DAL_AUDIO_RTAUDIO)

  set(RTAUDIO_INCLUDE_DIR external/rtaudio)
  set(RTAUDIO_SRC ${al_path}/external/rtaudio/RtAudio.cpp)

  if(AL_LINUX)
    if(RtAudioLinuxAPI STREQUAL "pulse")
      set(RTAUDIO_DEFINITIONS -D__LINUX_PULSE__)
      list(APPEND RTAUDIO_LIBRARIES
        pulse pulse-simple pthread
        )
    elseif(RtAudioLinuxAPI STREQUAL "alsa")
      set(RTAUDIO_DEFINITIONS -D__LINUX_ALSA__)
      list(APPEND RTAUDIO_LIBRARIES
        asound pthread
        )
    elseif(RtAudioLinuxAPI STREQUAL "jack")
      set(RTAUDIO_DEFINITIONS -D__UNIX_JACK__)
      list(APPEND RTAUDIO_LIBRARIES
        jack pthread
        )
      #  g++ -Wall -D__UNIX_JACK__ -o audioprobe audioprobe.cpp RtAudio.cpp $(pkg-config –cflags –libs jack) -lpthread
    endif()
  endif(AL_LINUX)

  if(AL_MACOS)
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
  endif(AL_MACOS)

  if(AL_WINDOWS)
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
  endif(AL_WINDOWS)
endif() # RTAUDIO
