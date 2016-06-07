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

// Our precise GC
//#define PRECISE_GC

// Boehm/Demers/Weiser conservative GC
//#define BDW_GC

// std::shared_ptr (reference count)
//#define SHARED_PTR

// raw pointers
//#define NO_GC

#ifdef BDW_GC
    #include <gc/gc.h>
#endif

#include "../../common/macro.h"

#ifdef PRECISE_GC
    #include "libprecisegc/libprecisegc.h"
    #include "libprecisegc/details/gc_heap.h"
    using namespace precisegc;
#endif

using namespace std;

//  These macros were a quick hack for the Macintosh.
#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

/* Get the current time in milliseconds */
unsigned stats_rtclock( void ) {
  struct timeval t;
  struct timezone tz;

  if (gettimeofday( &t, &tz ) == -1)
        return 0;
  return (unsigned)(t.tv_sec * 1000 + t.tv_usec / 1000);
}

static const int kStretchTreeDepth    = 18; //18;       // about 16Mb
static const int kLongLivedTreeDepth  = 16; //16;       // about 4Mb
static const int kArraySize  = 500000;      //500000;   // about 4Mb
static const int kMinTreeDepth = 4;         //4
static const int kMaxTreeDepth = 16;        //16;

typedef struct Node0 *Node;

struct Node0
{
    ptr_t(Node0) left;
    ptr_t(Node0) right;

    int i, j;

    Node0(ptr_in(Node0) l, ptr_in(Node0) r)
        : left(l)
        , right(r)
    {}

    #ifndef PRECISE_GC
        Node0()
            : left(nullptr)
            , right(nullptr)
        {}
    #else
        Node0() {}
    #endif

    ~Node0()
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
    static void Populate (int iDepth, ptr_in(Node0) thisNode)
    {
        if (iDepth<=0) {
            return;
        } else {
            iDepth--;
            thisNode->left  = new_(Node0);
            thisNode->right = new_(Node0);
            Populate (iDepth, thisNode->left);
            Populate (iDepth, thisNode->right);
        }
    }

    // Build tree bottom-up
    static ptr_t(Node0) MakeTree(int iDepth)
    {
        if (iDepth<=0) {
            return new_(Node0);
        } else {
            return new_args_(Node0, MakeTree(iDepth-1), MakeTree(iDepth-1));
        }
    }

    static void PrintDiagnostics() {
        #if 0
            long lFreeMemory = Runtime.getRuntime().freeMemory();
            long lTotalMemory = Runtime.getRuntime().totalMemory();

            System.out.print(" Total memory available="
                           + lTotalMemory + " bytes");
            System.out.println("  Free memory=" + lFreeMemory + " bytes");
        #endif
    }

    static void TimeConstruction(int depth) {
        long    tStart, tFinish;
        int     iNumIters = NumIters(depth);
        ptr_t(Node0) tempTree;

        cout << "Creating " << iNumIters << " trees of depth " << depth << endl;

        tStart = currentTime();
        for (int i = 0; i < iNumIters; ++i) {
            tempTree = new_(Node0);
            Populate(depth, tempTree);
            delete_(tempTree);
            set_null(tempTree);
        }

        tFinish = currentTime();
        cout << "\tTop down construction took " << elapsedTime(tFinish - tStart) << " msec" << endl;

        tStart = currentTime();
        for (int i = 0; i < iNumIters; ++i) {
            tempTree = MakeTree(depth);
            delete_(tempTree);
            set_null(tempTree);
        }
        tFinish = currentTime();
        cout << "\tBottom up construction took " << elapsedTime(tFinish - tStart) << " msec" << endl;
    }

    void main() {
        ptr_t(Node0) root;
        ptr_t(Node0) longLivedTree;
        ptr_t(Node0) tempTree;

        long    tStart, tFinish;
        long    tElapsed;

        cout << "Garbage Collector Test" << endl;
        cout << " Live storage will peak at "
             << 2 * sizeof(Node0) * TreeSize(kLongLivedTreeDepth) + sizeof(double) * kArraySize
             << " bytes." << endl << endl;
        cout << " Stretching memory with a binary tree of depth " << kStretchTreeDepth << endl;

        PrintDiagnostics();

        tStart = currentTime();

        // Stretch the memory space quickly
        tempTree = MakeTree(kStretchTreeDepth);
        delete_(tempTree);
        set_null(tempTree);

        // Create a long lived object
        cout << " Creating a long-lived binary tree of depth " << kLongLivedTreeDepth << endl;

        longLivedTree = new_(Node0);
        Populate(kLongLivedTreeDepth, longLivedTree);

        // Create long-lived array, filling half of it
        cout << " Creating a long-lived array of " << kArraySize << " doubles" << endl;

//        ptr_array_t(double) array = new_array_(double, kArraySize);
//        for (int i = 0; i < kArraySize/2; ++i) {
//            array[i] = 1.0/i;
//        }

        PrintDiagnostics();


        for (int d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2) {
            TimeConstruction(d);
        }

        if (!longLivedTree /* || array[1000] != 1.0/1000 */) {
            cout << "Failed" << endl;
            // fake reference to LongLivedTree
            // and array
            // to keep them from being optimized away
        }

        tFinish = currentTime();
        tElapsed = elapsedTime(tFinish-tStart);
        PrintDiagnostics();
        cout << "Completed in " << tElapsed << " msec" << endl;
        #if defined(BDW_GC)
            cout << "Completed " << GC_gc_no << " collections" <<endl;
            cout << "Heap size is " << GC_get_heap_size() << endl;
        #elif defined(PRECISE_GC)
//            cout << "Completed " << details::gc_garbage_collector::instance().get_gc_cycles_count() << " collections" <<endl;
//            cout << "Heap size is " << details::gc_heap::instance().size() << endl;
        #endif
    }
};

int main () {
    #if defined(PRECISE_GC)
        gc_options ops;
        ops.type        = gc_type::INCREMENTAL;
        ops.compacting  = gc_compacting::ENABLED;
        ops.loglevel    = gc_loglevel::OFF;
        ops.print_stat  = true;
        gc_init(ops);
    #elif defined(BDW_GC)
//        GC_full_freq = 30;
        GC_enable_incremental();
    #endif
    GCBench x;
    x.main();
    cout.flush();
    return 0;
}
