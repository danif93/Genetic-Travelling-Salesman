#!/bin/bash
#PBS -o out.txt
#PBS -e err.txt
#PBS -l select=3:ncpus=28:mpiprocs=1:ompthreads=28 -l place=scatter:excl

rm proj_HPC/code/launch/cluster/err.txt proj_HPC/code/launch/cluster/out.txt

########## UTILITIES ##########
function Round(){
    echo ${1%%.*}
}

function Compute_Pop_Size(){
    result=$(echo "l(1-e(l($2)/$1))/l(($1-3)/($1-1))" | bc -l)
    result=$(Round $result)
    echo $result
}
########## END UTILITIES ##########

nodes_tries=3
numThreads=28
#numCities=1000
#initialPop=10000
top=0.3              #percentage of top survivor
maxIt=100
mutP=0.5             #probability of mutation
earlyStRound=9
earlyStParam=1

########## SEQUENTIAL & PARALLEL MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_HPC/code/launch/cluster/seqPar proj_HPC/code/source_seqPar/gen_tsp.cpp
#mpic++ -std=c++11 -O3 -fopenmp -o proj_HPC/code/launch/cluster/seqPar_det proj_HPC/code/source_seqPar/gen_tsp_detailed.cpp

for numCities in 100 200 300 400 500 600 700 800 900 1000 2000 5000 9000; do
    pop_prob=0.1 #winning prob
    initialPop=$(Compute_Pop_Size $numCities $pop_prob)

# TOTAL COST - phase 2
    #sequential on 1 node
    mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/seqPar 1 $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/$numCities
    #parallel on 1 node
    mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/seqPar $numThreads $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/$numCities
# DETAILED COST - phase 3  
    #sequential on 1 node
    #mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/seqPar_det 1 $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/$numCities
    #parallel on 1 node
    #mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/seqPar_det $numThreads $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/$numCities
done

########## MPI MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_HPC/code/launch/cluster/mpi proj_HPC/code/source_mpi/gen_tsp.cpp
#mpic++ -std=c++11 -O3 -fopenmp -o proj_HPC/code/launch/cluster/mpi_det proj_HPC/code/source_mpi/gen_tsp_detailed.cpp

for numCities in 100 200 300 400 500 600 700 800 900 1000 2000 5000 9000; do
    for k in $(seq 1 $nodes_tries); do
        pop_prob=0.1 #winning prob
        initialPop=$(Compute_Pop_Size $numCities $pop_prob)
# TOTAL COST - phase 2
        #parallel on more nodes
        mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/mpi $numThreads $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/$numCities
# DETAILED COST - phase 3
        #parallel on more nodes
        #mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/mpi_det $numThreads $numCities $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/$numCities
    done
done

rm proj_HPC/code/launch/cluster/seqPar proj_HPC/code/launch/cluster/mpi 
#rm proj_HPC/code/launch/cluster/seqPar_det proj_HPC/code/launch/cluster/mpi_det 