rm proj_dani/code/launch/err.txt proj_dani/code/launch/out.txt

mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi proj_dani/code/gen_tsp.cpp

numThreads=4
numCities=10
initialPop=10
top=0.5              #percentage of top survivor
maxIt=5
mutP=0.5             #probability of mutation
earlyStRound=4
earlyStParam=1

mpiexec -n 1 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $numThreads $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat

rm proj_dani/code/launch/mpi