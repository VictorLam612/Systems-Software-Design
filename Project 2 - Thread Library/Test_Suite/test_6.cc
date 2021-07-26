#include <iostream>
#include <cstdlib>
#include <assert.h>
#include "thread.h"
#include <unistd.h>

using namespace std;

int lock;
int lock_one;
int cond;



int test_func_one(int num) {
    cout << "start " << num << endl;
    thread_lock(lock);
    thread_wait(lock, cond);

    cout << "Thread " << num << " Signaled" << endl;
    thread_unlock(lock);
}

int test_func_two(int num) {
    cout << "start " << num << endl;
    thread_lock(lock_one);
    thread_wait(lock_one, cond);

    cout << "Thread " << num << " Signaled" << endl;
    thread_unlock(lock_one);
}

int test_func_three(int num) {
    cout << "starting Broadcast calling function" << endl;

    thread_lock(lock_one);
    thread_yield();
    cout << "thread should be waiting" << endl;

    thread_broadcast(lock_one, cond);
    cout << "thread_unlock(lock_one): " << thread_unlock(lock_one) << endl;
    thread_yield();

}

int test_func_four(int num) {
    cout << "thread waiting on lock_one" << endl;
    thread_lock(lock_one);

    thread_unlock(lock_one);
}

void thread_func(int num) {
    lock = 0;
    lock_one = 1;
    cond = 0;
    for (int i = 0; i < 10; i++) thread_create((thread_startfunc_t) test_func_one, (void *) i);
    for (int i = 10; i < 20; i++) thread_create((thread_startfunc_t) test_func_two, (void *) i);
    thread_create((thread_startfunc_t) test_func_three, (void *) num);
    thread_create((thread_startfunc_t) test_func_four, (void *) num);
}

int main() {
    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}



