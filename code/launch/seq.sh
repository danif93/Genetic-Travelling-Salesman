g++ -std=c++11 -O3 -o seq ../gen_tsp.cpp

numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=3
mutP=0.5             #probability of mutation
earlyStRound=20
earlyStParam=1

./seq $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam input.dat

rm seq