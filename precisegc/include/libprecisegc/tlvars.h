//
// Created by evagl on 13.05.15.
//

#ifndef DIPLOMA_TLVARS_H
#define DIPLOMA_TLVARS_H
#include <vector>
#include <stddef.h>

struct tlvars {
    std::vector<size_t> offsets;
    bool new_active = false;
    bool no_active = false;
    int nesting_level = 0;
    size_t current_pointer_to_object = 0;
};

extern thread_local tlvars new_obj_flags_tl_instance;
#endif //DIPLOMA_TLVARS_H
