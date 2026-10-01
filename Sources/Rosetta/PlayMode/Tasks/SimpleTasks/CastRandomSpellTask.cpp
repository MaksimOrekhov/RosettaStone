// This code is based on Sabberstone project.
// Copyright (c) 2017-2021 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2017-2024 Chris Ohk

#include <Rosetta/PlayMode/Actions/CastSpell.hpp>
#include <Rosetta/PlayMode/Actions/Choose.hpp>
#include <Rosetta/PlayMode/Cards/Cards.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Models/Spell.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/CastRandomSpellTask.hpp>
#include <Rosetta/PlayMode/Zones/GraveyardZone.hpp>
#include <Rosetta/PlayMode/Zones/SecretZone.hpp>

#include <effolkronium/random.hpp>

#include <utility>

using Random = effolkronium::random_static;

namespace RosettaStone::PlayMode::SimpleTasks
{
CastRandomSpellTask::CastRandomSpellTask(SpellSchool spellSchool,
                                         int manaBudget,
                                         bool randomEnemyTargets)
    : m_spellSchool(spellSchool),
      m_manaBudget(manaBudget),
      m_randomEnemyTargets(randomEnemyTargets)
{
    // Defaults preserve the original one-random-spell behavior.
}

TaskStatus CastRandomSpellTask::Impl(Player* player)
{
    player->SetGameTag(GameTag::CAST_RANDOM_SPELLS, 1);

    const auto cards = m_source->game->GetFormatType() == FormatType::STANDARD
                           ? Cards::GetAllStandardCards()
                           : Cards::GetAllWildCards();

    int manaSpent = 0;
    int spellsCast = 0;
    auto choiceTemp = std::move(player->choice);
    while ((m_manaBudget <= 0 && spellsCast == 0) ||
           (m_manaBudget > 0 && manaSpent < m_manaBudget && spellsCast < 30))
    {
        std::vector<Card*> result;
        for (const auto& card : cards)
        {
            if (card->GetCardType() != CardType::SPELL || card->IsQuest() ||
                (m_spellSchool != SpellSchool::NONE &&
                 card->GetSpellSchool() != m_spellSchool))
            {
                continue;
            }

            const int cost = card->GetCost();
            if (m_manaBudget > 0 &&
                (cost <= 0 || cost > m_manaBudget - manaSpent))
            {
                continue;
            }

            // NOTE: Puzzle Box of Yogg-Saron can cast any collectible spell
            // except another Puzzle Box of Yogg-Saron.
            // References:
            // https://twitter.com/Celestalon/status/1158895101537607681
            if (m_source->card->dbfID == 53442 && card->dbfID == 53442)
            {
                continue;
            }

            std::unique_ptr<Spell> probe(
                dynamic_cast<Spell*>(Entity::GetFromCard(player, card)));
            if (m_randomEnemyTargets && probe != nullptr)
            {
                const auto targets = probe->GetValidPlayTargets();
                bool hasEnemyTarget = false;
                for (const Character* target : targets)
                {
                    if (target != nullptr && target->player == player->opponent)
                    {
                        hasEnemyTarget = true;
                        break;
                    }
                }

                const bool requiresTarget = card->playRequirements.contains(
                    PlayReq::REQ_TARGET_TO_PLAY);
                if (requiresTarget && !hasEnemyTarget)
                {
                    continue;
                }
            }

            result.emplace_back(card);
        }

        if (result.empty())
        {
            break;
        }

        const auto randIdx = Random::get<std::size_t>(0, result.size() - 1);
        Card* selectedCard = result[randIdx];
        const int cost = selectedCard->GetCost();
        const auto spellToCast =
            dynamic_cast<Spell*>(Entity::GetFromCard(player, selectedCard));

        if (spellToCast->IsSecret() && player->GetSecretZone()->IsFull())
        {
            player->GetGraveyardZone()->Add(spellToCast);
            manaSpent += cost;
            ++spellsCast;
            continue;
        }

        Character* randTarget = nullptr;
        if (m_randomEnemyTargets)
        {
            std::vector<Character*> enemyTargets;
            for (Character* target : spellToCast->GetValidPlayTargets())
            {
                if (target != nullptr && target->player == player->opponent)
                {
                    enemyTargets.emplace_back(target);
                }
            }
            if (!enemyTargets.empty())
            {
                randTarget = *Random::get(enemyTargets);
            }
        }
        else
        {
            randTarget = spellToCast->GetRandomValidTarget();
        }
        const int randChooseOne = Random::get<int>(1, 2);

        player->game->taskQueue.StartEvent();
        Generic::CastRandomSpell(player, spellToCast, randTarget, randChooseOne);
        player->game->ProcessDestroyAndUpdateAura();
        player->game->taskQueue.EndEvent();

        while (player->choice)
        {
            if (player->choice->choices.empty())
            {
                player->choice.reset();
                break;
            }
            const auto idx =
                Random::get<std::size_t>(0, player->choice->choices.size() - 1);
            Generic::ChoicePick(player, player->choice->choices[idx]);
        }

        player->game->ProcessDestroyAndUpdateAura();
        manaSpent += cost;
        ++spellsCast;
    }

    player->choice = std::move(choiceTemp);

    player->SetGameTag(GameTag::CAST_RANDOM_SPELLS, 0);

    player->game->taskStack.Reset();

    return TaskStatus::COMPLETE;
}

std::unique_ptr<ITask> CastRandomSpellTask::CloneImpl()
{
    return std::make_unique<CastRandomSpellTask>(
        m_spellSchool, m_manaBudget, m_randomEnemyTargets);
}
}  // namespace RosettaStone::PlayMode::SimpleTasks
