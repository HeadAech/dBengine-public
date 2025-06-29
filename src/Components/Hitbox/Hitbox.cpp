#include "Hitbox.h"
#include <algorithm>
#include <iostream>
#include "GameObject/GameObject.h"
#include <Signal/LuaSignal.h>
#include <Components/LuaComponent/LuaComponent.h>

Hitbox::Hitbox(const std::string &hitboxName) : CollisionShape(""), m_isActive(false), m_hitboxName(hitboxName) {
    name = "Hitbox";
    std::cout << "[Hitbox] Created hitbox '" << m_hitboxName << "'" << std::endl;
    m_oneHitPerTarget = true;
    m_hitTargetsThisAttack.clear();
    m_validTargetTags.push_back("Enemy");
}

void Hitbox::SetValidTargetTags(const std::vector<std::string> &tags) { m_validTargetTags = tags; }

void Hitbox::AddValidTargetTag(const std::string &tag) {
    auto it = std::find(m_validTargetTags.begin(), m_validTargetTags.end(), tag);
    if (it == m_validTargetTags.end()) {
        m_validTargetTags.push_back(tag);
    }
}

void Hitbox::RemoveValidTargetTag(const std::string &tag) {
    auto it = std::find(m_validTargetTags.begin(), m_validTargetTags.end(), tag);
    if (it != m_validTargetTags.end()) {
        m_validTargetTags.erase(it);
    }
}

void Hitbox::ClearValidTargetTags() { m_validTargetTags.clear(); }

void Hitbox::SetActive(bool active) {
    m_isActive = active;
}

void Hitbox::OnHitTarget(CollisionShape *target) {
    if (!m_isActive) {
        return;
    }

    if (target && target->gameObject && IsValidTarget(target->gameObject)) {
        ProcessHit(target->gameObject);
    }
}

bool Hitbox::IsValidTarget(GameObject *target) {
    if (!target)
        return false;

    Tag *tag = target->GetComponent<Tag>();
    if (!tag) {
        return false;
    }
    for (const std::string &validTag: m_validTargetTags) {
        if (tag->Name == validTag) {
            return true;
        }
    }

    return false;
}

void Hitbox::ProcessHit(GameObject *target) {
    if (!target)
        return;

    if (m_oneHitPerTarget) {
        std::string targetUUID = target->GetUUID();

        if (m_hitTargetsThisAttack.find(targetUUID) != m_hitTargetsThisAttack.end()) {
            return; 
        }

        m_hitTargetsThisAttack.insert(targetUUID);
    }

    Tag *tag = target->GetComponent<Tag>();
    std::string tagName = tag ? tag->Name : "Unknown";

    LuaComponent *targetLuaComp = target->GetComponent<LuaComponent>();
    if (targetLuaComp) {

        sol::table hitInfo = targetLuaComp->L.create_table();
        hitInfo["damage"] = m_damage;
        hitInfo["hitbox_name"] = m_hitboxName;
        hitInfo["attacker_uuid"] = gameObject->GetUUID();
        hitInfo["target_uuid"] = target->GetUUID();

        auto emit_signal_func = targetLuaComp->L["emit_signal"];
        if (emit_signal_func.valid()) {
            emit_signal_func("hitbox_hit", hitInfo);
        }
    }
    

    std::cout << "[SIGNAL] Emitted Hitbox_Hit signal - " << gameObject->GetUUID() << " hit " << target->GetUUID()
              << " for " << 1.0 << " damage" << std::endl;
}

void Hitbox::ResetForNewAttack() { m_hitTargetsThisAttack.clear(); }
