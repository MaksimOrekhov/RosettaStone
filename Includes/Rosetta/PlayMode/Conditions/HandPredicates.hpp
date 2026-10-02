// Copyright (c) 2017-2024 Chris Ohk

#ifndef ROSETTASTONE_PLAYMODE_HAND_PREDICATES_HPP
#define ROSETTASTONE_PLAYMODE_HAND_PREDICATES_HPP

#include <Rosetta/PlayMode/Cards/Card.hpp>
#include <Rosetta/PlayMode/Models/Playable.hpp>
#include <Rosetta/PlayMode/Models/Player.hpp>
#include <Rosetta/PlayMode/Zones/HandZone.hpp>

#include <algorithm>

namespace RosettaStone::PlayMode
{
//! Shared predicates over the controller's current hand. These are reused by
//! Battlecry conditions and optional target availability checks.
class HandPredicates
{
 public:
    static bool MinionRaceInHand(Player* player, Race race)
    {
        if (!player || race == Race::INVALID)
        {
            return false;
        }

        const auto cards = player->GetHandZone()->GetAll();
        return std::any_of(cards.begin(), cards.end(), [race](const auto* card) {
            return card && card->card &&
                   card->card->GetCardType() == CardType::MINION &&
                   card->card->HasRace(race);
        });
    }

    static bool SpellSchoolInHand(Player* player, SpellSchool school)
    {
        if (!player || school == SpellSchool::NONE)
        {
            return false;
        }

        const auto cards = player->GetHandZone()->GetAll();
        return std::any_of(cards.begin(), cards.end(),
                           [school](const auto* card) {
                               return card && card->card &&
                                      card->card->GetCardType() ==
                                          CardType::SPELL &&
                                      card->card->GetSpellSchool() == school;
                           });
    }

    static bool SpellCostAtLeastInHand(Player* player, int minimumCost)
    {
        if (!player || minimumCost < 0)
        {
            return false;
        }

        const auto cards = player->GetHandZone()->GetAll();
        return std::any_of(cards.begin(), cards.end(),
                           [minimumCost](const auto* card) {
                               return card && card->card &&
                                      card->card->GetCardType() ==
                                          CardType::SPELL &&
                                      card->GetCost() >= minimumCost;
                           });
    }
};
}  // namespace RosettaStone::PlayMode

#endif  // ROSETTASTONE_PLAYMODE_HAND_PREDICATES_HPP
