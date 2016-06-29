// This is adapted from a benchmark written by John Ellis and Pete Kovac
// of Post Communications.
// It was modified by Hans Boehm of Silicon Graphics.
// Translated to C++ 30 May 1997 by William D Clinger of Northeastern Univ.
//
//      This is no substitute for real applications.  No actual application
//      is likely to behave in exactly this way.  However, this benchmark was
//      designed to be more representative of real applications than other
//      Java GC benchmarks of which we are aware.
//      It attempts to model those properties of allocation requests that
//      are important to current GC techniques.
//      It is designed to be used either to obtain a single overall performance
//      number, or to give a more detailed estimate of how collector
//      performance varies with object lifetimes.  It prints the time
//      required to allocate and collect balanced binary trees of various
//      sizes.  Smaller trees result in shorter object lifetimes.  Each cycle
//      allocates roughly the same amount of memory.
//      Two data structures are kept around during the entire process, so
//      that the measured performance is representative of applications
//      that maintain some live in-memory data.  One of these is a tree
//      containing many pointers.  The other is a large array containing
//      double precision floating point numbers.  Both should be of comparable
//      size.
//
//      The results are only really meaningful together with a specification
//      of how much memory was used.  It is possible to trade memory for
//      better time performance.  This benchmark should be run in a 32 MB
//      heap, though we don't currently know how to enforce that uniformly.
//
//      Unlike the original Ellis and Kovac benchmark, we do not attempt
//      measure pause times.  This facility should eventually be added back
//      in.  There are several reasons for omitting it for now.  The original
//      implementation depended on assumptions about the thread scheduler
//      that don't hold uniformly.  The results really measure both the
//      scheduler and GC.  Pause time measurements tend to not fit well with
//      current benchmark suites.  As far as we know, none of the current
//      commercial Java implementations seriously attempt to minimize GC pause
//      times.

#include <new>
#include <iostream>
#include <type_traits>
#include <sys/time.h>

#include "../../common/macro.hpp"
#include "../../common/timer.hpp"

#ifdef BDW_GC
    #include <gc/gc.h>
#endif


#ifdef PRECISE_GC
    #include "libprecisegc/libprecisegc.hpp"
    #include "libprecisegc/details/gc_heap.h"
    using namespace precisegc;
#endif

using namespace std;

static const int kStretchTreeDepth    = 18; //18;       // about 16Mb
static const int kLongLivedTreeDepth  = 16; //16;       // about 4Mb
static const int kArraySize  = 500000;      //500000;   // about 4Mb
static const int kMinTreeDepth = 4;         //4
static const int kMaxTreeDepth = 16;        //16;

struct Node
{
    ptr_t(Node) left;
    ptr_t(Node) right;

    int i, j;

    Node(ptr_in(Node) l, ptr_in(Node) r)
        : left(l)
        , right(r)
    {}

    #ifndef PRECISE_GC
        Node()
            : left(nullptr)
            , right(nullptr)
        {}
    #else
        Node() {}
    #endif

    ~Node()
    {
        delete_(left);
        delete_(right);
    }
};


struct GCBench {

    // Nodes used by a tree of a given size
    static int TreeSize(int i) {
        return ((1 << (i + 1)) - 1);
    }

    // Number of iterations to use for a given tree depth
    static int NumIters(int i) {
        return 2 * TreeSize(kStretchTreeDepth) / TreeSize(i);
    }

    // Build tree top down, assigning to older objects.
    static void Populate (int iDepth, ptr_in(Node) thisNode)
    {
        if (iDepth<=0) {
            return;
        } else {
            iDepth--;
            thisNode->left  = new_(Node);
            thisNode->right = new_(Node);
            Populate (iDepth, thisNode->left);
            Populate (iDepth, thisNode->right);
        }
    }

    // Build tree bottom-up
    static ptr_t(Node) MakeTree(int iDepth)
    {
        if (iDepth<=0) {
            return new_(Node);
        } else {
            return new_args_(Node, MakeTree(iDepth-1), MakeTree(iDepth-1));
        }
    }

