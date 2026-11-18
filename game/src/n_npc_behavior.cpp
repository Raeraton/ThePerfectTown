#include "fixedpoint_num.hpp"
#include <algorithm>
#include <cstdlib>
#include <iterator>
#include <n_npc.hpp>

#include <fstream>
#include <string>
#include <vector>






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



Group_event Group_event::cleanup(Peoples& peoples){
    
    Group_event out;

    for( Event_idx e : m_valid_events ){
        for( People_idx p : participant(e) ){
            auto& events = peoples.event(p);
            events.erase(e);
            events.insert(out.count());
        }
        out.add(m_locations[e], m_participants[e], m_types[e], m_times[e]);
    }

    return out;

}







static void add_default_residence(World& world,
        Apartman_idx apartman, uint8_t max_resident_count){
    List<People_idx> residents{};
    int64_t rent = 150000 + (rand() % 100000);
    world.add_residence(max_resident_count, residents,
            apartman, rent);
}

static void add_default_apartman(World& world, Residence_idx max_resident_count, const Location& location){
    List<Residence_idx> residenceses{};

    world.add_apartman(max_resident_count, residenceses, location);
}

static void add_default_job(World& world, Workspace_idx workplace,
        uint16_t max_worker_count){
    uint8_t work_days = HETFO_FLAG ^ KEDD_FLAG ^ SZERDA_FLAG
        ^ CSUTORTOK_FLAG ^ PENTEK_FLAG;

    uint8_t schedule = ((rand()%12)<<4) + (rand() & 0b1111);
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

static void add_default_workplace(World& world, uint16_t max_job_count, const Location& location){
    List<Job_idx> jobs{};

    world.add_workplace(max_job_count, jobs, location);
}

static void add_default_people(World& world,const Namedb& namedb, Jobs& jobs, Job_idx job, Residence_idx home){
    bool sex = rand() & 1;
    bool alive = true;

    util::Fixpoint_num elszantsag = (rand() % 100) / 100.0;
    util::Fixpoint_num optimizmus = (rand() % 100) / 100.0;
    util::Fixpoint_num szabalykovetes = (rand() % 100) / 100.0;
    util::Fixpoint_num szepseg = (rand() % 100) / 100.0;
    util::Fixpoint_num erosseg = (rand() % 100) / 100.0;
    util::Fixpoint_num okossag = (rand() % 100) / 100.0;
    util::Fixpoint_num hangulat = (rand() % 100) / 100.0;

    int64_t balance = 0;

    std::string name = 
        namedb.firsts[rand() % namedb.firsts.size()]
        + " " + namedb.lasts[rand() % namedb.lasts.size()];   

    Set<Connection> connections;
    Set<Event_idx> events;

    Weekly_schedule weekly_schedule;
    weekly_schedule.add_work(jobs, job);

    world.add_people(
        sex,
        alive,
        elszantsag,
        optimizmus,
        szabalykovetes,
        szepseg,
        erosseg,
        okossag,
        hangulat,
        balance,
        name,
        job,
        home,
        connections,
        events,
        weekly_schedule
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
        .insert(Connection(type, p1));
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

void create_new_event(Peoples& peoples, Group_event& events, const Location& location, Group_event::Type type,
        const List<People_idx>& participants, Time_range now){

    Event_idx event_idx = events.count();
    
    List<Weekly_schedule*> schedules;
    std::transform(
        participants.begin(), participants.end(), std::back_inserter(schedules),
        [&](People_idx idx){
            return (Weekly_schedule*)(&peoples.weekly_schedule(idx));
        }
    );

    Time_range time = Weekly_schedule::find_first(schedules, now.raw_hour(), 2);

    if( time.duration() == 0) return;

    for( auto sc : schedules ){
        sc->add_time(time);
    } 

    events.add(location, participants, type, time);

    for(People_idx part : participants){
        peoples.event(part).insert(event_idx);
    }
}




void party(Peoples& peoples, const List<People_idx>& parts){
    for( auto people : parts ){
        double han = peoples.hangulat(people);
        peoples.hangulat(people) = han + 0.2;
    }
}

void attack(Peoples& peoples, People_idx p1, People_idx p2){
    double h1 = peoples.hangulat(p1);
    double h2 = peoples.hangulat(p2);

    peoples.hangulat(p1) = h1 - 0.5;
    peoples.hangulat(p2) = h2 - 0.5;

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

void do_event(Peoples& peoples, Group_event& events, Event_idx event, Time_range time, Set<Event_idx>& events_to_remove){
    auto etime = events.time(event);

    if(etime < time){
        events_to_remove.insert(event);
        return;
    }
    if(!etime.is_overlapping(time)) return;
    
    /*/
    std::cout << "day " << (int)time.day() << " hour " << (int)time.hour()
                << " eday " << (int)etime.day() << " ehour " << (int)etime.hour() << " edur "
                << (int)etime.duration() << "     --" << event << "\n";
    */

    switch(events.type(event)){
    case Group_event::FIGHT:{
        List<People_idx> parts = events.participant(event);
        if(parts.size()>=2)
            attack(peoples, parts[0], parts[1]);
    }break;
    case Group_event::PARTY:{
        party(peoples, events.participant(event) );
    }break;
    default: break;
    }
}

void small_talk(Peoples& peoples, People_idx p1, People_idx p2, Group_event& events,
        Time_range now, const std::vector<Location>& location_list){
    if(p1 == p2) return;
    if(! peoples.get_alive(p1) || !peoples.get_alive(p2) ) return;

    double simp = peoples.get_simpathy(p1, p2);
    if( simp < 0.01 ){
        const Location& loc = location_list[rand() % location_list.size()];
        create_new_event(peoples, events, loc, Group_event::FIGHT, {p1, p2}, now);
    }if(simp>0.76 && peoples.get_sex(p1)!=peoples.get_sex(p2) && peoples.get_simpathy(p2, p1)
            && count_connection_type(peoples, p1, Connection::CON_DATE) == 0
            && count_connection_type(peoples, p2, Connection::CON_DATE) == 0 ){
        create_new_2way_connection(peoples, p1, p2, Connection::CON_DATE);
    }if(simp>0.7 && peoples.get_sex(p1)==peoples.get_sex(p2) && p1<p2){
        for(const auto& con : peoples.connections(p1)){
            if(con.target==p2 && con.type==Connection::CON_FRIEND) return;
        }
        create_new_2way_connection(peoples, p1, p2, Connection::CON_FRIEND);
    }
}

void interact(Peoples& peoples, People_idx people, const Connection& con,
        Group_event& events, Time_range now, const std::vector<Location>& location_list) {
   
    const Location& loc = location_list[rand() % location_list.size()];

    switch(con.type){
    case Connection::CON_FRIEND:{
        if( con.target < people && (rand()%6)==0 ){
            create_new_event(peoples, events, loc, Group_event::PARTY, {con.target, people}, now);
        }
    }break;
    case Connection::CON_COOWORKER:{
        small_talk(peoples, people, con.target, events, now, location_list);
    }break;
    case Connection::CON_ROOMMATE:{
        small_talk(peoples, people, con.target, events, now, location_list);
    }break;
    case Connection::CON_CHILD:{
        if( con.target < people && (rand()%4==0) ){
            create_new_event(peoples, events, loc, Group_event::PARTY, {people, con.target}, now);
        }
    }break;
    case Connection::CON_PARENT:{
        if( con.target < people && (rand()%8==0) ){
            create_new_event(peoples, events, loc, Group_event::PARTY, {people, con.target}, now);
        }
    }break;
    case Connection::CON_DATE:{
        if( con.target < people && (rand()%4==0) ){
            create_new_event(peoples, events, loc, Group_event::PARTY, {people, con.target}, now);
        }
    }break;
    case Connection::CON_NONE:
    default:
    break; 
    }

}

void interact_with_connections(Peoples& peoples, People_idx people, Group_event& events,
        Time_range now, const std::vector<Location>& location_list){
    Set<Connection> connections_copy = peoples.connections(people);
    for( const auto& con : connections_copy )
        interact(peoples, people, con, events, now, location_list);
}


        



World::World(size_t people_count, const std::vector<Location>& locations){

    Namedb namedb{"first_names.txt", "last_names.txt"};
    
    add_default_workplace(*this, 5, locations[rand() % locations.size()]);
    Workspace_idx wpidx = 0;

    add_default_job(*this, wpidx, 7);
    Job_idx jobidx = 0;
    workplaces.jobs(wpidx).push_back(jobidx);

    add_default_apartman(*this, 12, locations[rand()%locations.size()]);
    Apartman_idx apartidx = 0;

    add_default_residence(*this, apartidx, 3);
    Residence_idx residx = 0;
    apartmans.residenceses(apartidx).push_back(residx);

    People_idx peid = 0;

    for( size_t i=0;  i<people_count;  i++){

        if(     jobs.max_worker_count(jobidx) <=
                jobs.workers(jobidx).size()){
            add_default_job(*this, wpidx, 7);
            jobidx++;
            workplaces.jobs(wpidx).push_back(jobidx);
        }

        if(     workplaces.max_job_count(wpidx) <=
                workplaces.jobs(wpidx).size()){
            add_default_workplace(*this, 5, locations[rand() % locations.size()]);
            wpidx++;
        }

        if(     residences.max_resident_count(residx) <=
                residences.residents(residx).size()){
            add_default_residence(*this, apartidx, 3);
            residx++;
            apartmans.residenceses(apartidx).push_back(residx);
        }

        if(     apartmans.max_resident_count(apartidx) <=
                apartmans.residenceses(apartidx).size()){
            add_default_apartman(*this, 12, locations[rand()%locations.size()]);
            apartidx++;
        }

        residences.residents(residx).push_back(peid);
        jobs.workers(jobidx).push_back(peid);
        add_default_people(*this, namedb, jobs, jobidx, residx);
        peid++; 
        
    }
    

    for(size_t i=0;  i<peoples.count();  i++){
        Job_idx job = peoples.job(i);
        Residence_idx home = peoples.home(i);

        for( auto roomate : residences.residents(home) )
            create_new_2way_connection(peoples, i, roomate, Connection::CON_ROOMMATE);

        for(auto cooworker : jobs.workers(job))
            create_new_2way_connection(peoples, i, cooworker, Connection::CON_COOWORKER);
    }

}



void World::update(Time_range time, bool time_to_pay_rent, const std::vector<Location>& locations){

    for(  size_t peid = 0;  peid < peoples.count();  peid++){
        if( ! peoples.get_alive(peid) ) continue;

        Job_idx job_idx = peoples.job(peid);
        Residence_idx res_idx = peoples.home(peid);

        int64_t money_diff=0;
        if(time_to_pay_rent) money_diff -= residences.rent(res_idx);
        if( jobs.is_working_now(job_idx, time) ){
            money_diff += jobs.salery4hour(job_idx);
        }
        peoples.balance(peid) += money_diff;

        interact_with_connections(peoples, peid, events, time, locations);

        People_idx rando = rand() % peoples.count();
        if(rand()%24 == 0)
            small_talk(peoples, peid, rando, events, time, locations);
    }

    Set<Event_idx> events_to_delete;
    for(  auto e : events ){
        do_event(peoples, events, e, time, events_to_delete);
    }

    events.remove(events_to_delete, peoples);

    if( time.hour() == 0 )
        events = events.cleanup(peoples);

}



static void print_people(Peoples& peoples, Jobs& jobs, People_idx idx) {
    Job_idx job = peoples.job(idx);

    std::cout
        << "name: " << peoples.name(idx)
        << "\t alive: " << (peoples.get_alive(idx) ? "alive" : "dead")
        << "\t sex: " << (peoples.get_sex(idx) ? "male" : "female")
        << "\t salery: " << jobs.salery4hour(job)
        << "\t work time: " << (int)jobs.get_work_len(job)
        << "\t work start " << (int)jobs.get_work_start(job)
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
    std::cout << "number of apartmans " << apartmans.count() << "\n";
    std::cout << "number of residents " << residences.count() << "\n";
    std::cout << "number of jobs " << jobs.count() << "\n";
    std::cout << "number of workplaces " << workplaces.count() << "\n";
    std::cout << "number of events: " << events.count() << "\n";
    std::cout << "number of valid events: " << events.valid_event_count() << "\n";
    std::cout << "number of dates: " << num_off_date << "\n";
    std::cout << "number of friendships: " << num_off_friend << "\n";
    std::cout << "number of murderers: " << num_off_murderers << "\n";
}





}






