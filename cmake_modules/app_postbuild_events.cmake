# copy dlls
# APP_OUTPUT_PATH needs to be set before this script

if (WINDOWS)
  set(post_build_command
    robocopy ${lib_path}/apr/ ${APP_OUTPUT_PATH} libapr-1.dll &
    robocopy ${lib_path}/glew/ ${APP_OUTPUT_PATH} glew32.dll &
    robocopy ${lib_path}/glfw/ ${APP_OUTPUT_PATH} glfw3.dll &
  )

  if (USE_PORTAUDIO)
  	list(APPEND post_build_command
  	  robocopy ${lib_path}/portaudio/ ${APP_OUTPUT_PATH} portaudio_x64.dll &
  	)
  endif (USE_PORTAUDIO)

  list(APPEND post_build_command
  	IF %ERRORLEVEL% LEQ 1 exit 0
  )

  add_custom_command(
    TARGET ${app_name}
    POST_BUILD
    COMMAND ${post_build_command}
  )
endif (WINDOWS)