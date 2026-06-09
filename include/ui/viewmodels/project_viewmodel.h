#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include "services/project_service.h"

// Exposes project dashboard data and actions to QML without leaking repository access.

namespace travis::ui::viewmodels {

class ProjectViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList projects READ projects NOTIFY projectsChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    explicit ProjectViewModel(
        travis::services::ProjectService& projectService,
        QObject* parent = nullptr
    );

    [[nodiscard]] QVariantList projects() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString lastError() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool refreshProjects();
    Q_INVOKABLE bool createProject(
        const QString& title,
        const QString& description,
        const QString& documentId
    );

signals:
    void projectsChanged();
    void loadingChanged();
    void lastErrorChanged();
    void statusMessageChanged();

private:
    void setLoading(bool loading);
    void setLastError(const QString& lastError);
    void setStatusMessage(const QString& statusMessage);

    travis::services::ProjectService& projectService_;
    QVariantList projects_;
    bool loading_ = false;
    QString lastError_;
    QString statusMessage_;
};

} // namespace travis::ui::viewmodels
