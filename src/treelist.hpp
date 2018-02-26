
template <typename T>
TreeList<T>::TreeList(TreeList&& rhs)
{
    numNodes = rhs.numNodes;
    root = rhs.root;
    head = rhs.head;
    rhs.root = nullptr;
    rhs.head = nullptr;
}

template <typename T>
TreeList<T>& TreeList<T>::operator = (TreeList&& rhs)
{
    if(this != &rhs)
    {
        clear();
        numNodes = rhs.numNodes;
        root = rhs.root;
        head = rhs.head;
        rhs.root = nullptr;
        rhs.head = nullptr;
    }
    return *this;
}

template <typename T>
void TreeList<T>::clear()
{
    recursiveDelete(root);
    root = head = nullptr;
    numNodes = 0;
}

template <typename T>
inline void TreeList<T>::recursiveDelete(Node* n)
{
    if(n)
    {
        recursiveDelete(n->left);
        recursiveDelete(n->right);
        delete n;
    }
}

template <typename T> auto TreeList<T>::begin() -> iterator                 { return iterator(this, head);          }
template <typename T> auto TreeList<T>::begin() const -> const_iterator     { return const_iterator(this, head);    }
template <typename T> auto TreeList<T>::end() -> iterator                   { return iterator(this, nullptr);       }
template <typename T> auto TreeList<T>::end() const -> const_iterator       { return const_iterator(this, nullptr); }

template <typename T>
auto TreeList<T>::erase(const iterator& i) -> iterator
{
    Node* out = i.node->next;

    // change prev/next pointers in the list
    if(i.node->next)            i.node->next->prev = i.node->prev;
    if(i.node->prev)            i.node->prev->next = i.node->next;
    else                        head = i.node->next;

    // change node pointers for the tree
    Node** mech = nullptr;      // 'mech' is where this node is stored in its parent (the pointer we need to replace)
    if(!i.node->parent)                         mech = &root;                 // no parent?  this is the root node
    else if(i.node->parent->left == i.node)     mech = &i.node->parent->left;
    else                                        mech = &i.node->parent->right;
    
    Node* newme = nullptr;
    Node* l = i.node->left;
    Node* r = i.node->right;

    if(l && r)          // we have two children!  take the rightmost left child (which is 'prev')
    {
        newme = i.node->prev;
        // 'newme' by definition can't have a right child, because it's the rightmost left child of this node
        if(newme == l)      // rightmost left child was our direct left child
        {
            newme->parent = i.node->parent;
            newme->right = r;
        }
        else                // it was some descendent
        {
            // newme must have been the right child of its parent
            newme->parent->right = newme->left;
            if(newme->left)     newme->left->parent = newme->parent;
            newme->left = l;
            newme->right = r;
        }
        if(newme->left)             // it's possible for the newme to not have a left if it's our direct child
            newme->left->parent = newme;
        newme->right->parent = newme;
    }
    else if(l)          // only 1 child
        newme = l;
    else if(r)
        newme = r;

    // adjust the parent
    if(newme)       newme->parent = i.node->parent;
    *mech = newme;

    --numNodes;
    delete i.node;
    return iterator(this, out);
}

template <typename T> void TreeList<T>::insert(const T& obj)    { internalInsert(new Node(obj));            }
template <typename T> void TreeList<T>::insert(T&& obj)         { internalInsert(new Node(std::move(obj))); }

template <typename T> void TreeList<T>::internalInsert(Node* n)
{
    ++numNodes;
    if(!root)
    {
        root = head = n;
        return;
    }

    // 'n' is the node we're inserting
    bool onLeft = false;
    Node* top = root;
    while(top)
    {
        n->parent = top;
        if(n->obj < top->obj)
        {
            onLeft = true;
            top = top->left;
        }
        else
        {
            onLeft = false;
            top = top->right;
        }
    }

    if(onLeft)
    {
        n->parent->left = n;
        n->next = n->parent;
        n->prev = n->parent->prev;
        n->next->prev = n;
        if(n->prev)     n->prev->next = n;
    }
    else
    {
        n->parent->right = n;
        n->prev = n->parent;
        n->next = n->parent->next;
        n->prev->next = n;
        if(n->next)     n->next->prev = n;
    }

    if(!n->prev)
        head = n;
}

template <typename T>
template <typename iter_t, typename node_t>
iter_t TreeList<T>::internalFind(node_t* node, const T& v) const
{
    if(!node)               return iter_t(this, nullptr);
    if(v < node->obj)       return internalFind<iter_t>(node->left, v);
    if(node->obj < v)       return internalFind<iter_t>(node->right, v);

    return iter_t(this, node);
}

//////////////////////////////////////////////
//////////////////////////////////////////////
template <typename T>
void TreeList<T>::validate() const
{
    if(!root != !head)          throw std::runtime_error("Head/Root mismatch");
    if(!root)                   return;

    int treecount = validateTree(root, 100);
    int listcount = validateList(head, 200);

    if(treecount != listcount)
        throw std::runtime_error("treecount / listcount mismatch");
}

template <typename T>
int TreeList<T>::validateTree(const Node* n, int rec) const
{
    if(rec <= 0)                    throw std::runtime_error("Tree recursive counter expired. Possible infinite loop");
    if(!n)                          return 0;
    if(!n->parent && n != root)     throw std::runtime_error("Node (" + std::to_string(n->obj) + ") has no parent but isn't root node");

    if(n->left)
    {
        if(n->left->obj > n->obj)   throw std::runtime_error("Node (" + std::to_string(n->obj) + ") has left child who is greater");
        if(n->left->parent != n)    throw std::runtime_error("Node (" + std::to_string(n->obj) + ") left child's parent is not this node");
    }
    if(n->right)
    {
        if(n->right->obj < n->obj)  throw std::runtime_error("Node (" + std::to_string(n->obj) + ") has right child who is less");
        if(n->right->parent != n)   throw std::runtime_error("Node (" + std::to_string(n->obj) + ") right child's parent is not this node");
    }

    return validateTree(n->left, rec-1) + validateTree(n->right, rec-1) + 1;
}

template <typename T>
int TreeList<T>::validateList(const Node* n, int rec) const
{
    if(rec <= 0)                    throw std::runtime_error("List recursive counter expired. Possible infinite loop");
    if(!n)                          return 0;
    if(!n->prev && n != head)       throw std::runtime_error("Node (" + std::to_string(n->obj) + ") has no prev but isn't head node");

    if(n->prev)
    {
        if(n->prev->obj > n->obj)   throw std::runtime_error("Node (" + std::to_string(n->obj) + ") is less than it's previous node");
        if(n->prev->next != n)      throw std::runtime_error("Node (" + std::to_string(n->obj) + ") prev node's next pointer is not this node");
    }

    return validateList(n->next, rec-1) + 1;
}