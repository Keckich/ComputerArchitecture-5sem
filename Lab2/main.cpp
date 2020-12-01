#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <condition_variable>
#include <atomic>

using namespace std;

mutex c_mutex;
const int num_tasks_1 = 1024 * 1024;
int num_tasks_2 = 4 * 1024 * 1024;
int mutex_counter = 0;
atomic<int> atomic_counter(0);

class Queue1 {
private:
    mutex _mutex;
    list<uint8_t> _list;
    atomic<int> producer_num{};
public:
    explicit Queue1(int producer_num) {
        this->producer_num = producer_num;
    }

    // Записывает элемент в очередь.
    // Если очередь фиксированного размер и заполнена,
    // поток повисает внутри функции пока не освободится место
    void push(uint8_t val) {
        _mutex.lock();
        _list.push_back(val);
        _mutex.unlock();
    }

    // Если очередь пуста, ждем 1 мс записи в очередь.
    // Если очередь не пуста, помещает значение головы в val,
    // удаляет голову и возвращает true.
    // Если очередь по прежнему пуста, возвращаем false
    bool pop(uint8_t &val) {
        _mutex.lock();
        if (_list.empty()) {
            _mutex.unlock();
            this_thread::sleep_for(chrono::milliseconds(1));
            _mutex.lock();
        }
        if (!_list.empty()) {
            val = _list.front();
            _list.pop_front();
            _mutex.unlock();
            return true;
        }
        _mutex.unlock();
        return false;
    }

    bool check_elements() const {
        if (producer_num == 0) {
            return true;
        }
        return false;
    }

    void producer_ends() {
        producer_num--;
    }
};

class Queue2 {
private:
    list<uint8_t> _list;
    int size;
    mutex _mutex;
    condition_variable cv;
    atomic<int> producer_num{};
public:

    explicit Queue2(int size, int producer_num) {
        this->size = size;
        this->producer_num = producer_num;
    }

    void push(uint8_t val) {
        unique_lock<mutex> lock(_mutex);
        cv.wait(lock, [this]() { return _list.size() < size; });
        _list.push_back(val);
        cv.notify_one();
    }

    bool pop(uint8_t &val) {
        unique_lock<mutex> lock(_mutex);
        if (_list.empty()) {
            cv.wait_for(lock, chrono::milliseconds(1), [this] { return _list.size() < size; });
        }
        if (!_list.empty()) {
            val = _list.front();
            _list.pop_front();
            cv.notify_one();
            return true;
        }
        return false;
    }

    bool check_elements() const {
        if (producer_num == 0) {
            return true;
        }
        return false;
    }

    void producer_ends() {
        producer_num--;
    }

};

template<typename T>
class Operation {
public:
    static void producer(T &queue) {
        for (int i = 0; i < num_tasks_2; i++) {
            queue.push(1);
        }
        queue.producer_ends();
    }

    static void consumer(T &queue, int &result) {
        int counter = 0;
        uint8_t val = 0;
        while (!queue.check_elements()) {
            while (queue.pop(val)) {
                counter += val;
            }
        }
        result += counter;
    }
};

template<typename T>
class Task {
public:
    static void task(T &queue, int p_num, int c_num, int size = -1) {
        int result = 0;
        auto start = chrono::high_resolution_clock::now();
        result = 0;
        vector<thread> _producer;
        vector<thread> _consumer;
        _producer.reserve(p_num);
        _consumer.reserve(c_num);
        for (int j = 0; j < p_num; j++) {
            _producer.emplace_back(Operation<T>::producer, ref(queue));
        }
        for (int j = 0; j < c_num; j++) {
            _consumer.emplace_back(Operation<T>::consumer, ref(queue), ref(result));
        }
        for (thread &t: _producer) {
            t.join();
        }
        for (thread &t: _consumer) {
            t.join();
        }
        if (size != -1) {
            cout << endl << "ProducerNum = " << p_num << ", ConsumerNum = " << c_num << ", QSize = " << size
                 << " : "
                 << result << endl;
        } else {
            cout << endl << "ProducerNum = " << p_num << ", ConsumerNum = " << c_num << " : " << result << endl;
        }
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<float> duration = end - start;
        cout << "Duration: " << duration.count() << endl;
    }
};

void increment(int *result, const string &method, bool sleep) {
    int counter;
    while (true) {
        if (method == "mutex") {
            c_mutex.lock();
            counter = mutex_counter++;
            c_mutex.unlock();
        } else {
            counter = atomic_counter++;
        }
        if (counter >= num_tasks_1) break;
        result[counter]++;
        if (sleep) this_thread::sleep_for(chrono::nanoseconds(10));
    }
}

void task_1(const string &method, bool sleep) {
    int mistakes = 0;
    int num_threads[5] = {2, 4, 8, 16, 32};
    for (int num_thread : num_threads) {
        mutex_counter = 0;
        atomic_counter = 0;
        int result[num_tasks_1] = {0};
        auto start = chrono::high_resolution_clock::now();
        vector<thread> threads;
        threads.reserve(num_thread);
        for (int j = 0; j < num_thread; j++) {
            threads.emplace_back(increment, result, method, sleep);
        }
        for (thread &t: threads) {
            t.join();
        }
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<float> duration = end - start;
        cout << endl << "Duration for " << num_thread << " threads:" << duration.count() << endl;
        for (int i : result) {
            if (i != 1) {
                mistakes++;
            }
        }
        cout << endl << "Mistakes:" << mistakes << endl;
    }
}

int main() {
    double task_n;
    int producer_num[3] = {1, 2, 4};
    int consumer_num[3] = {1, 2, 4};
    int q_size[3] = {1, 4, 16};
    cout << "Task number: ";
    cin >> task_n;

    if (task_n == 1) {
        string method;
        cout << "Method: ";
        cin >> method;
        if (method == "mutex") {
            cout << "Without sleep:" << endl;
            task_1(method, false);
            cout << "With sleep:" << endl;
            task_1(method, true);
        } else {
            cout << "Without sleep:" << endl;
            task_1("atomic", false);
            cout << "With sleep:" << endl;
            task_1("atomic", true);
        }

    } else if (task_n == 2.1) {
        for (int p_num : producer_num) {
            for (int c_num : consumer_num) {
                Queue1 queue1(p_num);
                Task<Queue1>::task(queue1, p_num, c_num);
            }
        }
    } else if (task_n == 2.2) {
        for (int p_num : producer_num) {
            for (int c_num : consumer_num) {
                for (int size : q_size) {
                    Queue2 queue2(size, p_num);
                    Task<Queue2>::task(queue2, p_num, c_num, size);
                }
            }
        }

    } else {
        cout << "Wrong task" << endl;
    }
    return 0;
}
