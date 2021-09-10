#include "decl.h"

//#############################################################################
//########################### TRANSACTION DEFS ################################
//#############################################################################
Transaction :: Transaction(int i1, int i2, double amt){
    payer_id = i1;
    payee_id = i2;
    amount = amt;
}

double Transaction :: get_size(){
    return size;
}

int Transaction :: get_payer_id(){
    return payer_id;
}

double Transaction :: get_amount(){
    return amount;
}

int Transaction :: get_payee_id(){
    return payee_id;
}