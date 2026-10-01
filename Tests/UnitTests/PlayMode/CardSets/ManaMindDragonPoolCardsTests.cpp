#include <doctest/doctest.h>

#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Cards/CardDefs.hpp>

#include <array>
#include <set>
#include <string_view>

using namespace RosettaStone;
using namespace RosettaStone::PlayMode;

TEST_CASE("[ManaMind Dragon pool] - current keyword-only outcomes are registered")
{
    struct ExpectedCard
    {
        std::string_view id;
        int attack;
        int health;
        std::array<GameTag, 2> keywords;
    };

    constexpr std::array<ExpectedCard, 2> expected{{
        { "TIME_045", 1, 4, { GameTag::POISONOUS, GameTag::REBORN } },
        { "TIME_056", 4, 1, { GameTag::LIFESTEAL, GameTag::DIVINE_SHIELD } },
    }};

    for (const auto& item : expected)
    {
        CAPTURE(item.id);
        const Card* card = Cards::FindCardByID(item.id);
        REQUIRE(card != nullptr);
        REQUIRE(card->IsCollectible());
        REQUIRE(card->IsStandardSet());
        CHECK_EQ(card->gameTags.at(GameTag::ATK), item.attack);
        CHECK_EQ(card->gameTags.at(GameTag::HEALTH), item.health);
        CHECK(CardDefs::HasCardDefByID(item.id));

        for (const auto keyword : item.keywords)
        {
            CAPTURE(keyword);
            CHECK(card->HasGameTag(keyword));
        }

        auto definition = CardDefs::FindCardDefByID(item.id);
        CHECK(definition.power.GetPowerTask().empty());
        CHECK(definition.power.GetTrigger() == nullptr);
        CHECK(definition.power.GetAura() == nullptr);
    }

    const Card* instructor = Cards::FindCardByID("TIME_856");
    REQUIRE(instructor != nullptr);
    CHECK(instructor->IsCollectible());
    CHECK(instructor->IsStandardSet());
    CHECK_EQ(instructor->gameTags.at(GameTag::ATK), 4);
    CHECK_EQ(instructor->gameTags.at(GameTag::HEALTH), 7);
    CHECK_EQ(instructor->gameTags.at(GameTag::SPELLPOWER), 1);
    CHECK(CardDefs::HasCardDefByID("TIME_856"));
    auto instructorDefinition = CardDefs::FindCardDefByID("TIME_856");
    CHECK(instructorDefinition.power.GetPowerTask().empty());
    CHECK(instructorDefinition.power.GetTrigger() == nullptr);
    CHECK(instructorDefinition.power.GetAura() == nullptr);
}

TEST_CASE("[ManaMind Dragon pool] - Chillmaw has its conditional board-clear deathrattle")
{
    const Card* chillmaw = Cards::FindCardByID("CORE_AT_123");
    REQUIRE(chillmaw != nullptr);
    CHECK(chillmaw->IsCollectible());
    CHECK(chillmaw->IsStandardSet());
    CHECK_EQ(chillmaw->GetCost(), 7);
    CHECK_EQ(chillmaw->gameTags.at(GameTag::ATK), 6);
    CHECK_EQ(chillmaw->gameTags.at(GameTag::HEALTH), 6);
    CHECK(chillmaw->HasGameTag(GameTag::TAUNT));
    CHECK_EQ(chillmaw->GetRace(), Race::DRAGON);
    REQUIRE(CardDefs::HasCardDefByID("CORE_AT_123"));

    auto definition = CardDefs::FindCardDefByID("CORE_AT_123");
    CHECK(definition.power.GetDeathrattleTask().size() == 2);
    CHECK(definition.power.GetPowerTask().empty());
}

