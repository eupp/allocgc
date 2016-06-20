#include <iostream>
#include <cstring>
#include <cmath>
#include "timing.h"
#include <libprecisegc/libprecisegc.h>

#define MAX_LEVEL 6
const float P = 0.5;
using namespace std;

/* value generator */
float frand (void) { return (float) rand() / RAND_MAX; }
/* level generator */
int random_level (void) {
    static bool first = true;
    if (first) {
        srand((unsigned)time(NULL));
        first = false;
    }
    int lvl = (int)(log(frand()) / log(1.-P));
    return lvl % (MAX_LEVEL + 1);
}

/* a node of skip list */
struct SNode {
    int value;
    // SNode **nextArr;
    gc_ptr<gc_ptr<SNode>> nextArr;
    SNode (int level = MAX_LEVEL, int value = 0) {
        // nextArr = new SNode * [level + 1];
        nextArr = gc_new<gc_ptr<SNode>>(level + 1);
        // memset(nextArr, 0, sizeof(SNode*) * (level + 1));
        this->value = value;
    }
    ~SNode () {}
};

/* a skip list */
class SkipList {
private:
    // SNode *header;
    gc_ptr<SNode> header;
    int value;
    int level;
public:
    SkipList () {
        // header = new SNode(MAX_LEVEL, value);
        header = gc_new<SNode, int, int>(MAX_LEVEL, value);
        level = 0;
    }
    ~SkipList () {}

    void insert_element (int value) {
        gc_ptr<SNode> x = header;
        gc_ptr<gc_ptr<SNode>> update = gc_new<gc_ptr<SNode>>(MAX_LEVEL + 1);
        // memset(update, 0, sizeof(SNode*) * (MAX_LEVEL + 1));
        for (int i = level; i >= 0; i--) {
            while (x->nextArr[i] != NULL && x->nextArr[i]->value < value) {
                x = x->nextArr[i];
            }
            update[i] = x;
        }
        x = x->nextArr[0];
        if (!x || x->value != value) {
            int lvl = random_level();
            if (lvl > level) {
                for (int i = level + 1;i <= lvl;i++) {
                    update[i] = header;
                }
                level = lvl;
            }
            // x = new SNode(lvl, value);
            x = gc_new<SNode, int, int>(lvl, value);
            for (int i = 0; i <= lvl; i++) {
                x->nextArr[i] = update[i]->nextArr[i];
                update[i]->nextArr[i] = x;
            }
        }
    }

    void delete_element (int value) {
        gc_ptr<SNode> x = header;
        gc_ptr<gc_ptr<SNode>> update = gc_new <gc_ptr<SNode>>(MAX_LEVEL + 1);
        // memset (update, 0, sizeof(SNode*) * (MAX_LEVEL + 1));
        for (int i = level; i >= 0; i--) {
            while (x->nextArr[i] != NULL && x->nextArr[i]->value < value) {
                x = x->nextArr[i];
            }
            update[i] = x;
        }
        x = x->nextArr[0];
        if (x->value == value) {
            for (int i = 0;i <= level;i++) {
                if (update[i]->nextArr[i] != x) {
                    break;
                }
                update[i]->nextArr[i] = x->nextArr[i];
            }
            // delete x;
            while (level > 0 && header->nextArr[level] == NULL) {
                level--;
            }
        }
    }

    void display (void) {
        // const SNode *x = header->nextArr[0];
        gc_ptr<SNode> x = header->nextArr[0];
        while (x != NULL) {
            cout << x->value;
            x = x->nextArr[0];
            if (x != NULL) {
                cout << " - ";
            }
        }
        cout <<endl;
    }

    bool contains (int val) {
        // SNode *x = header;
        gc_ptr<SNode> x = header;
        for (int i = level; i >= 0; i--) {
            while (x->nextArr[i] != NULL && x->nextArr[i]->value < val) {
                x = x->nextArr[i];
            }
        }
        x = x->nextArr[0];
        return x != NULL && x->value == val;
    }
};

int main (void) {
    const int N = 1000;//00;
    long tStart, tFinish;
    std::cout << "Raw pointers: skiplist sizeof " << N << " elements:" << std::endl;

    std::cout << "\tCreating         ";
    tStart = currentTime();
    // SkipList *sl = new SkipList();
    gc_ptr<SkipList> sl = gc_new<SkipList>();
    for (int i = 0; i < N; i++) {
        sl->insert_element((rand() % N));
    }
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

    std::cout << "\tCalculating      ";
    for (int i = 0; i < N; i++) {
        sl->contains((rand() % N));
    }
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

    sl->display();

    // std::cout << "\tDestroying       ";
    // sl->~SkipList();
    // tFinish = currentTime();
    // std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;
    return 0;
}