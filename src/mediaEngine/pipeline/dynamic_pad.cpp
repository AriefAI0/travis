#include "mediaEngine/pipeline/dynamic_pad.h"

#include "mediaEngine/pipeline/caps.h"

// Links dynamic pads while preventing duplicate video-pad attachments.

namespace travis::media_engine::pipeline {

bool linkDynamicPadToElement(GstPad* pad, GstElement* element) {
    GstPad* sinkPad = gst_element_get_static_pad(element, "sink");

    if (sinkPad == nullptr) {
        return false;
    }

    bool linked = false;

    if (!gst_pad_is_linked(sinkPad)) {
        linked = gst_pad_link(pad, sinkPad) == GST_PAD_LINK_OK;
    }

    gst_object_unref(sinkPad);
    return linked;
}

bool linkDynamicVideoPadToElement(GstPad* pad, GstElement* element) {
    if (!padHasMediaTypePrefix(pad, "video/")) {
        return false;
    }

    return linkDynamicPadToElement(pad, element);
}

bool linkFirstDynamicVideoPadToElement(
    GstPad* pad,
    GstElement* element,
    bool& linkedVideoPad
) {
    if (linkedVideoPad) {
        return false;
    }

    const bool linked = linkDynamicVideoPadToElement(pad, element);
    if (linked) {
        linkedVideoPad = true;
    }

    return linked;
}

} // namespace travis::media_engine::pipeline
