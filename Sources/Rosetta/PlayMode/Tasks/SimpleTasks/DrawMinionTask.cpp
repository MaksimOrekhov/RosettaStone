// This code is based on Sabberstone project.
// Copyright (c) 2017-2021 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2017-2024 Chris Ohk

#include <Rosetta/Common/Utils.hpp>
#include <Rosetta/PlayMode/Actions/Draw.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawMinionTask.hpp>
#include <Rosetta/PlayMode/Zones/DeckZone.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>

using Random = effolkronium::random_static;

namespace RosettaStone::PlayMode::SimpleTasks
{
DrawMinionTask::DrawMinionTask(int amount, bool addToStack)
    : m_amount(amount), m_addToStack(addToStack)
{
    // Do nothing
}

DrawMinionTask::DrawMinionTask(DrawMinionType drawMinionType, int amount,
                               bool addToStack, int minCost)
    : m_amount(amount),
      m_minCost(minCost),
      m_drawMinionType(drawMinionType),
      m_addToStack(addToStack)
{
    // Do nothing
}

TaskStatus DrawMinionTask::Impl(Player* player)
{
    if (m_addToStack)
    {
        player->game->taskStack.playables.clear();
    }

    auto deckCards = player->GetDeckZone()->GetAll();

    EraseIf(deckCards, [](const Playable* playable) {
        return playable->card->GetCardType() != CardType::MINION;
    });

    if (m_drawMinionType == DrawMinionType::MIN_COST_AT_LEAST)
    {
        EraseIf(deckCards, [this](const Playable* playable) {
            return playable->GetCost() < m_minCost;
        });
    }

    if (deckCards.empty())
    {
        return TaskStatus::STOP;
    }

    switch (m_drawMinionType)
    {
        case DrawMinionType::DEFAULT:
            std::shuffle(deckCards.begin(), deckCards.end(),
                         Random::get_engine());
            break;
        case DrawMinionType::LOWEST_COST:
            std::sort(deckCards.begin(), deckCards.end(),
                      [](const Playable* card1, const Playable* card2) {
                          return card1->GetCost() < card2->GetCost();
                      });
            break;
        case DrawMinionType::HIGHEST_COST:
            std::sort(deckCards.begin(), deckCards.end(),
                      [](const Playable* card1, const Playable* card2) {
                          return card1->GetCost() > card2->GetCost();
                      });
            break;
        case DrawMinionType::DEATHRATTLE:
            EraseIf(deckCards, [](const Playable* playable) {
                return !playable->HasDeathrattle();
            });
            break;
        case DrawMinionType::MIN_COST_AT_LEAST:
            std::shuffle(deckCards.begin(), deckCards.end(), Random::get_engine());
            break;
    }

    if (deckCards.empty())
    {
        return TaskStatus::STOP;
    }

    const auto drawCount = std::min(m_amount, static_cast<int>(deckCards.size()));
    for (int i = 0; i < drawCount; ++i)
    {
        if (m_addToStack)
        {
            player->game->taskStack.playables.emplace_back(deckCards[i]);
        }

        Generic::Draw(player, deckCards[i]);
    }

    return TaskStatus::COMPLETE;
}

std::unique_ptr<ITask> DrawMinionTask::CloneImpl()
{
    return std::make_unique<DrawMinionTask>(m_drawMinionType, m_amount,
                                            m_addToStack, m_minCost);
}
}  // namespace RosettaStone::PlayMode::SimpleTasks
