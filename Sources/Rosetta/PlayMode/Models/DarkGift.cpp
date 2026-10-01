// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeHyun Jeon

#include <Rosetta/PlayMode/Models/DarkGift.hpp>
#include <Rosetta/PlayMode/Models/Entity.hpp>
#include <Rosetta/PlayMode/Models/Character.hpp>
#include <Rosetta/PlayMode/Models/Minion.hpp>
#include <Rosetta/PlayMode/Models/Playable.hpp>

#include <algorithm>
#include <stdexcept>

namespace RosettaStone::PlayMode
{
DarkGiftCandidate MakeDarkGiftCandidate(const Card& card)
{
    const auto hasTag = [&card](GameTag tag) {
        const auto found = card.gameTags.find(tag);
        return found != card.gameTags.end() && found->second != 0;
    };

    DarkGiftCandidate candidate;
    candidate.isMinion = card.GetCardType() == CardType::MINION;
    candidate.attack = card.gameTags.contains(GameTag::ATK)
                           ? card.gameTags.at(GameTag::ATK)
                           : 0;
    candidate.lifesteal = hasTag(GameTag::LIFESTEAL);
    candidate.elusive = hasTag(GameTag::CANT_BE_TARGETED_BY_OPPONENTS);
    candidate.charge = hasTag(GameTag::CHARGE);
    candidate.battlecry = hasTag(GameTag::BATTLECRY);
    candidate.taunt = hasTag(GameTag::TAUNT);
    candidate.reborn = hasTag(GameTag::REBORN);
    candidate.divineShield = hasTag(GameTag::DIVINE_SHIELD);
    candidate.windfury = hasTag(GameTag::WINDFURY);
    return candidate;
}

DarkGiftCandidate MakeDarkGiftCandidate(const Entity& entity)
{
    DarkGiftCandidate candidate;
    candidate.isMinion = entity.card &&
                         entity.card->GetCardType() == CardType::MINION;
    candidate.attack = entity.GetGameTag(GameTag::ATK);
    candidate.lifesteal = entity.GetGameTag(GameTag::LIFESTEAL) != 0;
    candidate.elusive =
        entity.GetGameTag(GameTag::CANT_BE_TARGETED_BY_OPPONENTS) != 0;
    candidate.charge = entity.GetGameTag(GameTag::CHARGE) != 0;
    candidate.battlecry = entity.GetGameTag(GameTag::BATTLECRY) != 0;
    candidate.taunt = entity.GetGameTag(GameTag::TAUNT) != 0;
    candidate.reborn = entity.GetGameTag(GameTag::REBORN) != 0;
    candidate.divineShield =
        entity.GetGameTag(GameTag::DIVINE_SHIELD) != 0;
    candidate.windfury = entity.GetGameTag(GameTag::WINDFURY) != 0;
    return candidate;
}

std::vector<DarkGift> GetEligibleDarkGifts(
    const DarkGiftCandidate& candidate)
{
    if (!candidate.isMinion)
    {
        return {};
    }

    std::vector<DarkGift> result;
    result.reserve(10);

    if (!candidate.lifesteal)
    {
        result.emplace_back(DarkGift::ATTACK_LIFESTEAL);
    }
    if (!candidate.elusive)
    {
        result.emplace_back(DarkGift::STATS_ELUSIVE);
    }
    if (candidate.attack >= 3)
    {
        result.emplace_back(DarkGift::DISCOUNT_ATTACK);
    }
    if (!candidate.charge)
    {
        result.emplace_back(DarkGift::CHARGE);
    }
    result.emplace_back(DarkGift::SUMMON_COPY);
    if (candidate.battlecry)
    {
        result.emplace_back(DarkGift::DOUBLE_BATTLECRY);
    }
    if (!candidate.taunt)
    {
        result.emplace_back(DarkGift::HEALTH_TAUNT);
    }
    if (!candidate.reborn)
    {
        result.emplace_back(DarkGift::REBORN_FULL_HEALTH);
    }
    result.emplace_back(DarkGift::STATS_TOP_DECK);
    if (!candidate.divineShield && !candidate.windfury)
    {
        result.emplace_back(DarkGift::DIVINE_SHIELD_WINDFURY);
    }

    return result;
}

std::vector<DarkGift> AssignDistinctDarkGifts(
    const std::vector<DarkGiftCandidate>& candidates,
    std::mt19937& randomEngine)
{
    if (candidates.empty() || candidates.size() > 3 ||
        std::ranges::any_of(candidates, [](const DarkGiftCandidate& candidate) {
            return !candidate.isMinion;
        }))
    {
        return {};
    }

    std::vector<std::vector<DarkGift>> assignments;
    std::vector<DarkGift> current;
    current.reserve(candidates.size());

    const auto enumerate = [&](const auto& self, std::size_t index) -> void {
        if (index == candidates.size())
        {
            assignments.emplace_back(current);
            return;
        }

        for (const DarkGift gift : GetEligibleDarkGifts(candidates[index]))
        {
            if (std::ranges::find(current, gift) != current.end())
            {
                continue;
            }

            current.emplace_back(gift);
            self(self, index + 1);
            current.pop_back();
        }
    };
    enumerate(enumerate, 0);

    if (assignments.empty())
    {
        return {};
    }

    std::ranges::shuffle(assignments, randomEngine);
    return std::move(assignments.front());
}

void ApplyDarkGift(Playable& playable, DarkGift gift)
{
    auto* minion = dynamic_cast<Minion*>(&playable);
    if (!minion)
    {
        throw std::invalid_argument("Dark Gifts can only be applied to minions");
    }

    const auto addAttack = [minion](int amount) {
        minion->SetAttack(minion->GetAttack() + amount);
    };
    const auto addHealth = [minion](int amount) {
        minion->SetBaseHealth(minion->GetBaseHealth() + amount);
    };
    const auto setKeyword = [minion](GameTag tag) {
        minion->SetGameTag(tag, 1);
    };

    minion->SetGameTag(GameTag::MANAMIND_DARK_GIFT_ID,
                       static_cast<int>(gift));
    switch (gift)
    {
        case DarkGift::ATTACK_LIFESTEAL:
            addAttack(3);
            setKeyword(GameTag::LIFESTEAL);
            break;
        case DarkGift::STATS_ELUSIVE:
            addAttack(2);
            addHealth(2);
            setKeyword(GameTag::CANT_BE_TARGETED_BY_OPPONENTS);
            break;
        case DarkGift::DISCOUNT_ATTACK:
            minion->SetGameTag(GameTag::COST,
                               std::max(0, minion->GetGameTag(GameTag::COST) - 2));
            addAttack(-2);
            break;
        case DarkGift::CHARGE:
            setKeyword(GameTag::CHARGE);
            break;
        case DarkGift::SUMMON_COPY:
        case DarkGift::DOUBLE_BATTLECRY:
            break;
        case DarkGift::HEALTH_TAUNT:
            addHealth(4);
            setKeyword(GameTag::TAUNT);
            break;
        case DarkGift::REBORN_FULL_HEALTH:
            setKeyword(GameTag::REBORN);
            break;
        case DarkGift::STATS_TOP_DECK:
            addAttack(4);
            addHealth(5);
            break;
        case DarkGift::DIVINE_SHIELD_WINDFURY:
            setKeyword(GameTag::DIVINE_SHIELD);
            setKeyword(GameTag::WINDFURY);
            break;
    }
}
}  // namespace RosettaStone::PlayMode
