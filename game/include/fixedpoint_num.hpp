#ifndef FIXEDPOINT_NUM_HPP
#define FIXEDPOINT_NUM_HPP

#include <cstdint>
#include <iostream>

namespace util{

struct Fixpoint_num{
    typedef uint8_t num_type;
    static num_type max_val;

    num_type num;

    /// @brief 
    /// @param _num number between 0-1 
    Fixpoint_num(double _num = 0);
    Fixpoint_num(num_type _num);

    inline Fixpoint_num& operator=(double);

    inline operator double() const{
        return (double)num / max_val;
    }

    Fixpoint_num operator+(Fixpoint_num oth)const;
    Fixpoint_num operator*(Fixpoint_num oth)const;

    Fixpoint_num pow(double);

    static Fixpoint_num random();

    friend std::ostream& operator<<(std::ostream& out, Fixpoint_num num);

};




}


#endif