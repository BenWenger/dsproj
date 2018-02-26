

template<typename T>
class TreeList<T>::iterator
{
public:
    iterator() = default;
    iterator(const iterator& rhs) = default;
    iterator& operator = (const iterator& rhs) = default;
    bool operator == (const iterator& rhs) const    { return (node == rhs.node) && (host == rhs.host);  }
    bool operator != (const iterator& rhs) const    { return !(*this == rhs);                           }
    iterator& operator ++ ()            { node = node->next;        return *this;       }
    iterator  operator ++ (int)         { auto tmp = *this; ++(*this);  return tmp;     }
    
    T& operator * ()                    { return node->obj;    }
    const T& operator * () const        { return node->obj;    }

private:
    typedef TreeList<T>         Host;
    typedef typename Host::Node Node;
    friend class TreeList<T>;
    iterator(const Host* h, Node* n) : host(h), node(n) {}
    const Host* host = nullptr;
    Node*       node = nullptr;
};

template<typename T>
class TreeList<T>::const_iterator
{
public:
    const_iterator() = default;
    const_iterator(const const_iterator& rhs) = default;
    const_iterator& operator = (const const_iterator& rhs) = default;
    bool operator == (const const_iterator& rhs) const    { return (node == rhs.node) && (host == rhs.host);  }
    bool operator != (const const_iterator& rhs) const    { return !(*this == rhs);                           }
    const_iterator& operator ++ ()            { node = node->next;        return *this;       }
    const_iterator  operator ++ (int)         { auto tmp = *this; ++(*this);  return tmp;     }
    
    const T& operator * () const              { return node->obj;   }

private:
    typedef TreeList<T>         Host;
    typedef typename Host::Node Node;
    friend class TreeList<T>;
    const_iterator(const Host* h, const Node* n) : host(h), node(n) {}
    const Host* host = nullptr;
    const Node* node = nullptr;
};
