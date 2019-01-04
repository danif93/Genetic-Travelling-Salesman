rm proj_dani/code/launch/input.dat

g++ -std=c++11 -O3 -o proj_dani/code/launch/gen proj_dani/code/generator.cpp
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi proj_dani/code/gen_tsp.cpp

numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=100
mutP=0.8             #probability of mutation
earlyStRound=20
earlyStParam=10

proj_dani/code/launch/gen $numCities > proj_dani/code/launch/input.dat
mpiexec -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat

rm proj_dani/code/launch/gen proj_dani/code/launch/mpi