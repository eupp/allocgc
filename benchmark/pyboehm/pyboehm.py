import gc
import sys
import time

from array import array

kStretchTreeDepth = 18      # 18       // about 16Mb
kLongLivedTreeDepth = 16    # 16       // about 4Mb
kArraySize = 500000         # 500000   // about 4Mb
kMinTreeDepth = 4           # 4
kMaxTreeDepth = 16          # 16


class Node:

    def __init__(self, left=None, right=None):
        self.left  = left
        self.right = right
        self.i = self.j = 0


def timeElapsed(start, end):
    return round((end - start) * 1000)


def treeSize(i):
    return (1 << (i + 1)) - 1


def numIters(i):
    return 2 * treeSize(kStretchTreeDepth) // treeSize(i)


def populate(iDepth, thisNode):
    if iDepth <= 0:
        return
    else:
        iDepth -= 1
        thisNode.left  = Node()
        thisNode.right = Node()
        populate(iDepth, thisNode.left)
        populate(iDepth, thisNode.right)


def makeTree(iDepth):
    if iDepth <= 0:
        return Node()
    else:
        return Node(makeTree(iDepth-1), makeTree(iDepth-1))


def timeConstruction(depth):
    iNumIters = numIters(depth)

    print("Creating {} trees of depth {}".format(iNumIters, depth))

    tStart = time.monotonic()
    for i in range(0, iNumIters):
        tempTree = Node()
        populate(depth, tempTree)
        tempTree = None
    tFinish = time.monotonic()

    print("\tTop down construction took {} ms".format(timeElapsed(tStart, tFinish)))

    tStart = time.monotonic()
    for i in range(0, iNumIters):
        tempTree = makeTree(depth)
        tempTree = None
    tFinish = time.monotonic()

    print("\tBottom up construction took {} ms".format(timeElapsed(tStart, tFinish)))


def main():
    print("Garbage Collector Test")
    print(" Live storage will peak at {} bytes."
          .format(2 * sys.getsizeof(Node()) * treeSize(kLongLivedTreeDepth) + sys.getsizeof(0.0) * kArraySize))
    print(" Stretching memory with a binary tree of depth {}".format(kStretchTreeDepth))

    tStart = time.monotonic()

    # Stretch the memory space quickly
    tempTree = makeTree(kStretchTreeDepth)
    tempTree = None

    # Create a long lived object
    print(" Creating a long-lived binary tree of depth {}".format(kLongLivedTreeDepth))

    longLivedTree = Node()
    populate(kLongLivedTreeDepth, longLivedTree)

    # Create long-lived array, filling half of it
    print(" Creating a long-lived array of {} doubles".format(kArraySize))

    longLivedArray = array('d', [0.0] + [1.0 / x for x in range(1, kArraySize)])

    for d in range(kMinTreeDepth, kMaxTreeDepth+1, 2):
        timeConstruction(d)

    if longLivedTree is None or longLivedArray[1000] != 1.0 / 1000:
        print("Failed")

    tFinish = time.monotonic()

    print("Completed in {} ms".format(timeElapsed(tStart, tFinish)))
    print("Completed {} collections".format(sum(gc.get_count())))
    # print("Time spent in gc {} ms")
    # print("Average pause time {} us")

if __name__ == '__main__':
    # gc.set_debug(gc.DEBUG_STATS)
    main()
