rm proj_dani/code/launch/err.txt proj_dani/code/launch/out.txt

########## UTILITIES ##########
function Round(){
    echo ${1%%.*}
}

function Compute_Pop_Size(){
    result=$(echo "l(1-e(l($2)/$1))/l(($1-3)/($1-1))" | bc -l)
    result=$(Round $result)
    echo $result
}
########## END ##########

tries=3
numThreads=4
numCities=10
#initialPop=10000
#top=0.5              #percentage of top survivor
maxIt=100
mutP=0.5             #probability of mutation
earlyStRound=20
earlyStParam=100

########## COST GRAPH GENERATION ##########
rm proj_dani/code/launch/input.dat
g++ -std=c++11 -O3 -o proj_dani/code/launch/gen proj_dani/code/generator.cpp
proj_dani/code/launch/gen $numCities > proj_dani/code/launch/input.dat

########## SEQUENTIAL & PARALLEL MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/seqPar proj_dani/code/source_seqPar/gen_tsp.cpp

for i in 7 8 9; #5 6 7 8 9;
do
    pop_prob=$(echo "$i/10" | bc -l)
    initialPop=$(Compute_Pop_Size $numCities $pop_prob)
    for j in 0.3 0.4 0.5; do
        #sequential on 1 node
        mpiexec -n $tries -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/seqPar 1 $numCities $initialPop $j $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
        #parallel on 1 node
        mpiexec -n $tries -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/seqPar $numThreads $numCities $initialPop $j $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat    
    done
done

########## MPI MULTIPLE EXECUTION ##########
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi proj_dani/code/source_mpi/gen_tsp.cpp

for k in $(seq 1 $tries); do
    echo $k
    for i in 7 8 9; #5 6 7 8 9;
    do
        pop_prob=$(echo "$i/10" | bc -l)
        initialPop=$(Compute_Pop_Size $numCities $pop_prob)
        for j in 0.3 0.4 0.5; do
            #parallel on more nodes
            mpiexec -n 4 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $numThreads $numCities $initialPop $j $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
        done
    done
done

rm proj_dani/code/launch/seqPar proj_dani/code/launch/mpi proj_dani/code/launch/gen