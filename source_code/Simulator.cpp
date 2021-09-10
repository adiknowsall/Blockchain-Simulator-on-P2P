#include "decl.h"

//#############################################################################
//########################### SIMULATOR DEFS ##################################
//#############################################################################
bool compareTime :: operator()(Event* e1, Event* e2){
    return e1->get_sched_time() < e2->get_sched_time();
}


void gen_random_connected_graph(vector<Peer*> &peer_list, Simulator* s){
    //https://stackoverflow.com/questions/16962430/calling-python-script-from-c-and-using-its-output
    string file_path = "./gen_random_graph/script.py";
    string command = "python ";
    char line[100];
    vector<pair<int,int> > ret;
    command += file_path;
    command += " " + to_string(s->get_seed());
    command += " " + to_string(s->get_n());
    FILE* in = popen(command.c_str(), "r");
    while(fgets(line, 100, in)){
        string tmp = line;
        int p1, p2;
        int i = 0;
        string s1="", s2="";
        while(tmp[i] != ' '){
            s1 += tmp[i];
            i++;
        }
        p1 = stoi(s1);
        i++;
        for(int j=i; j<tmp.size(); j++){
            s2 += tmp[j]; 
        }
        p2 = stoi(s2);
        ret.push_back(make_pair(p1,p2));
    }
    pclose(in);
    for(int i=0; i<ret.size(); i++){
        //connect(peer[ret[i],first],peer[ret[i].second])
        int i1 = ret[i].first, i2 = ret[i].second;
        Link* l1 = new Link(peer_list[i1]->get_id(), peer_list[i2]->get_id(), s);
        Link* l2 = new Link(peer_list[i2]->get_id(), peer_list[i1]->get_id(), s);
        peer_list[i1]->addto_neigh(make_pair(peer_list[i2]->get_id(), l1));
        peer_list[i2]->addto_neigh(make_pair(peer_list[i1]->get_id(), l2));
    }
}

Peer* Simulator :: get_peer_list_ele(int i){
    return peer_list[i];
}

int Simulator :: get_next_block_id(){
    int tmp = next_block_id;
    next_block_id++;
    return tmp;
}

int Simulator :: get_n(){
    return n;
}

double Simulator :: get_cur_time(){
    return cur_time;
}

void Simulator :: addto_event_queue(Event* e){
    event_queue.insert(e);
}

double Simulator :: get_overall_txn_time(){
    return overall_txn_time;
}
double Simulator :: get_overall_block_time(){
    return overall_block_time;
}
int Simulator :: get_max_blocks(){
    return max_blocks;
}
Simulator :: Simulator(bool uneven, int seed1, int n1, double z1, double global_t_x1, double global_T_x1, double stop_time1, int total_cycles1, double invalid_block_probability_per_peer1, int max_blocks1){
    seed = seed1;
    overall_txn_time = global_t_x1/n1;
    overall_block_time = global_T_x1;
    n = n1;
    z = z1;
    global_t_x = global_t_x1;
    //generate a vector sampled from uniform distribution with mean global_T_x1
    // random_device rd;
    // uniform_real_distribution<double> distribution((3.0*global_T_x1)/4.0, (5.0*global_T_x1)/4.0);
    double f = 1e-3; //minimum hashing power
    uniform_real_distribution<double> distribution(max(10.0,f*10.0*n), 5.0*n); //s_i < S-s_i always
    if(uneven){
        distribution = uniform_real_distribution<double>(10, 100000);
    }
    vector<double> hashing_power;
    double tmp_sum = 0;
    for(int i=0; i<n; i++){
        hashing_power.push_back(distribution(mt));
        tmp_sum += hashing_power[i];
    }
    for(int i=0; i<n; i++){
        hashing_power[i] = hashing_power[i]/tmp_sum;
    }

    
    T_x = vector<double>();
    for(int i=0; i<n; i++){
        // T_x.push_back(global_T_x1);
        T_x.push_back(global_T_x1/hashing_power[i]);
    }
    stop_time = stop_time1;
    next_block_id = 0;
    total_cycles = total_cycles1;
    invalid_block_probability_per_peer = invalid_block_probability_per_peer1;
    max_blocks = max_blocks1;
}
int Simulator :: get_seed(){
    return seed;
}
vector<double> Simulator :: get_T_x(){
    return T_x;
}

// mt19937 Simulator :: get_mt(){
//     return mt;
// }

