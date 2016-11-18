[ ] support for std::vector in the places for al::Buffer

[ ] support for std::array in the places for al::Array

[ ] remove opengl implementations in al::Graphics (leave only enums?)

[ ] use matrix stack (cpu side) for al::Graphics -> divide graphics to graphics & glgraphics

[ ] platform definitions (AL_OSX, etc.) are originally done in cmake. do we want it do be done in c++? (maybe in al_Config.hpp)

mark fuctions that needs to be commented out temporarily with:
`/* !LATER! */`