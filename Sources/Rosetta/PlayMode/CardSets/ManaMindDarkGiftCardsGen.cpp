// ManaMind's audited current Standard Dark Gift slice.
#include <Rosetta/PlayMode/CardSets/ManaMindDarkGiftCardsGen.hpp>

#include <Rosetta/PlayMode/Tasks/SimpleTasks/DarkGiftDiscoverTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DamageTask.hpp>

#include <stdexcept>
#include <memory>

namespace RosettaStone::PlayMode
{
void ManaMindDarkGiftCardsGen::AddAll(std::map<std::string, CardDef>& cards)
{
    using namespace SimpleTasks;

    if (cards.contains("EDR_456") || cards.contains("FIR_939") ||
        cards.contains("EDR_856"))
    {
        throw std::logic_error("duplicate ManaMind Dark Gift CardDef");
    }

    CardDef darkrider;
    darkrider.ClearData();
    darkrider.power.AddPowerTask(std::make_shared<DarkGiftDiscoverTask>(
        DarkGiftPool::DRAGON, true));
    cards.emplace("EDR_456", std::move(darkrider));

    CardDef suffusion;
    suffusion.ClearData();
    suffusion.power.AddPowerTask(
        std::make_shared<DamageTask>(EntityType::ENEMY_HERO, 2, true));
    suffusion.power.AddPowerTask(
        std::make_shared<DarkGiftDiscoverTask>(DarkGiftPool::WARRIOR));
    cards.emplace("FIR_939", std::move(suffusion));

    CardDef xavius;
    xavius.ClearData();
    xavius.power.AddPowerTask(
        std::make_shared<DarkGiftDiscoverTask>(DarkGiftPool::DECK_MINION));
    cards.emplace("EDR_856", std::move(xavius));
}
}  // namespace RosettaStone::PlayMode
