#ifndef BTree_H
#define BTree_H
#include <iostream>
#include <vector>
#include "node.h"
#include <cmath>
#include <stack>

using namespace std;

template <typename T>
class BTree {
public:
    Node<T>* root;
    int M;
    int n;

public:
    explicit BTree(int M) : root(nullptr), M(M), n(0) {}

    bool search(T key) {
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
        if (root->count == M) {
            Node<T>* newRoot = new Node<T>(M);
            newRoot->leaf = false;
            newRoot->children[0] = root;
            ExtractionResult* splitResult = splitChild(newRoot, 0, root);
            root = newRoot;
            insertNonFull(root, key);
        } else {
            insertNonFull(root, key);
        }
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

            if (node->count == M) {
                ExtractionResult* splitResult = splitChild(node->parent, i, node);
            }
        } else {
            while (i >= 0 && key < node->keys[i]) {
                i--;
            }
            i++;

            if (node->children[i]->count == M) {
                ExtractionResult* splitResult = splitChild(node, i, node->children[i]);
                if (key > node->keys[i]) {
                    i++;
                }
            }

            insertNonFull(node->children[i], key);
        }
    }

    ExtractionResult* splitChild(Node<T>* parent, int i, Node<T>* fullChild) {

        Node<T>* newChild = new Node<T>(M);
        newChild->leaf = fullChild->leaf;

        int m = (M - 1) / 2;
        T middle = fullChild->keys[m];

        for (int j = 0; j < M - 1 - m; ++j) {
            newChild->keys[j] = fullChild->keys[m + 1 + j];
        }
        if (!fullChild->leaf) {
            for (int j = 0; j < M - m; ++j) {
                newChild->children[j] = fullChild->children[m + 1 + j];
            }
        }

        newChild->count = M - 1 - m;
        fullChild->count = m;

        for (int j = parent->count; j >= i + 1; --j) {
            parent->children[j + 1] = parent->children[j];
        }
        parent->children[i + 1] = newChild;

        for (int j = parent->count - 1; j >= i; --j) {
            parent->keys[j + 1] = parent->keys[j];
        }
        parent->keys[i] = middle;
        parent->count++;

        return new ExtractionResult(middle, nullptr, newChild);
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

    int height() {
        int height = (log(n) / log(M));
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

    T minKey() {
        Node<T>* node = root;
        while (!node->leaf) {
            node = node->children[0];
        }
        return node->keys[0];
    }

    T maxKey() {
        string s = toString(",");
        size_t last = s.find_last_of(',');
        return stoi(s.substr(last + 1));
    }

    void clear() {
        root->killSelf();
        root = nullptr;
        n = 0;
    }

    int size() {
        return n;
    }

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

    bool check_properties() {
        return check(root, true);
    }

    ~BTree() {
        if (root) root->killSelf();
    }

private:

    void toString(Node<T>* node, string &result, string sep) {
        int i = 0;
        for (; i < node->count; i++)  {
            if (!node->leaf) toString(node->children[i], result, sep);
            result += std::to_string(node->keys[i]) + sep;
        }
        if (!node->leaf) toString(node->children[i], result, sep);
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
