#ifndef DIPLOMA_DEOPTIMIZE_HPP
#define DIPLOMA_DEOPTIMIZE_HPP

/**
 * escape and clobber - functions that prevent compiler optimizations.
 * Implementation is taken from Chandler Carruth's speech on CppCon 2015 https://www.youtube.com/watch?v=nXaxk27zwlk&list=PLHTh1InhhwT75gykhs7pqcR_uSiG601oh&index=4
 */

static inline void escape(void* p)
{
    asm volatile("" : : "g"(p) : "memory");
}

static inline void clobber()
{
    asm volatile("" : : : "memory");
}

#endif //DIPLOMA_DEOPTIMIZE_HPP
