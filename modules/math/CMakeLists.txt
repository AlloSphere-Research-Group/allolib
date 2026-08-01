add_library(almath ${AL_LIBRARY_TYPE})
add_library(al::math ALIAS almath)

# Same explicit set as main/devel historically listed.
# Other headers under include/al/math/ still exist and are used by examples
# (e.g. al_Random.hpp); revisit FILE_SET completeness + types::Conversion later.
target_sources(almath
  PRIVATE
    src/math/al_StdRandom.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/math/al_Constants.hpp
        include/al/math/al_Mat.hpp
        include/al/math/al_Matrix4.hpp
        include/al/math/al_Quat.hpp
        include/al/math/al_StdRandom.hpp
        include/al/math/al_Vec.hpp
)

al_configure_module(almath EXPORT_NAME math)
al_install_module(almath)
