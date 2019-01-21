#!/bin/bash
#PBS -o out
#PBS -e err
#PBS -l select=6:ncpus=28:mpiprocs=1:ompthreads=28 -l place=scatter:excl

rm proj_HPC/code/launch/cluster/err.txt

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

nodes_tries=6
numThreads=28
numCities=10
#initialPop=10000
#top=0.5              #percentage of top survivor
maxIt=10
mutP=0.5             #probability of mutation
earlyStRound=9
earlyStParam=1

########## COST GRAPH GENERATION ##########
rm proj_HPC/code/launch/cluster/inputs/input_phase1.dat
g++ -std=c++11 -O3 -o proj_HPC/code/launch/cluster/gen proj_HPC/code/generator.cpp
proj_HPC/code/launch/cluster/gen $numCities > proj_HPC/code/launch/cluster/inputs/input_phase1.dat

########## SEQUENTIAL & PARALLEL MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_HPC/code/launch/cluster/seqPar proj_HPC/code/source_seqPar/gen_tsp.cpp

for i in 1 2 3; do
    pop_prob=$(echo "$i/10" | bc -l)
    initialPop=$(Compute_Pop_Size $numCities $pop_prob)
    for j in 0.3 0.4 0.5; do
        #sequential on 1 node
        mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/seqPar 1 $numCities $initialPop $j $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/input_phase1.dat
        #parallel on 1 node
        mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/seqPar $numThreads $numCities $initialPop $j $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/input_phase1.dat    
    done
done

########## MPI MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_HPC/code/launch/cluster/mpi proj_HPC/code/source_mpi/gen_tsp.cpp

for k in $(seq 1 $tries); do
    for i in 1 2 3; do
        pop_prob=$(echo "$i/10" | bc -l)
        initialPop=$(Compute_Pop_Size $numCities $pop_prob)
        for j in 0.3 0.4 0.5; do
            #parallel on more nodes
            mpiexec -n $nodes_tries proj_HPC/code/launch/cluster/mpi $numThreads $numCities $initialPop $j $maxIt $mutP $earlyStRound $earlyStParam proj_HPC/code/launch/cluster/inputs/input_phase1.dat
        done
    done
done

rm proj_HPC/code/launch/cluster/seqPar proj_HPC/code/launch/cluster/mpi proj_HPC/code/launch/cluster/gen