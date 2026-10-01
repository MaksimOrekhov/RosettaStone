// ManaMind's explicitly reviewed current Standard Dragon pool registrations.
#include <Rosetta/PlayMode/CardSets/ManaMindDragonPoolCardsGen.hpp>
#include <Rosetta/PlayMode/Cards/CardPowers.hpp>

#include <stdexcept>
#include <string>
#include <utility>

namespace RosettaStone::PlayMode
{
void ManaMindDragonPoolCardsGen::AddAll(std::map<std::string, CardDef>& cards)
{
    if (cards.contains("TIME_045") || cards.contains("TIME_056") ||
        cards.contains("TIME_856") || cards.contains("CORE_AT_123"))
    {
        throw std::logic_error("duplicate ManaMind Dragon pool CardDef");
    }

    CardDef whelpOfTheInfinite;
    whelpOfTheInfinite.ClearData();
    cards.emplace("TIME_045", std::move(whelpOfTheInfinite));

    CardDef whelpOfTheBronze;
    whelpOfTheBronze.ClearData();
    cards.emplace("TIME_056", std::move(whelpOfTheBronze));

    // Spell Damage is loaded from the card's spellDamage metadata field.
    CardDef algetharInstructor;
    algetharInstructor.ClearData();
    cards.emplace("TIME_856", std::move(algetharInstructor));

    // Chillmaw's Deathrattle only deals damage while its controller holds a Dragon.
    CardDef chillmaw;
    chillmaw.ClearData();
    chillmaw.power.AddDeathrattleTask(std::make_shared<ConditionTask>(
        EntityType::SOURCE, SelfCondList{ std::make_shared<SelfCondition>(
                                SelfCondition::IsHoldingRace(Race::DRAGON)) }));
    chillmaw.power.AddDeathrattleTask(std::make_shared<FlagTask>(
        true, TaskList{ std::make_shared<DamageTask>(EntityType::ALL_MINIONS, 3) }));
    cards.emplace("CORE_AT_123", std::move(chillmaw));
}
}  // namespace RosettaStone::PlayMode
