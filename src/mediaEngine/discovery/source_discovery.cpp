#include "mediaEngine/discovery/source_discovery.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>

// Uses gst-device-monitor so device discovery follows the same runtime as the media pipelines.

namespace travis::media_engine::discovery {

namespace {

constexpr int kDeviceMonitorTimeoutMs = 30000;
constexpr int kNdiDiscoveryProcessTimeoutMs = 15000;
constexpr int kNdiDiscoveryTimeoutMs = 2000;
constexpr int kNdiPollIntervalMs = 500;

const QStringList kNdiRuntimePathEnvKeys = {
    QStringLiteral("NDI_RUNTIME_DIR_V6"),
    QStringLiteral("NDI_RUNTIME_DIR_V5"),
    QStringLiteral("NDI_RUNTIME_DIR"),
    QStringLiteral("NDILIB_REDIST_FOLDER"),
};

const QStringList kNdiFallbackRuntimePaths = {
    QStringLiteral("C:/Program Files/NDI/NDI 6 Runtime/v6/Processing.NDI.Lib.x64.dll"),
    QStringLiteral("C:/Program Files/NDI/NDI 6 SDK/Bin/x64/Processing.NDI.Lib.x64.dll"),
    QStringLiteral("C:/Program Files (x86)/NDI/NDI 6 Runtime/v6/Processing.NDI.Lib.x64.dll"),
};

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

QString ndiSourceId(const QString& name, const QString& urlAddress) {
    const QRegularExpression portExpression(QStringLiteral(":(\\d+)$"));
    const QRegularExpressionMatch portMatch = portExpression.match(urlAddress);
    const QString sourceReference = portMatch.hasMatch()
        ? portMatch.captured(1)
        : (urlAddress.trimmed().isEmpty() ? QStringLiteral("unknown") : urlAddress.trimmed());

    return QStringLiteral("%1:%2").arg(name.trimmed(), sourceReference).toLower();
}

QString ndiUrlHost(const QString& urlAddress) {
    return urlAddress.trimmed().replace(QRegularExpression(QStringLiteral(":\\d+$")), QString{});
}

int ndiAddressPriority(const QString& urlAddress) {
    const QString host = ndiUrlHost(urlAddress);
    if (host.isEmpty()) {
        return 0;
    }

    if (QRegularExpression(QStringLiteral("^172\\.(1[6-9]|2\\d|3[0-1])\\.")).match(host).hasMatch()) {
        return 1;
    }

    if (QRegularExpression(QStringLiteral("^(10\\.|192\\.168\\.|169\\.254\\.)")).match(host).hasMatch()) {
        return 3;
    }

    return 2;
}

QString resolveNdiRuntimePath() {
    for (const QString& envKey : kNdiRuntimePathEnvKeys) {
        const QString value = qEnvironmentVariable(envKey.toUtf8().constData()).trimmed();
        if (value.isEmpty()) {
            continue;
        }

        const QFileInfo envPath(value);
        const QString candidatePath = envPath.isFile()
            ? envPath.absoluteFilePath()
            : QDir(value).filePath(QStringLiteral("Processing.NDI.Lib.x64.dll"));

        if (QFileInfo::exists(candidatePath)) {
            return QDir::toNativeSeparators(candidatePath);
        }
    }

    for (const QString& fallbackPath : kNdiFallbackRuntimePaths) {
        if (QFileInfo::exists(fallbackPath)) {
            return QDir::toNativeSeparators(fallbackPath);
        }
    }

    return QString{};
}

QString escapedPowerShellSingleQuotedString(const QString& value) {
    QString escaped = value;
    escaped.replace(QStringLiteral("'"), QStringLiteral("''"));
    return escaped;
}

QString buildNdiDiscoveryScript(const QString& runtimePath) {
    return QStringLiteral(R"(
$dllPath = '%1'
$source = @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

public static class NdiFindBridge {
    public class NdiSourceResult {
        public string name;
        public string urlAddress;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct NDIlib_find_create_t {
        [MarshalAs(UnmanagedType.I1)]
        public bool show_local_sources;
        public IntPtr p_groups;
        public IntPtr p_extra_ips;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct NDIlib_source_t {
        public IntPtr p_ndi_name;
        public IntPtr p_url_address;
    }

    [DllImport("Processing.NDI.Lib.x64.dll", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool NDIlib_initialize();

    [DllImport("Processing.NDI.Lib.x64.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void NDIlib_destroy();

    [DllImport("Processing.NDI.Lib.x64.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr NDIlib_find_create_v2(ref NDIlib_find_create_t createSettings);

    [DllImport("Processing.NDI.Lib.x64.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern void NDIlib_find_destroy(IntPtr instance);

    [DllImport("Processing.NDI.Lib.x64.dll", CallingConvention = CallingConvention.Cdecl)]
    [return: MarshalAs(UnmanagedType.I1)]
    public static extern bool NDIlib_find_wait_for_sources(IntPtr instance, UInt32 timeout_in_ms);

    [DllImport("Processing.NDI.Lib.x64.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr NDIlib_find_get_current_sources(IntPtr instance, out UInt32 no_sources);

    private static string PtrToUtf8String(IntPtr pointer) {
        if (pointer == IntPtr.Zero) return null;
        var bytes = new List<byte>();
        int offset = 0;
        while (true) {
            byte current = Marshal.ReadByte(pointer, offset++);
            if (current == 0) break;
            bytes.Add(current);
        }
        return Encoding.UTF8.GetString(bytes.ToArray());
    }

    public static NdiSourceResult[] Discover(int timeoutMs, int pollIntervalMs) {
        if (!NDIlib_initialize()) {
            throw new InvalidOperationException("NDI initialize failed");
        }

        IntPtr finder = IntPtr.Zero;
        try {
            var settings = new NDIlib_find_create_t {
                show_local_sources = true,
                p_groups = IntPtr.Zero,
                p_extra_ips = IntPtr.Zero
            };

            finder = NDIlib_find_create_v2(ref settings);
            if (finder == IntPtr.Zero) {
                throw new InvalidOperationException("NDI finder creation failed");
            }

            var discoveredSources = new Dictionary<string, NdiSourceResult>();
            int elapsed = 0;
            int structSize = Marshal.SizeOf(typeof(NDIlib_source_t));

            while (elapsed < timeoutMs) {
                NDIlib_find_wait_for_sources(finder, (UInt32)pollIntervalMs);

                UInt32 count;
                IntPtr sourcesPtr = NDIlib_find_get_current_sources(finder, out count);

                for (int i = 0; i < count; i++) {
                    IntPtr current = new IntPtr(sourcesPtr.ToInt64() + (i * structSize));
                    var source = (NDIlib_source_t)Marshal.PtrToStructure(current, typeof(NDIlib_source_t));

                    string name = PtrToUtf8String(source.p_ndi_name) ?? string.Empty;
                    string urlAddress = PtrToUtf8String(source.p_url_address);
                    string key = urlAddress ?? name;

                    if (!string.IsNullOrEmpty(key) && !discoveredSources.ContainsKey(key)) {
                        discoveredSources[key] = new NdiSourceResult {
                            name = name,
                            urlAddress = urlAddress
                        };
                    }
                }

                elapsed += pollIntervalMs;
            }

            var results = new List<NdiSourceResult>(discoveredSources.Values);
            return results.ToArray();
        } finally {
            if (finder != IntPtr.Zero) {
                NDIlib_find_destroy(finder);
            }
            NDIlib_destroy();
        }
    }
}
'@
Add-Type -TypeDefinition $source
$env:PATH = "$(Split-Path $dllPath);$env:PATH"
@([NdiFindBridge]::Discover(%2, %3)) | ConvertTo-Json -Compress
)").arg(
        escapedPowerShellSingleQuotedString(runtimePath),
        QString::number(kNdiDiscoveryTimeoutMs),
        QString::number(kNdiPollIntervalMs)
    );
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

QVector<DiscoveredMediaSource> parseNdiSources(const QString& output) {
    QVector<DiscoveredMediaSource> sources;
    QHash<QString, DiscoveredMediaSource> bestSourceByLogicalKey;

    const QByteArray outputBytes = output.trimmed().toUtf8();
    if (outputBytes.isEmpty()) {
        return sources;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(outputBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return sources;
    }

    const QJsonArray sourceArray = document.isArray()
        ? document.array()
        : QJsonArray{document.object()};

    for (const QJsonValue& value : sourceArray) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        const QString name = object.value(QStringLiteral("name")).toString().trimmed();
        const QString urlAddress = object.value(QStringLiteral("urlAddress")).toString().trimmed();

        if (name.isEmpty()) {
            continue;
        }

        const QRegularExpressionMatch portMatch = QRegularExpression(QStringLiteral(":(\\d+)$")).match(urlAddress);
        const QString logicalKey = QStringLiteral("%1:%2")
            .arg(name, portMatch.hasMatch() ? portMatch.captured(1) : (urlAddress.isEmpty() ? QStringLiteral("unknown") : urlAddress));

        const DiscoveredMediaSource source{
            .id = ndiSourceId(name, urlAddress),
            .kind = QStringLiteral("ndi"),
            .name = name,
            .urlAddress = urlAddress,
        };

        const auto existingSource = bestSourceByLogicalKey.constFind(logicalKey);
        if (existingSource == bestSourceByLogicalKey.constEnd() ||
            ndiAddressPriority(source.urlAddress) > ndiAddressPriority(existingSource->urlAddress)) {
            bestSourceByLogicalKey.insert(logicalKey, source);
        }
    }

    sources = bestSourceByLogicalKey.values().toVector();
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

SourceDiscoveryResult SourceDiscovery::listNdiSources() const {
    const QString runtimePath = resolveNdiRuntimePath();
    if (runtimePath.isEmpty()) {
        return SourceDiscoveryResult{
            .ok = true,
            .message = QStringLiteral("NDI runtime library was not found"),
        };
    }

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(
        QStringLiteral("PATH"),
        QFileInfo(runtimePath).absolutePath() + QStringLiteral(";") + environment.value(QStringLiteral("PATH"))
    );
    process.setProcessEnvironment(environment);

    process.start(
        QStringLiteral("powershell.exe"),
        {
            QStringLiteral("-NoProfile"),
            QStringLiteral("-ExecutionPolicy"),
            QStringLiteral("Bypass"),
            QStringLiteral("-Command"),
            buildNdiDiscoveryScript(runtimePath),
        }
    );

    if (!process.waitForStarted(kNdiDiscoveryProcessTimeoutMs)) {
        return SourceDiscoveryResult{
            .ok = false,
            .message = QStringLiteral("Failed to start NDI discovery process"),
        };
    }

    if (!process.waitForFinished(kNdiDiscoveryProcessTimeoutMs)) {
        process.kill();
        process.waitForFinished();
        return SourceDiscoveryResult{
            .ok = false,
            .message = QStringLiteral("NDI discovery timed out"),
        };
    }

    const QString stdoutText = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    const QString stderrText = QString::fromUtf8(process.readAllStandardError()).trimmed();

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return SourceDiscoveryResult{
            .ok = false,
            .message = stderrText.isEmpty() ? QStringLiteral("NDI discovery process failed") : stderrText,
        };
    }

    return SourceDiscoveryResult{
        .ok = true,
        .message = QStringLiteral("NDI discovery completed"),
        .sources = parseNdiSources(stdoutText),
    };
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
