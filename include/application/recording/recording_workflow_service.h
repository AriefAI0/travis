#pragma once

#include <QObject>

// Defines the application-layer entry point for future recording workflows.

namespace travis::application::recording {

class RecordingWorkflowService : public QObject {
    Q_OBJECT

public:
    explicit RecordingWorkflowService(QObject* parent = nullptr);
};

} // namespace travis::application::recording
