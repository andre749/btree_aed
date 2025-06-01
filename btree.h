#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include "node.h"
#include <cmath>
#include <stack>
using namespace std;

void siguiente_nodo(vector<int>& indices,int M){
    int i=indices.size()-1;
    indices[i]++;
    while(i>=0 && indices[i]==M){
        indices[i]=0;
        if(i==0) {
            indices.insert(indices.begin(), 1);
            return;
        }
        i--;
        if (i >= 0) indices[i]++;
    }
}
void print(vector<int> &a){
    for(auto& n:a){
        cout<<n;
    }
    cout<<endl;
}

template <typename T>
class BTree {
public:
    Node<T>* root;
    int M;
    int n;

public:
    explicit BTree(int M) : root(nullptr), M(M),n(0) {}

    bool search(T key){
        Node<T>* node = root;
        while (node != nullptr) {
            int i = 0;
            while (i < node->count && key > node->keys[i]) i++;
            if (i < node->count && key == node->keys[i]) return true;
            if (node->leaf) return false;
            node = node->children[i];
        }
        return false;
    }

    void insert(T key) {
        if (root == nullptr) {
            root = new Node<T>(M);
            root->keys[0] = key;
            root->count = 1;
            root->leaf = true;
            n++;
            return;
        }

        if (root->count == M - 1) {
            Node<T>* newRoot = new Node<T>(M);
            newRoot->leaf = false;
            newRoot->children[0] = root;

            splitChild(newRoot, 0, root);
            root = newRoot;
        }

        insertNonFull(root, key);
        n++;
    }

    void remove(T key) {
        removeInternal(root, key);
        if (root->count == 0 && !root->leaf) {
            Node<T>* oldRoot = root;
            root = root->children[0];
            delete oldRoot;
        }
        n--;
    }

    int height(){
        int height=(log(n)/log(M));

        return height;
    }

    string toString(string sep) {
        string ret;
        toString(root, ret, sep);
        if (!ret.empty()) {
            for (int i = 0; i < sep.size(); i++)
                ret.pop_back();
        }
        return ret;
    }

    vector<T> range_search(T begin, T end) {
        vector<T> result;
        range_search_helper(root, begin, end, result);
        return result;
    }

    T minKey(){
        Node<T>* node=root;
        while(!node->leaf){
            node=node->children[0];
        }
        return node->keys[0];
    }

    T maxKey() { // profe no se porque pero esto solo funcionaba asi y no se como arreglarlo,
        // se que es ineficiente pero por alguna razon la forma normal fallaba
        string s = toString(",");
        size_t last = s.find_last_of(',');
        return stoi(s.substr(last + 1));
    }



    void clear(){
        root->killSelf();
        root = nullptr;
        n = 0;
    }

    int size(){return n; }

    static BTree* build_from_ordered_vector(vector<T> elements, int M) {
        int n = elements.size();
        if (n == 0) return new BTree<T>(M);

        vector<Node<T>*> current_level;
        for (int i = 0; i < n; i += M - 1) {
            Node<T>* node = new Node<T>(M);
            int j = 0;
            for (; j < M - 1 && i + j < n; ++j) {
                node->keys[j] = elements[i + j];
            }
            node->count = j;
            node->leaf = true;
            for (int k = 0; k < M; ++k) node->children[k] = nullptr;
            current_level.push_back(node);
        }

        while (current_level.size() > 1) {
            vector<Node<T>*> next_level;
            for (int i = 0; i < current_level.size(); i += M) {
                Node<T>* parent = new Node<T>(M);
                int j = 0;
                for (; j < M && i + j < current_level.size(); ++j) {
                    parent->children[j] = current_level[i + j];
                }
                parent->count = j - 1;
                parent->leaf = false;
                for (int k = 0; k < parent->count; ++k) {
                    parent->keys[k] = parent->children[k + 1]->keys[0];
                }
                next_level.push_back(parent);
            }
            current_level = next_level;
        }

        BTree<T>* tree = new BTree<T>(M);
        tree->root = current_level[0];
        tree->n = n;
        return tree;
    }

