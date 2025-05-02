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

void fatal(char *message) {
    fprintf(stderr, "%s", message);
    exit(13);
}

void *safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (ptr == NULL) {
        fprinf(stderr, "error! memory could not be allocated");
        abort;
    }
    return ptr;
}

double cpuSecond() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ((double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9);
}

void matrix_vector_product_omp(double *a, double *b, double *c, int m, int n, int nthreads) {
    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        int items_per_thread = m / nthreads; 
        int lb = tid * items_per_thread;
        int ub = (tid == nthreads - 1) ? (m - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; i++) {
            c[i] = 0;
            for (int j = 0; j < n; j++) {
                c[i] += a[i * m + j] * b[j];
            }
        }
    }
}

double time_check_parallel(int matrix_size, int nthreads) {
    double *a = safe_malloc(sizeof(*a) * matrix_size * matrix_size);
    double *b = safe_malloc(sizeof(*b) * matrix_size);
    double *c = safe_malloc(sizeof(*c) * matrix_size);

    #pragma omp parallel num_threads(nthreads)
    {
        int tid = omp_get_thread_num();
        int items_per_thread = matrix_size / nthreads;
        int lb = tid * items_per_thread;
        int ub = (tid == nthreads - 1) ? (matrix_size - 1) : (lb + items_per_thread - 1);
        
        for (int i = lb; i <= ub; i++) {
            for (int j = 0; j < matrix_size; j++)
                a[i * matrix_size + j] = i + j;
            c[i] = 0.0;
        }
    }
    
    for (int j = 0; j < matrix_size; j++)
        b[j] = j;

    double min_time = 1e10;
    for (int i = 0; i < 5; i++) {
        double start = cpuSecond();
        matrix_vector_product_omp(a, b, c, matrix_size, matrix_size, nthreads);
        double stop = cpuSecond();
        min_time = (min_time < (stop - start)) ? min_time : (stop - start);
    }

    free(a);
    free(b);
    free(c);
    
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
        double time = time_check_parallel(matrix_size, thread_counts[i]);
        
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