void Simulator :: print_state(bool b){
    //b specifies whether to print general info or not
    if(b){
        cout << "Peer info: \n";
        for(int i=0; i<peer_list.size(); i++){
            cout << "P" << peer_list[i]->get_id() << ": " << (peer_list[i]->get_fast()?"F, ":"S, ") << "and hashing power = " << peer_list[i]->get_hashing_power() << "\n"; 
        }
        cout << "\n##################\n\n";
    }

    cout << "Events queue: \n";
    multiset<Event*, compareTime> tmp = event_queue;
    while(!tmp.empty()){
        cout << setprecision(3) <<  "time: " << (*tmp.begin())->get_sched_time() << ", creator: " << (*tmp.begin())->get_creator_id() << ", hearer: " << (*tmp.begin())->get_hearer_id();
        if((*tmp.begin())->get_gen()){
            if((*tmp.begin())->get_is_trans_event()){
                cout << ", Transaction Creation" << endl;
            }
            else{
                cout << ", Block Creation" << endl;
            }
        }
        else{
            if((*tmp.begin())->get_is_trans_event()){
                cout << ", Transaction Hearing" << endl;
            }
            else{
                cout << ", B" << (*tmp.begin())->get_block()->get_blk_id() << "," << (*tmp.begin())->get_block()->get_creator_id() << " Hearing" << endl;
            }
        }
        tmp.erase(tmp.begin());
    }
    cout << "#################\n\n";

    if(b){
        cout << "P2P Network\n";
        for(int i=0; i<peer_list.size(); i++){
            cout << "P" << i << ": {";
            for(int j=0; j<peer_list[i]->get_neigh_size(); j++){
                cout << "P" << peer_list[i]->get_neigh_ele(j).first << ",";
            }
            cout  << "}" << endl; 
        }
        cout << "#################\n\n";
    }

    if(b){
        for(int i=0; i<peer_list.size(); i++){
            cout << "P" << i << ":" << peer_list[i]->get_sum() << endl;
        }
    }

    if(b){
        for(int i=0; i<peer_list.size(); i++){
            cout << "P" << i << ":" << endl;
            // peer_list[i]->print_cur_longest_blockchain();
            peer_list[i]->print_tree();
            cout << "-------\n";
        }
    }
}

double Simulator :: get_total_hashing_power(){
    double ret = 0;
    for(int i=0; i<T_x.size(); i++){
        ret += 1.0/T_x[i];
    }
    return ret;
}

double Simulator :: get_invalid_block_probability_per_peer(){
    return invalid_block_probability_per_peer; 
}
void Simulator :: init_sim(){
    //peer list populate O(n^2)
    for(int i=0; i<n; i++){
        bool fast1 = true;
        peer_list.push_back(new Peer(i, fast1, global_t_x, T_x[i], this)); //O(n)
    }
    //randomly make n*z of them slow O(n)
    vector<Peer*> tmp = peer_list;
    random_shuffle(tmp.begin(), tmp.end());
    for(int i=0; i<(int)floor(n*z); i++){
        tmp[i]->set_fast(false);
    }

    //generate random connected graph O(n^2)
    gen_random_connected_graph(peer_list, this);

    //add genesis block in each peer's blockchain 
    int p_creator = rand()%n;
    Block* gen = new Block(p_creator, cur_time, this);
    gen->update_blk_id(this);
    //add coinbase transaction
    gen->add_trans(new Transaction(-1, p_creator, 50));
    for(int i=0; i<n; i++){
        Block* gen_copy = new Block(*gen);
        peer_list[i]->add_block(gen_copy, NULL, this, false);
    }

    //push transaction creation events in the queue for each peer O(nlogn)
    for(int i=0; i<n; i++){
        event_queue.insert(peer_list[i]->gen_transaction_create_event(this));
    }

    //push block creation events in the queue for each peer, NOTE THAT THE EVENTS CONTAIN THE BLOCKS O(n^2*log(n))
    for(int i=0; i<n; i++){
        event_queue.insert(peer_list[i]->gen_block_create_event(this));
    }
}

