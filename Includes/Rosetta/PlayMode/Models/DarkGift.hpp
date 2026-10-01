// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeHyun Jeon

#ifndef ROSETTASTONE_PLAYMODE_DARK_GIFT_HPP
#define ROSETTASTONE_PLAYMODE_DARK_GIFT_HPP

#include <Rosetta/PlayMode/Cards/Card.hpp>

#include <vector>
#include <random>

namespace RosettaStone::PlayMode
{
class Playable;

class Entity;

//! Dark Gift modifiers offered alongside Into the Emerald Dream minions.
enum class DarkGift
{
    ATTACK_LIFESTEAL = 1,
    STATS_ELUSIVE,
    DISCOUNT_ATTACK,
    CHARGE,
    SUMMON_COPY,
    DOUBLE_BATTLECRY,
    HEALTH_TAUNT,
    REBORN_FULL_HEALTH,
    STATS_TOP_DECK,
    DIVINE_SHIELD_WINDFURY,
};

//! The card properties used to decide which Dark Gifts can be offered.
struct DarkGiftCandidate
{
    bool isMinion = false;
    int attack = 0;
    bool lifesteal = false;
    bool elusive = false;
    bool charge = false;
    bool battlecry = false;
    bool taunt = false;
    bool reborn = false;
    bool divineShield = false;
    bool windfury = false;
};

//! Reads a candidate from static catalog properties or the current entity.
DarkGiftCandidate MakeDarkGiftCandidate(const Card& card);
DarkGiftCandidate MakeDarkGiftCandidate(const Entity& entity);

//! Returns gifts compatible with a minion's current stats and keywords.
//! This checks only option eligibility; it does not apply the gift.
std::vector<DarkGift> GetEligibleDarkGifts(
    const DarkGiftCandidate& candidate);

//! Assigns distinct eligible gifts to up to three candidates.
//! Returns an empty vector when no complete assignment exists.
std::vector<DarkGift> AssignDistinctDarkGifts(
    const std::vector<DarkGiftCandidate>& candidates,
    std::mt19937& randomEngine);

//! Applies an already-selected Dark Gift to the selected minion entity.
void ApplyDarkGift(Playable& playable, DarkGift gift);
}  // namespace RosettaStone::PlayMode

#endif  // ROSETTASTONE_PLAYMODE_DARK_GIFT_HPP
