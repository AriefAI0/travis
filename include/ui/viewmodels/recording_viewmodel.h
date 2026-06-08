#pragma once

#include <QObject>

// Exposes future recording state and actions to QML through a thin native viewmodel.

namespace travis::ui::viewmodels {

class RecordingViewModel : public QObject {
    Q_OBJECT

public:
    explicit RecordingViewModel(QObject* parent = nullptr);
};

} // namespace travis::ui::viewmodels
