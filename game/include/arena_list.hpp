#ifndef ARENA_LIST_HPP
#define ARENA_LIST_HPP

#include <vector>
#include <iostream>

namespace util{



template<typename T>
class Arena_list{


    const size_t m_block_size=0;
    size_t m_current_block_size=0;

    std::vector<T*> m_blocks;

    void increase_size_if_needed(){
        if( m_current_block_size < m_block_size ) return;
        m_blocks.push_back(new T[m_block_size]);
        m_current_block_size = 0;
    }

public:


    Arena_list( size_t block_size=1024 )
            :m_block_size{block_size}{
        
        m_blocks.push_back(new T[m_block_size]);

    }

    void push_back(const T& data){
        increase_size_if_needed();
        m_blocks[m_blocks.size()-1][m_current_block_size++] = data;
    }

    inline size_t size() const {
        return (m_blocks.size()-1) * m_block_size + m_current_block_size;
    }

    T& operator[](size_t idx){
        if( idx >= size() )
            throw std::string{"Arena_list[] idx out of range"};
        
        size_t block_idx = idx / m_block_size;
        size_t in_block_idx = idx % m_block_size;
        return m_blocks[block_idx][in_block_idx];
    }

    const T& operator[](size_t idx) const {
        if( idx >= size() )
            throw std::string{"Arena_list[] idx out of range"};
        
        size_t block_idx = idx / m_block_size;
        size_t in_block_idx = idx % m_block_size;
        return m_blocks[block_idx][in_block_idx];
    }

    
    class Iterator{
        Arena_list& target;
        size_t idx;

    public:

        Iterator(Arena_list& arena, size_t _idx)
            : target{arena}, idx{_idx} {}

        T& operator*() const {
            return target[idx];
        }

        T* operator->() const {
            return &target[idx];
        }

        Iterator& operator++(){
            ++idx;
            return *this;
        }
        Iterator operator++(int){
            Iterator tmp = *this;
            ++idx;
            return tmp;
        }
        bool operator==(const Iterator& oth) const {
            return &target == &oth.target && idx == oth.idx;
        }
        bool operator!=(const Iterator& oth) const {
            return !(*this==oth);
        }
    };


    Iterator begin(){
        return Iterator{*this, 0};
    }
    Iterator end(){
        return Iterator{*this, size()};
    }

    friend std::ostream& operator<<( std::ostream& out, Arena_list& list ){
        out << "{";
        for(auto elem : list){
            out << elem << ",";
        }
        out << "}";
        return out;
    }


};





}



#endif
