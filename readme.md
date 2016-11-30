[ ] support for std::vector in the places for al::Buffer

[ ] support for std::array in the places for al::Array

[ ] use matrix stack (cpu side) for al::Graphics -> divide graphics to graphics & glgraphics

[v] platform definitions (AL_OSX, etc.) are originally done in cmake. do we want it do be done in c++? (maybe in al_Config.hpp) --> "LETS NOT"

mark fuctions that needs to be commented out temporarily with:
`/* !LATER! */`

al_Memory -> smart pointer? not added for now because of APR dependency
[v] al_Time to use c++ std time classes/functions