    bool check_properties(){
        return check(root,true);
    }

    ~BTree(){
        if(root) root->killSelf();
    }

private:

    void toString(Node<T>* nodo, string &result, string sep){
        int i=0;
        for (; i < nodo->count; i++)  {
            if (!nodo->leaf) toString(nodo->children[i], result, sep);
            result += std::to_string(nodo->keys[i]) + sep;
        }
        if (!nodo->leaf) toString(nodo->children[i], result, sep);
    }

    bool check(Node<T>* node, bool root = true) {
        if (!node) return true;

        if (!root && node->count < (M + 1) / 2 - 1)
            return false;
        if (node->count > M - 1)
            return false;

        for (int i = 1; i < node->count; i++) {
            if (node->keys[i - 1] > node->keys[i])
                return false;
        }

        if (node->leaf) {
            for (int i = 0; i <= node->count; i++) {
                if (node->children[i] != nullptr) return false;
            }
        } else {
            for (int i = 0; i <= node->count; i++) {
                if (node->children[i] == nullptr) return false;
            }

            for (int i = 0; i < node->count; i++) {
                T element = node->keys[i];
                Node<T>* left = node->children[i];
                Node<T>* right = node->children[i + 1];

                for (int j = 0; j < left->count; j++) {
                    if (left->keys[j] > element) return false;
                }
                for (int j = 0; j < right->count; j++) {
                    if (right->keys[j] < element) return false;
                }
            }
        }

        for (int i = 0; i <= node->count; i++) {
            if (!check(node->children[i], false))
                return false;
        }

        return true;
    }

    void splitChild(Node<T>* parent, int i, Node<T>* fullChild) {
        Node<T>* newChild = new Node<T>(M);
        newChild->leaf = fullChild->leaf;

        int t = (M - 1) / 2;


        for (int j = 0; j < M - 1 - t - 1; ++j)
            newChild->keys[j] = fullChild->keys[j + t + 1];

        if (!fullChild->leaf) {
            for (int j = 0; j < M - t; ++j)
                newChild->children[j] = fullChild->children[j + t + 1];
        }

        newChild->count = M - 1 - t - 1;

        T middleKey = fullChild->keys[t];

        fullChild->count = t;

        for (int j = parent->count; j >= i + 1; --j)
            parent->children[j + 1] = parent->children[j];

        parent->children[i + 1] = newChild;

        for (int j = parent->count - 1; j >= i; --j)
            parent->keys[j + 1] = parent->keys[j];

        parent->keys[i] = middleKey;
        parent->count++;
    }



    void insertNonFull(Node<T>* node, T key) {
        int i = node->count - 1;

        if (node->leaf) {
            while (i >= 0 && key < node->keys[i]) {
                node->keys[i + 1] = node->keys[i];
                i--;
            }
            node->keys[i + 1] = key;
            node->count++;
        } else {
            while (i >= 0 && key < node->keys[i]) i--;
            i++;

            if (node->children[i]->count == M - 1) {
                splitChild(node, i, node->children[i]);
                if (key > node->keys[i])
                    i++;
            }

            insertNonFull(node->children[i], key);
        }
    }

