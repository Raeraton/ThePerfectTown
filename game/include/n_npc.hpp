#ifndef N_NPC_HPP
#define N_NPC_HPP

#include <algorithm>
#include <iterator>
#include <set>
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



class Time_range{
    uint16_t data=0;
public:
    Time_range()=default;
    /// day_of_the_week    hetfo..vasarnap 0..6
    /// hour_of_the_day    0..23
    /// duration           0..255   hours
    Time_range(int day_of_the_week, int hour_of_the_day, int duration) noexcept {
        int hour_of_the_week = 24*day_of_the_week + hour_of_the_day;
        data = (uint16_t)(hour_of_the_week&0xff) ^ ( (uint16_t)(duration&0xff) << 8);
    }
    /// day_of_the_week    hetfo..vasarnap 0..6
    /// hour_of_the_day    0..23
    Time_range(int day_of_the_week, int hour_of_the_day) noexcept {
        int hour_of_the_week = 24*day_of_the_week + hour_of_the_day;
        data = (uint16_t)(hour_of_the_week&0xff);
    }
    
    /// gives back the flag for the cooresponding day
    inline uint8_t day() const noexcept {
        return 1 << ((data&0xff) / 24);
    }

    inline int hour() const noexcept {
        return ((data&0xff) % 24);
    }

    inline int duration() const noexcept {
        return data >> 8;
    }

    inline int raw_hour() const noexcept {
        return data & 0xff;
    }

    inline int raw_end() const noexcept {
        return (int)(data&0xff) + (data>>8);
    }

    inline bool is_overlapping(Time_range oth) const noexcept{
        int this_start = raw_hour();
        int this_end = raw_end();
        int oth_start = oth.raw_hour();
        int oth_end = oth.raw_end();

        return  (this_start <= oth_start && oth_start <= this_end) ||
                (oth_start  <= this_start && this_start <= oth_end);
    }

    inline bool operator>(Time_range o)const noexcept{
        return raw_hour() > o.raw_end();
    }
    inline bool operator>=(Time_range o)const noexcept{
        return raw_hour() >= o.raw_hour();
    }
    inline bool operator<(Time_range o) const noexcept{
        return raw_end() < o.raw_hour();
    }
    inline bool operator<=(Time_range o)const noexcept{
        return raw_hour() <= o.raw_hour();
    }

};

struct Namedb{
    std::vector<std::string> firsts;
    std::vector<std::string> lasts;

    Namedb(const char* first_path, const char* last_path);
};


template<typename T>
using List = std::vector<T>;

template<typename T>
using Set = std::set<T>;

typedef uint32_t People_idx;
typedef uint32_t Apartman_idx;
typedef uint32_t Residence_idx;
typedef uint32_t Job_idx;
typedef uint32_t Workspace_idx;
typedef uint32_t Event_idx;



