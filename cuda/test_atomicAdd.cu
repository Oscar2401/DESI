#include <iostream>
#include <stdlib.h>
#include <stdio.h>


__global__ void kernel(float XX, float *data) {
    //int bin = 0;
    float sum = 2.4f;
    if (data[threadIdx.x]<15){
        printf("%f", sum);
        atomicAdd(&XX,sum);
        printf("%f", XX);
    }
}

int main(){
    float XX=0;
    float *data;
    cudaMallocManaged(&data, 50*sizeof(float));
    //cudaMallocManaged(&XX, 10*sizeof(float));
    for (int i=0 ; i<50 ;i++){
        data[i] = i*0.5;
    }
    /*
    for (int i=0 ; i<10 ;i++){
        XX[i] = 0.0;
    }
    */
    kernel<<<1,50>>>(XX, data);

    std::cout << XX << std::endl;

    return 0;
}