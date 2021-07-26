#include <ucontext.h>
#include "interrupt.h"
#include <assert.h>
#include "thread.h"
#include <cstdlib>
#include <iostream>

using namespace std;

// Global Variables
static int initialized = 0;
static int active_threads;
static int ready_size;
static ucontext_t *running_tcb;
static ucontext_t *library_tcb;

// Monitor Class
class BusyNode{
    public:
        ucontext_t * tcb;
        int lock;
        BusyNode * next;
        BusyNode(ucontext_t*, int, BusyNode*);
};

// BusyNode* Constructor
BusyNode::BusyNode(ucontext_t* input_tcb, int input_lock, BusyNode* next_node){
    tcb = input_tcb;
    lock = input_lock;
    next = next_node;
}

/************* Lock Class ***************/
class Lock{
    public:
        int state;
        int id;
        ucontext_t *thread;
        Lock *next;
        BusyNode* head;
};

static Lock *lock_head = NULL;

// Creates a new Lock* object
static Lock * lock_create(int id, ucontext_t* tcb) {
    Lock * temp = (Lock *) malloc(sizeof(Lock));
    temp->state = 1;
    temp->id = id;
    temp->thread = tcb;
    temp->next = NULL;
    temp->head = NULL;
    return temp;
}

// Search the list for a specific lock
static Lock * lock_search(int id){
    Lock * current = lock_head;
    while(current != NULL){
        if(id == current->id){
            //if(node->thread==current->thread) return -1;
            return current;
        }
        current = current->next;
    }
    return NULL;
}

// Add a particular lock to the list
static void lock_add(Lock* node){
    if(lock_head == NULL){
        lock_head = node;
    } else {
        Lock* curr = lock_head;
        while (curr->next != NULL) {
            curr->next = node;
        }
    }
}

// Remove a node from the list
static void lock_remove(Lock* node) {
    if (lock_head == node) lock_head = lock_head->next;
    else {
        Lock* curr = lock_head;
        while (curr->next != node) curr = curr->next;
        curr->next = node->next;
    }
    free(node);
}

/*********** Condition Class ***********/
class Condition{
    public:
        int cond;
        Condition* next;
        BusyNode* head;
};

static Condition *condition_head;
static Condition *condition_tail;

// Add a Condition* object to the list
static void condition_add(Condition *node){
    if (condition_head == NULL){
        condition_head = node;
        condition_tail = node;
    } else {
        condition_tail->next = node;
        condition_tail = node;
    }
}

// Creates a new Condition* object
static Condition * condition_create(int input_cond, BusyNode* waiter) {
    Condition * temp = (Condition *) malloc(sizeof(Condition));
    temp->cond = input_cond;
    temp->head = waiter;
    temp->next = NULL;
    return temp;
}

// Search the list for a specific condition
static Condition* condition_search(int cond){
    Condition * current = condition_head;
    while(current != NULL){
        if(current->cond == cond){
            return current;
        }
        current=current->next;
    }
    return NULL;
}

// Remove a Condition* from the list
static void condition_remove(int cond){
    Condition * current = condition_head;
    if(condition_head->cond == cond){
        if(condition_head == condition_tail){
            condition_tail == NULL;
        }
        Condition *temp = condition_head;
        condition_head = condition_head->next;
        free(temp);
    } else {
        while(current->next->cond != cond){
            current = current->next;
        }
        Condition * temp = current->next;
        if(temp == condition_tail){
            condition_tail = current;
        }
        current->next = temp->next;
        free(temp);
    }
}

/************ ReadyNode Class ************/
class ReadyNode{
    public:
        ucontext_t *tcb;
        ReadyNode *next;
        ReadyNode(ucontext_t*, ReadyNode*);
};

static ReadyNode *ready_head = NULL;
static ReadyNode *ready_tail = NULL;

// ReadyNode* Constructor
ReadyNode::ReadyNode(ucontext_t* input_tcb, ReadyNode* next_node){
    tcb = input_tcb;
    next = next_node;
}

//Add node to FIFO ready queue
static void ready_add(ReadyNode * node){
    if(ready_size == 0) {
        ready_head = node;
        ready_tail = node;
    } else {
        ready_tail->next = node;
        ready_tail = node;
    }
    ready_size++;
}

//Remove the node at the top of the FIFO queue
static ucontext_t * ready_out(){
    assert(ready_size != 0);
    ucontext_t * tcb = NULL;
    ReadyNode *out = NULL;
    out = ready_head;
    ready_head = ready_head->next;
    out->next = NULL;
    ready_size--;
    tcb = out->tcb;
    delete out;
    return tcb;
}

static void print_ready_queue(){
    ReadyNode * current = ready_head;
    while(current != NULL){
        cout << current->tcb << endl;
        current = current->next;
    }
}

