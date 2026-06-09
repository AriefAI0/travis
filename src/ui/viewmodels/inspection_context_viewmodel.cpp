#include "ui/viewmodels/inspection_context_viewmodel.h"

#include <QVariantMap>

// Keeps inspection workspace project/session selection separate from recording controls.

namespace travis::ui::viewmodels {

namespace {

QVariantMap toProjectVariant(const travis::models::Project& project) {
    return QVariantMap{
        {QStringLiteral("projectId"), project.projectId},
        {QStringLiteral("title"), project.title},
        {QStringLiteral("description"), project.description.value_or(QString{})},
        {QStringLiteral("documentId"), project.documentId.value_or(QString{})},
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

std::optional<QString> optionalTextFromInput(const QString& value) {
    const QString trimmed = value.trimmed();
    if (trimmed.isEmpty()) {
        return std::nullopt;
    }

    return trimmed;
}

} // namespace

InspectionContextViewModel::InspectionContextViewModel(
    travis::services::ProjectService& projectService,
    travis::services::SessionService& sessionService,
    QObject* parent
)
    : QObject(parent)
    , projectService_(projectService)
    , sessionService_(sessionService) {}

qint64 InspectionContextViewModel::projectId() const {
    return projectId_;
}

qint64 InspectionContextViewModel::sessionId() const {
    return sessionId_;
}

QVariantMap InspectionContextViewModel::project() const {
    return project_;
}

QVariantMap InspectionContextViewModel::selectedSession() const {
    return selectedSession_;
}

QVariantList InspectionContextViewModel::sessions() const {
    return sessions_;
}

bool InspectionContextViewModel::loading() const {
    return loading_;
}

QString InspectionContextViewModel::lastError() const {
    return lastError_;
}

QString InspectionContextViewModel::statusMessage() const {
    return statusMessage_;
}

bool InspectionContextViewModel::loadProject(qint64 projectId) {
    if (projectId <= 0) {
        setLastError(QStringLiteral("projectId is required"));
        return false;
    }

    setLoading(true);

    const auto project = projectService_.getProjectById(projectId);
    if (!project.has_value()) {
        projectId_ = 0;
        sessionId_ = 0;
        project_.clear();
        selectedSession_.clear();
        sessions_.clear();
        emit contextChanged();
        emit sessionChanged();
        emit sessionsChanged();
        setLoading(false);
        setLastError(QStringLiteral("Project not found"));
        return false;
    }

    projectId_ = projectId;
    project_ = toProjectVariant(*project);
    sessionId_ = 0;
    selectedSession_.clear();
    emit contextChanged();
    emit sessionChanged();

    const bool loadedSessions = reloadSessions();
    setLoading(false);
    setLastError(loadedSessions ? QString{} : lastError_);
    return loadedSessions;
}

bool InspectionContextViewModel::selectSession(qint64 sessionId) {
    if (sessionId <= 0) {
        setLastError(QStringLiteral("sessionId must be positive"));
        return false;
    }

    const auto session = sessionService_.getSessionById(sessionId);
    if (!session.has_value() || session->projectId != projectId_) {
        setLastError(QStringLiteral("Session does not belong to the selected project"));
        return false;
    }

    sessionId_ = sessionId;
    selectedSession_ = toSessionVariant(*session);
    emit sessionChanged();
    setLastError(QString{});
    return true;
}

bool InspectionContextViewModel::createSession(const QString& name) {
    if (projectId_ <= 0) {
        setLastError(QStringLiteral("Load a project before creating an inspection session"));
        return false;
    }

    setLoading(true);

    const auto session = sessionService_.createSession({
        .projectId = projectId_,
        .name = optionalTextFromInput(name),
    });

    if (!session.has_value()) {
        setLoading(false);
        setLastError(QStringLiteral("Failed to create inspection session"));
        return false;
    }

    sessionId_ = session->sessionId;
    selectedSession_ = toSessionVariant(*session);
    const bool loadedSessions = reloadSessions();
    setLoading(false);
    emit sessionChanged();
    setLastError(loadedSessions ? QString{} : lastError_);
    setStatusMessage(QStringLiteral("Inspection session created"));
    return loadedSessions;
}

void InspectionContextViewModel::setLoading(bool loading) {
    if (loading_ == loading) {
        return;
    }

    loading_ = loading;
    emit loadingChanged();
}

void InspectionContextViewModel::setLastError(const QString& lastError) {
    if (lastError_ == lastError) {
        return;
    }

    lastError_ = lastError;
    emit lastErrorChanged();
}

void InspectionContextViewModel::setStatusMessage(const QString& statusMessage) {
    if (statusMessage_ == statusMessage) {
        return;
    }

    statusMessage_ = statusMessage;
    emit statusMessageChanged();
}

bool InspectionContextViewModel::reloadSessions() {
    if (projectId_ <= 0) {
        setLastError(QStringLiteral("projectId is required"));
        return false;
    }

    QVariantList nextSessions;
    const QVector<travis::models::Session> sessions = sessionService_.listSessionsByProjectId(projectId_);
    nextSessions.reserve(sessions.size());

    for (const auto& session : sessions) {
        nextSessions.append(toSessionVariant(session));
    }

    sessions_ = nextSessions;
    emit sessionsChanged();
    return true;
}

} // namespace travis::ui::viewmodels
