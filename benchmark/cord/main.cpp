#include "../../common/macro.hpp"
#include "../../common/timer.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <iostream>
#include <random>
#include <memory>

#include <sys/resource.h>

#ifdef PRECISE_GC
    #include "libprecisegc/libprecisegc.hpp"
    using namespace precisegc;
#endif

#ifdef BDW_GC
    #include <gc/gc.h>
#endif

#include "cord.hpp"

std::unique_ptr<char[]> buf_content;

void set_stack_size()
{
    const rlim_t kStackSize = 256 * 1024 * 1024;   // min stack size = 16 MB
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0)
    {
        if (rl.rlim_cur < kStackSize)
        {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0)
            {
                fprintf(stderr, "setrlimit returned result = %d\n", result);
            }
        }
    }
}

void init(size_t buf_size)
{
    srand(time(nullptr));
    buf_content.reset(new char[buf_size + 1]);
    int mod = 'z' - 'a';
    for (size_t i = 0; i < buf_size; ++i) {
        buf_content[i] = (rand() % mod) + 'a';
    }
    buf_content[buf_size] = '\0';

//    set_stack_size();
}

std::unique_ptr<char[]> build_str(size_t len)
{
    std::unique_ptr<char[]> str(new char[len+1]);
    char* it = str.get();
    size_t buf_size = strlen(buf_content.get());
    for (size_t i = 0; i < len; i += buf_size, it += buf_size) {
        memcpy((void*) it, (void*) buf_content.get(), buf_size);
    }
    str[len] = '\0';
    return str;
}

bool check_rope(CORD_IN cord)
{
    size_t len = CORD_len(cord);
    std::unique_ptr<char[]> str = build_str(len);
    PCHAR cord_str = CORD_to_char_star(cord);
    pin_array_t(const char) cord_str_pin = pin(cord_str);
    const char* cord_str_raw = raw_ptr(cord_str_pin);
    return memcmp(cord_str_raw, str.get(), len) == 0;
}

bool check_substr(CORD_IN cord, CORD_IN cord_substr, size_t substr_pos, size_t substr_len)
{
    size_t len = CORD_len(cord);
    std::unique_ptr<char[]> str = build_str(len);
    PCHAR substr_str = CORD_to_char_star(cord_substr);
    pin_array_t(const char) substr_str_pin = pin(substr_str);
    const char* substr_str_raw = raw_ptr(substr_str_pin);

//    static int cnt = 0;
//    std::string dbg = std::string(str.get()).substr(substr_pos, substr_len);
//    printf("%d raw:  %s\n", cnt, dbg.c_str());
//    printf("%d cord: %s\n", cnt, substr_str_raw);
//    ++cnt;

    return memcmp(substr_str_raw, str.get() + substr_pos, substr_len) == 0;
}

bool check_flattening(PCHAR_IN flat_str)
{
    pin_array_t(const char) flat_str_pin = pin(flat_str);
    const char* flat_str_raw = raw_ptr(flat_str_pin);

    size_t len = strlen(flat_str_raw);
    std::unique_ptr<char[]> str = build_str(len);

    return memcmp(flat_str_raw, str.get(), len) == 0;
}

CORD build_rope(size_t total_len)
{
    CORD cord = CORD_EMPTY;
    size_t buf_size = strlen(buf_content.get());
    for (size_t len = 0; len < total_len; len += buf_size) {
        ptr_array_t(const char) buf = new_array_(const char, buf_size + 1);
        pin_array_t(const char) buf_pin = pin(buf);
        const char* buf_raw = raw_ptr(buf_pin);
        memcpy((void*) buf_raw, (void*) buf_content.get(), buf_size + 1);
        cord = CORD_cat_char_star(cord, buf, buf_size);
    }
    assert(check_rope(cord));
    return cord;
}

CORD substr(CORD_IN cord, size_t total_len)
{
    size_t substr_len_mean = 0.001 * total_len;
    size_t substr_len_var  = substr_len_mean / 10;

    static std::default_random_engine gen;
    static std::uniform_int_distribution<size_t> dist_pos;
    static std::normal_distribution<double> dist_len(substr_len_mean, substr_len_var);

    double min = dist_len.min();
    double max = dist_len.max();

    double substr_len_d = dist_len(gen);
    while ((substr_len_d < 0) || (substr_len_d > 10 * substr_len_mean)) {
        substr_len_d = dist_len(gen);
    }

    size_t substr_len = substr_len_d;
    size_t substr_pos = dist_pos(gen, std::uniform_int_distribution<size_t>::param_type(0, total_len - substr_len));

    CORD substr = CORD_substr(cord, substr_pos, substr_len);
    assert(check_substr(cord, substr, substr_pos, substr_len));

    return substr;
}

