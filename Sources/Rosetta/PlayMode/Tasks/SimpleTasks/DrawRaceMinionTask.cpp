// This code is based on Sabberstone project.
// Copyright (c) 2017-2021 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2017-2024 Chris Ohk

#include <Rosetta/PlayMode/Actions/Draw.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Models/Minion.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawRaceMinionTask.hpp>
#include <Rosetta/PlayMode/Zones/DeckZone.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>

using Random = effolkronium::random_static;

namespace RosettaStone::PlayMode::SimpleTasks
{
DrawRaceMinionTask::DrawRaceMinionTask(Race race, int amount, bool addToStack)
    : m_race(race), m_amount(amount), m_addToStack(addToStack)
{
    // Do nothing
}

TaskStatus DrawRaceMinionTask::Impl(Player* player)
{
    if (m_addToStack)
    {
        player->game->taskStack.playables.clear();
    }

    auto deck = player->GetDeckZone()->GetAll();
    if (deck.empty())
    {
        return TaskStatus::STOP;
    }

    std::vector<Playable*> cards;
    cards.reserve(m_amount);

    for (auto& deckCard : deck)
    {
        const auto race = deckCard->card->gameTags.find(GameTag::CARDRACE);
        if (dynamic_cast<Minion*>(deckCard) &&
            race != deckCard->card->gameTags.end() &&
            (race->second == static_cast<int>(m_race) ||
             race->second == static_cast<int>(Race::ALL)))
        {
            cards.emplace_back(deckCard);
        }
    }

    if (cards.empty())
    {
        return TaskStatus::STOP;
    }

    const auto drawCount = std::min(m_amount, static_cast<int>(cards.size()));

    if (static_cast<int>(cards.size()) <= drawCount)
    {
        for (int i = 0; i < drawCount; ++i)
        {
            if (m_addToStack)
            {
                player->game->taskStack.playables.emplace_back(cards[i]);
            }

            Generic::Draw(player, cards[i]);
        }
    }
    else
    {
        for (int i = 0; i < drawCount; ++i)
        {
            const auto pick = Random::get<std::size_t>(0, cards.size() - 1);

            if (m_addToStack)
            {
                player->game->taskStack.playables.emplace_back(cards[pick]);
            }

            Generic::Draw(player, cards[pick]);
            cards.erase(std::begin(cards) + static_cast<std::ptrdiff_t>(pick));
        }
    }

    return TaskStatus::COMPLETE;
}

std::unique_ptr<ITask> DrawRaceMinionTask::CloneImpl()
{
    return std::make_unique<DrawRaceMinionTask>(m_race, m_amount, m_addToStack);
}
}  // namespace RosettaStone::PlayMode::SimpleTasks
