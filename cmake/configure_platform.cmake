# sets:
#   MACOS || LINUX || WINDOWS_MINGW || WINDOWS
#   PLATFORM_DEFINITION

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(AL_MACOS 1)
  message("MACOS")
  # add_definitions(-DAL_OSX)
  set(PLATFORM_DEFINITION -DAL_OSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(AL_LINUX 1)
  message("LINUX")
  # add_definitions(-DAL_LINUX)
  set(PLATFORM_DEFINITION -DAL_LINUX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  # if (MSYS)
    # set(WINDOWS_MINGW 1)
    # message("WINDOWS_MINGW")
    # add_definitions(-DAL_WINDOWS_MSYS)
    # set(PLATFORM_DEFINITION -DAL_WINDOWS_MSYS)
  # elseif (MSVC) # Visual Studio
    set(AL_WINDOWS 1)
    message("WINDOWS")
    # add_definitions(-DAL_WINDOWS)
    set(PLATFORM_DEFINITION -DAL_WINDOWS)
  # else ()
    #
  # endif()
else ()
  # ???
endif ()

if (AL_WINDOWS)
  set(al_homepath $ENV{HOMEPATH})
else ()
  set(al_homepath $ENV{HOME})
endif ()