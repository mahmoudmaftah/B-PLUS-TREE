#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx; // Mutex for protecting shared resource
int shared_counter = 0;

void increment_counter(int id) {
    for (int i = 0; i < 5; ++i) {
        // Lock the mutex before accessing the shared resource
        std::lock_guard<std::mutex> lock(mtx); 
        ++shared_counter;
        std::cout << "Thread " << id << " incremented counter to " << shared_counter << std::endl;
    }
}

int main() {
    std::thread t1(increment_counter, 1);
    std::thread t2(increment_counter, 2);

    t1.join();
    t2.join();

    std::cout << "Final counter value: " << shared_counter << std::endl;
    return 0;
}