TEST_CASE("[ManaMind Dragon pool] - loaded Standard pools match audited counts")
{
    const std::set<std::string_view> expectedDragons{
        "CATA_111", "CATA_132", "CATA_133", "CATA_140", "CATA_154",
        "CATA_155", "CATA_160", "CATA_201", "CATA_305", "CATA_307",
        "CATA_432", "CATA_464", "CATA_469", "CATA_474", "CATA_476",
        "CATA_484", "CATA_494", "CATA_497", "CATA_556", "CATA_614",
        "CATA_723", "CATA_898", "CATA_999", "CORE_AT_123", "CORE_DRG_079",
        "CORE_EX1_043", "CORE_EX1_189", "CORE_LOOT_137", "CORE_NEW1_023",
        "CORE_UNG_848", "CORE_YOP_034", "CS3_035", "EDR_000", "EDR_256",
        "EDR_260", "EDR_451", "EDR_453", "EDR_459", "EDR_462", "EDR_465",
        "EDR_571", "EDR_572", "EDR_889", "EDR_890", "END_006", "END_010",
        "END_018", "END_021", "END_022", "END_030", "END_032", "END_033",
        "END_034", "END_035", "FIR_901", "FIR_927", "FIR_959", "FIR_960",
        "MEND_045", "TIME_003", "TIME_004", "TIME_024", "TIME_025", "TIME_028",
        "TIME_029", "TIME_032", "TIME_034", "TIME_045", "TIME_051", "TIME_052",
        "TIME_056", "TIME_063", "TIME_435", "TIME_714", "TIME_720", "TIME_852",
        "TIME_856", "TIME_871", "TIME_EVENT_301", "TLC_600"
    };
    const std::set<std::string_view> expectedLowCostDragons{
        "CATA_111", "CATA_464", "CATA_484", "CATA_556", "CATA_614",
        "CORE_EX1_189", "CORE_NEW1_023", "EDR_451", "EDR_571", "EDR_889",
        "EDR_890", "END_018", "END_021", "END_022", "FIR_927", "TIME_003",
        "TIME_025", "TIME_045", "TIME_056", "TIME_063"
    };
    const std::set<std::string_view> expectedWarriorMinions{
        "CAP_104", "CAP_106", "CAP_107", "CATA_150", "CATA_160", "CATA_586",
        "CATA_591", "CATA_EVENT_002", "CORE_BT_120", "CORE_DRG_024",
        "CORE_EX1_414", "CORE_EX1_604", "CORE_NX2_028", "CORE_OG_149",
        "CORE_OG_218", "CORE_WW_329", "DINO_400", "DINO_401", "EDR_456",
        "EDR_457", "EDR_459", "EDR_465", "EDR_468", "EDR_471", "END_021",
        "FIR_928", "FIR_956", "JAIL_029", "JAIL_311", "JAIL_384", "JAIL_421",
        "JAIL_435", "JAIL_455", "TIME_034", "TIME_714", "TIME_850", "TIME_871",
        "TIME_872", "TLC_600", "TLC_606", "TLC_623", "TLC_624"
    };
    const std::set<std::string_view> expectedFireSpells{
        "CATA_303", "CATA_581", "CATA_582", "CATA_585", "CORE_CS2_029",
        "CORE_CS2_032", "CORE_CS2_062", "CORE_EX1_610", "CORE_GIL_836",
        "CORE_LOOT_101", "CORE_SW_108", "CORE_WON_337", "DINO_406", "END_024",
        "END_025", "FIR_900", "FIR_906", "FIR_909", "FIR_910", "FIR_911",
        "FIR_914", "FIR_916", "FIR_920", "FIR_923", "FIR_939", "FIR_941",
        "FIR_954", "JAIL_307", "JAIL_801", "TLC_221", "TLC_222", "TLC_227",
        "TLC_632"
    };

    std::set<std::string_view> dragons;
    std::set<std::string_view> lowCostDragons;
    std::set<std::string_view> warriorMinions;
    std::set<std::string_view> eligibleFireSpells;

    for (const Card* card : Cards::GetAllStandardCards())
    {
        if (card->GetCardType() == CardType::MINION &&
            card->GetRace() == Race::DRAGON)
        {
            dragons.insert(card->id);
            if (card->GetCost() <= 3)
            {
                lowCostDragons.insert(card->id);
            }
        }

        if (card->GetCardType() == CardType::MINION &&
            card->GetCardClass() == CardClass::WARRIOR)
        {
            warriorMinions.insert(card->id);
        }

        if (card->GetCardType() == CardType::SPELL &&
            card->GetSpellSchool() == SpellSchool::FIRE && !card->IsQuest() &&
            card->GetCost() > 0)
        {
            eligibleFireSpells.insert(card->id);
        }
    }

    CHECK_EQ(dragons, expectedDragons);
    CHECK_EQ(lowCostDragons, expectedLowCostDragons);
    CHECK_EQ(warriorMinions, expectedWarriorMinions);
    CHECK_EQ(eligibleFireSpells, expectedFireSpells);

    const Card* currentCoreDragon = Cards::FindCardByID("CORE_DRG_079");
    const Card* retiredCoreDragon = Cards::FindCardByID("CORE_EX1_284");
    REQUIRE(currentCoreDragon != nullptr);
    REQUIRE(retiredCoreDragon != nullptr);
    CHECK(currentCoreDragon->IsStandardSet());
    CHECK_FALSE(retiredCoreDragon->IsStandardSet());
    CHECK(retiredCoreDragon->IsWildSet());
}
