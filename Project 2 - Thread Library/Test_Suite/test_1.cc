#include <iostream>
#include <cstdlib>
#include <assert.h>
#include "thread.h"
#include <unistd.h>

using namespace std;

int lock_one;
int lock_two;

int test_func_one() {
    thread_lock(lock_one);
    if (thread_lock(lock_one) == -1) cout << "Test 1 Passed" << endl;
    else  cout << "Test 1 Failed" << endl;
    thread_yield(); // GOTO Line 23
    if (thread_lock(lock_one) == -1) cout << "Test 3 Passed" << endl;
    else  cout << "Test 3 Failed" << endl;

}

int test_func_two() {
    thread_lock(lock_two);
    if (thread_lock(lock_two) == -1) cout << "Test 2 Passed" << endl;
    else cout << "Test 2 Passed" << endl;
    thread_yield(); // GOTO Line 17
}

void thread_func(int num) {
    lock_one = 1;
    lock_two = 2;
    thread_create((thread_startfunc_t) test_func_one, (void *) 0);
    thread_create((thread_startfunc_t) test_func_two, (void *) 0);
}

int main() {
    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}



