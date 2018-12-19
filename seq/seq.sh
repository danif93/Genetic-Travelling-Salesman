rm input.dat

g++ -std=c++11 -o3 -o ../gen ../generator.cpp
g++ -std=c++11 -o3 -o seq tsp_seq.cpp

numCities=10
initialPop=10
top=0.5         #percentage of top survivor
maxIt=2000
mutP=0.8       #probability of mutation
earlyStRound=100
earlyStParam=1

../gen $numCities > input.dat
./seq $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam cost

rm ../gen ./seq