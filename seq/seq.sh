rm input.dat

g++ -std=c++11 -o3 -o ../gen ../generator.cpp
g++ -std=c++11 -o3 -o seq tsp_seq.cpp

numCities=5
initialPop=10
top=0.5 #percentage of top survivor
maxIt=1000
mutP=0.15 #probability of mutation

../gen $numCities > input.dat
./seq $numCities $initialPop $top $maxIt $mutP input.dat

rm ../gen ./seq