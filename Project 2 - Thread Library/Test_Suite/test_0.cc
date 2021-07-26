#include <iostream>
#include <cstdlib>
#include <assert.h>
#include "thread.h"

using namespace std;

int test_func_one(int num) {
    if (thread_libinit((thread_startfunc_t) test_func_one, (void*) 0) != -1) cout << "Test Two Failed" << endl;
    else cout << "Test 9 Passed" << endl;
}

void thread_func(int num) {
    if (thread_libinit((thread_startfunc_t) test_func_one, (void *) 0) != -1) cout << "Test One Failed\n";
    else cout << "Test 8 Passed\n";
    thread_create((thread_startfunc_t) test_func_one, (void *) 0);
}

int main() {
    if (thread_create((thread_startfunc_t) test_func_one, (void*) 10) == -1) cout << "Test 1 passed\n";
    else cout << "Test 1 failed\n";

    if (thread_yield() == -1) cout << "Test 2 passed\n";
    else cout << "Test 2 failed\n";


    if (thread_lock(1) == -1) cout << "Test 3 passed\n";
    else cout << "Test 3 failed\n";

    if (thread_unlock(1) == -1) cout << "Test 4 passed\n";
    else cout << "Test 4 failed\n";

    if (thread_wait(5, 5) == -1) cout << "Test 5 passed\n";
    else cout << "Test 5 failed\n";

    if (thread_signal(1, 1) == -1) cout << "Test 6 passed\n";
    else cout << "Test 6 failed\n";

    if (thread_broadcast(4, 5) == -1) cout << "Test 7 passed\n";
    else cout << "Test 7 failed\n";

    thread_libinit((thread_startfunc_t) thread_func, (void*) 10);
    exit(0);
}


