#pragma once
/**
 * Implements X server connection
 * We make it into one class and hide the implementation details later
 * The class itself can be converted into xcb_connection_t*
*/

typedef unsigned int xcb_timestamp_t;
struct xcb_connection_t;
struct xcb_screen_t;

// Represent X Timestamp
// We combine it in connection header as it only contain small code
class Timestamp
{
public:
    operator xcb_timestamp_t() const
    { return _timestamp; }

    void update(const xcb_timestamp_t* t)
    { _timestamp = *t; }

private:
    xcb_timestamp_t _timestamp;
    Timestamp();
    friend class Connection;
};

class Connection
{
public:
    Timestamp timestamp;

    // It's init(), not instance()
    // So if it's called twice, it does funny stuff.
    static Connection& init();

    operator xcb_connection_t*() const
    { return _conn; }

    const xcb_screen_t* screen() const
    { return _screen; }

    int scr_id() const
    { return _scr_id; } 

    Connection(const Connection& conn)            = delete;
    Connection(Connection&& conn)                 = delete;
    Connection& operator=(const Connection& conn) = delete;
    Connection& operator=(Connection&& conn)      = delete;
    ~Connection();

private:
    int _scr_id;

    xcb_connection_t* _conn;
    xcb_screen_t*     _screen;

    Connection();
};
