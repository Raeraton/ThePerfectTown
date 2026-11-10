#ifndef ROTATING_BUFFER_HPP
#define ROTATING_BUFFER_HPP

#include <cstddef>

namespace util{


template<typename T>
class Rotating_buffer{
    size_t m_buffer_size=0;
    size_t m_first_idx=0;
    size_t m_size=0;
    T* m_buffer=nullptr;
    
    inline T& get_ref(size_t idx) noexcept {
        idx = (idx+m_first_idx) % m_buffer_size;
        return m_buffer[idx];
    }
    inline const T& get_ref(size_t idx) const noexcept {
        idx = (idx+m_first_idx) % m_buffer_size;
        return m_buffer[idx];
    }

public:
    
    Rotating_buffer( size_t bs )
            : m_buffer_size(bs), m_buffer(new T[bs]){
        if(bs == 0) throw std::string{"Rotating buffer buffer size cannot be 0"};
    }

    ~Rotating_buffer(){
        delete[] m_buffer;
    }

    inline void add( const T& data )noexcept{
        if(m_size < m_buffer_size){
            get_ref(m_size) = data;
            m_size++;
        }else{
            get_ref(0) = data;
            m_first_idx = (m_first_idx+1)%m_buffer_size;
        }
    }

    inline void remove_last()noexcept{
        if(m_size > 0) m_size--;
    }
    inline void remove_first()noexcept{
        if(m_size > 0){
            m_size--;
            m_first_idx = (m_first_idx+1)%m_buffer_size;
        }
    }

    inline T& operator[](size_t idx) noexcept {
        return get_ref(idx);
    }
    inline const T& operator[](size_t idx) const noexcept {
        return get_ref(idx);
    }

    inline T& last() noexcept {
        return get_ref(m_size-1);
    }inline const T& last() const noexcept {
        return get_ref(m_size-1);
    }
    inline T& first() noexcept {
        return get_ref(0);
    }
    inline const T& first() const noexcept {
        return get_ref(0);
    }

    inline size_t size() const noexcept{
        return m_size;
    }

    inline size_t max_size() const noexcept{
        return m_buffer_size;
    }

};




}



#endif
