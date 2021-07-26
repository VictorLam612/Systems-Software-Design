
#ifndef PHYFRAMES_H
#define PHYFRAMES_H

class Frame { 
    public:
    int valid;
    unsigned int time_of_use;
};

extern Frame physical_frames[];

extern int get_next_frame(unsigned int time);
extern void update_time(int frame, unsigned int time);



#endif