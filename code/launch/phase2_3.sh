rm proj_dani/code/launch/input.dat
rm proj_dani/code/launch/err.txt proj_dani/code/launch/out.txt

g++ -std=c++11 -O3 -o proj_dani/code/launch/gen proj_dani/code/generator.cpp
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi proj_dani/code/gen_tsp.cpp
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi_det proj_dani/code/gen_tsp_detailed.cpp


numThreads=4
#numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=100
mutP=0.5             #probability of mutation
earlyStRound=20
earlyStParam=100

for i in 10 50 100;# 300 500 700 1000 1500 2000;
do
    proj_dani/code/launch/gen $i > proj_dani/code/launch/input.dat
# TOTAL COST
    #sequential on 1 node
    mpiexec -n 1 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi 1 $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    #parallel on 1 node
    mpiexec -n 1 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat    
    #parallel on more nodes
    mpiexec -n 4 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
# DETAILED COST    
    #sequential on 1 node
    mpiexec -n 1 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi_det 1 $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    #parallel on 1 node
    mpiexec -n 1 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi_det $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    #parallel on more nodes
    mpiexec -n 4 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi_det $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    
    rm proj_dani/code/launch/input.dat
done

rm proj_dani/code/launch/mpi proj_dani/code/launch/mpi_det proj_dani/code/launch/gen