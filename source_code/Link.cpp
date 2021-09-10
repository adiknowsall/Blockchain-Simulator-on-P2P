#include "decl.h"

//#############################################################################
//########################### LINK DEFS #######################################
//#############################################################################

Link :: Link(int id11, int id21, Simulator* s){
    id1 = id11;
    id2 = id21;
    // random_device rd;
    uniform_real_distribution<double> distribution(0.01,0.5);
    p =  distribution(mt);
    c = (s->get_peer_list_ele(id1)->get_fast() && s->get_peer_list_ele(id2)->get_fast())?1e8:5e6;
}

double Link :: get_latency(double s, Simulator* sim){
    // random_device rd;
    exponential_distribution<double> distribution(c/96000);
    double d = distribution(mt);
    // return 0;
    return p+(s/c)+d;
}