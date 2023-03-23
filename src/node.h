#pragma once
#include "helper.h"
#include <concepts>
#include <list>

template <typename T>
class Node : public T
{
    std::list<Node<T>*> _children;
    Node<T>* _parent;

public:
    using Iterator       = typename std::list<Node<T>*>::iterator;
    using Const_iterator = typename std::list<Node<T>*>::const_iterator;

public:
    inline bool empty() const
    { return _children.empty(); }

    inline std::size_t size() const
    { return _children.size(); }

    DECLARE_ITERATOR_WRAPPER(_children)

public:
    inline Node<T>* parent() const
    { return _parent; }

    inline void parent(Node<T>* parent)
    { _parent = parent; }

public:
    Node() noexcept = default;

    void add(Node<T>* con)
    {
        con->parent(this);
        _children.push_back(con);
    }

    void remove(Node<T>* con)
    {
        con->parent(nullptr);
        auto it = std::find(_children.begin(), _children.end(), con);
        _children.erase(it);
    }

    void insert(Const_iterator position, Node<T>* con)
    {
        con->parent(this);
        _children.insert(position, con);
    }

    void shift(Const_iterator position, int shift_pos)
    {
        auto it_pos = std::next(position, (shift_pos > 0) ? 1 + shift_pos : shift_pos);
        _children.splice(it_pos, _children, position);
    }
    
    virtual ~Node() = default;
};

template <typename T>
void transfer_to(Node<T>* to, Node<T>* obj)
{
    assert_debug(obj->parent(), "obj is an orphan");
    obj->parent()->remove(obj);
    to->add(obj);
}