static void wrapper_func(int int_func, int int_arg){
    interrupt_disable();
    // Prepare to execute func(arg)
    thread_startfunc_t func = (thread_startfunc_t) int_func;
    void * arg = (void *) int_arg;
    interrupt_enable();

    func(arg);          // Execute func(arg)

    // Free current context & get ready to go back to library_tcb
    interrupt_disable();
    ucontext_t temp;
    ucontext_t* current = running_tcb;
    getcontext(&temp);
    running_tcb = ready_out();
    free(current);
    delete (char *) (temp.uc_stack.ss_sp);
    interrupt_enable();
    setcontext(running_tcb);
}


int thread_libinit(thread_startfunc_t func, void *arg) {
    if(initialized) return -1;  // Check for initialization
    initialized = 1;

    interrupt_disable();
    // Initialize new contexts/TCB objects
    ucontext_t * new_tcb = (ucontext_t *) malloc(sizeof(ucontext_t));
    library_tcb = (ucontext_t *) malloc(sizeof(ucontext_t));

    // Check for malloc() failure
    if (new_tcb == NULL || library_tcb == NULL) {
        cout << "Malloc Failed - Program will now exit" << endl;
        exit(-1);
    }

    // Set library_tcb to current context (which is library_tcb)
    assert(getcontext(library_tcb) != -1);

    // Set current context as the running context
    running_tcb = library_tcb;

    // Set new_tcb to current context
    assert(getcontext(new_tcb) != -1);

    // Initialize internal variables of new_tcb
    char *stack = new char [STACK_SIZE];
    new_tcb->uc_stack.ss_sp = stack;
    new_tcb->uc_stack.ss_size = STACK_SIZE;
    new_tcb->uc_stack.ss_flags = 0;
    new_tcb->uc_link = library_tcb;

    // Create new thread to run func (start func passed by user)
    makecontext(new_tcb, (void (*)()) &wrapper_func, 2, (int) func, (int) arg);

    // Add new_tcb & library_tcb to ready queue
    ready_add(new ReadyNode(new_tcb, NULL));
    ready_add(new ReadyNode(library_tcb, NULL));

    // Set running_tcb to new_tcb
    running_tcb = ready_out();
    interrupt_enable();

    // Swap context over to new tcb (new_tcb)
    assert(swapcontext(library_tcb, running_tcb) == 0);

    interrupt_disable();
    // Every time the thread ends, it will come back here & check for next thread to run
    // If there is a new thread to run, it will run that new thread
    while(ready_size > 0){
        ready_add(new ReadyNode(library_tcb, NULL));
        running_tcb = ready_out();
        interrupt_enable();
        swapcontext(library_tcb, running_tcb);
        interrupt_disable();
    }

    // Free library_tcb object
    free(library_tcb);

    // Exit thread library
    cout << "Thread library exiting." << endl;
    exit(0);
    return 0;
}

int thread_create(thread_startfunc_t func, void *arg) {
    if(!initialized) return -1;     // Check that library thread has been initialized

    interrupt_disable();

    // Create new tcb object
    ucontext_t * new_tcb = (ucontext_t *) malloc(sizeof(ucontext_t));

    // Check for malloc() failure
    if (new_tcb == NULL) {
        cout << "Malloc Failed - Program will now exit" << endl;
        exit(-1);
    }

    // PreReq for makecontext()
    assert(getcontext(new_tcb) != -1);
    char *stack = new char [STACK_SIZE];
    new_tcb->uc_stack.ss_sp = stack;
    new_tcb->uc_stack.ss_size = STACK_SIZE;
    new_tcb->uc_stack.ss_flags = 0;
    new_tcb->uc_link = library_tcb;

    // Create new thread
    makecontext(new_tcb, (void (*)()) &wrapper_func, 2, (int) func, (int) arg);

    // Add new thread to ready queue
    ready_add(new ReadyNode(new_tcb, NULL));
    interrupt_enable();
    return 0;
}

int thread_yield(void) {
    if(!initialized) return -1;     // Check that library thread has been initialized

    interrupt_disable();
    // Copy running_tcb to temp
    ucontext_t *temp = running_tcb;

    // Checks that there is another thread to run (other than library thread)
    if(ready_size > 1){
        ready_add(new ReadyNode(running_tcb, NULL));
        running_tcb = ready_out();
        interrupt_enable();
        swapcontext(temp, running_tcb);
        interrupt_disable();
    }
    interrupt_enable();
    return 0;
}

int thread_lock(unsigned int lock){
    if(!initialized) return -1;     // Check that library thread has been initialized

    interrupt_disable();

    Lock* lock_node = lock_search(lock);
    if (lock_node == NULL) {
        lock_node = lock_create(lock, running_tcb);
        if (lock_node == NULL) {    // Check for malloc() failure
            interrupt_enable();
            return -1;
        }
        lock_add(lock_node);
        interrupt_enable();
        return 0;
    } else {
        if (lock_node->thread == running_tcb) { // If caller already holds lock
            interrupt_enable();
            return -1;
        } else if (lock_node->head == NULL) {   // If there are no threads waiting
            lock_node->head = new BusyNode(running_tcb, lock, NULL);
        } else {                                // If there are threads waiting
            BusyNode* curr = lock_node->head;
            while (curr->next != NULL) curr = curr->next;
            curr->next = new BusyNode(running_tcb, lock, NULL);
        }
    }
    ucontext_t * temp = running_tcb;
    running_tcb = ready_out();
    interrupt_enable();
    swapcontext(temp, running_tcb);
    interrupt_disable();
    interrupt_enable();
    return 0;
}

