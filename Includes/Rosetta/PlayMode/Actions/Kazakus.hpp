#ifndef ROSETTASTONE_PLAYMODE_KAZAKUS_HPP
#define ROSETTASTONE_PLAYMODE_KAZAKUS_HPP

#include <Rosetta/PlayMode/Actions/Attack.hpp>
#include <Rosetta/PlayMode/Actions/Generic.hpp>
#include <Rosetta/PlayMode/Cards/Cards.hpp>
#include <Rosetta/PlayMode/Enchants/Effects.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Models/Minion.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/ControlTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/SummonTask.hpp>
#include <Rosetta/PlayMode/Zones/FieldZone.hpp>
#include <Rosetta/PlayMode/Zones/HandZone.hpp>

#include <effolkronium/random.hpp>

namespace RosettaStone::PlayMode::Generic
{
inline void ResolveKazakusEffect(Player* player, Entity* source, int effect)
{
    using Random = effolkronium::random_static;
    switch (effect)
    {
        case 1:
        {
            SimpleTasks::SummonTask task("CORE_LOOT_368");
            task.SetPlayer(player);
            task.SetSource(source);
            task.Run();
            break;
        }
        case 2:
            player->GetHero()->TakeHeal(dynamic_cast<Playable*>(source), 12);
            break;
        case 3:
        {
            SimpleTasks::DrawTask task(3);
            task.SetPlayer(player);
            task.SetSource(source);
            task.Run();
            break;
        }
        case 4:
            for (Playable* card : player->GetHandZone()->GetAll())
            {
                if (dynamic_cast<Minion*>(card))
                {
                    Effects::ReduceCost(2)->ApplyTo(card);
                }
            }
            break;
        case 5:
        {
            std::vector<Card*> candidates;
            for (Card* card : Cards::GetAllStandardCards())
            {
                if (card->GetCardType() == CardType::MINION &&
                    card->gameTags[GameTag::COST] == 3)
                {
                    candidates.emplace_back(card);
                }
            }
            if (!candidates.empty())
            {
                for (int i = 0; i < 3; ++i)
                {
                    const auto selectedIndex =
                        Random::get<std::size_t>(0, candidates.size() - 1);
                    Card* selected = candidates[selectedIndex];
                    SimpleTasks::SummonTask task(selected->id);
                    task.SetPlayer(player);
                    task.SetSource(source);
                    task.Run();
                }
            }
            break;
        }
        case 6:
        {
            const auto buff = [](Playable* card) {
                if (auto* minion = dynamic_cast<Minion*>(card))
                {
                    minion->SetAttack(minion->GetAttack() + 3);
                    minion->SetBaseHealth(minion->GetBaseHealth() + 3);
                }
            };
            for (Playable* card : player->GetHandZone()->GetAll())
                buff(card);
            for (Minion* minion : player->GetFieldZone()->GetMinions())
                buff(minion);
            break;
        }
        case 7:
        {
            const auto enemies = player->opponent->GetFieldZone()->GetMinions();
            if (!enemies.empty())
            {
                Minion* target =
                    enemies[Random::get<std::size_t>(0, enemies.size() - 1)];
                SimpleTasks::ControlTask task(EntityType::TARGET);
                task.SetPlayer(player);
                task.SetSource(source);
                task.SetTarget(target);
                task.Run();
            }
            break;
        }
        case 8:
            for (int i = 0; i < 2; ++i)
            {
                const auto hand = player->opponent->GetHandZone()->GetAll();
                if (hand.empty())
                    break;
                Playable* stolen =
                    hand[Random::get<std::size_t>(0, hand.size() - 1)];
                player->opponent->GetHandZone()->Remove(stolen);
                stolen->player = player;
                AddCardToHand(player, stolen);
            }
            break;
        case 9:
        {
            std::vector<Minion*> minions = player->GetFieldZone()->GetMinions();
            const auto enemyMinions =
                player->opponent->GetFieldZone()->GetMinions();
            minions.insert(minions.end(), enemyMinions.begin(),
                           enemyMinions.end());
            for (Minion* attacker : minions)
            {
                std::vector<Minion*> targets;
                for (Minion* minion : player->GetFieldZone()->GetMinions())
                    if (minion != attacker)
                        targets.emplace_back(minion);
                for (Minion* minion :
                     player->opponent->GetFieldZone()->GetMinions())
                    if (minion != attacker)
                        targets.emplace_back(minion);
                if (!targets.empty() &&
                    attacker->GetZoneType() == ZoneType::PLAY)
                {
                    Minion* target = targets[Random::get<std::size_t>(
                        0, targets.size() - 1)];
                    Attack(attacker->player, attacker, target, true);
                    player->game->ProcessDestroyAndUpdateAura();
                }
            }
            break;
        }
        default:
            break;
    }
}
}  // namespace RosettaStone::PlayMode::Generic

#endif  // ROSETTASTONE_PLAYMODE_KAZAKUS_HPP
