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
    // Test w/ non existent lock
    if (thread_wait(lock, cond) == -1) cout << "Test 1 Passed" << endl;
    else cout << "Test 1 Failed" << endl;

    thread_lock(lock);
    thread_wait(lock, cond); // GOTO Line 29

    // Fail if it gets here
    cout << "Test 3 Failed" << endl;
}

int test_func_two(int num) {
    // Test if thread_wait() released the lock earlier
    if (thread_lock(lock) == 0) cout << "Test 2 Passed" << endl;
    else cout << "Test 2 Failed" << endl;
    thread_unlock(lock);
    thread_lock(lock_one);
    thread_wait(lock_one, cond_one);   // Goto Line 41

    cout << "Test 3 Passed" << endl;
}

int test_func_three(int num) {
    thread_lock(lock_one);
    thread_signal(lock_one, cond_one);

}

void thread_func(int num) {
    lock = 0;
    lock_one = 1;
    cond = 0;
    cond_one = 1;
    thread_create((thread_startfunc_t) test_func_one, (void *) 1);
    thread_create((thread_startfunc_t) test_func_two, (void *) 0);
    thread_create((thread_startfunc_t) test_func_three, (void *) 0);
}

int main() {
    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}



