#include "fixedpoint_num.hpp"
#include <npc.hpp>


namespace game::npc
{
    

#define abs(x) (x<0?-x:x)

double People::get_simpathy( People& p, People& target ){
    double doptimizmus = (double)(p.optimizmus) - target.optimizmus;

    double hasonlosag = 1.0 - abs(doptimizmus);

    return hasonlosag * util::Fixpoint_num{(double)target.szepseg*2};
}

size_t count_peoples( People** peoples, size_t max_num ){
    for(size_t i=0;  i<max_num;  ++i ){
        if(peoples[i] == nullptr) return i;
    }
    return max_num;
}



int find_index_4_connection( People& p1, Connection::Connection_type type,
        People* target ){
    int prior = Connection::get_priority(type);
    int last_smaller_prior = -1;

    for( size_t i=0;  i<People::max_connection_amount;  i++ ){
        if( ! p1.connections[i].valid() )
            return i;
        int current_prior = p1.connections[i].get_priority();
        if(target == p1.connections[i].target ){
            if(current_prior > prior){
                return i;
            }else{
                return -2;
            }
        }
        if(current_prior > prior){
            last_smaller_prior = i;
        }
    }

    return last_smaller_prior;
}


void create_new_connection(People& p1, People& p2, 
        Connection::Connection_type type ){

    int p1i = find_index_4_connection(p1, type, &p2);
    int p2i = find_index_4_connection(p2, type, &p1);

    if( p1i < 0 || p2i < 0 ) return;

    p1.connections[p1i] = Connection(type, &p2);
    p2.connections[p2i] = Connection(type, &p1);
}

void create_one_way_connection(People& p1, People& p2, 
        Connection::Connection_type type ){
    int idx = find_index_4_connection(p1, type, &p2);

    if(idx < 0) return;

    p1.connections[idx] = Connection{type, &p2};
}

int count_connection_type(People& p, Connection::Connection_type type){
    int out=0;
    for(size_t i=0;  i<People::max_connection_amount;  i++){
        if( !p.connections[i].valid() ) break;
        if(type == p.connections[i].type) out++;
    }
    return out;
}
Connection* find_first_connection(People& p, Connection::Connection_type type){
    for(size_t i=0; i<People::max_connection_amount;  i++){
        if( ! p.connections[i].valid() ) break;
        if(type == p.connections[i].type) return &p.connections[i];
    }
    return nullptr;
}

void attack( People& p1, People& p2 ){
    int rn = rand();
    double pow =  (double)p1.erosseg / ((double)p1.erosseg+(double)p2.erosseg);
    if( pow * RAND_MAX > (double)rn ){
        p2.alive = false;
    }else{
        p1.alive = false;
    }
}


void interact(People& p1, People& p2){
    if(!p1.alive || !p2.alive) return; 

    double simp = People::get_simpathy(p1, p2);
    if(simp < 0.01){
        create_one_way_connection(p1, p2, Connection::CON_MURDERER);
        attack(p1, p2);
    }
    if(simp > 0.76 && p1.sex != p2.sex && People::get_simpathy(p2, p1) > 0.7
            && find_first_connection(p1, Connection::CON_DATE) == nullptr
            && find_first_connection(p2, Connection::CON_DATE) == nullptr) {
        create_new_connection(p1, p2, Connection::CON_DATE);
    }
    if(simp > 0.7 && p1.sex == p2.sex && People::get_simpathy(p2, p1) > 0.7) {
        create_new_connection(p1, p2, Connection::CON_FRIEND);
    }
}


void World::update(){

    for( auto people : peoples ){

        if( ! people->alive ) continue;

        int64_t diff = std::__popcount(people->job->work_days)
                            * people->job->get_work_lenght()
                            * people->job->salery4hour * 4;
        diff -= people->home->rent / people->home->max_resident_count;
        people->balance += diff;

        for( int i=0; i<people->job->max_worker_count; i++ ){
            if( people->job->workers[i] == nullptr || people->job->workers[i] == people )
                continue;
            interact(*people, *people->job->workers[i]);
        }

        for( int i=0; i<people->home->max_resident_count; i++ ){
            if( people->home->residents[i] == nullptr || people->home->residents[i] == people )
                continue;
            interact(*people, *people->home->residents[i]);
        }

        // interacting with a rando once an update
        auto rando = peoples[rand() % peoples.size()];
        if(rando != people){
            interact(*people, *rando);
        }

    }

}










}
