#include "ui/controllers/preview_surface_controller.h"

#include <QQuickItem>

// Keeps the first preview surface bridge thin while validating the native Qt/QML preview path.

namespace travis::ui::controllers {

PreviewSurfaceController::PreviewSurfaceController(QObject* parent)
    : QObject(parent)
    , mediaRuntime_()
    , previewEngine_(sessionManager_) {
    const auto healthCheck = mediaRuntime_.healthCheck();
    if (!healthCheck.ok) {
        setLastError(
            QStringLiteral("Missing GStreamer plugins: %1").arg(healthCheck.missingPlugins.join(QStringLiteral(", ")))
        );
    }
}

QObject* PreviewSurfaceController::previewItem() const {
    return previewItem_;
}

void PreviewSurfaceController::setPreviewItem(QObject* previewItem) {
    if (previewItem_ == previewItem) {
        return;
    }

    previewItem_ = previewItem;
    emit previewItemChanged();
}

bool PreviewSurfaceController::previewActive() const {
    return previewEngine_.hasActivePreview();
}

QString PreviewSurfaceController::lastError() const {
    return lastError_;
}

bool PreviewSurfaceController::startDeviceCapturePreview(
    const QString& sourceName,
    const QString& devicePath,
    const QString& sourceElement
) {
    auto* quickItem = qobject_cast<QQuickItem*>(previewItem_.data());
    if (quickItem == nullptr) {
        setLastError(QStringLiteral("previewItem must be a QQuickItem"));
        return false;
    }

    const auto result = previewEngine_.startPreview(
        "device-capture",
        sourceName.toStdString(),
        std::string{},
        devicePath.toStdString(),
        sourceElement.toStdString(),
        quickItem
    );

    if (!result.ok) {
        setLastError(QString::fromStdString(result.message));
        return false;
    }

    setLastError(QString{});
    emit previewActiveChanged();
    return true;
}

bool PreviewSurfaceController::startNdiPreview(
    const QString& sourceName,
    const QString& urlAddress
) {
    auto* quickItem = qobject_cast<QQuickItem*>(previewItem_.data());
    if (quickItem == nullptr) {
        setLastError(QStringLiteral("previewItem must be a QQuickItem"));
        return false;
    }

    const auto result = previewEngine_.startPreview(
        "ndi",
        sourceName.toStdString(),
        urlAddress.toStdString(),
        std::string{},
        std::string{},
        quickItem
    );

    if (!result.ok) {
        setLastError(QString::fromStdString(result.message));
        return false;
    }

    setLastError(QString{});
    emit previewActiveChanged();
    return true;
}

void PreviewSurfaceController::stopPreview() {
    const auto result = previewEngine_.stopPreview();
    if (!result.ok) {
        setLastError(QString::fromStdString(result.message));
    } else {
        setLastError(QString{});
    }

    emit previewActiveChanged();
}

void PreviewSurfaceController::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

} // namespace travis::ui::controllers
