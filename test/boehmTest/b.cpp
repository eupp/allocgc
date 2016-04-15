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

#include <iostream>
#include <sys/time.h>
#include <cstdlib>

#define GC
#define precise

#ifdef GC
  #ifdef precise
//    #include <libprecisegc/libprecisegc.h>
    #include "libprecisegc/libprecisegc.h"
  #else
    #include <gc.h>
  #endif
#endif

//  These macros were a quick hack for the Macintosh.
#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

using std::cout;
using std::endl;

/* Get the current time in milliseconds */
unsigned stats_rtclock( void ) {
  struct timeval t;
  struct timezone tz;

  if (gettimeofday( &t, &tz ) == -1)
    return 0;
  return (unsigned)(t.tv_sec * 1000 + t.tv_usec / 1000);
}

static const int kStretchTreeDepth    = 18;//18;      // about 16Mb
static const int kLongLivedTreeDepth  = 16;//16;  // about 4Mb
static const int kArraySize  = 500000;//500000;  // about 4Mb
static const int kMinTreeDepth = 4;//4
static const int kMaxTreeDepth = 16;//16;

typedef struct Node0 *Node;

#ifdef precise
struct Node0 {
	gc_ptr<Node0> left;
	gc_ptr<Node0> right;
	int i, j;
	Node0(gc_ptr<Node0> l, gc_ptr<Node0> r) : left(l), right(r) {}
	Node0() {
    #ifndef GC
      left = right = NULL;
    #endif
  } // { left = 0; right = 0; }
};
#else
struct Node0 {
  Node0 * left;
  Node0 * right;
  int i, j;
  Node0(Node0 * l, Node0 * r) { left = l; right = r; }
  Node0() {}
  ~Node0() { if (left) delete left; if (right) delete right; }
};
#endif

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
#ifdef precise
  static void Populate (int iDepth, gc_ptr<Node0> thisNode)
#else
  static void Populate (int iDepth, Node0 * thisNode)
#endif
  {
    if (iDepth<=0) {
      return;
    } else {
      iDepth--;
    #ifdef GC
    #ifdef precise
      thisNode->left = gc_new<Node0>(1);
      thisNode->right = gc_new<Node0>(1);
    #else
      thisNode->left  = new (GC_NEW(Node0)) Node0();
      thisNode->right = new (GC_NEW(Node0)) Node0();
    #endif
    #else
      thisNode->left  = new Node0();
      thisNode->right = new Node0();
    #endif

      Populate (iDepth, thisNode->left);
      Populate (iDepth, thisNode->right);
    }
  }

  // Build tree bottom-up
#ifdef precise
  static gc_ptr<Node0> MakeTree(int iDepth)
#else
  static Node0 * MakeTree(int iDepth)
