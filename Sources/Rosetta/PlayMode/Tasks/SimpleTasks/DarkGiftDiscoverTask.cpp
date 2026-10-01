// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeHyun Jeon

#include <Rosetta/PlayMode/Actions/Choose.hpp>
#include <Rosetta/PlayMode/Cards/Cards.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Models/Choice.hpp>
#include <Rosetta/PlayMode/Models/DarkGift.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DarkGiftDiscoverTask.hpp>
#include <Rosetta/PlayMode/Zones/DeckZone.hpp>
#include <Rosetta/PlayMode/Zones/HandZone.hpp>
#include <Rosetta/PlayMode/Zones/SetasideZone.hpp>

#include <effolkronium/random.hpp>

#include <algorithm>
#include <functional>
#include <random>
#include <unordered_set>
#include <utility>

using Random = effolkronium::random_static;

namespace RosettaStone::PlayMode::SimpleTasks
{
namespace
{
struct Candidate
{
    Card* card = nullptr;
    Playable* entity = nullptr;
};

std::vector<Candidate> GetCandidates(Player* player, DarkGiftPool pool)
{
    std::vector<Candidate> result;
    if (pool == DarkGiftPool::DECK_MINION)
    {
        std::unordered_set<int> seenDbfIDs;
        for (Playable* playable : player->GetDeckZone()->GetAll())
        {
            if (!playable->card ||
                playable->card->GetCardType() != CardType::MINION ||
                !seenDbfIDs.insert(playable->card->dbfID).second)
            {
                continue;
            }

            result.push_back({ playable->card, playable });
        }
        return result;
    }

    const FormatType format = player->game->GetFormatType();
    const std::vector<Card*>* cards = nullptr;
    if (pool == DarkGiftPool::WARRIOR)
    {
        cards = format == FormatType::STANDARD
                    ? &Cards::GetStandardCards(CardClass::WARRIOR)
                    : &Cards::GetWildCards(CardClass::WARRIOR);
    }
    else
    {
        cards = format == FormatType::STANDARD ? &Cards::GetAllStandardCards()
                                               : &Cards::GetAllWildCards();
    }

    for (Card* card : *cards)
    {
        if (!card || card->GetCardType() != CardType::MINION)
        {
            continue;
        }
        if (pool == DarkGiftPool::DRAGON && card->GetRace() != Race::DRAGON)
        {
            continue;
        }
        result.push_back({ card, nullptr });
    }
    return result;
}

bool SelectOptions(const std::vector<Candidate>& candidates,
                   std::mt19937& randomEngine,
                   std::vector<std::size_t>& selected,
                   std::vector<DarkGift>& gifts)
{
    const std::size_t targetCount = std::min<std::size_t>(3, candidates.size());
    if (targetCount == 0)
    {
        return false;
    }

    std::vector<DarkGiftCandidate> properties;
    properties.reserve(targetCount);

    const auto search = [&](const auto& self, std::size_t start) -> bool {
        if (selected.size() == targetCount)
        {
            gifts = AssignDistinctDarkGifts(properties, randomEngine);
            return gifts.size() == targetCount;
        }

        const std::size_t remaining = targetCount - selected.size();
        if (candidates.size() - start < remaining)
        {
            return false;
        }

        for (std::size_t index = start;
             index + remaining <= candidates.size(); ++index)
        {
            const Candidate& candidate = candidates[index];
            const DarkGiftCandidate candidateProperties =
                candidate.entity ? MakeDarkGiftCandidate(*candidate.entity)
                                 : MakeDarkGiftCandidate(*candidate.card);
            if (GetEligibleDarkGifts(candidateProperties).empty())
            {
                continue;
            }

            selected.emplace_back(index);
            properties.emplace_back(candidateProperties);
            if (AssignDistinctDarkGifts(properties, randomEngine).size() ==
                    properties.size() &&
                self(self, index + 1))
            {
                return true;
            }
            selected.pop_back();
            properties.pop_back();
        }

        return false;
    };

    return search(search, 0);
}
}  // namespace

DarkGiftDiscoverTask::DarkGiftDiscoverTask(DarkGiftPool pool,
                                           bool requiresDragonInHand)
    : m_pool(pool), m_requiresDragonInHand(requiresDragonInHand)
{
    // Do nothing
}

TaskStatus DarkGiftDiscoverTask::Impl(Player* player)
{
    if (m_requiresDragonInHand)
    {
        const auto hand = player->GetHandZone()->GetAll();
        if (std::ranges::none_of(hand, [](const Playable* playable) {
                return playable && playable->card &&
                       playable->card->GetRace() == Race::DRAGON;
            }))
        {
            return TaskStatus::STOP;
        }
    }

    std::vector<Candidate> candidates = GetCandidates(player, m_pool);
    if (candidates.empty())
    {
        return TaskStatus::STOP;
    }

    Random::shuffle(candidates.begin(), candidates.end());
    std::mt19937 randomEngine(Random::get<std::uint32_t>());
    std::vector<std::size_t> selected;
    std::vector<DarkGift> gifts;
    if (!SelectOptions(candidates, randomEngine, selected, gifts))
    {
        return TaskStatus::STOP;
    }

    auto choice = std::make_unique<Choice>(player);
    choice->choiceType = ChoiceType::GENERAL;
    choice->choiceAction = m_pool == DarkGiftPool::DECK_MINION
                               ? ChoiceAction::DRAW_FROM_DECK
                               : ChoiceAction::HAND;
    choice->source = m_source;

    for (std::size_t index = 0; index < selected.size(); ++index)
    {
        const Candidate& candidate = candidates[selected[index]];
        int entityID = 0;
        if (candidate.entity)
        {
            entityID = candidate.entity->GetGameTag(GameTag::ENTITY_ID);
        }
        else
        {
            Playable* option = Entity::GetFromCard(
                player, candidate.card, std::nullopt,
                player->GetSetasideZone());
            player->GetSetasideZone()->Add(option);
            entityID = option->GetGameTag(GameTag::ENTITY_ID);
        }

        choice->choices.emplace_back(entityID);
        choice->darkGiftOptionsByEntityID.emplace_back(entityID, gifts[index]);
    }

    player->choice = std::move(choice);
    return TaskStatus::COMPLETE;
}

std::unique_ptr<ITask> DarkGiftDiscoverTask::CloneImpl()
{
    return std::make_unique<DarkGiftDiscoverTask>(m_pool,
                                                  m_requiresDragonInHand);
}
}  // namespace RosettaStone::PlayMode::SimpleTasks
