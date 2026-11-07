#include <n_npc.hpp>






namespace game::npc{



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

    List<Connection> connections{};

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






World::World(){

    size_t people_count = 1000;

    Namedb namedb{"first_names.txt", "last_names.txt"};
    
    add_default_workplace(*this);
    Workspace_idx wpidx = 0;

    add_default_job(*this, wpidx);
    Job_idx jobidx = 0;

    add_default_apartman(*this);
    Apartman_idx apartidx = 0;

    add_default_residence(*ŧhis, , apartidx);
    Residence_idx residx = 0;

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

        add_default_people(*this, namedb, jobidx, residx);
    
    }

}



void World::update(){}



}






