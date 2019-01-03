rm input.dat

g++ -std=c++11 -O3 -o gen ../generator.cpp
g++ -std=c++11 -O3 -Xpreprocessor -fopenmp -lomp -o omp ../gen_tsp.cpp

numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=1000
mutP=0.8             #probability of mutation
earlyStRound=20
earlyStParam=10

./gen $numCities > input.dat
./omp $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam input.dat

rm gen omp