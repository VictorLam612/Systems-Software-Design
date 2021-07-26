#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string.h>
#include "thread.h"
#include "interrupt.h"

using namespace std;

#define page_size 128
#define

// Global Variables
int request_max;		// Max number of requests inqueue = # of living requesters
char** input_files;		// Input files
int request_threads;	// # of living threads
int num_files;			// Number of files
int queue_size;			// Current size of queue
int lock;
int request_condition;
int service_condition;
int manage_condition;

// Class def of Doubly Linked List for I/O Request Queue
class RequestNode{
    public:
        int track, requester;
        RequestNode* next;
        RequestNode* last;
     	RequestNode(int, int, RequestNode*, RequestNode*);
     	void remove();
};

RequestNode* head;		// Head of LL

// Define Constructor: RequestNode(int, int, RequestNode*, RequestNode*)
RequestNode::RequestNode(int track_in, int requester_in, RequestNode* next_node, RequestNode* last_node){
    track = track_in;
    requester = requester_in;
    next = next_node;
    last = last_node;
}

// Define remove();
void RequestNode::remove(){
    if (last != NULL) last->next = next;
	else head = next;

    if (next != NULL) next->last = last;
    last = NULL;
    next = NULL;
}


// Check if queue is full
int full(){
    if(queue_size == request_max || queue_size >= request_threads) return 1;
    return 0;
}

// Check if queue contains an I/O request from a specific thread
int contains(int requester){
    RequestNode * temp;
    temp = head;
    while(temp != NULL){
        if(temp->requester == requester) return 1;
        temp=temp->next;
    }
    return 0;
}


// Service thread function to execute I/O requests
void service_thread(void * input){
    int current_track = 0;
    int distance;

    while (request_threads != 0){
        thread_lock(lock);
        while(!full()){
            thread_wait(lock,service_condition);
        } 
        //Thread has lock and queue is ready to be serviced
        if(head == NULL) break;
        
        RequestNode* curr = head;
		RequestNode* foo = NULL; 
        distance = 1000;
        
		while (curr != NULL) {
			if (abs(curr->track - current_track) < distance) {
				distance = abs(curr->track - current_track);
				foo = curr;
			}
            curr = curr->next;
        }

        current_track = foo->track;
        (*foo).remove();
        queue_size--;
        cout << "service requester " << foo->requester << " track " << current_track << endl;
        free(foo);
        thread_unlock(lock);
        thread_broadcast(lock, request_condition);
    }
    thread_unlock(lock);
    thread_yield();
}

// Requester threads to parse input files and add to queue
void requester_thread(int requester){
    RequestNode* request;
    string line;
    ifstream input;
    input.open(input_files[requester]);
    
    while(getline(input, line)){
        int track = atoi(line.c_str());

        thread_lock(lock);
        
        while(full() || contains(requester)){
            thread_wait(lock, request_condition);
        }

        //Should have the lock
        request = (RequestNode *) malloc(sizeof(RequestNode));
        request->track = track;
        request->requester = requester;
        request->next = head;
        request->last = NULL;
        if(head != NULL) head->last = request;
        head = request;
        queue_size++;
        cout << "requester " << requester << " track " << track << endl;
        thread_unlock(lock);
        thread_signal(lock,service_condition);
    }
    input.close();
    
    thread_lock(lock);
    while (contains(requester)) {
        thread_wait(lock, request_condition);
    }
    request_threads--;
    thread_unlock(lock);
    thread_signal(lock, manage_condition);
    thread_signal(lock, service_condition);
    thread_yield();
}

// thread_libinit() function
void manage_threads(void * input){
    queue_size = 0;
    head = NULL;
    lock = 0;
    manage_condition = 2;
    request_condition = 1;
    service_condition = 0;
    thread_create((thread_startfunc_t) service_thread, (void *) NULL);
    for(int i = 0; i < request_threads; i++) {
        thread_create((thread_startfunc_t) requester_thread, (void *) i);
    }
    thread_lock(lock);
    while(request_threads != 0){
        thread_wait(lock, manage_condition);
    }

    //cleanup
    for (int i = 0; i < num_files; i++) {
        free(input_files[i]);
    }
    free(input_files);
    thread_unlock(lock);
    thread_yield();
    return;
}

// Main function
int main(int argc, char** argv){
    if(argc < 3){
        cout << "Please specify input files for disk requests\n";
        exit(1);
    }
    request_max = atoi(argv[1]);
    request_threads = argc - 2;
    num_files = argc-2;
    

    //need to allocate space for num_files file names
    input_files = (char**) malloc(sizeof(char *) * request_threads);

    for(int i = 2; i < argc; i++){

        //each file name in input_files gets each char mallocced
        input_files[i - 2] = (char*) malloc(sizeof(char)*(strlen(argv[i]) + 1));
        strcpy(input_files[i - 2], argv[i]);
    }
    //Initialing thread library with our main thread manager
    thread_libinit((thread_startfunc_t) manage_threads, NULL);
}

