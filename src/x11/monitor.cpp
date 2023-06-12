#include "monitor.h"
#include "atom.h"
#include "extension.h"
#include "x11.h"
#include "../config.h"
#include "../state.h"
#include "../logger.h"
#include "../helper/memory.h"

#include <xcb/randr.h>
#include <span>

namespace X11::monitor {

struct XRandR_output
{
    // XRandR names of the outuput
    std::string name;
    // Size in millimeters
    uint32_t mm_width;
    uint32_t mm_height;
    std::vector<xcb_randr_output_t> outputs;
};

static void _load_all_xinerama(State& state)
{

}

#ifdef XCB_RANDR_GET_MONITORS
static auto _get_randr_monitor_outputs(xcb_randr_monitor_info_t& data) -> XRandR_output
{
    XRandR_output rnr_output;
    rnr_output.mm_width  = data.width_in_millimeters;
    rnr_output.mm_height = data.height_in_millimeters;
    rnr_output.name      = [&] () -> std::string {
        try {
            return atom::name(data.name);
        } catch (const std::runtime_error&) {
            return "unknown";
        }
    }();
    auto*  arr_ptr     = xcb_randr_monitor_info_outputs(&data);
    size_t arr_len     = xcb_randr_monitor_info_outputs_length(&data);
    rnr_output.outputs = std::vector<xcb_randr_output_t>(arr_ptr, arr_ptr + arr_len);
    return rnr_output;
}

static void _load_all_xrandr_monitors(State& state)
{
    auto monitors = memory::c_own(xcb_randr_get_monitors_reply(
            state.conn(),
            xcb_randr_get_monitors(state.conn(), X11::root_window_id(state.conn()), true),
            nullptr));
    if (!monitors) {
        logger::error("_load_all_xrandr_monitors -> Get monitor fail");
        return;
    }
    int i = 0;
    for (auto monitor_iter = xcb_randr_get_monitors_monitors_iterator(monitors.get());
         monitor_iter.rem; xcb_randr_monitor_info_next(&monitor_iter)) {
        const auto output = _get_randr_monitor_outputs(*monitor_iter.data);

        Monitor& mon = state.monitors().manage(i, output.name);
        mon.rect({
            {monitor_iter.data->x, monitor_iter.data->y},
            {monitor_iter.data->width, monitor_iter.data->height}
        });
        logger::debug("XRandR monitor -> iteration: {}, name: {}", i, output.name);

        ++i;
    }
}
#else
static void _load_all_xrandr_monitors(State&) {}
#endif

static auto _get_randr_crtc_outputs(xcb_randr_get_crtc_info_reply_t& crtc_info) -> std::vector<XRandR_output>
{
    const auto outputs = std::span<xcb_randr_output_t>{
        xcb_randr_get_crtc_info_outputs(&crtc_info),
        static_cast<size_t>(xcb_randr_get_crtc_info_outputs_length(&crtc_info))
    };
    std::vector<XRandR_output> rnr_outputs;
    for (const auto& output : outputs) {
        auto output_info = memory::c_own(xcb_randr_get_output_info_reply(
                X11::detail::conn(),
                xcb_randr_get_output_info(X11::detail::conn(), output, Timestamp::get()),
                nullptr));
        if (!output_info) {
            logger::error("Get output info failed, this should have never happened");
            continue;
        }

        XRandR_output rnr_output;
        rnr_output.name      = (char*)xcb_randr_get_output_info_name(output_info.get());
        rnr_output.mm_width  = output_info->mm_width;
        rnr_output.mm_height = output_info->mm_height;
        rnr_output.outputs.push_back(output);
        rnr_outputs.push_back(rnr_output);
    }
    return rnr_outputs;
}

static void _load_all_xrandr_crtcs(State& state)
{
    auto screen_res = memory::c_own(xcb_randr_get_screen_resources_reply(
            state.conn(),
            xcb_randr_get_screen_resources(state.conn(), X11::root_window_id(state.conn())),
            nullptr));
    if (!screen_res) {
        logger::error("_load_all_xrandr_crtcs -> Get screen resources fail");
        return;
    }

    const auto crtcs = std::span<xcb_randr_crtc_t>{
        xcb_randr_get_screen_resources_crtcs(screen_res.get()),
        screen_res->num_crtcs
    };
    int i = 0;
    for(const auto& crtc : crtcs) {
        auto crtc_info = memory::c_own(xcb_randr_get_crtc_info_reply(
                state.conn(),
                xcb_randr_get_crtc_info(state.conn(), crtc, Timestamp::get()),
                nullptr));
        if (!crtc_info) {
            logger::error("Get crtc info failed, this should have never happened");
            continue;
        }
        if (!xcb_randr_get_crtc_info_outputs_length(crtc_info.get())) {
            continue;
        }

        const auto outputs = _get_randr_crtc_outputs(*crtc_info);
        const auto it = std::ranges::find_if(outputs, [] (const auto& output) {
            return !output.name.empty();
        });

        Monitor& mon = state.monitors().manage(i, (it != outputs.cend()) ? it->name : "unknown");
        mon.rect({
            {crtc_info->x, crtc_info->y},
            {crtc_info->width, crtc_info->height}
        });

        ++i;
    }
}

static void _load_all_xrandr(State& state)
{
    xcb_randr_select_input(state.conn(), X11::root_window_id(state.conn()), XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE);
    if (extension::xrandr().have_randr_15) {
        logger::debug("_load_all_xrandr -> load monitors");
        _load_all_xrandr_monitors(state);
    } else {
        logger::debug("_load_all_xrandr -> load crtcs");
        _load_all_xrandr_crtcs(state);
    }
    if (state.monitors().empty()) {
        xcb_randr_select_input(state.conn(), X11::root_window_id(state.conn()), 0);
    }
}

void load_all(State& state)
{
    if (config::enable_randr && extension::xrandr().is_supported) {
        logger::debug("Using XRandR to scan monitors");
        _load_all_xrandr(state);
    } else if (config::enable_xinerama) {
        logger::debug("Using Xinerama to scan monitors");
        _load_all_xinerama(state);
    }

    if (state.monitors().empty()) {
        logger::debug("Monitor is still empty, use default screen configuration");
        // update default monitor.
        Monitor& mon = state.monitors().manage(0, "default");
        const xcb_screen_t* xscreen = state.conn().xscreen();
        mon.rect({
            {0, 0},
            {xscreen->width_in_pixels, xscreen->height_in_pixels}
        });
    }
}

} // namespace X11::monitor