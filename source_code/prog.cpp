#include "decl.h"


//real	115m45.591s
//user	115m41.614s
//n = 30, max_blocks = 200, standard params
//Add sparseness to P2P network
mt19937 mt = mt19937(42);

int main(int argc, char *argv[]){
    // srand(time(0));
    string num_peers_str = "15";
    string z_str = "0.4";
    string t_str = "1";
    string T_str = "10";
    string max_blocks_str = "70";
    string seed_str = "420";
    if(argc == 7){
        num_peers_str = argv[1];
        z_str = argv[2];
        t_str = argv[3];
        T_str = argv[4];
        max_blocks_str = argv[5];
        seed_str = argv[6];
        
    }
    else{
        cout << "Using default paramters..\n";
    }
    cout << "num peers: " << num_peers_str << endl;
    int seed = stoi(seed_str);
    srand(seed);
    mt = mt19937(seed);
    int num_peers = stoi(num_peers_str);
    assert(num_peers>2);
    // cout << "num_peers: " << num_peers << endl;
    double z = stod(z_str);
    int max_blocks = stoi(max_blocks_str);

    //sum of independent poisson processes is poisson with sum of rates!
    //so we need R transactions per second, then R = sum(lambda) = sum(1/t_i) = n/t; t = n/R.
    double overall_txn_time = stod(t_str);
    double trans_per_second = 1/overall_txn_time;
    double mean_for_trans = num_peers/trans_per_second;
    
    double overall_block_time = stod(T_str);
    double stop_time = DBL_MAX;
    // int max_cycles = 1;
    int max_cycles = INT_MAX;

    double invalid_block_prob = 0; //this is not actual invalid block probability, it generate invalid block if possible
    bool uneven_hashing_power = false;
    
    Simulator cur = Simulator(uneven_hashing_power, seed, num_peers, z, mean_for_trans, overall_block_time, stop_time, max_cycles, invalid_block_prob, max_blocks); //O(n)
    cur.init_sim(); //O(n^2*log(n))
    // cur.print_state(1);
    cur.run_sim(); //
    cur.stop_sim();

    // for(int i=0; i<num_peers; i++){
    //     Peer* p = cur.get_peer_list_ele(i);
    //     assert(p->verify_blockchain());
    // }
    // cur.print_state(1);
    cur.save_state();
    // cur.analyse();
    // cur.export_network();
    // cur.export_blockchain_of(0);
    // cur.export_all_blockchain_data();
}