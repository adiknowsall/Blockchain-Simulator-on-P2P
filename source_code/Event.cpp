#include "decl.h"

//#############################################################################
//########################### EVENT DEFS ######################################
//#############################################################################
Event :: Event(double sched_time1, int creator_id1, bool gen1){
    sched_time = sched_time1;
    creator_id = creator_id1;
    gen = gen1;
}
double Event :: get_sched_time(){
    return sched_time;
}
int Event :: get_creator_id(){
    return creator_id;
}
bool Event :: get_gen(){
    return gen;
}
int Event :: get_hearer_id(){
    return hearer_id;
}
void Event :: set_hearer_id(int id1){
    hearer_id = id1;
}

//########################### TRANSACTION EVENT DEFS ######################################
Transaction_Event :: Transaction_Event(Transaction* trans1, double sched_time1, int creator_id1, bool gen1) : Event(sched_time1, creator_id1, gen1){
    trans = trans1;
}
bool Transaction_Event :: get_is_trans_event(){
    return true;
}
Transaction* Transaction_Event :: get_trans(){
    return trans;
}
Block* Transaction_Event :: get_block(){
    assert(1==2);
    return NULL;
}

//########################### BLOCK EVENT DEFS ######################################
Block_Event :: Block_Event(Block* block1, double sched_time1, int creator_id1, bool gen1) : Event(sched_time1, creator_id1, gen1){
    block = block1;
}
bool Block_Event :: get_is_trans_event(){
    return false;
}
Block* Block_Event :: get_block(){
    return block;
}
Transaction* Block_Event :: get_trans(){
    assert(1==2);
    return NULL;
}