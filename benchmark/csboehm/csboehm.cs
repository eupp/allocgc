using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

class CSBoehm
{
    public const int kStretchTreeDepth    = 18; //18;       // about 16Mb
    public const int kLongLivedTreeDepth  = 16; //16;       // about 4Mb
    public const int kArraySize  = 500000;      //500000;   // about 4Mb
    public const int kMinTreeDepth = 4;         //4
    public const int kMaxTreeDepth = 16;        //16;

    public class Node
    {
        public Node left;
        public Node right;
        public int i;
        public int j;

        public Node(Node left_ = null, Node right_ = null)
        {
            left = left_;
            right = right_;
        }
    }

    public static int TreeSize(int i) {
        return ((1 << (i + 1)) - 1);
    }

    public static int NumIters(int i) {
        return 2 * TreeSize(kStretchTreeDepth) / TreeSize(i);
    }

    public static void Populate(int iDepth, Node thisNode)
    {
        if (iDepth<=0) {
            return;
        } else {
            iDepth--;
            thisNode.left  = new Node();
            thisNode.right = new Node();
            Populate(iDepth, thisNode.left);
            Populate(iDepth, thisNode.right);
        }
    }

    static Node MakeTree(int iDepth)
    {
        if (iDepth<=0) {
            return new Node();
        } else {
            return new Node(MakeTree(iDepth-1), MakeTree(iDepth-1));
        }
    }

    static void TimeConstruction(int depth)
    {
        int iNumIters = NumIters(depth);
        Node tempTree = null;

        Console.WriteLine(String.Format("Creating {0} trees of depth {1}", iNumIters, depth));

        var sw = Stopwatch.StartNew();
        for (int i = 0; i < iNumIters; ++i) {
            tempTree = new Node();
            Populate(depth, tempTree);
            tempTree = null;
        }
        sw.Stop();

        Console.WriteLine(String.Format("\tTop down construction took {0} ms", sw.ElapsedMilliseconds));

        sw.Restart();
        for (int i = 0; i < iNumIters; ++i) {
            tempTree = MakeTree(depth);
            tempTree = null;
        }
        sw.Stop();

        Console.WriteLine(String.Format("\tBottom up construction took {0} ms", sw.ElapsedMilliseconds));
    }

    public static void Main()
    {
        Node root = null;
        Node longLivedTree = null;
        Node tempTree = null;

        Console.WriteLine("Garbage Collector Test");
        // Console.WriteLine(String.Format(" Live storage will peak at {0} bytes.",
        //     2 * Marshal.SizeOf(new Node()) * TreeSize(kLongLivedTreeDepth) + /*sizeof(double)*/ 8 * kArraySize));


        var sw = Stopwatch.StartNew();

        Console.WriteLine(String.Format(" Stretching memory with a binary tree of depth {0}", kStretchTreeDepth));
        tempTree = MakeTree(kStretchTreeDepth);
        tempTree = null;        

        Console.WriteLine(String.Format(" Creating a long-lived binary tree of depth {0}", kLongLivedTreeDepth ));
        longLivedTree = new Node();
        Populate(kLongLivedTreeDepth, longLivedTree);

        Console.WriteLine(String.Format(" Creating a long-lived array of {0} doubles", kArraySize));
        double[] array = new double[kArraySize];
        for (int i = 0; i < kArraySize/2; ++i) {
            array[i] = 1.0 / i;
        }

        for (int d = kMinTreeDepth; d <= kMaxTreeDepth; d += 2) {
            TimeConstruction(d);
        }

        if (longLivedTree == null || array[1000] != 1.0 / 1000 ) {
            Console.WriteLine("Failed");
            // fake reference to LongLivedTree
            // and array
            // to keep them from being optimized away
        }

        sw.Stop();

        Console.WriteLine(String.Format("Completed in {0} ms", sw.ElapsedMilliseconds));
    }
}