void Simulator :: run_sim(){
    int cycles = 0;
    // this->print_state(1);
    // cout << stop_time << endl;
    vector<int> num_txn;
    int old = next_block_id;
    double old_time = 0;

    int old_time_trans = 0;
    vector<double> inter_block, inter_trans;
    while(!event_queue.empty() && cycles < total_cycles){
        //to calculate intercreation time of blocks
        
        if(old != next_block_id){
            // if(next_block_id  == 0){
                cout << "number of blocks issued: " << next_block_id << endl;
                cout << "number of transactions: " << inter_trans.size() << endl;
            // }
            inter_block.push_back(cur_time - old_time);
            // cout << "number of blocks issued: " << next_block_id << endl;
            old = next_block_id;
            old_time = cur_time;
            // cout << "number of transactions: " << inter_trans.size() << endl;
            // cout << cur_time << endl;
        }
        if(next_block_id > max_blocks){
            break;
        }
        // this->print_state(0);
        cycles++;
        
        // cout << "################\n################\n\n";

        Event* cur_event = *event_queue.begin();
        event_queue.erase(event_queue.begin());
        cur_time = cur_event->get_sched_time();
        if(cur_time >= stop_time){
            break;
        }
        if(cur_event->get_is_trans_event()){
            //is a transaction event
            //NOTE: What to do about invalid transactions? if an invalid transaction comes, it is not eliminated. It may be used in future if it becomes valid!
            if(cur_event->get_gen()){
                // cout << "trans creation\n";
                //is a creation event

                //just for analysis
                inter_trans.push_back(cur_time - old_time_trans);
                old_time_trans = cur_time;

                Transaction* t = peer_list[cur_event->get_creator_id()]->gen_transaction(this); // generate the transaction
                peer_list[cur_event->get_creator_id()]->broadcast_transaction(t, this); //broadcast the transaction
                peer_list[cur_event->get_creator_id()]->addto_transaction_pool(t); //adds to its own transaction pool if not already in the current longest chain
                event_queue.insert(peer_list[cur_event->get_creator_id()]->gen_transaction_create_event(this)); // generate the next event for creation of transaction
            }
            else{
                // cout << "trans hearing\n";
                //is a reaching event
                //add that if already received
                Transaction* t = cur_event->get_trans();
                peer_list[cur_event->get_hearer_id()]->addto_transaction_log(make_pair(t, peer_list[cur_event->get_hearer_id()]));
                peer_list[cur_event->get_hearer_id()]->broadcast_transaction(t,this); //broadcast the transaction
                peer_list[cur_event->get_hearer_id()]->addto_transaction_pool(t);//adds to its own transaction pool only if already not present in the current longest chain
            }
        }
        else{
            //is a block event
            if(cur_event->get_gen()){
                // cout << "peer" << cur_event->get_creator_id() << ", BLOCK CREATION\n\n";
                //is a creation event
                Block* b = cur_event->get_block(); //get the block,
                b->update_blk_id(this); // finally the imaginary hash is set now
                peer_list[cur_event->get_creator_id()]->broadcast_block(b, this); //broadcast the block
                bool last_was_changed = peer_list[cur_event->get_creator_id()]->addto_blockchain(b, this, true); //verifies it and adds it to its blockchain
                //may not always be true, if block was invalid
                if(!last_was_changed){
                    cout << "Invalid Block Generated!" << endl;
                    // exit(1);
                }
                num_txn.push_back(b->get_trans_list_size());
                event_queue.insert(peer_list[cur_event->get_creator_id()]->gen_block_create_event(this));//start creating the next block
            }
            else{
                //is a reaching event
                //add if already received
                Block* b = cur_event->get_block(); 
                // cout << "peer" << cur_event->get_hearer_id() <<  ", BLOCK" << b->get_blk_id() << "," << b->get_creator_id() << " REACHING\n\n";
                // peer_list[cur_event->get_hearer_id()]->print_pending_set();

                
                peer_list[cur_event->get_hearer_id()]->addto_block_log(make_pair(b, peer_list[cur_event->get_creator_id()]));
                peer_list[cur_event->get_hearer_id()]->broadcast_block(b, this); //broadcast the block
                if(peer_list[cur_event->get_hearer_id()]->addto_blockchain(b, this, true)){ 
                    //verifies it and adds it to its blockchain
                    //last was changed, restart mining
                    //remove the future creation event in multiset if it exists
                    for(auto it = event_queue.begin(); it != event_queue.end(); it++){
                        Event* cur = *it;
                        if(!(cur->get_is_trans_event()) && cur->get_gen() && (cur->get_creator_id() == cur_event->get_hearer_id())){
                            event_queue.erase(it);
                            break;
                        }
                    }
                    //push the new mining session event
                    event_queue.insert(peer_list[cur_event->get_hearer_id()]->gen_block_create_event(this));//start creating the next block
                }
                else{
                    //last was not modified, no change
                }
            }
        }
        // for(int i=0; i<peer_list.size(); i++){
        //     cout << "P" << i << ":" << endl;
        //     // peer_list[i]->print_cur_longest_blockchain();
        //     peer_list[i]->print_tree();
        //     cout << "-------\n";
        // }
        delete cur_event;
    }
    double s = 0;
    for(int i=0; i<inter_block.size(); i++){
        s += inter_block[i];
    }
    cout << "average block time: " << s/inter_block.size() << endl;
    s = 0;
    for(int i=0; i<inter_trans.size(); i++){
        s += inter_trans[i];
    }
    cout << "total transactions: " << inter_trans.size() << endl;
    cout << "average trans time: " << s/inter_trans.size() << endl;
    long long int txn_sum = 0;
    for(int i=0; i<num_txn.size(); i++){
        txn_sum += (long long int) num_txn[i];
    }
    cout << "average number of transactions per block: " << txn_sum/(1.0*num_txn.size()) << "\n";
    
}


