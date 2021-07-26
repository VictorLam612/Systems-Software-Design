#include "pagetable.h"
#include "phyframes.h"
// Returns the frame number that a virtual page is stored on. 
// If the virtual page is not on a frame, the number of page faults is incremented and the page is assigned to a frame.
// If the frame was storing another virtual page, that virtual page must request a new frame next time it is requested.
Page page_table[32];
int requests=0;
int page_faults=0;

int request(unsigned long long virtual_page){
    requests++;
    if(page_table[virtual_page].valid){
        update_time(page_table[virtual_page].frame_number,requests);
        return page_table[virtual_page].frame_number;
    }
    page_faults++;
    int frame = get_next_frame(requests);
    upkeep(frame);
    page_table[virtual_page].valid = 1;
    page_table[virtual_page].frame_number = frame;
    return frame;
}

void upkeep(int frame){
    for(int i = 0; i<32;i++){
        if(page_table[i].valid){
            if(page_table[i].frame_number==frame){
                page_table[i].valid = 0;
            }
        }
    }
}