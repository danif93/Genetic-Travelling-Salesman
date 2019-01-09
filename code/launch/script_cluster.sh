rm proj_dani/code/launch/input.dat
rm proj_dani/code/launch/err.txt proj_dani/code/launch/out.txt

g++ -std=c++11 -O3 -o proj_dani/code/launch/gen proj_dani/code/generator.cpp
mpic++ -std=c++11 -O3 -fopenmp -o proj_dani/code/launch/mpi proj_dani/code/gen_tsp.cpp

numCities=1000
initialPop=10000
top=0.5              #percentage of top survivor
maxIt=100
mutP=0.5             #probability of mutation
earlyStRound=20
earlyStParam=100

for i in $(seq 10 50 100);# 300 500 700 1000 1500 2000);
do
    proj_dani/code/launch/gen $i > proj_dani/code/launch/input.dat
    mpiexec -n 4 -machinefile proj_dani/code/launch/nodelist proj_dani/code/launch/mpi $i $initialPop $top $maxIt $mutP $earlyStRound $earlyStParam proj_dani/code/launch/input.dat
    rm proj_dani/code/launch/input.dat
done

rm proj_dani/code/launch/mpi proj_dani/code/launch/gen