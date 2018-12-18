#include <iostream>
#include <stdlib.h>
#include <cmath>

using namespace std;

int main(int argc, const char* argv[]){
    if (argc<2){
        cerr << "need 1 args: nodes number\n";
        return 1;
    }

    srand(time(NULL));

    int rnd_val,numNodes,i,j;
    
    numNodes = atoi(argv[1]);

    for (j=0; j<numNodes; j++) {
        cout << j << ' ' << j << ' ' << 0 <<endl;
        for (i=j+1; i<numNodes; i++) {
            rnd_val = rand()%100+1; // 1 to 100
            if (rnd_val > 80)
                rnd_val = rnd_val*2;
            cout << j << ' ' << i << ' ' << rnd_val <<endl;
        } 
    }
    return 0;
}