#include "mediaEngine/discovery/source_discovery.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>

// Uses gst-device-monitor so device discovery follows the same runtime as the media pipelines.

namespace travis::media_engine::discovery {

namespace {

constexpr int kDeviceMonitorTimeoutMs = 30000;

QString gstreamerRuntimeRoot() {
    const QString explicitRuntimeRoot = qEnvironmentVariable("TRAVIS_GSTREAMER_RUNTIME_ROOT");
    if (!explicitRuntimeRoot.trimmed().isEmpty()) {
        return explicitRuntimeRoot;
    }

    QDir appDir(QCoreApplication::applicationDirPath());

    for (int depth = 0; depth < 4; ++depth) {
        const QString runtimeRoot = appDir.filePath(QStringLiteral("runtime/gstreamer/1.0/msvc_x86_64"));
        if (QFileInfo::exists(QDir(runtimeRoot).filePath(QStringLiteral("bin/gst-device-monitor-1.0.exe")))) {
            return runtimeRoot;
        }

        if (!appDir.cdUp()) {
            break;
        }
    }

    return QStringLiteral("runtime/gstreamer/1.0/msvc_x86_64");
}

QString parseProperty(const QString& block, const QString& propertyName) {
    const QRegularExpression expression(
        QStringLiteral("^\\s*%1\\s*=\\s*(.+)$").arg(QRegularExpression::escape(propertyName)),
        QRegularExpression::MultilineOption
    );
    const QRegularExpressionMatch match = expression.match(block);
    return match.hasMatch() ? match.captured(1).trimmed() : QString{};
}

QString parseSourceElement(const QString& block) {
    const QRegularExpression expression(QStringLiteral("gst-launch-1\\.0\\s+([a-zA-Z0-9_-]+)\\s"));
    const QRegularExpressionMatch match = expression.match(block);
    return match.hasMatch() ? match.captured(1).trimmed() : QString{};
}

QString parseDeviceName(const QString& block) {
    const QString propertyName = parseProperty(block, QStringLiteral("device.name"));
    if (!propertyName.isEmpty()) {
        return propertyName;
    }

    const QRegularExpression expression(QStringLiteral("^\\s*name\\s+:\\s*(.+)$"), QRegularExpression::MultilineOption);
    const QRegularExpressionMatch match = expression.match(block);
    return match.hasMatch() ? match.captured(1).trimmed() : QString{};
}

QString stableSourceId(const QString& sourceElement, const QString& devicePath, const QString& name) {
    const QString sourceReference = devicePath.trimmed().isEmpty() ? name : devicePath;
    return QStringLiteral("%1:%2").arg(sourceElement.isEmpty() ? QStringLiteral("unknown") : sourceElement, sourceReference).toLower();
}

QVector<QString> splitDeviceBlocks(const QString& output) {
    QVector<QString> blocks;
    const QStringList parts = output.split(QRegularExpression(QStringLiteral("\\r?\\n\\s*\\r?\\nDevice found:\\r?\\n")));

    for (const QString& part : parts) {
        if (part.contains(QStringLiteral("class :"))) {
            blocks.append(part);
        }
    }

    return blocks;
}

QVector<DiscoveredMediaSource> parseDeviceCaptureSources(const QString& output) {
    QVector<DiscoveredMediaSource> sources;
    QSet<QString> seenIds;

    for (const QString& block : splitDeviceBlocks(output)) {
        const QString name = parseDeviceName(block);
        const QString sourceElement = parseSourceElement(block);
        const QString devicePath = parseProperty(block, QStringLiteral("device.path"));

        if (name.isEmpty() || sourceElement != QStringLiteral("mfvideosrc")) {
            continue;
        }

        const QString id = stableSourceId(sourceElement, devicePath, name);
        if (seenIds.contains(id)) {
            continue;
        }

        seenIds.insert(id);
        sources.append(DiscoveredMediaSource{
            .id = id,
            .kind = QStringLiteral("device-capture"),
            .name = name,
            .devicePath = devicePath,
            .sourceElement = sourceElement,
        });
    }

    std::sort(sources.begin(), sources.end(), [](const auto& left, const auto& right) {
        return QString::localeAwareCompare(left.name, right.name) < 0;
    });
    return sources;
}

QVector<DiscoveredMediaSource> parseAudioCaptureSources(const QString& output) {
    QVector<DiscoveredMediaSource> sources;
    QSet<QString> seenIds;
    const QSet<QString> supportedElements = {
        QStringLiteral("wasapi2src"),
        QStringLiteral("wasapisrc"),
    };

    for (const QString& block : splitDeviceBlocks(output)) {
        if (!block.contains(QStringLiteral("class : Audio/Source"))) {
            continue;
        }

        const QString name = parseDeviceName(block);
        const QString sourceElement = parseSourceElement(block);
        const QString devicePath = !parseProperty(block, QStringLiteral("device.id")).isEmpty()
            ? parseProperty(block, QStringLiteral("device.id"))
            : parseProperty(block, QStringLiteral("device.path"));
        const bool loopback = parseProperty(block, QStringLiteral("wasapi2.device.loopback")).startsWith(QStringLiteral("true"));

        if (name.isEmpty() || !supportedElements.contains(sourceElement) || loopback) {
            continue;
        }

        const QString id = stableSourceId(sourceElement, devicePath, name);
        if (seenIds.contains(id)) {
            continue;
        }

        seenIds.insert(id);
        sources.append(DiscoveredMediaSource{
            .id = id,
            .kind = QStringLiteral("audio-capture"),
            .name = name,
            .devicePath = devicePath,
            .sourceElement = sourceElement,
        });
    }

    std::sort(sources.begin(), sources.end(), [](const auto& left, const auto& right) {
        return QString::localeAwareCompare(left.name, right.name) < 0;
    });
    return sources;
}

} // namespace

SourceDiscoveryResult SourceDiscovery::listDeviceCaptureSources() const {
    auto result = runDeviceMonitor({QStringLiteral("Video/Source")});
    if (!result.ok) {
        return result;
    }

    result.sources = parseDeviceCaptureSources(result.message);
    result.message = QStringLiteral("Device capture discovery completed");
    return result;
}

SourceDiscoveryResult SourceDiscovery::listAudioCaptureSources() const {
    auto result = runDeviceMonitor({QStringLiteral("Audio")});
    if (!result.ok) {
        return result;
    }

    result.sources = parseAudioCaptureSources(result.message);
    result.message = QStringLiteral("Audio capture discovery completed");
    return result;
}

SourceDiscoveryResult SourceDiscovery::runDeviceMonitor(const QStringList& classes) const {
    const QString runtimeRoot = gstreamerRuntimeRoot();
    const QDir runtimeDir(runtimeRoot);
    const QString executablePath = runtimeDir.filePath(QStringLiteral("bin/gst-device-monitor-1.0.exe"));

    if (!QFileInfo::exists(executablePath)) {
        return SourceDiscoveryResult{
            .ok = false,
            .message = QStringLiteral("gst-device-monitor-1.0.exe was not found in bundled GStreamer runtime"),
        };
    }

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(
        QStringLiteral("PATH"),
        runtimeDir.filePath(QStringLiteral("bin")) + QStringLiteral(";") + environment.value(QStringLiteral("PATH"))
    );
    environment.insert(
        QStringLiteral("GST_PLUGIN_PATH_1_0"),
        runtimeDir.filePath(QStringLiteral("lib/gstreamer-1.0"))
    );
    environment.insert(
        QStringLiteral("GST_PLUGIN_SYSTEM_PATH_1_0"),
        runtimeDir.filePath(QStringLiteral("lib/gstreamer-1.0"))
    );
    process.setProcessEnvironment(environment);
    process.start(executablePath, classes);

    if (!process.waitForStarted(kDeviceMonitorTimeoutMs)) {
        return SourceDiscoveryResult{
            .ok = false,
            .message = QStringLiteral("Failed to start GStreamer device monitor"),
        };
    }

    bool timedOut = false;
    if (!process.waitForFinished(kDeviceMonitorTimeoutMs)) {
        timedOut = true;
        process.kill();
        process.waitForFinished();
    }

    const QString stdoutText = QString::fromUtf8(process.readAllStandardOutput());
    const QString stderrText = QString::fromUtf8(process.readAllStandardError()).trimmed();

    if (!timedOut && process.exitStatus() != QProcess::NormalExit && stdoutText.trimmed().isEmpty() && !stderrText.isEmpty()) {
        return SourceDiscoveryResult{
            .ok = false,
            .message = stderrText,
        };
    }

    return SourceDiscoveryResult{
        .ok = true,
        .message = stdoutText,
    };
}

} // namespace travis::media_engine::discovery