struct Connection{
    enum Connection_type{
        CON_FRIEND,
        CON_DATE,
        CON_PARENT,
        CON_CHILD,
        CON_MURDERER,
        CON_COOWORKER,
        CON_ROOMMATE,
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
        case CON_MURDERER:
            return 0;
        case CON_DATE:
            return 1;
        case CON_PARENT:
            return 2;
        case CON_FRIEND:
            return 3;
        case CON_ROOMMATE:
        case CON_COOWORKER:
            return 4;
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
        case CON_ROOMMATE:
        case CON_COOWORKER:
            return 4;
        case CON_NONE:
        default:
            return 0xfff;
        }
    }

    inline bool operator==(const Connection& oth) const noexcept{
        return target==oth.target && type==oth.type;
    }
    inline bool operator<(const Connection& oth) const noexcept{
        return ((unsigned)type << 31) + target < ((unsigned)oth.type << 31) + oth.target;
    }
    inline bool operator>(const Connection& oth) const noexcept{
        return ((unsigned)type << 31) + target > ((unsigned)oth.type << 31) + oth.target;
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

    inline uint8_t get_work_start(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return (m_schedules[idx] >> 4) + 4;
    }

    inline uint8_t get_work_len(Job_idx idx){
        if(idx >= m_count) throw std::string{"Job idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_schedules[idx] & 0b1111;
    }

    inline bool is_working_now(Job_idx idx, Time_range now){
        // working today
        if(! (work_days(idx) & now.day()) )return false;
        
        int start = get_work_start(idx);
        int len = get_work_len(idx);

        int now_hour = now.hour();

        return start <= now_hour && now_hour <= start+len;

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
    List<util::Fixpoint_num> m_hangulat;

    List<int64_t> m_balances;

    List<std::string> m_names;
    List<Job_idx> m_jobs;
    List<Residence_idx> m_homes;

    List<Set<Connection>> m_connections;
    List<Set<Event_idx>> m_events;

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
            m_count != m_hangulat.size() ||
            m_count != m_balances.size() ||
            m_count != m_names.size() ||
            m_count != m_jobs.size() ||
            m_count != m_homes.size() ||
            m_count != m_connections.size() ||
            m_count != m_events.size()
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
        util::Fixpoint_num hangulat,
        int64_t balance,
        const std::string& name,
        Job_idx job,
        Residence_idx home,
        const Set<Connection>& connections,
        const Set<Event_idx>& events
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
        m_hangulat.push_back(hangulat);
        m_balances.push_back(balance);
        m_names.push_back(name);
        m_jobs.push_back(job);
        m_homes.push_back(home);
        m_connections.push_back(connections);
        m_events.push_back(events);
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

    inline util::Fixpoint_num& hangulat(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_hangulat[idx];
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

    inline Set<Event_idx>& event(People_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_events[idx];
    }

    double get_simpathy( People_idx p, People_idx target );
};



struct Location{ // todo
                 };

class Group_event{
public:
    enum Type{
        FIGHT,
        PARTY
    };

private:

    size_t m_count=0;
    List<Location> m_locations;
    List<List<People_idx>> m_participants;
    List<Type> m_types;
    List<Time_range> m_times;

    Set<Event_idx> m_valid_events;

    inline void check_integrity(){
        if( m_count != m_locations.size() ||
            m_count != m_participants.size() ||
            m_count != m_types.size() ||
            m_count != m_times.size() )
            throw std::string{"Group_events integrity error"};
    }

public:

    Group_event() = default;

    inline void add(
            const Location& location,
            const List<People_idx>& participant,
            Type type,
            Time_range time){

        m_valid_events.insert(m_count);
        m_count++;
        m_locations.push_back(location);
        m_participants.push_back(participant);
        m_types.push_back(type);
        m_times.push_back(time);
        check_integrity();

    }

    inline size_t count() const {return m_count;}

    inline size_t valid_event_count() const {return m_valid_events.size();}

    inline Set<Event_idx>::iterator begin(){
        return m_valid_events.begin();
    }

    inline Set<Event_idx>::iterator end(){
        return m_valid_events.end();
    }


    inline List<People_idx>& participant(Event_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_participants[idx];
    }

    inline Location& location(Event_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_locations[idx];
    }

    inline Type& type(Event_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_types[idx];
    }

    inline Time_range& time(Event_idx idx){
        if(idx >= m_count) throw std::string{"People idx out of range: index "} + std::to_string(idx) + ", size " + std::to_string(m_count);
        return m_times[idx];
    }



    inline void remove(const Set<Event_idx>& events, Peoples& peoples){

        for( Event_idx e : events ){
            for( People_idx p : participant(e)){
                peoples.event(p).erase(e);
            }
        }

        Set<Event_idx> result;
        std::set_difference(
            m_valid_events.begin(),
            m_valid_events.end(),
            events.begin(),
            events.end(),
            std::inserter(result, result.begin()) );
        m_valid_events = result;
    }

    Group_event cleanup(Peoples& peoples);


};




class World{

    Peoples peoples;

    Workplaces workplaces;
    Jobs jobs;

    Apartmans apartmans;
    Residenceses residences;

    Group_event events;

public:

    World(size_t people_count);

    void update(Time_range time, bool rent_pay_time);

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
        util::Fixpoint_num hangulat,
        int64_t balance,
        const std::string& name,
        Job_idx job,
        Residence_idx home,
        const Set<Connection>& connections,
        const Set<Event_idx>& events){

        peoples.add(sex, alive, elszantsag, optimizmus, szabalykovetes, szepseg,
            erosseg, okossag, hangulat, balance, name, job, home, connections, events );
    }
 
};


}






namespace std{
template<> struct hash<game::npc::Connection> { size_t operator()(const game::npc::Connection& con)const noexcept{ return ((size_t)con.target << 31) + (size_t)con.type; } }; }


#endif
