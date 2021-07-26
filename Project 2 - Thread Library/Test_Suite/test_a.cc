#include <iostream>
#include <cstdlib>
#include <assert.h>
#include "thread.h"
#include <unistd.h>

using namespace std;

int lock;
int lock_one;
int cond;
int cond_one;

int test_func_one(int num) {
    thread_lock(lock);
    cout << "test_func_one" << endl;
    thread_wait(lock, cond);
    cout << "test_func_one exiting" << endl;
}

int test_func_two(int num) {
    thread_lock(lock);
    thread_lock(lock_one);
    cout << "test_func_two" << endl;
    thread_wait(lock_one, cond);
    cout << "test_func_two exiting" << endl;
}

int test_func_three(int num) {
    thread_lock(lock_one);
    cout << "thread_signal()" << endl;
    thread_signal(lock_one, cond);
    cout << "Post Signal" << endl;
}

void thread_func(int num) {
    lock = 0;
    cond = 0;
    lock_one = 1;
    cond_one = 1;
    thread_create((thread_startfunc_t) test_func_one, (void *) 1);
    thread_create((thread_startfunc_t) test_func_two, (void *) 2);
    thread_create((thread_startfunc_t) test_func_three, (void *) 3);
}

int main() {
    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}



