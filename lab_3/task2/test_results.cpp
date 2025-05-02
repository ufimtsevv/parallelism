#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <stdexcept>
#include <iomanip>

const double SIN_EPSILON = 1e-5;
const double SQRT_EPSILON = 1e-5;
const double POW_EPSILON = 1e-5;

void verify_results(const std::string& filename, 
                   const std::string& func_name,
                   double epsilon) 
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    std::string line;
    int line_num = 0;
    while (std::getline(file, line)) {
        line_num++;
        try {
            size_t start = line.find('(');
            size_t end = line.find(')');
            if (start == std::string::npos || end == std::string::npos) {
                throw std::runtime_error("Invalid format");
            }

            std::string args_str = line.substr(start + 1, end - start - 1);
            double result = std::stod(line.substr(line.find('=') + 1));
            double expected;

            if (func_name == "pow") {
                size_t comma = args_str.find(',');
                if (comma == std::string::npos) {
                    throw std::runtime_error("Invalid arguments format for pow");
                }
                double base = std::stod(args_str.substr(0, comma));
                double exp = std::stod(args_str.substr(comma + 1));
                expected = std::pow(base, exp);
            } else if (func_name == "sin") {
                double arg = std::stod(args_str);
                expected = std::sin(arg);
            } else if (func_name == "sqrt") {
                double arg = std::stod(args_str);
                expected = std::sqrt(arg);
            }

            double diff = std::abs(result - expected);
            if (diff > epsilon) {
                std::ostringstream oss;
                oss << std::setprecision(10);
                oss << "Line " << line_num << ": Verification failed for " 
                    << func_name << "(" << args_str << ")\n"
                    << "  Expected: " << expected << "\n"
                    << "  Got:      " << result << "\n"
                    << "  Diff:     " << diff 
                    << " (allowed: " << epsilon << ")";
                throw std::runtime_error(oss.str());
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(filename + ":" + std::to_string(line_num) + " - " + e.what());
        }
    }

    std::cout << func_name << " results verification passed! (" 
              << line_num << " records)\n";
}

int main() {
    try {
        std::cout << "Starting verification with tolerances:\n"
                  << "  sin: " << SIN_EPSILON << "\n"
                  << "  sqrt: " << SQRT_EPSILON << "\n"
                  << "  pow: " << POW_EPSILON << "\n\n";
        
        verify_results("sin_results.txt", "sin", SIN_EPSILON);
        verify_results("sqrt_results.txt", "sqrt", SQRT_EPSILON);
        verify_results("pow_results.txt", "pow", POW_EPSILON);
        
        std::cout << "\nAll tests passed successfully!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTEST FAILED:\n" << e.what() << "\n";
        return 1;
    }
}
