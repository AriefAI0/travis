#include "mediaEngine/core/native_video_overlay.h"

#include <QPointF>
#include <QQuickItem>
#include <QQuickWindow>
#include <gst/video/videooverlay.h>

// Keeps native video sinks aligned with the QML item that represents their surface.

namespace travis::media_engine::core {

NativeVideoOverlayResult syncNativeVideoOverlayGeometry(
    GstElement* sink,
    QQuickItem* targetItem
) {
    if (sink == nullptr) {
        return NativeVideoOverlayResult{false, "Native video overlay sink is required"};
    }

    if (targetItem == nullptr) {
        return NativeVideoOverlayResult{false, "Native video overlay target item is required"};
    }

    QQuickWindow* targetWindow = targetItem->window();
    if (targetWindow == nullptr) {
        return NativeVideoOverlayResult{false, "Native video overlay target item is not attached to a window"};
    }

    if (targetItem->width() <= 0 || targetItem->height() <= 0) {
        return NativeVideoOverlayResult{false, "Native video overlay target item has invalid geometry"};
    }

    auto* overlay = GST_VIDEO_OVERLAY(sink);
    gst_video_overlay_set_window_handle(
        overlay,
        static_cast<guintptr>(targetWindow->winId())
    );

    const QPointF scenePosition = targetItem->mapToScene(QPointF(0, 0));
    const qreal devicePixelRatio = targetWindow->devicePixelRatio();

    gst_video_overlay_set_render_rectangle(
        overlay,
        static_cast<gint>(scenePosition.x() * devicePixelRatio),
        static_cast<gint>(scenePosition.y() * devicePixelRatio),
        static_cast<gint>(targetItem->width() * devicePixelRatio),
        static_cast<gint>(targetItem->height() * devicePixelRatio)
    );
    gst_video_overlay_expose(overlay);

    return NativeVideoOverlayResult{true, "Native video overlay geometry synced"};
}

} // namespace travis::media_engine::core
