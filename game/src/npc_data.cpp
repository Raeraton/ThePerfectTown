#include <npc.hpp>
#include <fstream>
#include <bit>

namespace game::npc{



Namedb::Namedb(const char* first_path, const char* last_path){
    std::ifstream first_file{first_path};
    if( ! first_file.is_open() ) 
        throw "namedb cannot open first_path";
    std::string line;
    while( std::getline(first_file, line) ){
        if( line == "" ) continue;
        firsts.push_back(line);
    }
    first_file.close();


    std::ifstream last_file{last_path};
    if( ! last_file.is_open() ) 
        throw "namedb cannot open last_path";
    while( std::getline(last_file, line) ){
        if(line == "") continue;
        lasts.push_back(line);
    }
    last_file.close();

}


Residence::Residence(uint8_t _max_resident_count, Apartman* _apartman, uint16_t apartman_idx )
        : max_resident_count{_max_resident_count}{
    
    residents = new People*[max_resident_count];
    for( int i=0;  i<max_resident_count;  i++ ){
        residents[i] = nullptr;
    }

    rent = 150000 + (rand()%100000);

    apartman = _apartman;
    apartman->residenceses[apartman_idx] = this;

}
Apartman::Apartman(uint16_t max_residence_count)
        : max_residence_count{max_residence_count}{
    
    residenceses = new Residence*[max_residence_count];
    for( int i=0;  i<max_residence_count;  i++ ){
        residenceses[i] = nullptr;
    }
}

Job::Job( uint16_t _max_worker_count, Workplace* _workplace, uint16_t workplace_idx)
        : max_worker_count{_max_worker_count}{
    
    workers = new People*[max_worker_count];
    for(int i=0;  i<max_worker_count;  i++){
        workers[i] = nullptr;
    }

    salery4hour = 1800 + (rand() % 800);
    schedule = rand() & 0xff;
    work_days = HETFO_FLAG + KEDD_FLAG + SZERDA_FLAG + CSUTORTOK_FLAG + PENTEK_FLAG;

    workplace = _workplace;
    workplace->jobs[workplace_idx] = this;

}



Workplace::Workplace( uint16_t _max_job_count)
        : max_job_count{_max_job_count}{

    jobs = new Job*[max_job_count];
    for(int i=0;  i<max_job_count;  i++){
        jobs[i] = nullptr;
    }
    
}





People::People( const Namedb& namedb, Job* _job, size_t job_idx, Residence* residence, size_t residence_idx){
    std::string first_name = namedb.firsts[rand() % namedb.firsts.size()];
    std::string last_name = namedb.lasts[rand() % namedb.lasts.size()];
    
    name = new char[32]{0};
    std::snprintf(name, 32, "%s %s", first_name.c_str(), last_name.c_str());

    sex = rand() & 1;

    elszantsag = util::Fixpoint_num::random();
    optimizmus = util::Fixpoint_num::random();
    szabalykovetes = util::Fixpoint_num::random();
    szepseg = util::Fixpoint_num::random();
    erosseg = util::Fixpoint_num::random();
    okossag = util::Fixpoint_num::random();

    connections = new Connection[max_connection_amount];

    job = _job;
    job->workers[job_idx] = this;

    home = residence;
    home->residents[residence_idx] = this;
}


World::World(size_t people_count){

    Namedb namedb{"first_names.txt", "last_names.txt"};

    Workplace* wp = new Workplace{2};
    uint16_t wp_idx=1;

    Job* cj = new Job{6, wp, 0};
    uint16_t cj_idx = 0;

    Apartman* ap  = new Apartman{8};
    uint16_t ap_idx=1;

    Residence* cr = new Residence{2, ap, 0};
    uint8_t cr_idx = 0;



    for(size_t i=0;  i<people_count;  i++){

        if(cj_idx >= cj->max_worker_count){
            jobs.push_back(cj);
            cj = new Job{6, wp, wp_idx};
            wp_idx++;
            cj_idx = 0;
        }

        if( wp_idx >= wp->max_job_count ){
            workplaceses.push_back(wp);
            wp = new Workplace{2};
            wp_idx = 0;
        }

        if( cr_idx >= cr->max_resident_count ){
            residences.push_back(cr);
            cr = new Residence{2, ap, ap_idx};
            ap_idx++;
            cr_idx = 0;
        }

        if( ap_idx >= ap->max_residence_count ){
            apartmans.push_back(ap);
            ap = new Apartman{8};
            ap_idx = 0;
        }



        People* people = new People{namedb, cj, cj_idx, cr, cr_idx};
        cj_idx++;
        cr_idx++;

        peoples.push_back(people);

    }
}



std::ostream& operator<<(std::ostream& out, const People& people){
    out
        << "name: " << people.name
        << "\t alive: " << (people.alive?"alive":"dead")
        << "\t sex: " << (people.sex?"male":"female")
        << "\t salery: " << people.job->salery4hour
        << "\t work time: " << people.job->get_work_lenght()
        << "\t balance: " << people.balance
        << "\t will: " << people.elszantsag
        << "\t optimism: " << people.optimizmus
        << "\t caotism-1: " << people.szabalykovetes
        << "\t look: " << people.szepseg
        << "\t power: " << people.erosseg
        << "\t smart: " << people.okossag;
    return out;
}


void World::print(){

    std::cout << "===============================\n"
        << "PEOPLES"
        <<"\n===============================\n";
    for( auto people : peoples ){
        std::cout << *people << "\n";
    }

    std::cout << "===============================\n"
        << "CONNECTIONS"
        <<"\n===============================\n";
    // count everything
    size_t num_off_date=0;
    size_t num_off_friend=0;
    size_t num_off_murderers=0;
    for( auto p : peoples ){
        if(!p->alive) num_off_murderers++;
            
        for(size_t i=0;  i<People::max_connection_amount;  i++){
            // implicit nullptr checking (p < ....) because p is never null
            // p->... is smaller if null
            if(p->connections[i].type == Connection::CON_FRIEND && p < p->connections[i].target && p->connections[i].target != nullptr){
                std::cout << "FRIEND--------------------------\n";
                std::cout << *p << "\n";
                std::cout << *p->connections[i].target << "\n";
                std::cout << People::get_simpathy(*p, *p->connections[i].target)
                    << " <-> " << People::get_simpathy(*p->connections[i].target, *p) << "\n";
                num_off_friend++;
            }else if(p->connections[i].type == Connection::CON_DATE && p < p->connections[i].target && p->connections[i].target != nullptr){
                std::cout << "DATE----------------------------\n";
                std::cout << *p << "\n";
                std::cout << *p->connections[i].target << "\n";
                std::cout << People::get_simpathy(*p, *p->connections[i].target)
                    << " <-> " << People::get_simpathy(*p->connections[i].target, *p) << "\n";
                num_off_date++;
            }else if(p->connections[i].type == Connection::CON_MURDERER && p->connections[i].target != nullptr){
                std::cout << "MURDERED------------------------\n";
                std::cout << *p << "\n";
                std::cout << *p->connections[i].target << "\n";
                std::cout << People::get_simpathy(*p, *p->connections[i].target)
                    << " <-> " << People::get_simpathy(*p->connections[i].target, *p) << "\n";
            }
        }
    }

    std::cout << "===============================\n"
        << "OVERALL"
        <<"\n===============================\n";
    std::cout << "number off dates: " << num_off_date<< "\n";
    std::cout << "number off friendships: " << num_off_friend<< "\n";
    std::cout << "number off murderers: " << num_off_murderers << "\n";


}



}

