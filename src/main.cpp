#include <Geode/loader/Mod.hpp>
#include <Geode/modify/EditLevelLayer.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/LevelPage.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/LevelAreaInnerLayer.hpp>

using namespace geode::prelude;

// all of the warning logic here
static bool activeFor(GJGameLevel* level) {
    return Mod::get()->getSettingValue<bool>("enabled") && level && level->isPlatformer();
}

static bool showWarnFor(GJGameLevel* level) {
    if (!activeFor(level)) return false;

    createQuickPopup(
        "Notice",
        "The <cl>No Coyote Jumping</c> mod is enabled. It is recommended that you <cr>disable the mod</c> if you intend to make <cy>real progress</c> on the level.",
        "Close", "Disable Mod",
        [](auto, bool btn2) {
            if (btn2) {
                Mod::get()->setSettingValue<bool>("enabled", false);
            }
        }
    );

    return true;
}

#define WARN_PLATFORMER_HOOK(name, cls)            \
class $modify(name, cls) {                         \
    struct Fields {                                \
        bool shownWarn = false;                    \
    };                                             \
                                                   \
    void onPlay(CCObject* sender) {                \
        if (!m_fields->shownWarn && showWarnFor(m_level)) { \
            m_fields->shownWarn = true;            \
            return;                                \
        }                                          \
        cls::onPlay(sender);                       \
    }                                              \
};

WARN_PLATFORMER_HOOK(NoCoyoteLevelInfoLayer, LevelInfoLayer)
WARN_PLATFORMER_HOOK(NoCoyoteLevelPage, LevelPage)
WARN_PLATFORMER_HOOK(NoCoyoteEditLevelLayer, EditLevelLayer)

    // the tower warning
class $modify(NoCoyoteLevelAreaInnerLayer, LevelAreaInnerLayer) {
    struct Fields {
        bool shownWarn = false;
    };

    void onDoor(CCObject* sender) {
        auto levelID = sender->getTag();
        auto level = GameLevelManager::get()->getMainLevel(levelID, true);

        auto fields = m_fields.self();

        if (!fields->shownWarn && showWarnFor(level)) {
            fields->shownWarn = true;
            return;
        }

        LevelAreaInnerLayer::onDoor(sender);
    }
};

// the actual coyote jump disabling logic
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
