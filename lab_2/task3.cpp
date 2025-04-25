#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>
#include <string.h>

#define MATRIX_SIZE 10000
#define MAX_ITER 10000
#define EPSILON 1e-5
#define ITERATION_STEP (1.0/100000.0)


void print_system_info() {
    printf("=== System Information ===\n");
    
    int ret;
    printf("\nCPU Info:\n");
    ret = system("lscpu | grep -E 'Model name|CPU\\(s\\)|Thread\\(s\\) per core|Core\\(s\\) per socket|Socket\\(s\\)|NUMA node\\(s\\)'");
    (void)ret;
    
    printf("\nServer Info:\n");
    ret = system("cat /sys/devices/virtual/dmi/id/product_name 2>/dev/null || echo 'Unknown'");
    (void)ret;
    
    printf("\nNUMA Info:\n");
    ret = system("numactl --hardware | grep -E 'available|node [0-9] free'");
    (void)ret;
    
    printf("\nOS Info:\n");
    ret = system("cat /etc/os-release | grep PRETTY_NAME");
    (void)ret;
    
    printf("\n=========================\n");
}

long double* create_vector() {
    return (long double*)aligned_alloc(64, MATRIX_SIZE * sizeof(long double));
}

long double* create_matrix() {
    return (long double*)aligned_alloc(64, MATRIX_SIZE * MATRIX_SIZE * sizeof(long double));
}

void initialize(long double* A, long double* b, long double* x) {
    #pragma omp parallel for
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        b[i] = MATRIX_SIZE + 1.0;
        x[i] = 0.0;
        for (size_t j = 0; j < MATRIX_SIZE; j++) {
            A[i * MATRIX_SIZE + j] = (i == j) ? 2.0 : 1.0;
        }
    }
}

long double vector_norm(const long double* v) {
    long double norm = 0.0;
    #pragma omp parallel for reduction(+:norm)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        norm += v[i] * v[i];
    }
    return sqrtl(norm);
}

void solve_system(long double* A, long double* b, long double* x, int threads, int version) {
    long double* tmp = create_vector();
    long double b_norm = vector_norm(b);
    long double rel_residual;
    int iter = 0;
    
    if (version == 1) {
        // Метод 1: Отдельные parallel for
        do {
            #pragma omp parallel for num_threads(threads)
            for (size_t i = 0; i < MATRIX_SIZE; i++) {
                tmp[i] = -b[i];
                for (size_t j = 0; j < MATRIX_SIZE; j++) {
                    tmp[i] += A[i * MATRIX_SIZE + j] * x[j];
                }
            }
            
            rel_residual = vector_norm(tmp) / b_norm;
            
            #pragma omp parallel for num_threads(threads)
            for (size_t i = 0; i < MATRIX_SIZE; i++) {
                x[i] -= ITERATION_STEP * tmp[i];
            }
            
            iter++;
        } while (rel_residual > EPSILON && iter < MAX_ITER);
    } else {
        // Метод 2: Единая parallel секция
        #pragma omp parallel num_threads(threads)
        {
            while (iter < MAX_ITER) {
                #pragma omp for
                for (size_t i = 0; i < MATRIX_SIZE; i++) {
                    tmp[i] = -b[i];
                    for (size_t j = 0; j < MATRIX_SIZE; j++) {
                        tmp[i] += A[i * MATRIX_SIZE + j] * x[j];
                    }
                }
                
                #pragma omp single
                {
                    rel_residual = vector_norm(tmp) / b_norm;
                    iter++;
                }
                
                if (rel_residual <= EPSILON) break;
                
                #pragma omp for
                for (size_t i = 0; i < MATRIX_SIZE; i++) {
                    x[i] -= ITERATION_STEP * tmp[i];
                }
            }
        }
    }
    
    printf("Method %d: %d iterations, residual: %.3Le\n", version, iter, rel_residual);
    free(tmp);
}

void check_solution(const long double* x) {
    long double error = 0.0;
    #pragma omp parallel for reduction(+:error)
    for (size_t i = 0; i < MATRIX_SIZE; i++) {
        error += fabsl(x[i] - 1.0L);
    }
    printf("Average error: %.3Le\n", error/MATRIX_SIZE);
}

int main() {
    print_system_info();
    
    long double* A = create_matrix();
    long double* b = create_vector();
    long double* x = create_vector();
    
    initialize(A, b, x);
    
    int threads[] = {1, 2, 4, 8, 16, 32, 40};
    int num_tests = sizeof(threads)/sizeof(threads[0]);
    
    for (int i = 0; i < num_tests; i++) {
        printf("\nThreads: %d\n", threads[i]);
        
        for (int version = 1; version <= 2; version++) {
            memset(x, 0, MATRIX_SIZE * sizeof(long double));
            double start = omp_get_wtime();
            solve_system(A, b, x, threads[i], version);
            printf("Time: %.3f sec\n", omp_get_wtime() - start);
            check_solution(x);
        }
    }
    
    free(A); free(b); free(x);
    return 0;
}