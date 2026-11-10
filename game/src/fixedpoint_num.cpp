#include <fixedpoint_num.hpp>
#include <cmath>


namespace util{
    
Fixpoint_num::num_type Fixpoint_num::max_val = (Fixpoint_num::num_type)-1;

Fixpoint_num::Fixpoint_num(double _num){
    num = (num_type)(_num * max_val);
}
Fixpoint_num::Fixpoint_num(num_type _num): num{_num} {}


Fixpoint_num::Fixpoint_num(const Fixpoint_num& o){
    num = o.num;
}



Fixpoint_num Fixpoint_num::operator+(Fixpoint_num oth)const{
    uint64_t val = (uint64_t)num + oth.num;
    if(val > max_val)
        val = max_val;
    return Fixpoint_num{(num_type)val};
}

Fixpoint_num Fixpoint_num::operator*(Fixpoint_num oth)const{
    double prod = (double)((uint64_t)num*oth.num) / (max_val*max_val);
    return Fixpoint_num{prod};
}

Fixpoint_num Fixpoint_num::pow(double p){
    double val = (double)num / max_val;
    val = std::pow( val, p );
    return Fixpoint_num{val};
}



Fixpoint_num Fixpoint_num::random(){
    return Fixpoint_num(
        (double)rand() / RAND_MAX
    );
}


std::ostream& operator<<(std::ostream& out, Fixpoint_num num){
    out << (double)num;
    return out;
}


} // namespace util
