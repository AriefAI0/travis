#pragma once

namespace travis::data::database {

class ApplicationIdentity {
public:
    // Sets the Qt identity values used for AppData and settings paths.
    static void apply();
};

} // namespace travis::data::database
