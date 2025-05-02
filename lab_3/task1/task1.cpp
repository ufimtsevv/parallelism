#include <iostream>
#include <vector>
#include <deque>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace std;
using namespace chrono;

template<typename Func>
double measure_time(Func func) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    return duration_cast<duration<double>>(end - start).count();
}

template<typename Container>
void initialize_parallel(Container& matrix, Container& vector, size_t size, size_t threads_num) {
    auto init_part = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            for (size_t j = 0; j < size; ++j) {
                matrix[i * size + j] = (i + j) % 100;
            }
            vector[i] = i % 100;
        }
    };

    std::vector<std::thread> threads;
    size_t chunk_size = size / threads_num;
    
    for (size_t i = 0; i < threads_num; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == threads_num - 1) ? size : start + chunk_size;
        threads.emplace_back(init_part, start, end);
    }

    for (auto& t : threads) {
        t.join();
    }
}

template<typename Container>
Container multiply_parallel(const Container& matrix, const Container& vector, 
                          size_t size, size_t threads_num) {
    Container result(size);
    
    auto multiply_part = [&](size_t start, size_t end) {
        for (size_t i = start; i < end; ++i) {
            double sum = 0;
            for (size_t j = 0; j < size; ++j) {
                sum += matrix[i * size + j] * vector[j];
            }
            result[i] = sum;
        }
    };

    std::vector<std::thread> threads;
    size_t chunk_size = size / threads_num;
    
    for (size_t i = 0; i < threads_num; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == threads_num - 1) ? size : start + chunk_size;
        threads.emplace_back(multiply_part, start, end);
    }

    for (auto& t : threads) {
        t.join();
    }
    
    return result;
}

template<typename Container>
void test_container(const string& container_name, size_t matrix_size, 
                   const vector<size_t>& threads_counts) {
    cout << "Testing " << container_name << " with size " << matrix_size << "x" << matrix_size << endl;
    cout << "Threads\tInit Time (s)\tMult Time (s)\tTotal Time (s)\tSpeedup" << endl;

    double single_thread_time = 0;
    
    for (size_t threads : threads_counts) {
        Container matrix(matrix_size * matrix_size);
        Container vector(matrix_size);
        Container result;

        double init_time = measure_time([&]() {
            initialize_parallel(matrix, vector, matrix_size, threads);
        });

        double mult_time = measure_time([&]() {
            result = multiply_parallel(matrix, vector, matrix_size, threads);
        });

        double total_time = init_time + mult_time;
        
        if (threads == 1) {
            single_thread_time = total_time;
        }
        double speedup = single_thread_time / total_time;

        cout << threads << "\t" 
             << fixed << setprecision(4) 
             << init_time << "\t" 
             << mult_time << "\t" 
             << total_time << "\t" 
             << speedup << endl;
    }
    cout << endl;
}

int main() {
    vector<size_t> sizes = {20000, 40000};
    vector<size_t> threads_counts = {1, 2, 4, 7, 8, 16, 20, 40};

    for (size_t size : sizes) {
        cout << "=============================================" << endl;
        cout << " MATRIX SIZE: " << size << "x" << size << endl;
        cout << "=============================================" << endl;
        
        test_container<vector<double>>("std::vector", size, threads_counts);
        test_container<deque<double>>("std::deque", size, threads_counts);
    }

    return 0;
}
