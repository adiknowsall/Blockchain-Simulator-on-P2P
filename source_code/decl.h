#ifndef DEF
#define DEF

#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
using namespace std;

extern mt19937 mt;

class Block;
class Link;
class Peer;
class Transaction;
class Event;
class Simulator;

Block* LCA(Block*, Block*);
class Block{
    int blk_id; //if someone generates invalid blocks, then there may be holes
    int creator_id; //id of peer which created this block
    int parent_blk_id; //supposed parent block id
    int parent_blk_creator_id; //(parent_blk_id, parent_blk_creator_id) uniquely identifies a block in any blockchain
    vector<Transaction*> trans_list; //list of transactions in the block 
    int size; //in bits (<=8*10^6); max transactions in a block = 999 
    double time_creation; //stores time when the block is created (mined)

    //DETAILS FOR THE RESPECTIVE BLOCKCHAIN:
    int depth; //length from the genesis block
    vector<Block*> children; //children of the current block
    Block* parent; //actual parent of the current block

    public:
        Block(int, double, Simulator*);
        int get_blk_id();
        int get_creator_id();
        void set_depth(int depth1);
        int get_depth();
        void addto_children(Block*);
        int get_children_size();
        Block* get_children_ele(int);
        Transaction* get_trans_list_ele(int i);
        int get_trans_list_size();
        Block* get_parent();
        void set_parent(Block*);
        void add_trans(Transaction*);
        int get_parent_blk_id();
        void set_parent_blk_id(int);
        int get_parent_blk_creator_id();
        void set_parent_blk_creator_id(int);
        int get_size();
        void update_blk_id(Simulator*);
        void export_tree_edges(int);
        bool exists_in_trans_list(Transaction*);
        double get_time_creation();
        bool verify_block(vector<double>);
        void fill_leaf_depths(vector<int> &ret);
};





class Link{
    double p; //initialised at start
    double c; //initialised at start 
    int id1, id2;
    public:
        Link(int id11, int id21, Simulator*);
        double get_p();
        double get_c();
        double sample_d();
        double get_latency(double size, Simulator*);
    
};




void total_count(Block*, vector<int>&);
bool custom_compare(pair<Block*, double> p1, pair<Block*, double> p2);
struct compareTransaction_amount{
    bool operator()(Transaction* e1, Transaction* e2);
};
class Peer{
    //MAY WANT TO STORE DETAILS ABOUT TRANSACTION WHERE IT CAME FROM AND WHERE THE PEER SENT IT, FOR BROADCAST FUNCTION
    int id;
    bool fast;
    //note that the coins owned by a peer may depend on the blockchain on each peer
    Block *head; //Blockchain's genesis block's pointer
    Block *last; //Pointer to the block where the peer is mining
    queue<Block*> pending; //queue containing blocks whose parents are not yet found in the blockchain
    map<pair<int,int>, Block*> id2addr; //maps (block id, creator id) to the address of the block
    unordered_map<Block*, double> arrival_time; //stores the arrival time of each block

    double t_x; //mean time for exp dist of interarrival time of transactions
    double T_x; //mean time for block creation
    vector<pair<int, Link*> > neigh; //list[(pid_j, link_ij)]
    double hashing_power; //fraction

    set<Transaction*, compareTransaction_amount> transaction_pool;
    //may want to store a state vector, storing things like coins owned by all the peers according to this peer's blockchain
    vector<double> balance; //stores the balance at "last" block

    set<pair<Transaction*, Peer*> > transaction_log; //stores (trans, peer) if the transaction was sent/received with that peer
    set<pair<Block*, Peer*> > block_log; //stores (block, peer) if the block was sent/heard to/from that peer

    int nv; //number of vertices in blockchain