PCHAR flatten(CORD_IN cord)
{
    PCHAR flat_str = CORD_to_char_star(cord);
    assert(check_flattening(flat_str));
    return flat_str;
}

enum class test_type {
      BUILD_ROPE
    , SUBSTR
    , FLATTENING
};

int main(int argc, const char* argv[])
{
    size_t len = 0;
    size_t buf_size = 127;
    bool compacting_flag = false;
    bool incremental_flag = false;
    bool conservative_flag = false;
    test_type ttype;
    for (int i = 1; i < argc; ++i) {
        auto arg = std::string(argv[i]);
        if (arg == "--incremental") {
            incremental_flag = true;
        } else if (arg == "--compacting") {
            compacting_flag = true;
        } else if (arg == "--conservative") {
            conservative_flag = true;
        } else if (arg == "--len") {
            assert(i + 1 < argc);
            ++i;
            std::string len_str = std::string(argv[i]);
            len = std::stoi(len_str);
        } else if (arg == "--build") {
            ttype = test_type::BUILD_ROPE;
        } else if (arg == "--substr") {
            ttype = test_type::SUBSTR;
        } else if (arg == "--flatten") {
            ttype = test_type::FLATTENING;
        }
    }

    init(buf_size);

    #if defined(PRECISE_GC)
        gc_factory::options ops;
//        ops.heapsize        = 36 * 1024 * 1024;      // 32 Mb
        ops.conservative    = conservative_flag;
        ops.incremental     = incremental_flag;
        ops.compacting      = compacting_flag;
//            ops.loglevel    = gc_loglevel::WARNING;
//            ops.print_stat  = true;
//            ops.threads_available =q 4;

        auto strategy = gc_factory::create(ops);
        gc_init(std::move(strategy));
    #elif defined(BDW_GC)
        GC_INIT();
        if (incremental_flag) {
            GC_enable_incremental();
        }
    #endif


    timer tm;

    size_t total_len = std::pow(10, len);
    if (ttype == test_type::BUILD_ROPE) {
        const size_t REPEAT_CNT = 100;

        std::cout << "Building " << REPEAT_CNT << " ropes with length = 10^" << len << std::endl;

        tm.reset();
        for (size_t i = 0; i < REPEAT_CNT; ++i) {
            build_rope(total_len);
        }
    } else if (ttype == test_type::SUBSTR) {
        const size_t REPEAT_CNT = 10000;

        std::cout << "Requesting " << REPEAT_CNT << " substrings from rope with length = 10^" << len << std::endl;

        CORD cord = build_rope(total_len);
        tm.reset();
        for (size_t i = 0; i < REPEAT_CNT; ++i) {
//            std::cout << i << std::endl;
            substr(cord, total_len);
        }
    } else if (ttype == test_type::FLATTENING) {
        const size_t REPEAT_CNT = 1000;

        std::cout << "Flattening " << REPEAT_CNT << " ropes with length = 10^" << len << std::endl;

        CORD cord = build_rope(total_len);
        tm.reset();
        for (size_t i = 0; i < REPEAT_CNT; ++i) {
            flatten(cord);
        }
    }

    std::cout << "Completed in " << tm.elapsed<std::chrono::milliseconds>() << " ms" << std::endl;

    #if defined(PRECISE_GC)
        gc_stat stat = gc_stats();
        std::cout << "Completed " << stat.gc_count << " collections" << std::endl;
        std::cout << "Time spent in gc " << std::chrono::duration_cast<std::chrono::milliseconds>(stat.gc_time).count() << " ms" << std::endl;
        std::cout << "Average pause time " << std::chrono::duration_cast<std::chrono::microseconds>(stat.gc_time / stat.gc_count).count() << " us" << std::endl;
    #elif defined(BDW_GC)
        std::cout << "Completed " << GC_get_gc_no() << " collections" << std::endl;
        std::cout << "Heap size is " << GC_get_heap_size() << std::endl;
    #endif
}

