#pragma once

template <typename I>
class Managed
{
public:
    using Index = I;

private:
    Index _id;

public:
    Managed(const Index& id) noexcept
        : _id(id)
    {}

    inline Index index() const noexcept
    { return _id; }
};