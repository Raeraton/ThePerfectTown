#ifndef N_NPC_HPP
#define N_NPC_HPP

#include <unordered_set>
#include <vector>
#include <cstdint>
#include <string>

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


template<typename T>
using List = std::vector<T>;

template<typename T>
using Set = std::unordered_set<T>;

typedef uint32_t People_idx;
typedef uint32_t Apartman_idx;
typedef uint32_t Residence_idx;
typedef uint32_t Job_idx;
typedef uint32_t Workspace_idx;




struct Connection{
    enum Connection_type{
        CON_FRIEND,
        CON_DATE,
        CON_PARENT,
        CON_CHILD,
        CON_MURDERER,
        CON_COOWORKER,
        CON_NONE
    };

    Connection_type type=CON_NONE;
    People_idx target=0;

    Connection() = default;
    Connection(Connection_type type, People_idx people)
        : type{type}, target{people} {}

    inline bool valid()const{
        return type != CON_NONE;
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
        case CON_NONE:
        default:
            return 0xfff;
        }
    }

    /// smaller the bigger
    static inline int get_priority(Connection_type type){
        switch (type) {
        case CON_CHILD:
        case CON_MURDERER:
            return 0;
        case CON_DATE:
            return 1;
        case CON_PARENT:
            return 2;
        case CON_FRIEND:
            return 3;
        case CON_NONE:
        default:
            return 0xfff;
        }
    }

    inline bool operator==(const Connection& oth) const noexcept{
        return target==oth.target;
    }

};




class Residenceses {
    size_t m_count=0;
    List<uint8_t> m_max_resident_counts;
    List<List<People_idx>> m_residents;
    List<Apartman_idx> m_apartmans;
    List<int64_t> m_rents;

    inline void check_integrity()const{
        if(
            m_count != m_max_resident_counts.size() ||
            m_count != m_residents.size() ||
            m_count != m_apartmans.size() ||
            m_count != m_rents.size()
        ) throw std::string{"Residenceses integrity error"};
    }

public:

    Residenceses() = default;

    inline void add(
            uint8_t max_resident_count,
            const List<People_idx>& residents,
            Apartman_idx apartman,
            int64_t rent
    ){
        m_count++;
        m_max_resident_counts.push_back(max_resident_count);
        m_residents.push_back(residents);
        m_apartmans.push_back(apartman);
        m_rents.push_back(rent);
        check_integrity();
    }

    inline size_t count() const {
        return m_count;
    }

