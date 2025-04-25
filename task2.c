#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#include <time.h>


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

double f(double x) {
    return sin(x);
}

double integrate_omp_atomic(double a, double b, int nsteps, int nthreads) {
    double h = (b - a) / nsteps;
    double sum = 0.0;
    
    #pragma omp parallel num_threads(nthreads)
    {
        double local_sum = 0.0;
        #pragma omp for
        for (int i = 0; i < nsteps; i++) {
            double x = a + (i + 0.5) * h;
            local_sum += f(x);
        }
        
        #pragma omp atomic
        sum += local_sum;
    }
    
    return sum * h;
}

double measure_performance(double a, double b, int nsteps, int nthreads) {
    double min_time = 1e10;
    double result;
    
    for (int i = 0; i < 5; i++) {
        double start = omp_get_wtime();
        result = integrate_omp_atomic(a, b, nsteps, nthreads);
        double end = omp_get_wtime();
        min_time = (end - start < min_time) ? (end - start) : min_time;
    }
    
    return min_time;
}

int main() {
    print_system_info();
    
    const double a = 0.0;
    const double b = M_PI;
    const int nsteps = 40000000;
    const int threads[] = {1, 2, 4, 7, 8, 16, 20, 40};
    const int num_threads = sizeof(threads)/sizeof(threads[0]);
    
    double reference = -cos(b) - (-cos(a));
    printf("\nReference value: %.15f\n", reference);
    
    double test_result = integrate_omp_atomic(a, b, 1000, 1);
    printf("Sanity check (1000 steps): %.15f (error: %e)\n", 
           test_result, fabs(test_result - (-cos(b) - (-cos(a)))));
    
    printf("\n=== Performance Analysis ===\n");
    printf("| Threads | Time (s) | Speedup |\n");
    printf("|---------|----------|---------|\n");
    
    double base_time = 0.0;
    for (int i = 0; i < num_threads; i++) {
        double time = measure_performance(a, b, nsteps, threads[i]);
        
        if (threads[i] == 1) {
            base_time = time;
        }
        
        printf("| %7d | %8.4f | %7.2f |\n", 
               threads[i], time, base_time / time);
    }
    
    return 0;
}