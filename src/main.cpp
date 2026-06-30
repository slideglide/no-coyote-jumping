#include <Geode/loader/Mod.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

class $modify(NoCoyotePlayerObject, PlayerObject) {
    struct Fields {
        bool landedThisCollision = false;
    };

    void hitGround(GameObject* object, bool notFlipped) {
        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            PlayerObject::hitGround(object, notFlipped);
            return;
        }

        auto fields = m_fields.self();
        fields->landedThisCollision = true;

        PlayerObject::hitGround(object, notFlipped);
    }

    void postCollision(float dt, bool betweenSteps) {
        if (!Mod::get()->getSettingValue<bool>("enabled")) {
            PlayerObject::postCollision(dt, betweenSteps);
            return;
        }

        auto fields = m_fields.self();

        PlayerObject::postCollision(dt, betweenSteps);

        if (m_isPlatformer && !fields->landedThisCollision) {
            m_lastLandTime = 0.0;
            m_isOnGround = false;
        }

        fields->landedThisCollision = false;
    }
};
