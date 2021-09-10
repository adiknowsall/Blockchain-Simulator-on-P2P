#include "decl.h"

//#############################################################################
//########################### PEER DEFS #######################################
//#############################################################################

bool compareTransaction_amount :: operator()(Transaction* e1, Transaction* e2){
    return e1->get_amount() > e2->get_amount();
}
Peer :: Peer(int id1, bool fast1, double t_x1, double T_x1, Simulator* s){
    //O(n)
    id = id1;
    fast = fast1;
    head = NULL;
    last = NULL;
    t_x = t_x1;
    T_x = T_x1;
    hashing_power = 100.0*((1.0/T_x1)/s->get_total_hashing_power());
    arrival_time = unordered_map<Block*, double>();
    id2addr = map<pair<int,int>, Block*>();
    neigh = vector<pair<int, Link*> >();
    transaction_pool = set<Transaction*, compareTransaction_amount>();
    balance = vector<double>(s->get_n(), 0);
    transaction_log = set<pair<Transaction*, Peer*> >();
    block_log = set<pair<Block*, Peer*> >();
    nv = 0;

}
Block* Peer :: get_last(){
    return last;
}
bool Peer :: get_fast(){
    return fast;
}

double Peer :: get_hashing_power(){
    return hashing_power;
}

void Peer :: addto_neigh(pair<int, Link*> p){
    neigh.push_back(p);
}

void Peer :: set_fast(bool fast1){
    fast = fast1;
} 

int Peer :: get_id(){
    return id;
}
int Peer :: get_neigh_size(){
    return neigh.size();
}
pair<int,Link*> Peer :: get_neigh_ele(int i){
    return neigh[i];
}

void Peer :: addto_transaction_log(pair<Transaction*, Peer*> p){
    // assert(transaction_log.find(p) == transaction_log.end()); (may not be true) 
    transaction_log.insert(p);
}

void Peer :: addto_block_log(pair<Block*, Peer*> p){
    // assert(block_log.find(p) == block_log.end()); (may not be true)
    block_log.insert(p);
}


Event* Peer :: gen_transaction_create_event(Simulator* s){//O(1)
    // random_device rd;
    exponential_distribution<double> distribution(1.0/(t_x+1e-6));
    double next_time = distribution(mt); //https://stackoverflow.com/questions/19665818/generate-random-numbers-using-c11-random-library
     //generates event for transaction creation with random sched_time
    Event* cur_event = new Transaction_Event(NULL, s->get_cur_time() + next_time, id, true);
    cur_event->set_hearer_id(-1);
    return cur_event;
}

Event* Peer :: gen_transaction_hear_event(Transaction* t, int i, Simulator* s){
    int pid = neigh[i].first;
    Link* l = neigh[i].second;
    double latency = l->get_latency(t->get_size(), s);
    Event* cur_event = new Transaction_Event(t, s->get_cur_time() + latency, id, false);
    cur_event->set_hearer_id(pid);
    return cur_event;
}

Event* Peer :: gen_block_create_event(Simulator* s){//O(n+T)
    //generates event for block creation with random sched_time 
    // random_device rd;
    exponential_distribution<double> distribution(1.0/(T_x+1e-6));
    double next_time = distribution(mt); //http://www.cplusplus.com/reference/random/exponential_distribution/
    Block* b = new Block(id, s->get_cur_time() + next_time, s);
    //add coinbase transaction
    b->add_trans(new Transaction(-1, id, 50));

    //GENERATE INVALID BLOCKS WITH SOME PROBABILITY
    //set transactions in the block (max 999 including coinbase transaction)
    //NOTE: THE BLOCK DOESNT REMOVE THESE TRANSACTIONS FROM THE POOL RIGHT NOW, IT DOES WHEN IT ACCEPTS THE BLOCK FOR THE BLOCKCHAIN
    // random_device rd1;
    bernoulli_distribution distribution1(s->get_invalid_block_probability_per_peer());
    if(distribution1(mt)){
        //gen invalid
        for(auto it = transaction_pool.begin(); it != transaction_pool.end(); it++){
            Transaction* cur = *it;
            if(balance[cur->get_payer_id()] < cur->get_amount()){
                b->add_trans(cur);
                break;
            }
        }
    }
    else{
        //gen valid
        // cout << "what" << endl;
        int added = 0;
        vector<double> tmp_balance = balance; //dynamically update tmp_balance according to the proposed transactions
        for(auto it = transaction_pool.begin(); it != transaction_pool.end() && added < 998; it++){
            //NOTE: THIS MAY LEAVE OUT SOME TRANSACTIONS THAT BECOME VALID AFTER PASSING THEM
            Transaction* cur = *it;
            if(tmp_balance[cur->get_payer_id()] >= cur->get_amount()){
                b->add_trans(cur);
                tmp_balance[cur->get_payer_id()] -= cur->get_amount();
                tmp_balance[cur->get_payee_id()] += cur->get_amount();
                added++;
            }
        }
    }
    
    
    //set parent_block id to last id, indicating the block where it is being mined for (or was mined for, in the future)
    if(last == NULL){
        b->set_parent_blk_id(-1);
        b->set_parent_blk_creator_id(-1);
    }
    else{
        b->set_parent_blk_id(last->get_blk_id());
        b->set_parent_blk_creator_id(last->get_creator_id());
    }
    Event* cur_event = new Block_Event(b, s->get_cur_time() + next_time, id, true);
    cur_event->set_hearer_id(-1);
    return cur_event;
} 

