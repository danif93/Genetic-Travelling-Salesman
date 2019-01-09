rm proj_dani/code/launch/input.dat

g++ -std=c++11 -O3 -o proj_dani/code/launch/gen proj_dani/code/generator.cpp

proj_dani/code/launch/gen $numCities > proj_dani/code/launch/input.dat

rm proj_dani/code/launch/gen