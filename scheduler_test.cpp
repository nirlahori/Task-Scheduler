#include <iostream>
#include <chrono>
#include <thread>
#include "task_scheduler2.hpp"


using callable = int (*) (int, int);

struct sum{
    int operator () (int a, int b){
        return a + b;
    }
};

int divide(int a, int b){
    return a / b;
}

using into_t = int (*) (int, int);

int into(int a, int b){
    return a * b;
}

auto minus {[](int a, int b) -> int {
   return a - b;
}};


void print_status(unsigned int id, execution_status e){
    std::cout << "id: " << id << ", status: " << static_cast<unsigned int>(e) << std::endl;
}
/*
void thread1(scheduler<int, int, int>& sc, int v1, int v2, unsigned int secs, callable c){
    std::function<int(int, int)> fn{c};
    auto id = sc.submit(std::move(fn), std::move(v1), std::move(v2), secs);
    std::cout << "t1 got id: " << id << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    int res;
    std::cout << "Getting result for id:" << id << ", ";
    sc.get_result_of(id, res);
    std::cout << "result: " << res << std::endl;
}

void thread2(scheduler<int, int, int>& sc, int v1, int v2, unsigned int secs){
    sum s;
    auto id = sc.submit(std::move(s), std::move(v1), std::move(v2), secs);
    std::cout << "t2 got id: " << id << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    int res;
    std::cout << "Getting result for id:" << id << ", ";
    sc.get_result_of(id, res);
    std::cout << "result: " << res << std::endl;
}

void thread3(scheduler<int, int, int>& sc, int v1, int v2, unsigned int secs){
    auto id = sc.submit(std::move([](int x, int y){return x + y;}), std::move(v1), std::move(v2), secs);
    std::cout << "t3 got id: " << id << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    int res;
    std::cout << "Getting result for id:" << id << ", ";
    sc.get_result_of(id, res);
    std::cout << "result: " << res << std::endl;
}*/

void thread4(scheduler<int, int, int>& sc,  unsigned int secs){
    using fptr = int (*) (int, int);
    fptr fdiv{divide};
    //fptr f2(into);
    //fptr f3{minus};



    into_t mulptr {into};
    //sum s;
    //fptr fsum {s};



    auto f1id = sc.submit(std::move(fdiv), 100, 200, secs);
    auto f2id = sc.submit(std::move(mulptr), 800, 240, secs + 3);
    auto f3id = sc.submit(std::move(minus), 900, 45, secs + 5);
    auto f4id = sc.submit(std::move(sum()), 1200, 780, secs + 12);

    std::cout << "f1id: " << f1id << std::endl;
    std::cout << "f2id: " << f2id << std::endl;
    std::cout << "f3id: " << f3id << std::endl;
    std::cout << "f4id: " << f4id << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(9));

    print_status(f1id, sc.status(f1id));
    print_status(f2id, sc.status(f2id));
    print_status(f3id, sc.status(f3id));
    print_status(f4id, sc.status(f4id));

    std::this_thread::sleep_for(std::chrono::seconds(3));

    print_status(f1id, sc.status(f1id));
    print_status(f2id, sc.status(f2id));
    print_status(f3id, sc.status(f3id));
    print_status(f4id, sc.status(f4id));

    std::this_thread::sleep_for(std::chrono::seconds(4));

    print_status(f1id, sc.status(f1id));
    print_status(f2id, sc.status(f2id));
    print_status(f3id, sc.status(f3id));
    print_status(f4id, sc.status(f4id));

    std::this_thread::sleep_for(std::chrono::seconds(4));

    print_status(f1id, sc.status(f1id));
    print_status(f2id, sc.status(f2id));
    print_status(f3id, sc.status(f3id));
    print_status(f4id, sc.status(f4id));

    std::this_thread::sleep_for(std::chrono::seconds(6));

    print_status(f1id, sc.status(f1id));
    print_status(f2id, sc.status(f2id));
    print_status(f3id, sc.status(f3id));
    print_status(f4id, sc.status(f4id));

    std::this_thread::sleep_for(std::chrono::seconds(55));

    auto res { sc.get_result_of(f1id)};
    std::cout << res.first << std::endl;
    res = sc.get_result_of(f2id);
    std::cout << res.first << std::endl;
    res = sc.get_result_of(f3id);
    std::cout << res.first << std::endl;
    res = sc.get_result_of(f4id);
    std::cout << res.first << std::endl;
}

int main(){


    scheduler<int, int, int> sh1;

    /*std::thread th1{thread1, std::ref(sh1), 10, 10, 2, plus};
    std::thread th2{thread2, std::ref(sh1), 23, 12, 5};
    std::thread th3{thread3, std::ref(sh1), 35, 8, 8};*/
    std::thread th4{thread4, std::ref(sh1), 5};

    std::thread thstart {&scheduler<int, int, int>::start, std::ref(sh1)};

    /*th1.join();
    th2.join();
    th3.join();*/
    th4.join();
    thstart.join();

    return 0;
}
