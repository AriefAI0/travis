#pragma once

// Defines the Qt app identity used to build stable AppData and settings paths.

namespace travis::data::database {

class ApplicationIdentity {
public:
    // Sets the Qt identity values used for AppData and settings paths.
    static void apply();
};

} // namespace travis::data::database