void Simulator :: stop_sim(){
    // print_state(0);
    while(!event_queue.empty()){
        // print_state(0);
        // if(event_queue.size() % 1000 == 0){
            // cout << event_queue.size() << endl;
        // }
        Event* cur_event = *event_queue.begin();
        event_queue.erase(event_queue.begin());
        if(cur_event->get_is_trans_event()){
            //is a transaction event
            //NOTE: What to do about invalid transactions? if an invalid transaction comes, it is not eliminated. It may be used in future if it becomes valid!
            if(cur_event->get_gen()){
                
            }
            else{
                // cout << "trans hearing\n";
                //is a reaching event
                //add that if already received
                Transaction* t = cur_event->get_trans();
                peer_list[cur_event->get_hearer_id()]->addto_transaction_log(make_pair(t, peer_list[cur_event->get_hearer_id()]));
                peer_list[cur_event->get_hearer_id()]->broadcast_transaction(t,this); //broadcast the transaction
                peer_list[cur_event->get_hearer_id()]->addto_transaction_pool(t);//adds to its own transaction pool only if already not present in the current longest chain
            }
        }
        else{
            //is a block event
            if(cur_event->get_gen()){

            }
            else{
                //is a reaching event
                //add if already received
                Block* b = cur_event->get_block(); 
                // cout << "peer" << cur_event->get_hearer_id() <<  ", BLOCK" << b->get_blk_id() << "," << b->get_creator_id() << " REACHING\n";
                // peer_list[cur_event->get_hearer_id()]->print_pending_set();

                peer_list[cur_event->get_hearer_id()]->addto_block_log(make_pair(b, peer_list[cur_event->get_creator_id()]));
                peer_list[cur_event->get_hearer_id()]->broadcast_block(b, this); //broadcast the block
                peer_list[cur_event->get_hearer_id()]->addto_blockchain(b, this, true);
            }
        }
        delete cur_event;
    }
    vector<int> lf_vec = peer_list[0]->get_leaf_depths();
    cout << "[";
    for(int i=0; i<lf_vec.size(); i++){
        cout << lf_vec[i] << ", ";
    }
    cout << "]\n";
    cout << "[";
    for(int i=0; i<n; i++){
        cout << peer_list[i]->get_hashing_power() << ", ";
    }
    cout << "]\n";
}



void Simulator :: save_state(){
    mkdir("./block_arrival_times",0777);
    for(int i=0; i<peer_list.size(); i++){
        peer_list[i]->save_arrival_time("./block_arrival_times/peer"+to_string(i));
    }
}

void Simulator :: analyse(){
    vector<vector<int> > p = peer_list[0]->analyse_blockchain(this);
    vector<int> longest_chain = p[0];
    vector<int> total = p[1];
    cout << "Peer no. (speed), hashing power: R\n";
    for(int i=0; i<n; i++){
        cout << "Peer " << i << (peer_list[i]->get_fast()?" (fast)":" (slow)") << ", " << peer_list[i]->get_hashing_power() << ": " << (1.0*longest_chain[i])/total[i] << "\n";
    }
}

void Simulator :: export_network(){
    ofstream f("./Visual/network.txt");

    f << n << endl;
    for(int i=0; i<peer_list.size(); i++){
        f << (peer_list[i]->get_fast()) << endl;
    }
    for(int i=0; i<peer_list.size(); i++){
        for(int j=0; j<peer_list[i]->get_neigh_size(); j++){
            pair<int,Link*> p = peer_list[i]->get_neigh_ele(j);
            f << i  << "," << p.first  << "," << p.second->get_latency(1e8, this) << endl;
        }
    }
    f.close();

    string new_name = "num_peers"+to_string(n);
    //https://stackoverflow.com/questions/16962430/calling-python-script-from-c-and-using-its-output
    string file_path = "./Visual/create_graph.py";
    string command = "python ";
    command += file_path;
    command += " " + to_string(seed);
    command += " " + new_name;
    cout << command << endl;
    FILE* in = popen(command.c_str(), "r");
    pclose(in);
}

double Simulator :: get_z(){
    return z;
}
void Simulator :: export_blockchain_of(int i){
    peer_list[i]->export_blockchain(this);
}

void Simulator :: export_all_blockchain_data(){
    for(int i=0; i<n; i++){
        peer_list[i]->export_blockchain_data();
    }
}