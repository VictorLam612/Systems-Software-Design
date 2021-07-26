
#ifndef PAGETABLE_H
#define PAGETABLE_H

class Page{
    public:
    int valid;
    int frame_number;
};

extern Page page_table[];
extern int requests;
extern int page_faults;

extern int request(unsigned long long virtual_page);
extern void upkeep(int frame);
#endif