int thread_unlock(unsigned int lock) {
    if(!initialized) return -1;     // Check that library thread has been initialized

    interrupt_disable();
    Lock* lock_node = lock_search(lock);
    if (lock_node == NULL || lock_node->thread != running_tcb || lock_node->state == 0) {
        interrupt_enable();
        return -1;
    }

    if (lock_node->head != NULL) {      // There are threads waiting on lock
        BusyNode* temp = lock_node->head;
        lock_node->head = temp->next;               // Update waiting queue
        ready_add(new ReadyNode(temp->tcb, NULL));  // Add next thread onto ready queue
        lock_node->thread = temp->tcb;              // Handoff lock
        delete temp;                                // Delete BusyNode* object
    } else lock_remove(lock_node);                  // There are no threads waiting on lock

    interrupt_enable();
    return 0;
}

int thread_wait(unsigned int lock, unsigned int cond){
    if(!initialized) return -1;     // Check that library thread has been initialized
    if (thread_unlock(lock) == -1) return -1;

    interrupt_disable();
    Condition* condition = condition_search(cond);
    BusyNode* waiter = new BusyNode(running_tcb, lock, NULL);

    if (condition != NULL) {    // Condition exists; threads are waiting on this condition
        BusyNode* curr = condition->head;
        while (curr->next != NULL) curr = curr->next;
        curr->next = waiter;
    } else {                    // Condition doesn't exist
        condition = condition_create(cond, waiter);
        condition_add(condition);
    }
    if (waiter->tcb != NULL) {
        running_tcb = ready_out();
        interrupt_enable();
        swapcontext(waiter->tcb, running_tcb);
        interrupt_disable();
    }
    delete waiter;
    interrupt_enable();
    return 0;
}

int thread_signal(unsigned int lock, unsigned int cond){
    if(!initialized) return -1;     // Check that library thread has been initialized

    interrupt_disable();
    Lock* lock_held = lock_search(lock);
    Condition* condition = condition_search(cond);
    if (lock_held == NULL || lock_held->thread != running_tcb || condition == NULL) {
        interrupt_enable();
        return 0;
    }
    BusyNode* prev;
    BusyNode* curr = condition->head;
    while (curr->lock != lock) {
        prev = curr;
        curr = curr->next;
    }
    if (curr == NULL) {     // No thread waiting on this lock
        interrupt_enable();
        return 0;
    }
    ready_add(new ReadyNode(curr->tcb, NULL));
    lock_held->thread = curr->tcb;

    if (curr == condition->head) {
        condition->head = curr->next;
        if (condition->head == NULL) condition_remove(cond);
    } else prev->next = curr->next;

    interrupt_enable();
    return 0;
}

int thread_broadcast(unsigned int lock, unsigned int condition){
    if(!initialized) return -1;     // Check that library thread is initialized

    interrupt_disable();
    Condition* cond = condition_search(condition);
    if (cond == NULL) return 0;     // No such condition exists

    // Save current last node on ready list
    ReadyNode* new_node = ready_tail;

    // Add to ready queue and remove from cond waiting queue all threads waiting on Lock* lock
    BusyNode* prev;
    BusyNode* curr = cond->head;
    while (curr != NULL) {
        if (curr->lock == lock) {
            ready_add(new ReadyNode(curr->tcb, NULL));
            curr->tcb = NULL;
            if (curr == cond->head) {
                cond->head = curr->next;
                curr = cond->head;
                continue;
            }
            prev->next = curr->next;
            curr = curr->next;
            continue;
        }
        prev = curr;
        curr = curr->next;
    }
    // If there are no more threads waiting on this cond
    if (cond->head == NULL) condition_remove(condition);

    // Set to first node added to ready queue
    new_node = new_node->next;
    if (new_node == NULL) {
        interrupt_enable();
        return 0;
    }

    Lock* lock_node = lock_search(lock);
    if (lock_node == NULL) { // Lock does not exist
        lock_node = lock_create(lock, new_node->tcb);
        lock_add(lock_node);
        new_node = new_node->next;
    }
    curr = new BusyNode(new_node->tcb, lock, NULL);
    if (lock_node->head == NULL) {      // If lock has no threads waiting
        lock_node->head = curr;
    } else {                            // If lock has threads waiting
        prev = lock_node->head;
        while (prev->next != NULL) prev = prev->next;
        prev->next = curr;
    }
    new_node = new_node->next;
    while (new_node != NULL) {
        curr->next = new BusyNode(new_node->tcb, lock, NULL);
        curr = curr->next;
        new_node = new_node->next;
    }

    interrupt_enable();
    return 0;
}

//Need to implement monitor classes for lock, unlock, wait, signal, and broadcast

