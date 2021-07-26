#include "phyframes.h"

Frame physical_frames[8];

int get_next_frame(unsigned int time) {
    int min = 0;
    int val = physical_frames[0].time_of_use;
    for (int i = 0; i < 8; i++) {
        if (physical_frames[i].valid == 0){
            physical_frames[i].valid = 1;
            physical_frames[i].time_of_use = time;
            return i;
        }
        if (physical_frames[i].time_of_use < val) {
            val = physical_frames[i].time_of_use;
            min = i;
        }
    }
    physical_frames[min].time_of_use = time;
    return min;
}
void update_time(int frame, unsigned int time){
    physical_frames[frame].time_of_use = time;
}