    inline uint8_t& max_resident_count(Residence_idx idx){
        if(idx >= m_count) throw std::string{"Residence idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_max_resident_counts[idx];
    }
    
    inline List<People_idx>& residents(Residence_idx idx){
        if(idx >= m_count) throw std::string{"Residence idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_residents[idx];
    }
    
    inline Apartman_idx apartman(Residence_idx idx){
        if(idx >= m_count) throw std::string{"Residence idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_apartmans[idx];
    }

    inline int64_t& rent(Residence_idx idx){
        if(idx >= m_count) throw std::string{"Residence idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_rents[idx];
    }

};


class Apartmans{

    size_t m_count=0;
    List<Residence_idx> m_max_residence_count;
    List<List<Residence_idx>> m_residenceses;

    inline void check_integrity() const {
        if(
                m_count != m_max_residence_count.size() ||
                m_count != m_residenceses.size()
          ) throw std::string{"Apartmans integrity error"};
    }

public:

    Apartmans() = default;

    inline void add(
                Residence_idx max_resident_count,
                const List<Residence_idx>& residenceses
            ){
        m_count++;
        m_max_residence_count.push_back(max_resident_count);
        m_residenceses.push_back(residenceses);
        check_integrity();
    }
    
    inline size_t count() const {
        return m_count;
    }

    inline Residence_idx max_resident_count(Apartman_idx idx){
        if( idx >= m_count) throw std::string{"Apartmans idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_max_residence_count[idx];
    }
    inline List<Residence_idx>& residenceses(Apartman_idx idx){
        if( idx >= m_count) throw std::string{"Apartmans idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_residenceses[idx];
    }

};

class Jobs {
    size_t m_count=0;
    List<uint8_t> m_work_days;
    List<uint8_t> m_schedules;
    List<uint16_t> m_max_worker_counts;
    List<List<People_idx>> m_workers;
    List<Workspace_idx> m_workplaces;
    List<int64_t> m_salery4hours;

    inline void check_integrity() const {
        if(
            m_count != m_work_days.size() ||
            m_count != m_schedules.size() ||
            m_count != m_max_worker_counts.size() ||
            m_count != m_workers.size() ||
            m_count != m_workplaces.size() ||
            m_count != m_salery4hours.size()
        ) throw std::string{"Jobs integrity error"};
    }

public:

    Jobs() = default;

    inline void add(
        uint8_t work_days,
        uint8_t schedule,
        uint16_t max_worker_count,
        const List<People_idx>& workers,
        Workspace_idx workplace,
        int64_t salery4hour
    ){
        m_count++;
        m_work_days.push_back(work_days);
        m_schedules.push_back(schedule);
        m_max_worker_counts.push_back(max_worker_count);
        m_workers.push_back(workers);
        m_workplaces.push_back(workplace);
        m_salery4hours.push_back(salery4hour);
        check_integrity();
    }

    inline size_t count() const {
        return m_count;
    }

    inline uint8_t& work_days(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_work_days[idx];
    }

    inline uint8_t& schedule(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_schedules[idx];
    }

    inline uint16_t& max_worker_count(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_max_worker_counts[idx];
    }

    inline List<People_idx>& workers(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_workers[idx];
    }

    inline Workspace_idx& workplace(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_workplaces[idx];
    }

    inline int64_t& salery4hour(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_salery4hours[idx];
    }
};



class Workplaces {

    size_t m_count=0;
    List<uint16_t> m_max_job_counts;
    List<List<Job_idx>> m_jobs;

    inline void check_integrity() const {
        if(
            m_count != m_max_job_counts.size() ||
            m_count != m_jobs.size()
        ) throw std::string{"Workplaces integrity error"};
    }

public:

    Workplaces() = default;

    inline void add(
        uint16_t max_job_count,
        const List<Job_idx>& jobs
    ){
        m_count++;
        m_max_job_counts.push_back(max_job_count);
        m_jobs.push_back(jobs);
        check_integrity();
    }

    inline size_t count() const {
        return m_count;
    }

    inline uint16_t& max_job_count(Workspace_idx idx){
        if(idx >= m_count) throw std::string{"Workplace idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_max_job_counts[idx];
    }

    inline List<Job_idx>& jobs(Workspace_idx idx){
        if(idx >= m_count) throw std::string{"Workplace idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_jobs[idx];
    }
};



class Peoples {

    size_t m_count=0;

    List<bool> m_sexes;
    List<bool> m_alives;

    List<util::Fixpoint_num> m_elszantsags;
    List<util::Fixpoint_num> m_optimizmuses;
    List<util::Fixpoint_num> m_szabalykovetesek;
    List<util::Fixpoint_num> m_szepsegek;
    List<util::Fixpoint_num> m_erossegek;
    List<util::Fixpoint_num> m_okossagok;

    List<int64_t> m_balances;

    List<std::string> m_names;
    List<Job_idx> m_jobs;
    List<Residence_idx> m_homes;

    List<Set<Connection>> m_connections;

    inline void check_integrity() const {
        if(
            m_count != m_sexes.size() ||
            m_count != m_alives.size() ||
            m_count != m_elszantsags.size() ||
            m_count != m_optimizmuses.size() ||
            m_count != m_szabalykovetesek.size() ||
            m_count != m_szepsegek.size() ||
            m_count != m_erossegek.size() ||
            m_count != m_okossagok.size() ||
            m_count != m_balances.size() ||
            m_count != m_names.size() ||
            m_count != m_jobs.size() ||
            m_count != m_homes.size() ||
            m_count != m_connections.size()
        ) throw std::string{"Peoples integrity error"};
    }

public:

    Peoples() = default;

    inline void add(
        bool sex,
        bool alive,
        util::Fixpoint_num elszantsag,
        util::Fixpoint_num optimizmus,
        util::Fixpoint_num szabalykovetes,
        util::Fixpoint_num szepseg,
        util::Fixpoint_num erosseg,
        util::Fixpoint_num okossag,
        int64_t balance,
        const std::string& name,
        Job_idx job,
        Residence_idx home,
        const Set<Connection>& connections
    ){
        m_count++;
        m_sexes.push_back(sex);
        m_alives.push_back(alive);
        m_elszantsags.push_back(elszantsag);
        m_optimizmuses.push_back(optimizmus);
        m_szabalykovetesek.push_back(szabalykovetes);
        m_szepsegek.push_back(szepseg);
        m_erossegek.push_back(erosseg);
        m_okossagok.push_back(okossag);
        m_balances.push_back(balance);
        m_names.push_back(name);
        m_jobs.push_back(job);
        m_homes.push_back(home);
        m_connections.push_back(connections);
        check_integrity();
    }

    inline size_t count() const {
        return m_count;
    }

    inline bool get_sex(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_sexes[idx];
    }

    inline bool get_alive(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_alives[idx];
    }

    inline void set_sex(People_idx idx, bool v){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        m_sexes[idx] = v;
    }

    inline void set_alive(People_idx idx, bool v){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        m_alives[idx] = v;
    }

    inline util::Fixpoint_num& elszantsag(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_elszantsags[idx];
    }

    inline util::Fixpoint_num& optimizmus(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_optimizmuses[idx];
    }

    inline util::Fixpoint_num& szabalykovetes(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_szabalykovetesek[idx];
    }

    inline util::Fixpoint_num& szepseg(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_szepsegek[idx];
    }

    inline util::Fixpoint_num& erosseg(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_erossegek[idx];
    }

    inline util::Fixpoint_num& okossag(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_okossagok[idx];
    }

    inline int64_t& balance(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_balances[idx];
    }

    inline std::string& name(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_names[idx];
    }

    inline Job_idx& job(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_jobs[idx];
    }

    inline Residence_idx& home(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_homes[idx];
    }

    inline Set<Connection>& connections(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_connections[idx];
    }

    double get_simpathy( People_idx p, People_idx target );
};





class World{

    Peoples peoples;

    Workplaces workplaces;
    Jobs jobs;

    Apartmans apartmans;
    Residenceses residences;

public:

    World(size_t people_count);

    void update();

    void print();

    void add_residence(
            uint8_t max_resident_count,
            const List<People_idx>& residents,
            Apartman_idx apartman,
            int64_t rent
    ){ residences.add(max_resident_count, residents, apartman, rent); }

    inline void add_apartman(
            Residence_idx max_resident_count,
            const List<Residence_idx>& _residenceses){
        apartmans.add(max_resident_count, _residenceses );
    }

    inline void add_job(
            uint8_t work_days,
            uint8_t schedule,
            uint16_t max_worker_count,
            const List<People_idx>& workers,
            Workspace_idx workplace,
            int64_t salery4hour){
        jobs.add( work_days, schedule, max_worker_count, workers,
                workplace, salery4hour );
    }

    inline void add_workplace(
            uint16_t max_job_count,
            const List<Job_idx>& _jobs){
        workplaces.add(max_job_count, _jobs);
    }

    inline void add_people(
        bool sex,
        bool alive,
        util::Fixpoint_num elszantsag,
        util::Fixpoint_num optimizmus,
        util::Fixpoint_num szabalykovetes,
        util::Fixpoint_num szepseg,
        util::Fixpoint_num erosseg,
        util::Fixpoint_num okossag,
        int64_t balance,
        const std::string& name,
        Job_idx job,
        Residence_idx home,
        const Set<Connection>& connections){

        peoples.add(sex, alive, elszantsag, optimizmus, szabalykovetes, szepseg,
            erosseg, okossag, balance, name, job, home, connections );
    }
 
};


}






namespace std{
template<>
struct hash<game::npc::Connection> {
    size_t operator()(const game::npc::Connection& con)const noexcept{
        return con.target;
    }
};
}








#endif
