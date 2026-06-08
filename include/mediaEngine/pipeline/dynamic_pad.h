#pragma once

#include <gst/gst.h>

// Handles dynamic pad linking for decodebin- and demux-style elements.

namespace travis::media_engine::pipeline {

[[nodiscard]] bool linkDynamicPadToElement(GstPad* pad, GstElement* element);
[[nodiscard]] bool linkDynamicVideoPadToElement(GstPad* pad, GstElement* element);
[[nodiscard]] bool linkFirstDynamicVideoPadToElement(
    GstPad* pad,
    GstElement* element,
    bool& linkedVideoPad
);

} // namespace travis::media_engine::pipeline
