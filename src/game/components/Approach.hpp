#pragma once

namespace game::components {

struct Approach {
    float startTimeSeconds;
    float targetTimeSeconds;
    float startScale{4.0f};
    float targetScale{1.0f};
};

} // namespace game::components
