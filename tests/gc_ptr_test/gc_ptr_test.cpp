#include <iostream>
#include <sys/time.h>
#define currentTime() stats_rtclock()
#define elapsedTime(x) (x)

using namespace std;

#define GC_TEST

#ifndef GC_TEST
	#ifdef GC
		#undef GC
	#endif
	#include <msmalloc.h>
#else
	#define GC
	#include <libgc/libgc.h>
#endif

unsigned
stats_rtclock( void )
{
  struct timeval t;
  struct timezone tz;

  if (gettimeofday( &t, &tz ) == -1)
    return 0;
  return (unsigned)(t.tv_sec * 1000 + t.tv_usec / 1000);
}

#ifdef GC
struct Node0 {
	gc_ptr<Node0> left;
	gc_ptr<Node0> right;
    int i, j;
    Node0(gc_ptr<Node0> l, gc_ptr<Node0> r) { left = l; right = r; }
    Node0() {}
};
#else
struct Node0 {
	Node0 * left;
	Node0 * right;
    int i, j;
    Node0(Node0 * l, Node0 * r) {	left = l; right = r;	}
    Node0() {	left = right = NULL;	}
};
#endif

// Build tree top down, assigning to older objects.
#ifdef GC
static void Populate(int iDepth, gc_ptr<Node0> thisNode) {
	if (iDepth <= 0) {
		return;
	} else {
		iDepth--;
		thisNode->left = gc_new<Node0>();
		thisNode->right = gc_new<Node0>();
		Populate (iDepth, thisNode->left);
		Populate (iDepth, thisNode->right);
	}
}
#else
static void Populate(int iDepth, Node0 * thisNode) {
	if (iDepth <= 0) {
		return;
	} else {
		iDepth--;
		thisNode->left  = new Node0();
		thisNode->right = new Node0();
		Populate (iDepth, thisNode->left);
		Populate (iDepth, thisNode->right);
	}
}
#endif

// Build tree bottom-up
#ifdef GC
static gc_ptr<Node0> MakeTree(int iDepth) {
    if (iDepth <= 0) {
		return gc_new<Node0>();
	} else {
		return gc_new<Node0, gc_ptr<Node0>, gc_ptr<Node0>>(MakeTree(iDepth-1), MakeTree(iDepth-1));
	}
}
#endif

#ifndef GC
static Node0 * MakeTree(int iDepth) {
    if (iDepth <= 0) {
    	return new Node0();
	} else {
	    return new Node0(MakeTree(iDepth-1), MakeTree(iDepth-1));
	}
}
#endif

void test () {
	long tStart, tFinish;
	int depth = 20, count = 1;
	tStart = currentTime();
	for (int i = 0; i < count; i++) {
	#ifdef GC
		// gc_ptr<Node0> temp = MakeTree(depth);
		gc_ptr<Node0> temp = gc_new<Node0>();
		Populate(depth, temp);
	#else
		// Node0 * temp = MakeTree(depth);
		Node0 * temp = new Node0();
		Populate(depth, temp);
	#endif
	}
	tFinish = currentTime();
#ifdef GC
	cout << "GC Yes" << endl;
#else
	cout << "GC NO" << endl;
#endif
	cout << elapsedTime(tFinish - tStart) << " msec" << endl;
}

int main (void) {
	test();
#ifdef GC
	long tStart, tFinish;
	tStart = currentTime();
	gc();
	tFinish = currentTime();
	cout << "gc time: " << elapsedTime(tFinish - tStart) << " msec" << endl;
#endif
	return 0;
}