Event* Peer :: gen_block_hear_event(Block* b, int i, Simulator* s){
    int pid = neigh[i].first;
    Link* l = neigh[i].second;
    double latency = l->get_latency(b->get_size(), s);
    Event* cur_event = new Block_Event(b, s->get_cur_time() + latency, id, false);
    cur_event->set_hearer_id(pid);
    return cur_event;
}

void Peer :: update_balance(Transaction* t){
    if(t->get_payer_id() == -1){
        balance[t->get_payee_id()]+=t->get_amount();
    }
    else{
        balance[t->get_payer_id()]-=t->get_amount();
        balance[t->get_payee_id()]+=t->get_amount();
    }
}

double Peer :: get_sum(){
    double s = 0;
    for(int i=0; i<balance.size(); i++){
        s += balance[i];
    }
    return s;
}
void Peer :: print_cur_longest_blockchain(){
    stack<Block*> s;
    Block* cur = last;
    while(cur != NULL){
        s.push(cur);
        cur = cur->get_parent();
    }
    while(!s.empty()){
        Block* t = s.top();
        s.pop();
        cout << "B" << t->get_blk_id() << ", created by: " << t->get_creator_id() << endl;
    }
    cout << "--------\n\n";
}

void calculate_balance(vector<double> &tmp_balance, Block* cur){
    //calculates balances upto block cur, and updates into v
    while(cur != NULL){
        for(int i=0; i<cur->get_trans_list_size(); i++){
            Transaction* tmp = cur->get_trans_list_ele(i);
            if(tmp->get_payer_id() == -1){
                //coinbase
                tmp_balance[tmp->get_payee_id()] += tmp->get_amount();
            }
            else{
                tmp_balance[tmp->get_payer_id()] -= tmp->get_amount();
                tmp_balance[tmp->get_payee_id()] += tmp->get_amount();
            }
        }
        cur = cur->get_parent();
    }
}


