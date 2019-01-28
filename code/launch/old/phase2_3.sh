rm proj_dani/code/launch/input.dat
rm proj_dani/code/launch/err.txt proj_dani/code/launch/out.txt

g++ -std=c++11 -O3 -o proj_dani/code/launch/gen proj_dani/code/generator.cpp

tries=3
numThreads=4
#numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=100
mutP=0.5             #probability of mutation
earlyStRound=20
earlyStParam=100

########## SEQUENTIAL & PARALLEL MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/seqPar proj_dani/code/source_seqPar/gen_tsp.cpp
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/seqPar_det proj_dani/code/source_seqPar/gen_tsp_detailed.cpp

for i in 10 50 100;# 300 500 700 1000 1500 2000;
do
    proj_dani/code/launch/gen $i > proj_dani/code/launch/input.dat
# TOTAL COST - phase 2
    #sequential on 1 node
    mpiexec -n $tries -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/seqPar 1 $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    #parallel on 1 node
    mpiexec -n $tries -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/seqPar $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat    
# DETAILED COST - phase 3
    #sequential on 1 node
    mpiexec -n $tries -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/seqPar_det 1 $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    #parallel on 1 node
    mpiexec -n $tries -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/seqPar_det $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
     
    rm proj_dani/code/launch/input.dat
done

########## MPI MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi proj_dani/code/source_mpi/gen_tsp.cpp
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi_det proj_dani/code/source_mpi/gen_tsp_detailed.cpp

for i in 10 50 100;# 300 500 700 1000 1500 2000;
    do
    proj_dani/code/launch/gen $i > proj_dani/code/launch/input.dat
    for k in $(seq 1 $tries); do
    # TOTAL COST - phase 2
        #parallel on more nodes
        mpiexec -n 4 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    # DETAILED COST - phase 3 
        #parallel on more nodes
        mpiexec -n 4 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi_det $numThreads $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    done
    rm proj_dani/code/launch/input.dat
done

rm proj_dani/code/launch/seqPar proj_dani/code/launch/seqPar_det proj_dani/code/launch/mpi proj_dani/code/launch/mpi_det proj_dani/code/launch/gen