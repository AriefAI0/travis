#pragma once

#include <QObject>
#include <QString>

#include <QPointer>

#include "mediaEngine/core/media_runtime.h"
#include "mediaEngine/preview/preview_engine.h"
#include "mediaEngine/session/media_source_session.h"

class QQuickItem;

// Exposes the native Qt/QML preview controls and status to QML.

namespace travis::ui::controllers {

class PreviewSurfaceController : public QObject {
    Q_OBJECT
    Q_PROPERTY(QObject* previewItem READ previewItem WRITE setPreviewItem NOTIFY previewItemChanged)
    Q_PROPERTY(bool previewActive READ previewActive NOTIFY previewActiveChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    explicit PreviewSurfaceController(QObject* parent = nullptr);

    QObject* previewItem() const;
    void setPreviewItem(QObject* previewItem);

    [[nodiscard]] bool previewActive() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool startDeviceCapturePreview(
        const QString& sourceName,
        const QString& devicePath,
        const QString& sourceElement
    );
    Q_INVOKABLE bool startNdiPreview(const QString& sourceName, const QString& urlAddress);
    Q_INVOKABLE void stopPreview();

signals:
    void previewItemChanged();
    void previewActiveChanged();
    void lastErrorChanged();

private:
    void setLastError(const QString& lastError);

    travis::media_engine::core::MediaRuntime mediaRuntime_;
    travis::media_engine::session::MediaSourceSessionManager sessionManager_;
    travis::media_engine::preview::PreviewEngine previewEngine_;
    QPointer<QObject> previewItem_;
    QString lastError_;
};

} // namespace travis::ui::controllers
