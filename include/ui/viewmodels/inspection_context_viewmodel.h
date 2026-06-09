#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include "services/project_service.h"
#include "services/session_service.h"
#include "services/structure_service.h"

// Exposes selected project/session context for the inspection workspace.

namespace travis::ui::viewmodels {

class InspectionContextViewModel : public QObject {
    Q_OBJECT
    Q_PROPERTY(qint64 projectId READ projectId NOTIFY contextChanged)
    Q_PROPERTY(qint64 sessionId READ sessionId NOTIFY sessionChanged)
    Q_PROPERTY(QVariantMap project READ project NOTIFY contextChanged)
    Q_PROPERTY(QVariantMap selectedSession READ selectedSession NOTIFY sessionChanged)
    Q_PROPERTY(qint64 selectedItemId READ selectedItemId NOTIFY selectedItemChanged)
    Q_PROPERTY(QVariantMap selectedItem READ selectedItem NOTIFY selectedItemChanged)
    Q_PROPERTY(QVariantList sessions READ sessions NOTIFY sessionsChanged)
    Q_PROPERTY(QVariantList inspectionItems READ inspectionItems NOTIFY inspectionItemsChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    InspectionContextViewModel(
        travis::services::ProjectService& projectService,
        travis::services::SessionService& sessionService,
        travis::services::StructureService& structureService,
        QObject* parent = nullptr
    );

    [[nodiscard]] qint64 projectId() const;
    [[nodiscard]] qint64 sessionId() const;
    [[nodiscard]] QVariantMap project() const;
    [[nodiscard]] QVariantMap selectedSession() const;
    [[nodiscard]] qint64 selectedItemId() const;
    [[nodiscard]] QVariantMap selectedItem() const;
    [[nodiscard]] QVariantList sessions() const;
    [[nodiscard]] QVariantList inspectionItems() const;
    [[nodiscard]] bool loading() const;
    [[nodiscard]] QString lastError() const;
    [[nodiscard]] QString statusMessage() const;

    Q_INVOKABLE bool loadProject(qint64 projectId);
    Q_INVOKABLE bool selectSession(qint64 sessionId);
    Q_INVOKABLE bool selectItem(qint64 itemId);
    Q_INVOKABLE bool createSession(const QString& name);

signals:
    void contextChanged();
    void sessionChanged();
    void selectedItemChanged();
    void sessionsChanged();
    void inspectionItemsChanged();
    void loadingChanged();
    void lastErrorChanged();
    void statusMessageChanged();

private:
    void setLoading(bool loading);
    void setLastError(const QString& lastError);
    void setStatusMessage(const QString& statusMessage);
    bool reloadSessions();
    bool reloadInspectionItems();

    travis::services::ProjectService& projectService_;
    travis::services::SessionService& sessionService_;
    travis::services::StructureService& structureService_;
    qint64 projectId_ = 0;
    qint64 sessionId_ = 0;
    qint64 selectedItemId_ = 0;
    QVariantMap project_;
    QVariantMap selectedSession_;
    QVariantMap selectedItem_;
    QVariantList sessions_;
    QVariantList inspectionItems_;
    bool loading_ = false;
    QString lastError_;
    QString statusMessage_;
};

} // namespace travis::ui::viewmodels
