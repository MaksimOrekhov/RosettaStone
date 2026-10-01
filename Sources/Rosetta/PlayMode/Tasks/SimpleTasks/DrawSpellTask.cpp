// This code is based on Sabberstone project.
// Copyright (c) 2017-2021 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2017-2024 Chris Ohk

#include <Rosetta/Common/Utils.hpp>
#include <Rosetta/PlayMode/Actions/Draw.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Models/Spell.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawSpellTask.hpp>
#include <Rosetta/PlayMode/Zones/DeckZone.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>

using Random = effolkronium::random_static;

namespace RosettaStone::PlayMode::SimpleTasks
{
DrawSpellTask::DrawSpellTask(int amount, bool addToStack)
    : m_amount(amount), m_addToStack(addToStack)
{
    // Do nothing
}

DrawSpellTask::DrawSpellTask(SpellSchool spellSchool, int amount,
                             bool addToStack)
    : m_spellSchool(spellSchool), m_amount(amount), m_addToStack(addToStack)
{
    // Do nothing
}

DrawSpellTask::DrawSpellTask(DrawSpellType drawSpellType, int amount,
                             bool addToStack)
    : m_drawSpellType(drawSpellType), m_amount(amount), m_addToStack(addToStack)
{
    // Do nothing
}

DrawSpellTask::DrawSpellTask(SpellSchool spellSchool,
                             DrawSpellType drawSpellType, int amount,
                             bool addToStack, int minCost)
    : m_spellSchool(spellSchool),
      m_drawSpellType(drawSpellType),
      m_amount(amount),
      m_minCost(minCost),
      m_addToStack(addToStack)
{
    // Do nothing
}

TaskStatus DrawSpellTask::Impl(Player* player)
{
    if (m_addToStack)
    {
        player->game->taskStack.playables.clear();
    }

    auto deckCards = player->GetDeckZone()->GetAll();

    EraseIf(deckCards, [](const Playable* playable) {
        return playable->card->GetCardType() != CardType::SPELL;
    });

    if (m_spellSchool != SpellSchool::NONE)
    {
        EraseIf(deckCards, [this](const Playable* playable) {
            return playable->card->GetSpellSchool() != m_spellSchool;
        });
    }

    if (m_drawSpellType == DrawSpellType::MIN_COST_AT_LEAST)
    {
        EraseIf(deckCards, [this](const Playable* playable) {
            return playable->GetCost() < m_minCost;
        });
    }

    if (deckCards.empty())
    {
        return TaskStatus::STOP;
    }

    switch (m_drawSpellType)
    {
        case DrawSpellType::DEFAULT:
            std::ranges::shuffle(deckCards, Random::get_engine());
            break;
        case DrawSpellType::HIGHEST_COST:
            std::ranges::sort(deckCards,
                              [](const Playable* card1, const Playable* card2) {
                                  return card1->GetCost() > card2->GetCost();
                              });
            break;
        case DrawSpellType::MIN_COST_AT_LEAST:
            std::ranges::shuffle(deckCards, Random::get_engine());
            break;
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

std::unique_ptr<ITask> DrawSpellTask::CloneImpl()
{
    return std::make_unique<DrawSpellTask>(m_spellSchool, m_drawSpellType,
                                           m_amount, m_addToStack, m_minCost);
}
}  // namespace RosettaStone::PlayMode::SimpleTasks
