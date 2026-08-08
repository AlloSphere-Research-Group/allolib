add_library(alspatial ${AL_LIBRARY_TYPE})
add_library(al::spatial ALIAS alspatial)

target_sources(alspatial
  PRIVATE
    src/spatial/al_HashSpace.cpp
    src/spatial/al_Pose.cpp
  PUBLIC
    FILE_SET HEADERS
      BASE_DIRS include
      FILES
        include/al/spatial/al_HashSpace.hpp
        include/al/spatial/al_Pose.hpp
        include/al/spatial/al_Curve.hpp
        include/al/spatial/al_DistAtten.hpp
)

al_configure_module(alspatial EXPORT_NAME spatial)
target_link_libraries(alspatial PUBLIC al::math)
al_install_module(alspatial)
