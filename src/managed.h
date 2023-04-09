#pragma once

template <typename Key>
class Managed
{
public:
    using Index = Key;

private:
    Index _id;

public:
    Managed(const Index& id) noexcept
        : _id(id)
    {}

    inline Index index() const noexcept
    { return _id; }
};