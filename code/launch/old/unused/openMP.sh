g++ -std=c++11 -O3 -Xpreprocessor -fopenmp -lomp -o omp ../gen_tsp.cpp

numThreads=4
numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=1000
mutP=0.5             #probability of mutation
earlyStRound=20
earlyStParam=10

./omp $numThreads $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam input.dat

rm omp