    void removeInternal(Node<T>* node, T key) {
        int idx = 0;
        while (idx < node->count && key > node->keys[idx]) idx++;

        if (idx < node->count && node->keys[idx] == key) {
            if (node->leaf) {
                for (int i = idx; i < node->count - 1; ++i)
                    node->keys[i] = node->keys[i + 1];
                node->count--;
            } else {
                Node<T>* pred = node->children[idx];
                if (pred->count >= (M + 1) / 2) {
                    while (!pred->leaf)
                        pred = pred->children[pred->count];
                    node->keys[idx] = pred->keys[pred->count - 1];
                    removeInternal(node->children[idx], node->keys[idx]);
                } else {
                    Node<T>* succ = node->children[idx + 1];
                    if (succ->count >= (M + 1) / 2) {
                        while (!succ->leaf)
                            succ = succ->children[0];
                        node->keys[idx] = succ->keys[0];
                        removeInternal(node->children[idx + 1], node->keys[idx]);
                    } else {
                        merge(node, idx);
                        removeInternal(node->children[idx], key);
                    }
                }
            }
        } else {
            if (node->leaf) return;
            bool flag = (idx == node->count);
            if (node->children[idx]->count < (M + 1) / 2) {
                if (idx != 0 && node->children[idx - 1]->count >= (M + 1) / 2)
                    borrowFromPrev(node, idx);
                else if (idx != node->count && node->children[idx + 1]->count >= (M + 1) / 2)
                    borrowFromNext(node, idx);
                else {
                    if (idx != node->count)
                        merge(node, idx);
                    else
                        merge(node, idx - 1);
                }
            }
            if (flag && idx > node->count)
                removeInternal(node->children[idx - 1], key);
            else
                removeInternal(node->children[idx], key);
        }
    }

    void merge(Node<T>* node, int idx) {
        Node<T>* child = node->children[idx];
        Node<T>* sibling = node->children[idx + 1];

        child->keys[(M - 1) / 2] = node->keys[idx];

        for (int i = 0; i < sibling->count; ++i)
            child->keys[i + (M - 1) / 2 + 1] = sibling->keys[i];

        if (!child->leaf) {
            for (int i = 0; i <= sibling->count; ++i)
                child->children[i + (M - 1) / 2 + 1] = sibling->children[i];
        }

        for (int i = idx + 1; i < node->count; ++i)
            node->keys[i - 1] = node->keys[i];
        for (int i = idx + 2; i <= node->count; ++i)
            node->children[i - 1] = node->children[i];

        child->count += sibling->count + 1;
        node->count--;

        delete sibling;
    }

    void borrowFromPrev(Node<T>* node, int idx) {
        Node<T>* child = node->children[idx];
        Node<T>* sibling = node->children[idx - 1];

        for (int i = child->count - 1; i >= 0; --i)
            child->keys[i + 1] = child->keys[i];

        if (!child->leaf) {
            for (int i = child->count; i >= 0; --i)
                child->children[i + 1] = child->children[i];
        }

        child->keys[0] = node->keys[idx - 1];
        if (!child->leaf)
            child->children[0] = sibling->children[sibling->count];

        node->keys[idx - 1] = sibling->keys[sibling->count - 1];
        child->count++;
        sibling->count--;
    }

    void borrowFromNext(Node<T>* node, int idx) {
        Node<T>* child = node->children[idx];
        Node<T>* sibling = node->children[idx + 1];

        child->keys[child->count] = node->keys[idx];

        if (!child->leaf)
            child->children[child->count + 1] = sibling->children[0];

        node->keys[idx] = sibling->keys[0];

        for (int i = 1; i < sibling->count; ++i)
            sibling->keys[i - 1] = sibling->keys[i];
        if (!sibling->leaf) {
            for (int i = 1; i <= sibling->count; ++i)
                sibling->children[i - 1] = sibling->children[i];
        }

        child->count++;
        sibling->count--;
    }
    void range_search_helper(Node<T>* node, T begin, T end, vector<T>& result) {
        if (!node) return;

        int i = 0;
        while (i < node->count) {
            if (!node->leaf)
                range_search_helper(node->children[i], begin, end, result);

            if (node->keys[i] >= begin && node->keys[i] <= end)
                result.push_back(node->keys[i]);

            if (node->keys[i] > end)
                return;

            i++;
        }

        if (!node->leaf)
            range_search_helper(node->children[i], begin, end, result);
    }


};

#endif
