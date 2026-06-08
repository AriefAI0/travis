#include "mediaEngine/sources/ndi/ndi_video_source.h"

#include "mediaEngine/pipeline/dynamic_pad.h"
#include "mediaEngine/sources/ndi/ndi_source_reference.h"

// Builds the NDI source pipeline and forwards its decoded video pad into a queue.

namespace travis::media_engine::sources::ndi {

namespace {

struct NdiVideoSourcePadLinkContext {
    GstElement* queue = nullptr;
    bool* linkedVideoPad = nullptr;
};

struct NdiDemuxPadLinkContext {
    GstElement* decoder = nullptr;
    GstElement* discardQueue = nullptr;
};

void freeNdiVideoSourcePadLinkContext(gpointer data, GClosure*) {
    delete static_cast<NdiVideoSourcePadLinkContext*>(data);
}

void freeNdiDemuxPadLinkContext(gpointer data, GClosure*) {
    delete static_cast<NdiDemuxPadLinkContext*>(data);
}

void onNdiDemuxPadAdded(GstElement*, GstPad* pad, gpointer userData) {
    auto* context = static_cast<NdiDemuxPadLinkContext*>(userData);

    if (pipeline::linkDynamicVideoPadToElement(pad, context->decoder)) {
        return;
    }

    pipeline::linkDynamicPadToElement(pad, context->discardQueue);
}

void onNdiDecoderPadAdded(GstElement*, GstPad* pad, gpointer userData) {
    auto* context = static_cast<NdiVideoSourcePadLinkContext*>(userData);
    pipeline::linkFirstDynamicVideoPadToElement(pad, context->queue, *context->linkedVideoPad);
}

} // namespace

NdiVideoSource createNdiVideoSource(
    const std::string& sourceName,
    const std::string& urlAddress
) {
    NdiVideoSource source{
        gst_element_factory_make("ndisrc", nullptr),
        gst_element_factory_make("ndisrcdemux", nullptr),
        gst_element_factory_make("decodebin", nullptr),
        gst_element_factory_make("queue", nullptr),
        gst_element_factory_make("fakesink", nullptr),
    };

    if (!isNdiVideoSourceValid(source)) {
        return source;
    }

    const auto sourceReference = parseNdiSourceReference(sourceName);
    g_object_set(
        source.source,
        "ndi-name",
        sourceReference.name.c_str(),
        "connect-timeout",
        30000,
        "timeout",
        30000,
        nullptr
    );

    const std::string* effectiveUrl = !urlAddress.empty() ? &urlAddress : nullptr;
    if (effectiveUrl == nullptr && sourceReference.urlAddress.has_value()) {
        effectiveUrl = &*sourceReference.urlAddress;
    }

    if (effectiveUrl != nullptr) {
        g_object_set(source.source, "url-address", effectiveUrl->c_str(), nullptr);
    }

    return source;
}

bool isNdiVideoSourceValid(const NdiVideoSource& source) {
    return source.source != nullptr &&
           source.demux != nullptr &&
           source.decoder != nullptr &&
           source.discardQueue != nullptr &&
           source.discardSink != nullptr;
}

void addNdiVideoSourceToBin(GstBin* bin, const NdiVideoSource& source) {
    gst_bin_add_many(
        bin,
        source.source,
        source.demux,
        source.decoder,
        source.discardQueue,
        source.discardSink,
        nullptr
    );
}

bool linkNdiVideoSource(const NdiVideoSource& source) {
    return gst_element_link(source.source, source.demux) &&
           gst_element_link(source.discardQueue, source.discardSink);
}

void connectNdiVideoSourceToQueue(
    const NdiVideoSource& source,
    GstElement* queue,
    bool& linkedVideoPad
) {
    auto* demuxContext = new NdiDemuxPadLinkContext{source.decoder, source.discardQueue};
    auto* decoderContext = new NdiVideoSourcePadLinkContext{queue, &linkedVideoPad};

    g_signal_connect_data(
        source.demux,
        "pad-added",
        G_CALLBACK(onNdiDemuxPadAdded),
        demuxContext,
        freeNdiDemuxPadLinkContext,
        static_cast<GConnectFlags>(0)
    );
    g_signal_connect_data(
        source.decoder,
        "pad-added",
        G_CALLBACK(onNdiDecoderPadAdded),
        decoderContext,
        freeNdiVideoSourcePadLinkContext,
        static_cast<GConnectFlags>(0)
    );
}

} // namespace travis::media_engine::sources::ndi