bool Peer :: add_block(Block* b, Block* prev, Simulator* s, bool recurse){
    
    //adds block to the blockchain after prev. (If null, then creates new blockchain)
    //returns whether last was changed or not (so mining needs to restart or not)
    // if(prev == NULL){
        // cout << "weird: " << b->get_blk_id() << endl;
    // }
    // else{
        // cout << "more: " << "previous block id: " << prev->get_blk_id() << ", cur_block_id: " << b->get_blk_id() << ", parent block id: " << b->get_parent_blk_id() << endl;
    // }

    bool tmp = id2addr.find(make_pair(b->get_blk_id(), b->get_creator_id())) == id2addr.end();
    if(!tmp){
        // cout << "new: " << b->get_blk_id() << ", " << b->get_creator_id() << endl;
        // print_tree();
        assert(tmp);
    }
    id2addr[make_pair(b->get_blk_id(), b->get_creator_id())] = b;
    arrival_time[b] = s->get_cur_time();

    
    Block* old = last;
    bool ans;
    nv += 1;
    if(prev == NULL){
        //genesis
        head = b;
        last = b;
        b->set_depth(0);
        for(int i=0; i<b->get_trans_list_size(); i++){
            update_balance(b->get_trans_list_ele(i));
        }
        ans = true;
        //no need to check pending when inserting the genesis block!
        
    }
    else{
        prev->addto_children(b);
        b->set_depth(prev->get_depth()+1);
        b->set_parent(prev);
        if(b->get_depth() > last->get_depth()){
            if(prev != last){
                //if the longest chain is changed
                balance = vector<double>(balance.size(), 0);
                calculate_balance(balance, prev);
            }
            //invariant: balance contains the balances till prev
            for(int i=0; i<b->get_trans_list_size(); i++){
                update_balance(b->get_trans_list_ele(i));
            }
            last = b;
            //invariant: balance contains the balances till last
            ans = true;
            if(recurse){
                //try inserting pending ones now
                // ans = true;
                bool changed = true;
                while(changed){
                    changed = false;
                    int sz = pending.size();
                    for(int i=0; i<sz; i++){
                        Block* cur = pending.front();
                        pending.pop();
                        bool tmp = addto_blockchain(cur, s, false);
                        ans = ans || tmp;
                        if(tmp){
                            changed = true;
                            break;
                        }
                    }
                }
            }
            // }
            
        }
        else{
            //last wasnt changed
            ans = false;
            if(recurse){
                //try inserting pending ones now
                bool changed = true;
                while(changed){
                    changed = false;
                    int sz = pending.size();
                    for(int i=0; i<sz; i++){
                        Block* cur = pending.front();
                        pending.pop();
                        bool tmp = addto_blockchain(cur, s, false);
                        ans = ans || tmp;
                        if(tmp){
                            changed = true;
                            break;
                        }
                    }
                }
            }
            // }
        }
    }
    //at this point, the transaction pool contains the transactions upto the last block
    //now, if chain is changed (ITS DIFFERENT FROM last BEING CHANGED) then we need to include all the transactions till the start of the blockchain again and then go to the new last, and remove the transactions
    //else, we just need to remove the transactions in the current block
    if(last->get_parent() == old){
        //this means that the block has been added to the current chain
        //remove transactions from transaction pool, that are in the block
        for(int i=0; i<last->get_trans_list_size(); i++){
            auto it = transaction_pool.find(last->get_trans_list_ele(i));
            if(it != transaction_pool.end()){
                transaction_pool.erase(it);
            }   
        }
    }
    else if(last == old){
        //longest chain no changes
        //do nothing
    }
    else{
        //chain is changed
        //find LCA, we know the depths of all the blocks
        Block* lca = LCA(old, last);
        //add these transactions (from old to lca)
        Block* cur = old;
        while(cur != lca){
            for(int i=0; i<cur->get_trans_list_size(); i++){
                transaction_pool.insert(cur->get_trans_list_ele(i));   
            }
            cur = cur->get_parent();
        }
        cur = last;
        //remove these transactions (from lca to new last)
        while(cur != lca){
            for(int i=0; i<cur->get_trans_list_size(); i++){
                auto it = transaction_pool.find(cur->get_trans_list_ele(i));
                if(it != transaction_pool.end()){
                    transaction_pool.erase(it);
                }
            }
            cur = cur->get_parent();
        }
    }
    return ans;
} 

int gen_random_except(int n, int i){
    assert(n>1);

    int cur = rand()%n;
    while(cur == i){
        cur = rand()%n;
    }
    return cur;
}
Transaction* Peer :: gen_transaction(Simulator* s){
    //generates a transaction
    int mod = (int)floor(1.5*balance[id]);
    double amt = 0;
    if(mod == 0){
        amt = (double)rand()/(double)(RAND_MAX);
    }
    else{
        amt = rand()%mod;
        amt += (double)rand()/(double)(RAND_MAX);
    }
    Transaction* trans = new Transaction(id, gen_random_except(s->get_n(), id), amt+1e-5);
    return trans;
} 
// Block* Peer :: gen_block(Simulator* s){
//     // generates a block
//     Block* b = Block(id, s->get_cur_time(), s);
//     //add greedy transactions
//     //
//     return NULL;
// } 
void Peer :: broadcast_transaction(Transaction* t, Simulator* s){
    for(int i=0; i<neigh.size(); i++){
        if(transaction_log.find(make_pair(t, s->get_peer_list_ele(neigh[i].first))) == transaction_log.end()){
            s->addto_event_queue(gen_transaction_hear_event(t, i, s));
            transaction_log.insert(make_pair(t, s->get_peer_list_ele(neigh[i].first)));
        }
    }
}
void Peer :: broadcast_block(Block* b, Simulator* s){
    for(int i=0; i<neigh.size(); i++){
        if(block_log.find(make_pair(b, s->get_peer_list_ele(neigh[i].first))) == block_log.end()){
            s->addto_event_queue(gen_block_hear_event(b, i, s));
            block_log.insert(make_pair(b, s->get_peer_list_ele(neigh[i].first)));
        }
    }
}

bool Peer :: trans_in_blockchain(Transaction* t){
    Block* cur = last;
    while(cur != NULL){
        if(cur->exists_in_trans_list(t)){
            return true;
        }
        cur = cur->get_parent();
    }
    return false;
}

