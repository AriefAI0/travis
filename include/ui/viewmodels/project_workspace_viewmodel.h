#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "services/project_service.h"
#include "services/session_service.h"
#include "services/structure_service.h"
#include "services/video_service.h"

// Exposes selected-project workspace overview data to QML.

namespace travis::ui::viewmodels {

class ProjectWorkspaceViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(qint64 projectId READ projectId NOTIFY projectChanged)
    Q_PROPERTY(QVariantMap project READ project NOTIFY projectChanged)
    Q_PROPERTY(QVariantList structureTree READ structureTree NOTIFY structureTreeChanged)
    Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)
    Q_PROPERTY(QVariantList masterVideos READ masterVideos NOTIFY masterVideosChanged)
    Q_PROPERTY(int assetCount READ assetCount NOTIFY structureTreeChanged)
    Q_PROPERTY(int componentCount READ componentCount NOTIFY structureTreeChanged)
    Q_PROPERTY(int itemCount READ itemCount NOTIFY structureTreeChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
    ProjectWorkspaceViewModel(
        travis::services::ProjectService& projectService,
        travis::services::StructureService& structureService,
        travis::services::SessionService& sessionService,
        travis::services::VideoService& videoService,
        QObject* parent = nullptr
    );

    [[nodiscard]] qint64 projectId() const;
    [[nodiscard]] QVariantMap project() const;
    [[nodiscard]] QVariantList structureTree() const;
    [[nodiscard]] QVariantList sessions() const;
    [[nodiscard]] QVariantList masterVideos() const;
    [[nodiscard]] int assetCount() const;
    [[nodiscard]] int componentCount() const;
    [[nodiscard]] int itemCount() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString lastError() const;

    Q_INVOKABLE bool loadProject(qint64 projectId);
    Q_INVOKABLE bool createAsset(const QString& name);
    Q_INVOKABLE bool createComponent(qint64 assetId, const QString& name);
    Q_INVOKABLE bool createItem(
        qint64 componentId,
        const QString& itemLabel,
        const QString& position,
        int status
    );

signals:
    void projectChanged();
    void structureTreeChanged();
    void sessionsChanged();
    void masterVideosChanged();
    void loadingChanged();
    void lastErrorChanged();

private:
    void setLoading(bool loading);
    void setLastError(const QString& lastError);

    travis::services::ProjectService& projectService_;
    travis::services::StructureService& structureService_;
    travis::services::SessionService& sessionService_;
    travis::services::VideoService& videoService_;
    qint64 projectId_ = 0;
    QVariantMap project_;
    QVariantList structureTree_;
    QVariantList sessions_;
    QVariantList masterVideos_;
    int assetCount_ = 0;
    int componentCount_ = 0;
    int itemCount_ = 0;
    bool loading_ = false;
    QString lastError_;
};

} // namespace travis::ui::viewmodels
