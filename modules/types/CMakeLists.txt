add_library(altypes ${AL_LIBRARY_TYPE})
add_library(al::types ALIAS altypes)

target_sources(altypes
  PRIVATE
    src/types/al_Color.cpp
    src/types/al_VariantValue.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/types/al_Color.hpp
        include/al/types/al_VariantValue.hpp
)

al_configure_module(altypes EXPORT_NAME types)
target_link_libraries(altypes PUBLIC al::math)
al_install_module(altypes)
