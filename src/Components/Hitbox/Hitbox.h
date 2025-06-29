#ifndef HITBOX_H
#define HITBOX_H

#include <string>
#include <vector>
#include "Components/CollisionShape/CollisionShape.h"
#include "Components/Tag/Tag.h"
#include <set>

class Hitbox : public CollisionShape {
public:
    Hitbox(const std::string &hitboxName = "Hitbox");
    ~Hitbox() = default;

    bool IsHitbox() const override { return true; }

    void SetValidTargetTags(const std::vector<std::string> &tags);
    void AddValidTargetTag(const std::string &tag);
    void RemoveValidTargetTag(const std::string &tag);
    void ClearValidTargetTags();

    void SetActive(bool active);
    bool IsActive() const { return m_isActive; }

    void OnHitTarget(CollisionShape *target);
    void ResetForNewAttack();
    void SetDamage(float damage) { m_damage = damage; }
    float GetDamage() const { return m_damage; }
    std::string GetName() const { return m_hitboxName; }
    bool IsOneHitPerTarget() const { return m_oneHitPerTarget; }
    void SetOneHitPerTarget(bool oneHit) { this->m_oneHitPerTarget = oneHit; }

private:
    std::vector<std::string> m_validTargetTags;
    bool m_isActive;
    std::string m_hitboxName;
    float m_damage = 10.0f;
    std::set<std::string> m_hitTargetsThisAttack; 
    bool m_oneHitPerTarget;
    bool IsValidTarget(GameObject *target);
    void ProcessHit(GameObject *target);

};

#endif // HITBOX_H
