rm proj_dani/code/launch/err.txt proj_dani/code/launch/out.txt

mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi proj_dani/code/gen_tsp.cpp

numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=100
mutP=0.5             #probability of mutation
earlyStRound=20
earlyStParam=100

#for i in $(seq 1 4);
#do
    mpiexec -n 1 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
#done

rm proj_dani/code/launch/mpi