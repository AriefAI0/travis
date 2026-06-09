#include "ui/viewmodels/project_workspace_viewmodel.h"

#include <QVariantMap>

// Builds a QML-friendly project workspace snapshot from native services.

namespace travis::ui::viewmodels {

namespace {

QVariantMap toProjectVariant(const travis::models::Project& project) {
    return QVariantMap{
        {QStringLiteral("projectId"), project.projectId},
        {QStringLiteral("title"), project.title},
        {QStringLiteral("description"), project.description.value_or(QString{})},
        {QStringLiteral("documentId"), project.documentId.value_or(QString{})},
        {QStringLiteral("createdAt"), project.createdAt},
        {QStringLiteral("updatedAt"), project.updatedAt},
    };
}

QVariantMap toItemVariant(const travis::services::StructureItemNode& item) {
    return QVariantMap{
        {QStringLiteral("itemId"), item.itemId},
        {QStringLiteral("componentId"), item.componentId},
        {QStringLiteral("itemLabel"), item.itemLabel},
        {QStringLiteral("position"), item.position.value_or(QString{})},
        {QStringLiteral("status"), item.status.has_value() ? QVariant(*item.status) : QVariant{}},
    };
}

QVariantMap toComponentVariant(const travis::services::StructureComponentNode& component) {
    QVariantList items;
    items.reserve(component.items.size());

    for (const auto& item : component.items) {
        items.append(toItemVariant(item));
    }

    return QVariantMap{
        {QStringLiteral("componentId"), component.componentId},
        {QStringLiteral("assetId"), component.assetId},
        {QStringLiteral("name"), component.name},
        {QStringLiteral("items"), items},
    };
}

QVariantMap toAssetVariant(const travis::services::StructureAssetNode& asset) {
    QVariantList components;
    components.reserve(asset.components.size());

    for (const auto& component : asset.components) {
        components.append(toComponentVariant(component));
    }

    return QVariantMap{
        {QStringLiteral("assetId"), asset.assetId},
        {QStringLiteral("projectId"), asset.projectId},
        {QStringLiteral("name"), asset.name},
        {QStringLiteral("components"), components},
    };
}

QVariantMap toSessionVariant(const travis::models::Session& session) {
    return QVariantMap{
        {QStringLiteral("sessionId"), session.sessionId},
        {QStringLiteral("projectId"), session.projectId},
        {QStringLiteral("name"), session.name.value_or(QStringLiteral("Untitled session"))},
        {QStringLiteral("createdAt"), session.createdAt},
        {QStringLiteral("updatedAt"), session.updatedAt},
    };
}

} // namespace

ProjectWorkspaceViewModel::ProjectWorkspaceViewModel(
    travis::services::ProjectService& projectService,
    travis::services::StructureService& structureService,
    travis::services::SessionService& sessionService,
    QObject* parent
)
    : QObject(parent)
    , projectService_(projectService)
    , structureService_(structureService)
    , sessionService_(sessionService) {}

qint64 ProjectWorkspaceViewModel::projectId() const {
    return projectId_;
}

QVariantMap ProjectWorkspaceViewModel::project() const {
    return project_;
}

QVariantList ProjectWorkspaceViewModel::structureTree() const {
    return structureTree_;
}

QVariantList ProjectWorkspaceViewModel::sessions() const {
    return sessions_;
}

int ProjectWorkspaceViewModel::assetCount() const {
    return assetCount_;
}

int ProjectWorkspaceViewModel::componentCount() const {
    return componentCount_;
}

int ProjectWorkspaceViewModel::itemCount() const {
    return itemCount_;
}

bool ProjectWorkspaceViewModel::loading() const {
    return loading_;
}

QString ProjectWorkspaceViewModel::lastError() const {
    return lastError_;
}

bool ProjectWorkspaceViewModel::loadProject(qint64 projectId) {
    if (projectId <= 0) {
        setLastError(QStringLiteral("projectId is required"));
        return false;
    }

    setLoading(true);

    const auto project = projectService_.getProjectById(projectId);
    if (!project.has_value()) {
        projectId_ = 0;
        project_.clear();
        structureTree_.clear();
        sessions_.clear();
        assetCount_ = 0;
        componentCount_ = 0;
        itemCount_ = 0;
        emit projectChanged();
        emit structureTreeChanged();
        emit sessionsChanged();
        setLoading(false);
        setLastError(QStringLiteral("Project not found"));
        return false;
    }

    QVariantList nextStructureTree;
    const QVector<travis::services::StructureAssetNode> structureTree =
        structureService_.listProjectStructureTree(projectId);
    nextStructureTree.reserve(structureTree.size());

    int nextComponentCount = 0;
    int nextItemCount = 0;
    for (const auto& asset : structureTree) {
        nextStructureTree.append(toAssetVariant(asset));
        nextComponentCount += asset.components.size();
        for (const auto& component : asset.components) {
            nextItemCount += component.items.size();
        }
    }

    QVariantList nextSessions;
    const QVector<travis::models::Session> sessions = sessionService_.listSessionsByProjectId(projectId);
    nextSessions.reserve(sessions.size());

    for (const auto& session : sessions) {
        nextSessions.append(toSessionVariant(session));
    }

    projectId_ = projectId;
    project_ = toProjectVariant(*project);
    structureTree_ = nextStructureTree;
    sessions_ = nextSessions;
    assetCount_ = structureTree.size();
    componentCount_ = nextComponentCount;
    itemCount_ = nextItemCount;

    emit projectChanged();
    emit structureTreeChanged();
    emit sessionsChanged();
    setLoading(false);
    setLastError(QString{});
    return true;
}

void ProjectWorkspaceViewModel::setLoading(bool loading) {
    if (loading_ == loading) {
        return;
    }

    loading_ = loading;
    emit loadingChanged();
}

void ProjectWorkspaceViewModel::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

} // namespace travis::ui::viewmodels
