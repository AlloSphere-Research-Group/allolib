
message("Flags. cmake")

# other directories to include. You can use relative paths to the
# source file being built.
set(app_include_dirs extra
)

# other libraries to link
set(app_link_libs
)

# definitions. Prepend -D to any defines
set(app_definitions
    -DUSE_COLOR
)

# compile flags
# This flag ignores unused variable warning. You probably don't want to do this...
set(app_compile_flags
    -Wno-unused-variable
)

# linker flags, with `-` in the beginning
set(app_linker_flags
)
