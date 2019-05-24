
# Other useful variable available are:

message("binary dir: ${CMAKE_CURRENT_BINARY_DIR}")
message("file full path: ${file_full_path}")
message("app path: ${app_path}")
message("app name: ${app_name}")
message("al path: ${al_path}")

# You can set verbose cmake messages:
set(AL_VERBOSE_OUTPUT 1)

# other directories to include. You can use relative paths to the
# source file being built.
set(app_include_dirs)

# other libraries to link
set(app_link_libs tcc)

# definitions. Prepend -D to any defines
set(app_definitions -DUSE_COLOR)

# compile flags
# This flag ignores unused variable warning. You probably don't want to do this...
set(app_compile_flags -Wno-unused-variable)

# linker flags, with `-` in the beginning
set(app_linker_flags -L/usr/local/lib)
