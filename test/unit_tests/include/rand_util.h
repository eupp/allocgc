#ifndef DIPLOMA_RAND_UTIL_H
#define DIPLOMA_RAND_UTIL_H

#include <random>

template <typename T>
class uniform_rand_generator
{
public:
    uniform_rand_generator(T a, T b)
        : m_rand_gen(m_rand_dev())
        , m_rand_distr(a, b)
    {}

    uniform_rand_generator(const uniform_rand_generator& other)
        : m_rand_gen(m_rand_dev())
        , m_rand_distr(other.m_rand_distr.a(), other.m_rand_distr.b())
    {}

    T operator()()
    {
        return m_rand_distr(m_rand_gen);
    }

private:
    std::random_device m_rand_dev;
    std::default_random_engine m_rand_gen;
    std::uniform_int_distribution<T> m_rand_distr;
};

#endif //DIPLOMA_RAND_UTIL_H
