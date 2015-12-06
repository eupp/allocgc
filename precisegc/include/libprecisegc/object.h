#ifndef DIPLOMA_OBJECT_H
#define DIPLOMA_OBJECT_H

// header of each object in the heap (stored at the end of the object space)
/// its size might be aligned on power of two
struct Object {
    //	first to fields together === base_meta
    void *meta; // pointer on the meta information
    size_t count;
    void *begin; // pointer on th object begin
    // NB: FALSE in current version: we use last two bits of begin pointer to be a pin and mask bits respectively
};

#endif //DIPLOMA_OBJECT_H
