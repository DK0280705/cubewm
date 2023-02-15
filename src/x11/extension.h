namespace X11 {
namespace extension {

class Extension {
    int  _base;
    bool _supported;
public:
    Extension(int base, bool supported) noexcept
        : _base(base)
        , _supported(supported)
    {}

    inline int event_base() const
    { return _base; }

    bool supported() const
    { return _supported; }
};

extern Extension xkb;
extern Extension xrandr;
extern Extension xshape;

void init();

}
}
