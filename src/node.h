#pragma once
#include "helper.h"
#include <list>

// Forward decl
template <typename T> class Node_box;

template <typename T>
concept Node = requires(T& t, Node_box<T>* p)
{
    { t.parent() } -> std::same_as<Node_box<T>*>;
    { t.parent(p) };
};

template <typename T>
class Node_box : public T
{
public:
    using Iterator       = typename std::list<T*>::iterator;
    using Const_iterator = typename std::list<T*>::const_iterator;

protected:
    std::list<T*> _children;

public:
    inline bool empty() const
    { return _children.empty(); }

    inline std::size_t size() const
    { return _children.size(); }

    inline Iterator begin()
    { return _children.begin(); }

    inline Iterator end()
    { return _children.end(); }

    inline Const_iterator cbegin() const
    { return _children.cbegin(); }

    inline Const_iterator cend() const
    { return _children.cend(); }

public:
    Node_box() requires Node<T> = default;

    void add(T* con)
    {
        con->parent(this);
        _children.push_back(con);
    }

    void remove(T* con)
    {
        con->parent(nullptr);
        auto it = std::find(_children.begin(), _children.end(), con);
        _children.erase(it);
    }

    void insert(Const_iterator position, T* con)
    {
        con->parent(this);
        _children.insert(position, con);
    }

    void shift(Const_iterator position, int shift_pos)
    {
        auto it_pos = std::next(position, (shift_pos > 0) ? 1 + shift_pos : shift_pos);
        _children.splice(it_pos, _children, position);
    }

    virtual ~Node_box() {}
};

template <typename T>
void transfer_to(Node_box<T>* to, T* obj)
{
    assert_debug(obj->parent(), "obj is an orphan");
    obj->parent()->remove(obj);
    to->add(obj);
}