void Peer :: addto_transaction_pool(Transaction* t){
    //if not in the current longest chain
    if(!trans_in_blockchain(t)){
        if(transaction_pool.find(t) == transaction_pool.end()){
            assert(t->get_payer_id() != -1);
            // if(balance[t->get_payer_id()] >= t->get_amount()){
                // transaction_pool.insert(t);
            // }
            // else{
                // cout << "wrong transaction!" << endl;
            // }
            transaction_pool.insert(t);
            //not checking invalid transaction here
        }
    }
}

// void Peer :: print_pending_set(){
    // cout << "pending:" << id << " {";
    // for(auto it = pending.begin(); it != pending.end(); it++){
    //     cout << (*it)->get_blk_id() << "," << (*it)->get_creator_id() << "| ";
    // }
    // cout << "}\n";
// }


bool Peer :: addto_blockchain(Block* b, Simulator* s, bool recurse){
    //returns if you need to restart mining
    
    assert(b->get_parent_blk_id() >= 0 && b->get_parent_blk_creator_id() >= 0);
    if(id2addr.find(make_pair(b->get_blk_id(), b->get_creator_id())) != id2addr.end()){
        //already has the block
        // cout << "already has, peer: " << id << ", block id: " << b->get_blk_id() << "," << b->get_creator_id() << endl;
        return false;
    }

    if(id2addr.find(make_pair(b->get_parent_blk_id(), b->get_parent_blk_creator_id())) == id2addr.end()){
        //parent doesnt exist
        pending.push(b);
        return false;
    }
    
    
    //find the balances upto the supposed parent_blk_id whose creator is parent_creator_id
    vector<double> tmp_balance(balance.size(), 0);
    //cur contains the supposed parent block
    Block* cur = id2addr[make_pair(b->get_parent_blk_id(), b->get_parent_blk_creator_id())];
    if(cur != last){
        // cout << "parent of this: " << cur->get_blk_id() << "\n";
        // cout << "last: " << last->get_blk_id() << "\n";
        calculate_balance(tmp_balance, cur);
    }
    else{
        tmp_balance = balance;
    }

    //verification of block
    vector<double> balance_needed(balance.size(), 0);
    for(int i=0; i<b->get_trans_list_size(); i++){
        Transaction* tmp = b->get_trans_list_ele(i);
        if(tmp->get_payer_id() != -1){
            balance_needed[tmp->get_payer_id()] += tmp->get_amount();
            balance_needed[tmp->get_payee_id()] -= tmp->get_amount();
        }
        else{
            balance_needed[tmp->get_payee_id()] -= tmp->get_amount();
        }
    }

    for(int i=0; i<balance_needed.size(); i++){
        if(tmp_balance[i] < balance_needed[i]){
            //invalid
            return false;
        }
    }
    // cout << "adding\n";
    Block* b_copy = new Block(*b);
    return add_block(b_copy, cur, s, recurse);
}

void total_count(Block* b, vector<int>& v){
    if(b != NULL){
        v[b->get_creator_id()]+=1;
        for(int i=0; i<b->get_children_size(); i++){
            total_count(b->get_children_ele(i), v);
        }
    }
}

vector<vector<int> > Peer :: analyse_blockchain(Simulator* s){
    vector<int> longest_chain(s->get_n(), 0), total(s->get_n(), 0);
    Block* cur = last;
    while(cur != NULL){
        longest_chain[cur->get_creator_id()]+=1;
        cur = cur->get_parent();
    }
    total_count(head, total);

    // for(int i=0; i<s->get_n(); i++){
    //     cout << longest_chain[i] << 
    // }

    return vector<vector<int> >{longest_chain, total};
}

void printNTree(Block* x,
    vector<bool> flag,
    int depth = 0, bool isLast = false)
{
    //TAKEN FROM GEEKS FOR GEEKS
    // Condition when node is None
    if (x == NULL)
        return;
     
    // Loop to print the depths of the
    // current node
    for (int i = 1; i < depth; ++i) {
         
        // Condition when the depth
        // is exploring
        if (flag[i] == true) {
            cout << "| "
                << " "
                << " "
                << " ";
        }
         
        // Otherwise print
        // the blank spaces
        else {
            cout << " "
                << " "
                << " "
                << " ";
        }
    }
     
    // Condition when the current
    // node is the root node
    if (depth == 0)
        cout << x->get_blk_id() << "," << x->get_creator_id() << '\n';
     
    // Condition when the node is
    // the last node of
    // the exploring depth
    else if (isLast) {
        cout << "+--- " << x->get_blk_id() << "," << x->get_creator_id()  << '\n';
         
        // No more childrens turn it
        // to the non-exploring depth
        flag[depth] = false;
    }
    else {
        cout << "+--- " << x->get_blk_id() << "," << x->get_creator_id()  << '\n';
    }
 
    int it = 0;
    for (int i = 0;
    i < x->get_children_size(); ++i, ++it)
 
        // Recursive call for the
        // children nodes
        printNTree(x->get_children_ele(i), flag, depth + 1,
            it == (x->get_children_size()) - 1);
    flag[depth] = true;
}


