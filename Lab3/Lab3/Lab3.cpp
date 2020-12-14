#include <iostream>
#include <omp.h>
#include <chrono>

using namespace std;

double* arr_generation(double* arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += rand() % 30;
    }
    return arr;
}

int main() {
    int n = 100000;

    auto* a = new double[n] {0};
    auto* b = new double[n] {0};
    
    a = arr_generation(a, n);
    b = arr_generation(b, n);

    auto* res_parallel = new double[n] {0};
    auto* res = new double[n] {0};
    int i, j, threads;
    
    auto start = chrono::high_resolution_clock::now();
#pragma omp parallel //shared(a,b,res_parallel) 
    {
        threads = omp_get_num_threads();
#pragma omp for private(i, j)
        for (i = 0; i < n; i++) {
            for (j = 0; j < n - i; j++) {
                res_parallel[i] += a[j] * b[j + i];
            }            
        }    
    
    }  

    auto end = chrono::high_resolution_clock::now();
    chrono::duration<float> duration = end - start;
    cout << "Thread num = " << threads << ": " << duration.count() << endl;

    auto start2 = chrono::high_resolution_clock::now();
    threads = omp_get_num_threads();
    for (i = 0; i < n; i++) {
        for (j = 0; j < n - i; j++) {
                res[i] += a[j] * b[j + i];
        }
    }   
    
    auto end2 = chrono::high_resolution_clock::now();
    chrono::duration<float> duration2 = end2 - start2;
    cout << endl << "Thread num = " << threads << ": " << duration2.count() << endl;

    bool correct = true;
    for (int i = 0; i < n; i++) {
        if (res_parallel[i] != res[i]) {
            correct = false;
            cout << "There are some mistakes in res_parallel" << endl;
            break;
        }    
    }

    if (correct) cout << "No mistakes were found" << endl;

    delete[] a;
    delete[] b;
    delete[] res;
    delete[] res_parallel;
    return 0;
}