    public:
        Peer(int, bool, double, double, Simulator*);
        Event* gen_transaction_create_event(Simulator*); //generates event for transaction creation with random sched_time
        Event* gen_transaction_hear_event(Transaction*, int, Simulator*);
        Event* gen_block_create_event(Simulator*); //generates event for block creation with random sched_time
        Event* gen_block_hear_event(Block*, int, Simulator*); //generates event for block receiving
        bool add_block(Block*, Block*, Simulator*, bool); //adds block to the blockchain aptly
        Transaction* gen_transaction(Simulator*); //generates a transaction
        // Block* gen_block(Simulator*); // generates a block
        void broadcast_transaction(Transaction* t, Simulator* s);
        void broadcast_block(Block* b, Simulator* s);
        void addto_transaction_pool(Transaction* t);
        bool addto_blockchain(Block*, Simulator*, bool);
        void addto_block_log(pair<Block*, Peer*>);
        void addto_transaction_log(pair<Transaction*, Peer*>);
        // void addto_arrival_time(Block*, double, Simulator*);
        void save_arrival_time(string);

        int get_id();
        pair<int,Link*> get_neigh_ele(int i);
        void addto_neigh(pair<int,Link*>);
        void set_fast(bool fast1);
        bool get_fast();
        Block* get_last();
        int get_neigh_size();
        double get_sum();
        double get_hashing_power();
        void update_balance(Transaction*);
        void print_cur_longest_blockchain();
        void print_tree();
        vector<vector<int> > analyse_blockchain(Simulator*);
        void export_blockchain(Simulator*);
        bool trans_in_blockchain(Transaction*);
        void export_blockchain_data(); //exports the longest chain
        void print_pending_set();
        bool verify_blockchain();
        vector<int> get_leaf_depths();

};






class Transaction{
    int payer_id; //if payer_id == -1, then coinbase transaction
    int payee_id;
    double amount;
    const double size = 8*1000; //1KB = 8*10^3 bits
    public:
        Transaction(int payer_id1, int payee_id1, double amount1);
        double get_size();
        int get_payer_id();
        double get_amount();
        int get_payee_id();
        
};






class Event{
    double sched_time;
    int creator_id; //id of person who created the event
    int hearer_id; //id of person who hears the event
    bool gen; //true if it is creation event, else false
    public:
        Event(double sched_time1, int creator_id1, bool gen1);
        double get_sched_time();
        int get_creator_id();
        bool get_gen();
        virtual bool get_is_trans_event() = 0;
        virtual Transaction* get_trans() = 0;
        virtual Block* get_block() = 0;
        virtual int get_hearer_id();
        virtual void set_hearer_id(int id1);
};
class Transaction_Event : public Event{
    Transaction* trans;
    public:
        Transaction_Event(Transaction* trans1, double sched_time1, int creator_id1, bool gen1);
        bool get_is_trans_event();
        Block* get_block();
        Transaction* get_trans();
};
class Block_Event : public Event{
    Block* block;
    public:
        Block_Event(Block*, double, int, bool);
        bool get_is_trans_event();
        Block* get_block();
        Transaction* get_trans();
};




struct compareTime{
    bool operator()(Event* e1, Event* e2);
};
void gen_random_connected_graph(vector<Peer*> &peer_list, Simulator*);

class Simulator{
    double overall_txn_time;
    double overall_block_time;
    int seed;
    int n;
    double z;
    double stop_time;
    int next_block_id;
    multiset<Event*, compareTime> event_queue;
    double cur_time;
    vector<Peer*> peer_list; //peer_list[id] contains the peer with ID=id
    double global_t_x;
    vector<double> T_x;
    int total_cycles;
    int max_blocks;
    double invalid_block_probability_per_peer; 
    public:
        Simulator(bool uneven, int seed1, int n1, double z1, double global_t_x1, double global_T_x1, double stop_time1, int total_cycles1, double invalid_block_probability_per_peer1, int max_blocks1);
        void init_sim();
        double get_cur_time();
        double get_total_hashing_power();
        void run_sim();
        void stop_sim();
        int get_n();
        double get_z();
        int get_max_blocks();
        int get_seed();
        double get_overall_txn_time();
        double get_overall_block_time();
        Peer* get_peer_list_ele(int);
        void addto_event_queue(Event*);
        int get_next_block_id();
        void print_state(bool);
        void save_state();
        void analyse(); //just for calculating the ratios
        void export_network();
        void export_blockchain_of(int);
        vector<double> get_T_x();
        double get_invalid_block_probability_per_peer();
        void export_all_blockchain_data(); // exports all longest chains

};






#endif