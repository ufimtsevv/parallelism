#define _USE_MATH_DEFINES

#include <cmath>
#include <iostream>
#include <chrono>

#ifdef USE_FLOAT
using ArrayType = float;
#else
using ArrayType = double;
#endif

int main() {
    const size_t size = 1e7;
    ArrayType* array = new ArrayType[size];
    std::cout << "type: " << typeid(array[0]).name() << std::endl;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < size; ++i) {
        array[i] = std::sin(2 * M_PI * i / size);
    }

    ArrayType sum = 0;
    for (size_t i = 0; i < size; ++i) {
        sum += array[i];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "time: " << elapsed.count() << " sec" << std::endl;
    std::cout << "sum: " << sum << std::endl;

    delete[] array;
    return 0;
}