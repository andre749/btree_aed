#ifndef NODE_H
#define NODE_H

template <typename T>
class Node {
public:
    int count;
    T* keys;
    Node** children;
    bool leaf;
    int M;

    Node(int M, bool leaf = true) : M(M), leaf(leaf), count(0) {
        keys = new T[M - 1];
        children = new Node*[M];
        for (int i = 0; i < M; ++i)
            children[i] = nullptr;
    }

    void killSelf() {
        if (!leaf) {
            for (int i = 0; i <= count; ++i) {
                if (children[i]) {
                    children[i]->killSelf();
                    delete children[i];
                }
            }
        }
        delete[] keys;
        delete[] children;
    }
};

#endif
