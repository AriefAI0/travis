#include "ui/viewmodels/project_viewmodel.h"

#include <QVariantMap>

#include <exception>

// Converts native project service results into simple QML dashboard state.

namespace travis::ui::viewmodels {

namespace {

std::optional<QString> optionalTextFromInput(const QString& value) {
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty()) {
        return std::nullopt;
    }

    return trimmed;
}

QVariantMap toProjectVariant(const travis::models::Project& project) {
    return QVariantMap{
        {QStringLiteral("projectId"), project.projectId},
        {QStringLiteral("title"), project.title},
        {QStringLiteral("description"), project.description.value_or(QString{})},
        {QStringLiteral("documentId"), project.documentId.value_or(QString{})},
        {QStringLiteral("overallProgress"), 0},
        {QStringLiteral("totalAssets"), 0},
        {QStringLiteral("totalComponents"), 0},
        {QStringLiteral("totalItems"), 0},
        {QStringLiteral("completedItems"), 0},
        {QStringLiteral("pendingItems"), 0},
        {QStringLiteral("createdAt"), project.createdAt},
        {QStringLiteral("updatedAt"), project.updatedAt},
    };
}

} // namespace

ProjectViewModel::ProjectViewModel(
    travis::services::ProjectService& projectService,
    QObject* parent
)
    : QObject(parent)
    , projectService_(projectService) {}

QVariantList ProjectViewModel::projects() const {
    return projects_;
}

bool ProjectViewModel::loading() const {
    return loading_;
}

QString ProjectViewModel::lastError() const {
    return lastError_;
}

QString ProjectViewModel::statusMessage() const {
    return statusMessage_;
}

bool ProjectViewModel::refreshProjects() {
    setLoading(true);

    QVariantList nextProjects;
    const QVector<travis::models::Project> projects = projectService_.listProjects();
    nextProjects.reserve(projects.size());

    for (const auto& project : projects) {
        nextProjects.append(toProjectVariant(project));
    }

    projects_ = nextProjects;
    emit projectsChanged();
    setLoading(false);
    setLastError(QString{});
    return true;
}

bool ProjectViewModel::createProject(
    const QString& title,
    const QString& description,
    const QString& documentId
) {
    setLoading(true);

    try {
        const auto project = projectService_.createProject({
            .title = title,
            .description = optionalTextFromInput(description),
            .documentId = optionalTextFromInput(documentId),
        });

        setLoading(false);

        if (!project.has_value()) {
            setLastError(QStringLiteral("Failed to create project"));
            return false;
        }

        setStatusMessage(QStringLiteral("Project created"));
        return refreshProjects();
    } catch (const std::exception& exception) {
        setLoading(false);
        setLastError(QString::fromUtf8(exception.what()));
        return false;
    }
}

void ProjectViewModel::setLoading(bool loading) {
    if (loading_ == loading) {
        return;
    }

    loading_ = loading;
    emit loadingChanged();
}

void ProjectViewModel::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

void ProjectViewModel::setStatusMessage(const QString& statusMessage) {
    if (statusMessage_ == statusMessage) {
        return;
    }

    statusMessage_ = statusMessage;
    emit statusMessageChanged();
}

} // namespace travis::ui::viewmodels