    static void TimeConstruction(int depth) {
        long    tStart, tFinish;
        int     iNumIters = NumIters(depth);
        ptr_t(Node) tempTree;

        cout << "Creating " << iNumIters << " trees of depth " << depth << endl;

        timer tm;
        for (int i = 0; i < iNumIters; ++i) {
            tempTree = new_(Node);
            Populate(depth, tempTree);
            delete_(tempTree);
            set_null(tempTree);
        }

        cout << "\tTop down construction took " << tm.elapsed<std::chrono::milliseconds>() << " msec" << endl;

        tm.reset();
        for (int i = 0; i < iNumIters; ++i) {
            tempTree = MakeTree(depth);
            delete_(tempTree);
            set_null(tempTree);
        }
        cout << "\tBottom up construction took " << tm.elapsed<std::chrono::milliseconds>() << " msec" << endl;
    }

    void main() {
        ptr_t(Node) root;
        ptr_t(Node) longLivedTree;
        ptr_t(Node) tempTree;

        long    tStart, tFinish;
        long    tElapsed;

        cout << "Garbage Collector Test" << endl;
        cout << " Live storage will peak at "
             << 2 * sizeof(Node) * TreeSize(kLongLivedTreeDepth) /*+ sizeof(double) * kArraySize*/
             << " bytes." << endl << endl;
        cout << " Stretching memory with a binary tree of depth " << kStretchTreeDepth << endl;

        timer tm;

        // Stretch the memory space quickly
        tempTree = MakeTree(kStretchTreeDepth);
        delete_(tempTree);
        set_null(tempTree);

        // Create a long lived object
        cout << " Creating a long-lived binary tree of depth " << kLongLivedTreeDepth << endl;

        longLivedTree = new_(Node);
        Populate(kLongLivedTreeDepth, longLivedTree);

        // Create long-lived array, filling half of it
        cout << " Creating a long-lived array of " << kArraySize << " doubles" << endl;

//        ptr_array_t(double) array = new_array_(double, kArraySize);
//        for (int i = 0; i < kArraySize/2; ++i) {
//            array[i] = 1.0/i;
//        }

        for (int d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2) {
            TimeConstruction(d);
        }

        if (!longLivedTree /* || array[1000] != 1.0/1000 */) {
            cout << "Failed" << endl;
            // fake reference to LongLivedTree
            // and array
            // to keep them from being optimized away
        }

        cout << "Completed in " << tm.elapsed<std::chrono::milliseconds>() << " msec" << endl;
        #if defined(BDW_GC)
            cout << "Completed " << GC_get_gc_no() << " collections" << endl;
            cout << "Heap size is " << GC_get_heap_size() << endl;
        #elif defined(PRECISE_GC)
            gc_stat stat = gc_stats();
            cout << "Completed " << stat.gc_count << " collections" << endl;
            cout << "Time spent in gc " << std::chrono::duration_cast<std::chrono::milliseconds>(stat.gc_time).count() << " msec" << endl;
            cout << "Average pause time " << std::chrono::duration_cast<std::chrono::milliseconds>(stat.gc_time / stat.gc_count).count() << " msec" << endl;
        #endif
    }
};

int main () {
    #if defined(PRECISE_GC)
        gc_options ops;
        ops.heapsize    = 64 * 1024 * 1024;      // 64 Mb
        ops.type        = gc_type::SERIAL;
        ops.init        = gc_init_strategy::SPACE_BASED;
        ops.compacting  = gc_compacting::DISABLED;
        ops.loglevel    = gc_loglevel::OFF;
        ops.print_stat  = false;
        gc_init(ops);
    #elif defined(BDW_GC)
        GC_INIT();
//        GC_enable_incremental();
    #endif
    GCBench x;
    x.main();
    cout.flush();
    return 0;
}
