#pragma once

#include <optional>
#include <string>

// Parses NDI source labels that may include a discoverable url-address suffix.

namespace travis::media_engine::sources::ndi {

struct NdiSourceReference {
    std::string name;
    std::optional<std::string> urlAddress;
};

[[nodiscard]] NdiSourceReference parseNdiSourceReference(const std::string& sourceName);

} // namespace travis::media_engine::sources::ndi
