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
    // Test (lock == NULL && cond == NULL)
    if (thread_signal(lock, cond) == 0) cout << "Test 1 Passed" << endl;
    else cout << "Test 1 Failed" << endl;

    thread_lock(lock); // Create Lock* for lock; hold lock

    // Test (lock && cond == NULL)
    if (thread_signal(lock, cond) == 0) cout << "Test 2 Passed" << endl;
    else cout << "Test 2 Failed" << endl;

    thread_lock(lock);
    thread_wait(lock, cond);    // Goto Line 29
}

int test_func_two(int num) {
    // Test if thread_wait() released the lock
    if (thread_lock(lock) == -1) cout << "Test 3 Failed" << endl;

    // Test (lock == cond)
    if (thread_signal(lock_one, cond) == 0) cout << "Test 3 Passed" << endl;
    else cout << "Test 3 Failed" << endl;

    thread_lock(lock_one);
    thread_wait(lock_one, cond);    // Goto Line 45

    cout << "Test 5 Passed" << endl;
}

int test_func_three(int num) {
    // Test if lock is held by someone else, should not succeed but should not return -1 b/c Mesa Monitor
    if (thread_signal(lock, cond) == 0) cout << "Test 4 Passed" << endl;
    else cout << "Test 4 Failed" << endl;

    thread_lock(lock_one);
    thread_signal(lock_one, cond_one);
}

void thread_func(int num) {
    lock = 0;
    cond = 0;
    lock_one = 1;
    cond_one = 1;
    thread_create((thread_startfunc_t) test_func_one, (void *) 1);
    thread_create((thread_startfunc_t) test_func_two, (void *) 2);
    thread_create((thread_startfunc_t) test_func_three, (void *) 4);

    thread_lock(lock_one);
    thread_wait(lock_one, cond_one);
    cout << "Test Passed" << endl;
}

int main() {
    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}