void Peer :: print_tree(){
    printNTree(head, vector<bool>(nv, true));
}

bool custom_compare(pair<Block*, double> p1, pair<Block*, double> p2){
    return p1.second < p2.second;
}
void Peer :: save_arrival_time(string file_name){
    vector<pair<Block*, double> > v;
    for(auto it = arrival_time.begin(); it != arrival_time.end(); it++){
        v.push_back(*it);
    }
    sort(v.begin(), v.end(), custom_compare);
    cout << file_name << endl;
    ofstream MyFile(file_name);
    if(!MyFile){
        cout << "Couldn't open file" << endl;
    }
    else{
        for(auto it = v.begin(); it != v.end(); it++){
            MyFile << "Block" << it->first->get_blk_id() << ", Creator: P" << it->first->get_creator_id() << " | " << it->second << "\n";
        }
        MyFile.close();
    }
}

void Peer :: export_blockchain(Simulator* s){
    ofstream ofs;
    ofs.open("./Visual/tree"+to_string(id)+".txt", ofstream::out | ofstream::trunc);
    ofs.close();
    if(head){
        head->export_tree_edges(id);
    }
    string new_name = "\"(" + to_string(s->get_n()) + "," + to_string(s->get_z()) + "," + to_string(s->get_overall_txn_time()) + "," + to_string(s->get_overall_block_time()) + "," + to_string(s->get_max_blocks()) + ")\"";
    //https://stackoverflow.com/questions/16962430/calling-python-script-from-c-and-using-its-output
    string file_path = "./Visual/create_blockchain.py";
    string command = "python ";
    command += file_path;
    command += " " + to_string(s->get_seed());
    command += " " + new_name;
    cout << command << endl;
    FILE* in = popen(command.c_str(), "r");
    pclose(in);
}

void Peer :: export_blockchain_data(){
    ofstream ofs;
    ofs.open("./blockchain_data/peer_"+to_string(id)+".txt", ofstream::out | ofstream::trunc);
    
    stack<Block*> s;
    Block* cur = last;
    while(cur != NULL){
        s.push(cur);
        cur = cur->get_parent();
    }

    while(!s.empty()){
        Block* t = s.top();
        s.pop();
        ofs << "\n-----------------------------------\n";
        ofs << "Block ID: " << t->get_blk_id() << "\n";
        ofs << "Creator ID: " << t->get_creator_id() << "\n";
        ofs << "Block Creation Time: " << t->get_time_creation() << "\n";
        ofs << "Block Arrival Time: " << arrival_time[t] << "\n";

        ofs << "(Parent blk_ID, Parent creator_ID): (" << t->get_parent_blk_id() << "," << t->get_parent_blk_creator_id() << ")\n";
        ofs << "TRANSACTIONS:\n";
        for(int j=0; j<t->get_trans_list_size(); j++){
            Transaction* tmp = t->get_trans_list_ele(j);
            if(tmp->get_payer_id() == -1){
                ofs << j << ": " << "Coinbase, p" << tmp->get_payee_id() <<  " gets " << tmp->get_amount() << " coins\n";
            }
            else{
                ofs << j << ": " << "p" << tmp->get_payer_id() << " pays " << "p" << tmp->get_payee_id() << " " << tmp->get_amount() << " coins\n";
            }
        }
        ofs << "\n";
        ofs << "Block Size (in bits): " << t->get_size() << "\n";

    }
    ofs.close();
}

void Peer :: print_pending_set(){
    queue<Block*> tmp;
    cout << "{";
    while(!pending.empty()){
        Block* cur = pending.front();
        cout << cur->get_blk_id() << "," << cur->get_creator_id() << " | ";
        tmp.push(cur);
        pending.pop();
    }
    cout << "}\n";
    pending = tmp;
}

bool Peer :: verify_blockchain(){
    return head->verify_block(vector<double>(balance.size(), 0));
}

vector<int> Peer :: get_leaf_depths(){
    vector<int> ret;
    head->fill_leaf_depths(ret);
    return ret;
}