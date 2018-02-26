
#ifndef TREELIST_H_INCLUDED
#define TREELIST_H_INCLUDED

#include <string>

template <typename T>
class TreeList
{
public:
    class iterator;
    class const_iterator;

    TreeList() = default;
    ~TreeList()                 { clear();      }

    // no copying
    TreeList(const TreeList&) = delete;
    TreeList& operator = (const TreeList&) = delete;

    // but moving is OK
    TreeList(TreeList&& rhs);
    TreeList& operator = (TreeList&& rhs);

    
    void insert(const T& obj);
    void insert(T&& obj);
    void clear();

    iterator        erase(const iterator& i);

    iterator        begin();
    const_iterator  begin() const;
    iterator        end();
    const_iterator  end() const;

    iterator        find(const T& v)            { return internalFind<iterator>(root, v);       }
    const_iterator  find(const T& v) const      { return internalFind<const_iterator>(root, v); }

    int             size() const  {     return numNodes;    }
    bool            empty() const {     return !root;       }

    // For debugging
    void            validate() const;


private:
    friend class iterator;
    friend class const_iterator;
    struct Node
    {
        Node*   parent = nullptr;
        Node*   left = nullptr;
        Node*   right = nullptr;
        Node*   prev = nullptr;
        Node*   next = nullptr;
        T       obj;

        Node(const T& rhs) : obj(rhs) {}
        Node(T&& rhs) : obj(std::move(rhs)) {}
    };

    Node*       root = nullptr;
    Node*       head = nullptr;
    int         numNodes = 0;

    static void recursiveDelete(Node* n);
    void        internalInsert(Node* n);

    template <typename iter_t, typename node_t>
    iter_t internalFind(node_t* node, const T& v) const;


    // For debugging
    int validateTree(const Node* n, int rec) const;
    int validateList(const Node* n, int rec) const;

};

#include "treelist_iterators.hpp"
#include "treelist.hpp"


#endif