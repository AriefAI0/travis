#include "mediaEngine/sources/ndi/ndi_source_reference.h"

#include <cctype>

// Splits an NDI source label into its display name and optional host suffix.

namespace travis::media_engine::sources::ndi {

namespace {

bool isUrlAddressSuffix(const std::string& value) {
    bool hasColon = false;

    for (const char character : value) {
        const auto unsignedCharacter = static_cast<unsigned char>(character);

        if (std::isdigit(unsignedCharacter) || character == '.') {
            continue;
        }

        if (character == ':') {
            hasColon = true;
            continue;
        }

        return false;
    }

    return hasColon;
}

} // namespace

NdiSourceReference parseNdiSourceReference(const std::string& sourceName) {
    const auto suffixStart = sourceName.rfind(" (");

    if (suffixStart == std::string::npos || sourceName.empty() || sourceName.back() != ')') {
        return NdiSourceReference{sourceName, std::nullopt};
    }

    const auto suffixValueStart = suffixStart + 2;
    const auto suffixValueLength = sourceName.size() - suffixValueStart - 1;
    const auto suffixValue = sourceName.substr(suffixValueStart, suffixValueLength);

    if (!isUrlAddressSuffix(suffixValue)) {
        return NdiSourceReference{sourceName, std::nullopt};
    }

    const auto parsedName = sourceName.substr(0, suffixStart);

    if (parsedName.empty()) {
        return NdiSourceReference{sourceName, suffixValue};
    }

    return NdiSourceReference{parsedName, suffixValue};
}

} // namespace travis::media_engine::sources::ndi