#endif
  {
    if (iDepth<=0) {
    #ifdef GC
    #ifdef precise
      return gc_new<Node0>(1);
    #else
      return new (GC_NEW(Node0)) Node0();
    #endif
    #else
      return new Node0();
    #endif
    } else {
    #ifdef GC
    #ifdef precise
      return gc_new<Node0, gc_ptr<Node0>, gc_ptr<Node0>>(MakeTree(iDepth-1),MakeTree(iDepth-1));;
    #else
      return new (GC_NEW(Node0)) Node0(MakeTree(iDepth-1), MakeTree(iDepth-1));
    #endif
    #else
      return new Node0(MakeTree(iDepth-1), MakeTree(iDepth-1));
    #endif
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
  #ifdef precise
    gc_ptr<Node0>    tempTree;
  #else
    Node0 * tempTree;
  #endif
    cout << "Creating " << iNumIters << " trees of depth " << depth << endl;

    tStart = currentTime();
    for (int i = 0; i < iNumIters; ++i) {
    #ifdef GC
    #ifdef precise
      tempTree = gc_new<Node0> (1);
    #else
      tempTree = new (GC_NEW(Node0)) Node0();
    #endif
    #else
      tempTree = new Node0();
    #endif
      Populate(depth, tempTree);
    #ifndef GC
      // cout << "delete" << endl;
      // delete tempTree;
      // cout << "deleted" << endl;
    #else
      #ifdef precise
        tempTree.setNULL();
      #endif
    #endif
    }
    gc(true);

    tFinish = currentTime();
    cout << "\tTop down construction took " << elapsedTime(tFinish - tStart) << " msec" << endl;

    tStart = currentTime();
    for (int i = 0; i < iNumIters; ++i) {
        tempTree = MakeTree(depth);
    #ifndef GC
      // delete tempTree;
    #else
      #ifdef precise
        tempTree.setNULL();
        gc(true);
      #endif
    #endif
    }
    tFinish = currentTime();
    cout << "\tBottom up construction took " << elapsedTime(tFinish - tStart) << " msec" << endl;
  }

  void main() {
  #ifdef precise
    gc_ptr<Node0>    root;
    gc_ptr<Node0>    longLivedTree;
    gc_ptr<Node0>    tempTree;
  #else
    Node0 * root, * longLivedTree, * tempTree;
  #endif
    long    tStart, tFinish;
    long    tElapsed;

  #ifdef GC
  #ifndef precise
    GC_full_freq = 30;
    GC_enable_incremental();
  #endif
  #endif
    cout << "Garbage Collector Test" << endl;
    cout << " Live storage will peak at "
      << 2 * sizeof(Node0) * TreeSize(kLongLivedTreeDepth) + sizeof(double) * kArraySize
      << " bytes." << endl << endl;
    cout << " Stretching memory with a binary tree of depth " << kStretchTreeDepth << endl;
  #ifdef GC
  #ifndef precise
    PrintDiagnostics();
  #endif
  #endif
    tStart = currentTime();

    // Stretch the memory space quickly
    tempTree = MakeTree(kStretchTreeDepth);
  #ifndef GC
    // delete tempTree;
  #else
  #ifdef precise
    tempTree.setNULL();
  #endif
  #endif

    // Create a long lived object
    cout << " Creating a long-lived binary tree of depth " << kLongLivedTreeDepth << endl;
  #ifdef GC
  #ifdef precise
    longLivedTree = gc_new<Node0>(1);
  #else
    longLivedTree = new (GC_NEW(Node0)) Node0();
  #endif
  #else
    longLivedTree = new Node0();
  #endif

    Populate(kLongLivedTreeDepth, longLivedTree);
    // Create long-lived array, filling half of it
    cout << " Creating a long-lived array of " << kArraySize << " doubles" << endl;
  #ifdef GC
  #ifdef precise
    gc_ptr<double> array = gc_new<double>(kArraySize);
  #else
    double * array = (double *) GC_MALLOC_ATOMIC(sizeof(double) * kArraySize);
  #endif
  #else
    double * array = (double *) malloc (sizeof(double) * kArraySize);
  #endif
    for (int i = 0; i < kArraySize/2; ++i) {
      array[i] = 1.0/i;
    }

  #ifdef GC
  #ifndef precise
    PrintDiagnostics();
  #endif
  #endif

    for (int d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2) {
      TimeConstruction(d);
    }

    if (longLivedTree == 0) cout << "Tree fail" << endl;
    if ( array[1000] != 1.0/1000) cout << "Array Failed" << endl;
    // fake reference to LongLivedTree
    // and array
    // to keep them from being optimized away

    tFinish = currentTime();
    tElapsed = elapsedTime(tFinish-tStart);
    PrintDiagnostics();
    cout << "Completed in " << tElapsed << " msec" << endl;
  #ifdef GC
  //		  cout << "Completed " << GC_gc_no << " collections" <<endl;
  //		  cout << "Heap size is " << GC_get_heap_size() << endl;
  #endif
  }
};

int main () {
  init_segregated_storage();
  GCBench x;
  x.main();
  return 0;
}
