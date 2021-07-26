#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include "thread.h"
using namespace std;

int lock;
int shared;
int tz = 0;
int to = 0;
int tt = 0;
int foo = 0;
int bar = 0;

int test_func_two(int num) {
    thread_lock(lock); // Should be in waiting queue for lock now, not in ready queue
    bar = 1;
    if (foo) cout << "Test 2 Passed" << endl;
    else {
        cout << "Test 2 Failed" << endl;
        exit(-1);
    }
}

int test_func_one(int num) {
    thread_lock(lock);
    thread_yield(); // GOTO 17
    thread_yield(); // GOTO 30
    if (!bar) foo = 1;
    else {
        cout << "Test 2 Failed" << endl;
        exit(-1);
    }
    thread_unlock(lock);
    thread_yield();
}

void thread_zero(void * test){
    shared = (int) test;
    while (shared == 0) {
        tz = 1;
        thread_yield();
    }
    if (to && tt) cout << "Test 1 Passed" << endl;
    lock = 0;
    thread_create((thread_startfunc_t) test_func_one, (void *) 0);
    thread_create((thread_startfunc_t) test_func_two, (void *) 0);
}

void thread_one(void * test) {
    shared = (int) test;
    while (shared == 0) {
        thread_yield();
    }
    if (tz && !tt) to = 1;
}

void thread_two(void * test) {
    shared = (int) test;
    /*
    if(test==0){
        cout <<"lock " << thread_lock(lock)<<endl;
    }*/
    while (shared == 0) {
        thread_yield();
    }
    /*
    if(test==0){
        cout << "unlock " << thread_unlock(lock) <<endl;
    }*/
    if (tz && to) tt = 1;
}
void manage(void * empty){
    thread_create(&thread_zero,(void *) 0);
    thread_create(&thread_one,(void *) 1);
    thread_create(&thread_two,(void *) 2);
}
int main(){
    thread_libinit(&manage, NULL);
}