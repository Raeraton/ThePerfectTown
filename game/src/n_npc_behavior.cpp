#include "fixedpoint_num.hpp"
#include <cstdlib>
#include <n_npc.hpp>

#include <fstream>
#include <string>






namespace game::npc{


Namedb::Namedb(const char* first_path, const char* last_path){
    std::ifstream first_file{first_path};
    if( ! first_file.is_open() ) 
        throw std::string{"namedb cannot open first_path"};
    std::string line;
    while( std::getline(first_file, line) ){
        if( line == "" ) continue;
        firsts.push_back(line);
    }
    first_file.close();


    std::ifstream last_file{last_path};
    if( ! last_file.is_open() ) 
        throw std::string{"namedb cannot open last_path"};
    while( std::getline(last_file, line) ){
        if(line == "") continue;
        lasts.push_back(line);
    }
    last_file.close();

}



static void add_default_residence(World& world,
        Apartman_idx apartman){
    uint8_t max_resident_count = 8;
    List<People_idx> residents{};
    int64_t rent = 150000 + (rand() % 100000);
    world.add_residence(max_resident_count, residents,
            apartman, rent);
}

static void add_default_apartman(World& world){
    Residence_idx max_resident_count = 16;
    List<Residence_idx> residenceses{};

    world.add_apartman(max_resident_count, residenceses);
}

static void add_default_job(World& world, Workspace_idx workplace){
    uint8_t work_days = HETFO_FLAG & KEDD_FLAG & SZERDA_FLAG
        & CSUTORTOK_FLAG & PENTEK_FLAG;

    uint8_t schedule = (rand() % 10) + (rand() % 4) * 32;

    uint16_t max_worker_count = 8;
    List<People_idx> workers{};

    int64_t salery4hour = 1200 + (rand() % 2000);

    world.add_job(
        work_days,
        schedule,
        max_worker_count,
        workers,
        workplace,
        salery4hour
    );
}

static void add_default_workplace(World& world){
    uint16_t max_job_count = 12;
    List<Job_idx> jobs{};

    world.add_workplace(max_job_count, jobs);
}

static void add_default_people(World& world,const Namedb& namedb, Job_idx job, Residence_idx home){
    bool sex = rand() % 2;
    bool alive = true;

    util::Fixpoint_num elszantsag = (rand() % 100) / 100.0;
    util::Fixpoint_num optimizmus = (rand() % 100) / 100.0;
    util::Fixpoint_num szabalykovetes = (rand() % 100) / 100.0;
    util::Fixpoint_num szepseg = (rand() % 100) / 100.0;
    util::Fixpoint_num erosseg = (rand() % 100) / 100.0;
    util::Fixpoint_num okossag = (rand() % 100) / 100.0;

    int64_t balance = rand() % 500000;

    std::string name = 
        namedb.firsts[rand() % namedb.firsts.size()]
        + " " + namedb.lasts[rand() % namedb.lasts.size()];   

    Set<Connection> connections;

    world.add_people(
        sex,
        alive,
        elszantsag,
        optimizmus,
        szabalykovetes,
        szepseg,
        erosseg,
        okossag,
        balance,
        name,
        job,
        home,
        connections
    );
}


#define abs(x) (x<0?-x:x)

double Peoples::get_simpathy(People_idx p, People_idx target){
    double dopt = (double)optimizmus(p) - optimizmus(target);

    double hasonlosag = 1.0 - abs(dopt);

    return hasonlosag * util::Fixpoint_num{(double)szepseg(target) * 2};
}

void create_new_2way_connection( Peoples& peoples,
        People_idx p1, People_idx p2, Connection::Connection_type type){
    if(p1 >= peoples.count() || p2 >= peoples.count() )
        throw std::string{"Creating invalid connection "}
        + std::to_string(p1) + " <-> " + std::to_string(p2);

    peoples.connections(p1)
        .insert(Connection(type, p2));
    peoples.connections(p2)
        .insert(Connection(type, p2));
}

void create_new_1way_connection( Peoples& peoples,
        People_idx from, People_idx _for, Connection::Connection_type type){
    if(from >= peoples.count() || _for >= peoples.count() )
        throw std::string{"Creating invalid connection "}
        + std::to_string(from) + " -> " + std::to_string(_for);

    peoples.connections(from)
        .insert(Connection(type, _for));
}

size_t count_connection_type(Peoples& peoples, People_idx idx,
        Connection::Connection_type type){
    size_t out=0;
    for(const auto& con : peoples.connections(idx))
        if( con.type == type ) out++;
    return out;
}






void attack(Peoples& peoples, People_idx p1, People_idx p2){
    int rn = rand();
    double pow = (double)peoples.erosseg(p1) / ((double)peoples.erosseg(p1) + peoples.erosseg(p2));
    if( pow*RAND_MAX > rn ){
        peoples.set_alive(p2, false);
        create_new_1way_connection(peoples, p2, p1, Connection::CON_MURDERER);
    }else{
        peoples.set_alive(p1, false);
        create_new_1way_connection(peoples, p1, p2, Connection::CON_MURDERER);
    }
}

void interact(Peoples& peoples, People_idx p1, People_idx p2){
    if(p1 == p2) return;
    if(! peoples.get_alive(p1) || !peoples.get_alive(p2) ) return;

    double simp = peoples.get_simpathy(p1, p2);
    if( simp < 0.01 ){
        attack(peoples, p1, p2);
    }if(simp>0.76 && peoples.get_sex(p1)!=peoples.get_sex(p2) && peoples.get_simpathy(p2, p1)
            && count_connection_type(peoples, p1, Connection::CON_DATE) == 0
            && count_connection_type(peoples, p2, Connection::CON_DATE) == 0 ){
        create_new_2way_connection(peoples, p1, p2, Connection::CON_DATE);
    }if(simp>0.7 && peoples.get_sex(p1)==peoples.get_sex(p2)){
        create_new_2way_connection(peoples, p1, p2, Connection::CON_FRIEND);
    }
}

void interact_with_connections(Peoples& peoples, People_idx people){
    Set<Connection> connections_copy = peoples.connections(people);
    for( const auto& con : connections_copy )
        interact(peoples, people, con.target );
}

void interact_with_roomates(Residenceses& residences, Peoples& peoples, People_idx people){
    Residence_idx residx = peoples.home(people);
    for( People_idx oth : residences.residents(residx) )
        interact(peoples, people, oth);
}

void interact_with_cooworkers( Jobs& jobs, Peoples& peoples, People_idx people){
    Job_idx job = peoples.job(people);
    for( People_idx oth : jobs.workers(job) )
        interact(peoples, people, oth);
}

        



World::World(size_t people_count){

    Namedb namedb{"first_names.txt", "last_names.txt"};
    
    add_default_workplace(*this);
    Workspace_idx wpidx = 0;

    add_default_job(*this, wpidx);
    Job_idx jobidx = 0;
    workplaces.jobs(wpidx).push_back(jobidx);

    add_default_apartman(*this);
    Apartman_idx apartidx = 0;

    add_default_residence(*this, apartidx);
    Residence_idx residx = 0;
    apartmans.residenceses(apartidx).push_back(residx);

    People_idx peid = 0;

    for( size_t i=0;  i<people_count;  i++){

        if(     jobs.max_worker_count(jobidx) <=
                jobs.workers(jobidx).size()){
            add_default_job(*this, wpidx);
            jobidx++;
            workplaces.jobs(wpidx).push_back(jobidx);
        }

        if(     workplaces.max_job_count(wpidx) <=
                workplaces.jobs(wpidx).size()){
            add_default_workplace(*this);
            wpidx++;
        }

        if(     residences.max_resident_count(residx) <=
                residences.residents(residx).size()){
            add_default_residence(*this, apartidx);
            residx++;
            apartmans.residenceses(apartidx).push_back(residx);
        }

        if(     apartmans.max_resident_count(apartidx) <=
                apartmans.residenceses(apartidx).size()){
            add_default_apartman(*this);
            apartidx++;
        }

        residences.residents(residx).push_back(peid);
        jobs.workers(jobidx).push_back(peid);
        add_default_people(*this, namedb, jobidx, residx);
        peid++; 
        
    }

}



void World::update(){

    for(  size_t peid = 0;  peid < peoples.count();  peid++){
        if( ! peoples.get_alive(peid) ) continue;

        Job_idx job_idx = peoples.job(peid);

        int64_t diff = std::__popcount(jobs.work_days(job_idx))
            * 8 * jobs.salery4hour(job_idx);
        peoples.balance(peid) += diff;

        interact_with_roomates(residences, peoples, peid);
        interact_with_cooworkers(jobs, peoples, peid);
        interact_with_connections(peoples, peid);

        People_idx rando = rand() % peoples.count();
        interact(peoples, peid, rando);
    }

}



static void print_people(Peoples& peoples, Jobs& jobs, People_idx idx) {
    Job_idx job = peoples.job(idx);

    std::cout
        << "name: " << peoples.name(idx)
        << "\t alive: " << (peoples.get_alive(idx) ? "alive" : "dead")
        << "\t sex: " << (peoples.get_sex(idx) ? "male" : "female")
        << "\t salery: " << jobs.salery4hour(job)
        << "\t work time: " << ((jobs.schedule(job) >> 5) + 4)
        << "\t balance: " << peoples.balance(idx)
        << "\t will: " << peoples.elszantsag(idx)
        << "\t optimism: " << peoples.optimizmus(idx)
        << "\t caotism-1: " << peoples.szabalykovetes(idx)
        << "\t look: " << peoples.szepseg(idx)
        << "\t power: " << peoples.erosseg(idx)
        << "\t smart: " << peoples.okossag(idx);
}

void World::print(){

    std::cout << "===============================\n"
        << "PEOPLES"
        << "\n===============================\n";

    for(People_idx i = 0; i < peoples.count(); i++) {
        print_people(peoples, jobs, i);
        std::cout << "\n";
    }

    std::cout << "===============================\n"
        << "CONNECTIONS"
        << "\n===============================\n";

    size_t num_off_date = 0;
    size_t num_off_friend = 0;
    size_t num_off_murderers = 0;

    for(People_idx i = 0; i < peoples.count(); i++) {

        if(!peoples.get_alive(i))
            num_off_murderers++;

        const auto& connections = peoples.connections(i);

        for(const Connection& connection : connections) {

            People_idx target = connection.target;

            // Invalid target index
            if(target >= peoples.count())
                continue;

            if(connection.type == Connection::CON_FRIEND && i < target) {

                std::cout << "FRIEND--------------------------\n";

                print_people(peoples, jobs, i);
                std::cout << "\n";

                print_people(peoples, jobs, target);
                std::cout << "\n";

                std::cout
                    << peoples.get_simpathy(i, target)
                    << " <-> "
                    << peoples.get_simpathy(target, i)
                    << "\n";

                num_off_friend++;

            }
            else if(connection.type == Connection::CON_DATE && i < target) {

                std::cout << "DATE----------------------------\n";

                print_people(peoples, jobs, i);
                std::cout << "\n";

                print_people(peoples, jobs, target);
                std::cout << "\n";

                std::cout
                    << peoples.get_simpathy(i, target)
                    << " <-> "
                    << peoples.get_simpathy(target, i)
                    << "\n";

                num_off_date++;

            }
            else if(connection.type == Connection::CON_MURDERER) {

                std::cout << "MURDERED------------------------\n";

                print_people(peoples, jobs, i);
                std::cout << "\n";

                print_people(peoples, jobs, target);
                std::cout << "\n";

                std::cout
                    << peoples.get_simpathy(i, target)
                    << " <-> "
                    << peoples.get_simpathy(target, i)
                    << "\n";
            }
        }
    }

    std::cout << "===============================\n"
        << "OVERALL"
        << "\n===============================\n";

    std::cout << "number off dates: " << num_off_date << "\n";
    std::cout << "number off friendships: " << num_off_friend << "\n";
    std::cout << "number off murderers: " << num_off_murderers << "\n";
}





}






