#pragma once
#include <stdexcept>
#include <typeinfo>
#include <type_traits>

class Init_error : public std::runtime_error
{
public:
    explicit Init_error(const char* type_name);
};

inline void assert_runtime(bool b, const char* msg)
{
    if (!b) throw std::runtime_error(msg);
}

template<class T>
inline void assert_init(const T& obj)
{
    static const T* _init_p = nullptr;
#define type_name(x) #x
    if (_init_p) throw Init_error(type_name(T));
#undef type_name
    _init_p = &obj;
}
