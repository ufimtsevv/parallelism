#include <iostream>
#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <random>
#include <fstream>
#include <cmath>
#include <functional>
#include <stop_token>
#include <iomanip>

template<typename T>
class TaskServer {
public:
    using TaskType = std::function<T()>;
    
    TaskServer() : running_(false) {}
    
    ~TaskServer() {
        if (running_) {
            stop();
        }
    }
    
    void start() {
        running_ = true;
        server_thread_ = std::jthread([this](std::stop_token stoken) {
            this->run(stoken);
        });
    }
    
    void stop() {
        if (!running_) return;
        
        running_ = false;
        cv_.notify_all();
        server_thread_.request_stop();
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
    }
    
    size_t add_task(TaskType task) {
        std::packaged_task<T()> pt(task);
        std::future<T> future = pt.get_future();
        
        size_t id;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            id = next_id_++;
            tasks_.emplace(std::move(pt));
        }
        
        {
            std::lock_guard<std::mutex> lock(futures_mutex_);
            futures_[id] = std::move(future);
        }
        
        cv_.notify_one();
        return id;
    }
    
    T request_result(size_t id) {
        std::future<T> future;
        {
            std::lock_guard<std::mutex> lock(futures_mutex_);
            auto it = futures_.find(id);
            if (it == futures_.end()) {
                throw std::runtime_error("Task id not found");
            }
            future = std::move(it->second);
            futures_.erase(it);
        }
        
        return future.get();
    }
    
private:
    void run(std::stop_token stoken) {
        while (!stoken.stop_requested()) {
            std::packaged_task<T()> task;
            
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this, &stoken] {
                    return !tasks_.empty() || stoken.stop_requested();
                });
                
                if (stoken.stop_requested()) {
                    break;
                }
                
                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }
            
            if (task.valid()) {
                task();
            }
        }
    }
    
    std::jthread server_thread_;
    std::queue<std::packaged_task<T()>> tasks_;
    std::map<size_t, std::future<T>> futures_;
    std::mutex mutex_;
    std::mutex futures_mutex_;
    std::condition_variable cv_;
    size_t next_id_ = 0;
    bool running_;
};

#include <cmath>
#include <iomanip>

template<typename T>
void client_function(TaskServer<T>& server, const std::string& task_name, 
                     size_t num_tasks, const std::string& filename) {
    std::ofstream outfile(filename);
    outfile << std::fixed << std::setprecision(12);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    try {
        if (task_name == "sin") {
            std::uniform_real_distribution<T> dis(-3.14, 3.14);
            for (size_t i = 0; i < num_tasks; ++i) {
                T arg = dis(gen);
                size_t id = server.add_task([arg]() { 
                    return std::sin(arg); 
                });
                T result = server.request_result(id);
                outfile << "sin(" << arg << ") = " << result << "\n";
            }
        } 
        else if (task_name == "sqrt") {
            std::uniform_real_distribution<T> dis(0.0, 100.0);
            for (size_t i = 0; i < num_tasks; ++i) {
                T arg = dis(gen);
                size_t id = server.add_task([arg]() { 
                    return std::sqrt(arg); 
                });
                T result = server.request_result(id);
                outfile << "sqrt(" << arg << ") = " << result << "\n";
            }
        } 
        else if (task_name == "pow") {
            std::uniform_real_distribution<T> dis_base(1.0, 10.0);
            std::uniform_real_distribution<T> dis_exp(1.0, 5.0);
            for (size_t i = 0; i < num_tasks; ++i) {
                T base = dis_base(gen);
                T exp = dis_exp(gen);
                size_t id = server.add_task([base, exp]() { 
                    return std::exp(std::log(base) * exp); 
                });
                T result = server.request_result(id);
                outfile << "pow(" << base << ", " << exp << ") = " << result << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in client " << task_name << ": " << e.what() << std::endl;
    }
    
    outfile.close();
}

int main() {
    try {
        TaskServer<double> server;
        server.start();
        
        const size_t num_tasks = 100;
        
        std::thread client1([&server]() {
            client_function(server, "sin", num_tasks, "sin_results.txt");
        });
        
        std::thread client2([&server]() {
            client_function(server, "sqrt", num_tasks, "sqrt_results.txt");
        });
        
        std::thread client3([&server]() {
            client_function(server, "pow", num_tasks, "pow_results.txt");
        });
        
        client1.join();
        client2.join();
        client3.join();
        
        server.stop();
    } catch (const std::exception& e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
