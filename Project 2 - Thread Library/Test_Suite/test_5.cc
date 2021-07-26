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
    // Testing w/ no condition or lock objects
    if (thread_signal(lock, cond) == -1) cout << "Test 1 Failed" << endl;
    else cout << "Test 1 Passed" << endl;

    thread_lock(lock);

    // Testing w/ just no Condition object
    if (thread_signal(lock, cond) == -1) cout << "Test 2 Failed" << endl;
    else cout << "Test 2 Passed" << endl;

    thread_yield();

    if (thread_signal(lock_one, cond) == -1) cout << "Test 4 Failed" << endl;
    else cout << "Test 4 Passed" << endl;

}

int test_func_two(int num) {
    // Testing w/o holding lock
    if (thread_signal(lock, cond) == -1) cout << "Test 3 Failed" << endl;
    else cout << "Test 3 Passed" << endl;

    // Create Condition* object
    thread_wait(lock_one, cond);
}

void thread_func(int num) {
    lock = 0;
    lock_one = 1;
    cond = 0;
    thread_create((thread_startfunc_t) test_func_one, (void *) 1);
    thread_create((thread_startfunc_t) test_func_two, (void *) 0);
}

int main() {
    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}



