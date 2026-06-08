#include "mediaEngine/pipeline/caps.h"

// Reads the current or queryable caps to determine whether a pad carries a media type family.

namespace travis::media_engine::pipeline {

bool padHasMediaTypePrefix(GstPad* pad, const char* mediaTypePrefix) {
    GstCaps* caps = gst_pad_get_current_caps(pad);

    if (caps == nullptr) {
        caps = gst_pad_query_caps(pad, nullptr);
    }

    if (caps == nullptr) {
        return false;
    }

    const GstStructure* structure = gst_caps_get_structure(caps, 0);
    const gchar* mediaType = gst_structure_get_name(structure);
    const bool matches = g_str_has_prefix(mediaType, mediaTypePrefix);

    gst_caps_unref(caps);
    return matches;
}

} // namespace travis::media_engine::pipeline
