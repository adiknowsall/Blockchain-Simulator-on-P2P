#include "decl.h"

//#############################################################################
//########################### BLOCK DEFS ######################################
//#############################################################################
Block* LCA(Block* p, Block* q){
    int pd = p->get_depth();
    int qd = q->get_depth();
    Block* curp = p, *curq = q;
    if(pd >= qd){
        for(int i=0; i<pd-qd; i++){
            curp = curp->get_parent();
        }
        while(curp != curq){
            curp = curp->get_parent();
            curq = curq->get_parent();
        }
        assert(curp != NULL);
        return curp;
    }
    else{
        return LCA(q,p);
    }
}

Block :: Block(int x, double t, Simulator* s){
    creator_id = x;
    blk_id = -1;
    trans_list = vector<Transaction*>();
    children = vector<Block*>();
    parent = NULL;
    time_creation = t;
    size = 8*1000;
    parent_blk_id = -1;
    parent_blk_creator_id = -1;
}
int Block :: get_blk_id(){
    return blk_id;
}
void Block :: update_blk_id(Simulator* s){
    blk_id = s->get_next_block_id();
}
int Block :: get_creator_id(){
    return creator_id;
}
void Block :: set_depth(int depth1){
    depth = depth1;
}
void Block :: addto_children(Block* b){
    children.push_back(b);
}
Block* Block :: get_children_ele(int x){
    return children[x];
}
int Block :: get_children_size(){
    return children.size();
}
int Block :: get_depth(){
    return depth;
}
int Block :: get_trans_list_size(){
    return trans_list.size();
}
Transaction* Block :: get_trans_list_ele(int i){
    return trans_list[i];
}
Block* Block :: get_parent(){
    return parent;
}
void Block :: set_parent(Block* b){
    parent = b;
}
int Block :: get_parent_blk_id(){
    return parent_blk_id;
}
void Block :: set_parent_blk_id(int x){
    parent_blk_id = x;
}
int Block :: get_parent_blk_creator_id(){
    return parent_blk_creator_id;
}
void Block :: set_parent_blk_creator_id(int x){
    parent_blk_creator_id = x;
}
void Block :: add_trans(Transaction* t){
    bool tmp = trans_list.size() < 999;
    if(!tmp){
        cout << "block id: " << blk_id << ", creator id: " << creator_id << "| size: " << trans_list.size() << endl;
        assert(trans_list.size() < 999); //in order to add one more, it should be <=998 
    }
    trans_list.push_back(t);
    size += 8*1000;
}
int Block :: get_size(){
    assert(size <= 8e6);
    bool tmp = (size == 8000 + (trans_list.size())*8000);
    if(!tmp){
        // cout << "size: " << size << ", trans_list_size: " << trans_list.size() << endl;
        assert(tmp);
    }
    return size;
}

void Block :: export_tree_edges(int id){
    for(int i=0; i<children.size(); i++){
        ofstream outfile;
        outfile.open("./Visual/tree"+to_string(id)+".txt", ios_base::app); // append instead of overwrite
        outfile << blk_id << "," << creator_id << ":" << children[i]->get_blk_id() << "," << children[i]->get_creator_id() << endl; 
        outfile.close();
        children[i]->export_tree_edges(id);
    }
}
bool Block :: exists_in_trans_list(Transaction* t){
    return find(trans_list.begin(), trans_list.end(), t) != trans_list.end();
}
double Block :: get_time_creation(){
    return time_creation;
}

bool Block :: verify_block(vector<double> bal){
    // cout << "(" << blk_id << "," << creator_id << ")\n";
    vector<double> req(bal.size(), 0);
    for(int i=0; i<trans_list.size(); i++){
        int payer = trans_list[i]->get_payer_id();
        int payee = trans_list[i]->get_payee_id();
        double amt = trans_list[i]->get_amount();
        if(payer == -1){
            //coinbase
            req[payee] -= amt; 
        }
        else{
            req[payer] += amt;
            req[payee] -= amt;
        }
    }
    for(int i=0; i<bal.size(); i++){
        if(bal[i] < req[i]){
            return false;
        }
        else{
            bal[i] -= req[i];
        }
    }
    bool ans = true;
    for(int i=0; i<children.size(); i++){
        ans = ans && children[i]->verify_block(bal);
    }
    return ans;
}

void Block :: fill_leaf_depths(vector<int> &ret){
    if(children.size() == 0){
        //is leaf
        ret.push_back(depth);
    }
    else{
        for(int i=0; i<children.size(); i++){
            children[i]->fill_leaf_depths(ret);
        }
    }
}