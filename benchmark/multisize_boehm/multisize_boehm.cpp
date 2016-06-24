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
#include <random>
#include <utility>
#include <sys/time.h>

#include "../../common/macro.h"

#ifdef BDW_GC
#include <gc/gc.h>
#endif


#ifdef PRECISE_GC
    #include <libprecisegc/libprecisegc.h>
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

static const int kStretchTreeDepth    = 18; //18;
static const int kLongLivedTreeDepth  = 16; //16;
static const int kArraySize  = 500000;      //500000;
static const int kMinTreeDepth = 4;         //4
static const int kMaxTreeDepth = 16;        //16;

// distribution of size of nodes
static const double smallProb  = 0.5;
static const double mediumProb = 0.25;
static const double largeProb  = 0.25;
static const double xlargeProb = 0.00;


struct NodeBase
{
    ptr_t(NodeBase) left;
    ptr_t(NodeBase) right;

    int i, j;

    NodeBase(ptr_in(NodeBase) l, ptr_in(NodeBase) r)
        : left(l)
        , right(r)
    {}

#ifndef PRECISE_GC
    NodeBase()
            : left(nullptr)
            , right(nullptr)
        {}
#else
    NodeBase() {}
#endif

    ~NodeBase()
    {
        delete_(left);
        delete_(right);
    }
};

// 40b -> 64b
struct SmallNode : public NodeBase
{
    SmallNode() = default;

    SmallNode(ptr_in(NodeBase) l, ptr_in(NodeBase) r)
        : NodeBase(l, r)
    {}
};

// 168b -> 256b
struct MediumNode : public NodeBase
{
    MediumNode() = default;

    MediumNode(ptr_in(NodeBase) l, ptr_in(NodeBase) r)
        : NodeBase(l, r)
    {}

    char data[128];
};

// 4000b -> ~4Kb
struct LargeNode : public NodeBase
{
    LargeNode() = default;

    LargeNode(ptr_in(NodeBase) l, ptr_in(NodeBase) r)
        : NodeBase(l, r)
    {}

    char data[4000];
};

// a bit more than 1Mb
struct XLargeNode : public NodeBase
{
    XLargeNode() = default;

    XLargeNode(ptr_in(NodeBase) l, ptr_in(NodeBase) r)
        : NodeBase(l, r)
    {}

    char data[1024 * 1024];
};

enum class NodeType
{
      SMALL
    , MEDIUM
    , LARGE
    , XLARGE
};

NodeType generateNodeType()
{
    static std::random_device rd;
    static std::default_random_engine gen(rd());
    static std::uniform_real_distribution<double> distr(0.0, 1.0);

    double rnd = distr(gen);
    if (rnd < smallProb) {
        return NodeType::SMALL;
    } else if (rnd < smallProb + mediumProb) {
        return NodeType::MEDIUM;
    } else if (rnd < smallProb + mediumProb + largeProb) {
        return NodeType::LARGE;
    } else {
        return NodeType::XLARGE;
    }
}

ptr_t(NodeBase) createNode()
{
    switch (generateNodeType()) {
        case NodeType::SMALL :
            return new_(SmallNode);
        case NodeType::MEDIUM :
            return new_(MediumNode);
        case NodeType::LARGE :
            return new_(LargeNode);
        case NodeType::XLARGE :
            return new_(XLargeNode);
    }
}

template <typename... Args>
ptr_t(NodeBase) createNode(Args&&... args)
{
    switch (generateNodeType()) {
        case NodeType::SMALL :
            return new_args_(SmallNode, std::forward<Args>(args)...);
        case NodeType::MEDIUM :
            return new_args_(MediumNode, std::forward<Args>(args)...);
        case NodeType::LARGE :
            return new_args_(LargeNode, std::forward<Args>(args)...);
        case NodeType::XLARGE :
            return new_args_(XLargeNode, std::forward<Args>(args)...);
    }
}

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
    static void Populate (int iDepth, ptr_in(NodeBase) thisNode)
    {
        if (iDepth<=0) {
            return;
        } else {
            iDepth--;
            thisNode->left  = createNode();
            thisNode->right = createNode();
            Populate (iDepth, thisNode->left);
            Populate (iDepth, thisNode->right);
        }
    }

    // Build tree bottom-up
    static ptr_t(NodeBase) MakeTree(int iDepth)
    {
        if (iDepth<=0) {
            return createNode();
        } else {
            return createNode(MakeTree(iDepth-1), MakeTree(iDepth-1));
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
        ptr_t(NodeBase) tempTree;

        cout << "Creating " << iNumIters << " trees of depth " << depth << endl;

        tStart = currentTime();
        for (int i = 0; i < iNumIters; ++i) {
            tempTree = createNode();
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
        ptr_t(NodeBase) root;
        ptr_t(NodeBase) longLivedTree;
        ptr_t(NodeBase) tempTree;

        long    tStart, tFinish;
        long    tElapsed;

        size_t sizes = smallProb * sizeof(SmallNode)
                     + mediumProb * sizeof(MediumNode)
                     + largeProb * sizeof(LargeNode)
                     + xlargeProb * sizeof(XLargeNode);

        cout << "Garbage Collector Test" << endl;
        cout << " Live storage will peak at "
        << sizes * TreeSize(kLongLivedTreeDepth) /*+ sizeof(double) * kArraySize*/
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

        longLivedTree = createNode();
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
    ops.type        = gc_type::SERIAL;
    ops.compacting  = gc_compacting::DISABLED;
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
