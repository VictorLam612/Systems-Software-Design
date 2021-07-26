#include <iostream>
#include <cstdlib>
#include <assert.h>
#include "thread.h"
#include <unistd.h>

using namespace std;

int lock_zero;
int lock_one;

int test_func_one(int num) {
    thread_lock(lock_zero);
    thread_yield(); // GOTO Line 23
    if (thread_unlock(lock_one) == -1) cout << "Test 2 Passed" << endl;
    else cout << "Test 2 Failed" << endl;

    if (thread_unlock(lock_zero) == -1) cout << "Test 3 Failed" << endl;
    else cout << "Test 3 Passed" << endl;
}

int test_func_two(int num) {
    if (thread_unlock(lock_one) == -1) cout << "Test 1 Passed" << endl;
    else cout << "Test 1 Failed" << endl;

    thread_lock(lock_one);
    thread_yield(); // GOTO Line 15
    if (thread_unlock(lock_one) == -1) cout << "Test 4 Failed" << endl;
    else cout << "Test 4 Passed" << endl;

    if (thread_unlock(999) == -1) cout << "Test 5 Passed" << endl;
    else cout << "Test 5 Failed" << endl;
}

void thread_func(int num) {
    lock_zero = 0;
    lock_one = 1;
    thread_create((thread_startfunc_t) test_func_one, (void *) 1);
    thread_create((thread_startfunc_t) test_func_two, (void *) 0);
}

int main() {
    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}



