#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>


void print_system_info() {
    printf("=== System Information ===\n");
    
    printf("\nCPU Info:\n");
    int ret1 = system("lscpu | grep -E 'Model name|CPU\\(s\\)|Thread\\(s\\) per core|Core\\(s\\) per socket|Socket\\(s\\)|NUMA node\\(s\\)'");
    (void)ret1;
    
    printf("\nServer Info:\n");
    int ret2 = system("cat /sys/devices/virtual/dmi/id/product_name 2>/dev/null || echo 'Unknown'");
    (void)ret2;
    
    printf("\nNUMA Info:\n");
    int ret3 = system("numactl --hardware | grep -E 'available|node [0-9] free'");
    (void)ret3;
    
    printf("\nOS Info:\n");
    int ret4 = system("cat /etc/os-release | grep PRETTY_NAME");
    (void)ret4;
    
    printf("\n=========================\n");
}

void *safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprinf(stderr, "error! memory could not be allocated");
        abort;
    }
    return ptr;
}

void parallel_matrix_operation(int size, int nthreads, void (*row_operation)(int, int, double*, double*, double*), double* a, double* b, double* c) {
    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        int items_per_thread = size / nthreads;
        int lb = tid * items_per_thread;
        int ub = (tid == nthreads - 1) ? (size - 1) : (lb + items_per_thread - 1);
        
        for (int i = lb; i <= ub; i++) {
            row_operation(i, size, a, b, c);
        }
    }
}

void init_row(int i, int size, double* a, double* b, double* c) {
    for (int j = 0; j < size; j++) {
        a[i * size + j] = i + j;
    }
    c[i] = 0.0;
}

void matvec_row(int i, int size, double* a, double* b, double* c) {
    c[i] = 0;
    for (int j = 0; j < size; j++) {
        c[i] += a[i * size + j] * b[j];
    }
}

double benchmark_matrix_mult(int matrix_size, int nthreads) {
    double *a = safe_malloc(sizeof(*a) * matrix_size * matrix_size);
    double *b = safe_malloc(sizeof(*b) * matrix_size);
    double *c = safe_malloc(sizeof(*c) * matrix_size);

    parallel_matrix_operation(matrix_size, nthreads, init_row, a, b, c);
    
    for (int j = 0; j < matrix_size; j++) {
        b[j] = j;
    }

    double min_time = 1e10;
    for (int i = 0; i < 5; i++) {
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        
        parallel_matrix_operation(matrix_size, nthreads, matvec_row, a, b, c);
        
        clock_gettime(CLOCK_MONOTONIC, &end);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        min_time = elapsed < min_time ? elapsed : min_time;
    }

    free(a); free(b); free(c);
    return min_time;
}

void run_scalability_test(int matrix_size) {
    int thread_counts[] = {1, 2, 4, 7, 8, 16, 20, 40};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    double base_time = 0.0;
    
    printf("\nMatrix size: %dx%d\n", matrix_size, matrix_size);
    printf("| Threads | Time (sec) | Speedup |\n");
    printf("|---------|------------|---------|\n");
    
    for (int i = 0; i < num_tests; i++) {
        double time = benchmark_matrix_mult(matrix_size, thread_counts[i]);
        
        if (thread_counts[i] == 1) {
            base_time = time;
        }
        
        printf("| %7d | %10.4f | %7.2f |\n", 
               thread_counts[i], time, base_time / time);
    }
}

int main() {
    print_system_info();
    
    printf("\n=== Scalability Analysis ===\n");
    
    run_scalability_test(20000);
    run_scalability_test(40000);
    
    return 0;
}
