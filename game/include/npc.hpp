#ifndef NPC_DATA_HPP
#define NPC_DATA_HPP

#include <cstdint>
#include <string>

#include <arena_list.hpp>
#include <fixedpoint_num.hpp>

namespace game::npc{

constexpr uint8_t HETFO_FLAG        = 0b1;
constexpr uint8_t KEDD_FLAG         = 0b10;
constexpr uint8_t SZERDA_FLAG       = 0b100;
constexpr uint8_t CSUTORTOK_FLAG    = 0b1000;
constexpr uint8_t PENTEK_FLAG       = 0b10000;
constexpr uint8_t SZOMBAT_FLAG      = 0b100000;
constexpr uint8_t VASARNAP_FLAG     = 0b1000000;

struct Namedb{
    std::vector<std::string> firsts;
    std::vector<std::string> lasts;

    Namedb(const char* first_path, const char* last_path);
};

class World;

struct Residence;
struct Apartman;
struct Job;
struct Workplace;
struct People;
struct Connection;




struct Residence{
    uint8_t max_resident_count=0;
    People** residents=nullptr;
    Apartman* apartman=nullptr;

    uint64_t rent=0;

    Residence( uint8_t _max_resident_count, Apartman* _apartman, uint16_t apartman_idx );

};

struct Apartman{
    uint16_t max_residence_count=0;
    Residence** residenceses=nullptr;

    Apartman( uint16_t max_residence_count);

};

struct Job{
    uint8_t work_days = 0;
    uint8_t schedule = 0;

    uint16_t max_worker_count=0;
    People** workers=nullptr;
    Workplace* workplace;

    int64_t salery4hour = 0;

    Job( uint16_t _max_worker_count, Workplace* _workplace, uint16_t workplace_idx);
    
    int get_work_start()const{
        return schedule & 0b11111;
    }
    int get_work_lenght()const{
        return (schedule >> 5) + 4;
    }

};

struct Workplace{
    uint16_t max_job_count=0;
    Job** jobs=nullptr;

    Workplace( uint16_t _max_job_count);
};

struct People{
    bool sex = false;
    bool alive = true;

    util::Fixpoint_num elszantsag = 0.0;
    util::Fixpoint_num optimizmus = 0.0;
    util::Fixpoint_num szabalykovetes = 0.0;
    util::Fixpoint_num szepseg = 0.0;
    util::Fixpoint_num erosseg = 0.0;
    util::Fixpoint_num okossag = 0.0;

    int64_t balance=0;

    char* name=nullptr;
    Job* job=nullptr;
    Residence* home=nullptr;

    static constexpr uint64_t max_connection_amount = 32;
    Connection* connections = nullptr;

    People( const Namedb& namedb,  Job* _job, size_t job_idx, Residence* residence, size_t residence_idx);


    static double get_simpathy( People& p, People& target );
};

struct Connection{
    enum Connection_type{
        CON_FRIEND,
        CON_DATE,
        CON_PARENT,
        CON_CHILD,
        CON_MURDERER
    };

    Connection_type type = CON_FRIEND;
    People* target=nullptr;

    Connection() = default;
    Connection(Connection_type type, People* people)
        : type{type}, target{people} {}

    inline bool valid()const{
        return target != nullptr;
    }

    /// smaller the bigger
    inline int get_priority()const{
        switch (type) {
        case CON_CHILD:
            return 0;
        case CON_MURDERER:
            return 0;
        case CON_DATE:
            return 1;
        case CON_PARENT:
            return 2;
        case CON_FRIEND:
            return 3;
        default:
            return 0xfff;
        }
    }



    /// smaller the bigger
    static inline int get_priority(Connection_type type){
        switch (type) {
        case CON_CHILD:
            return 0;
        case CON_DATE:
            return 1;
        case CON_PARENT:
            return 2;
        case CON_FRIEND:
            return 3;
        default:
            return 0xfff;
        }
    }

};




class World{

    util::Arena_list<Residence*> residences;
    util::Arena_list<Apartman*> apartmans;
    util::Arena_list<Job*> jobs;
    util::Arena_list<Workplace*> workplaceses;
    util::Arena_list<People*> peoples;

public:

    World(size_t people_count);

    void update();

    void print();

};



}


#endif
