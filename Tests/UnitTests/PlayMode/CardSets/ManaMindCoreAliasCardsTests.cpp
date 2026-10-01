// Copyright (c) 2017-2024 Chris Ohk

#include <doctest/doctest.h>

#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Cards/CardDefs.hpp>

#include <array>
#include <string_view>

using namespace RosettaStone;
using namespace RosettaStone::PlayMode;

TEST_CASE("[ManaMind definition aliases] - all declared aliases preserve their base CardDef")
{
    constexpr std::array<std::pair<std::string_view, std::string_view>, 65> aliases{{
        {"CORE_BAR_801", "BAR_801"},
        {"CORE_SW_108", "SW_108"},
        {"CORE_BT_072", "BT_072"},
        {"CORE_BAR_310", "BAR_310"},
        {"CORE_AV_337", "AV_337"},
        {"CORE_BAR_541", "BAR_541"},
        {"CORE_KAR_062", "KAR_062"},
        {"CORE_BT_156", "BT_156"},
        {"CORE_SW_068", "SW_068"},
        {"CORE_SW_088", "SW_088"},
        {"CORE_EX1_131", "EX1_131"},
        {"CORE_EX1_278", "EX1_278"},
        {"CORE_DRG_107", "DRG_107"},
        {"CORE_WC_701", "WC_701"},
        {"CORE_BT_701", "BT_701"},
        {"CORE_BAR_313", "BAR_313"},
        {"CORE_EX1_058", "EX1_058"},
        {"CORE_ONY_018", "ONY_018"},
        {"CORE_SW_439", "SW_439"},
        {"CORE_TSC_650", "TSC_650"},
        {"CORE_ULD_133", "ULD_133"},
        {"CORE_BAR_812", "BAR_812"},
        {"CORE_BAR_878", "BAR_878"},
        {"CORE_BT_510", "BT_510"},
        {"CORE_EX1_559", "EX1_559"},
        {"CORE_WC_042", "WC_042"},
        {"CORE_BT_120", "BT_120"},
        {"CORE_BT_321", "BT_321"},
        {"CORE_BT_493", "BT_493"},
        {"CORE_DAL_575", "DAL_575"},
        {"CORE_DAL_720", "DAL_720"},
        {"CORE_DRG_024", "DRG_024"},
        {"CORE_EX1_100", "EX1_100"},
        {"CORE_KAR_057", "KAR_057"},
        {"CORE_ONY_022", "ONY_022"},
        {"CORE_REV_308", "REV_308"},
        {"CORE_SCH_717", "SCH_717"},
        {"CORE_ULD_152", "ULD_152"},
        {"CORE_ULD_165", "ULD_165"},
        {"CORE_ULD_280", "ULD_280"},
        {"CORE_BT_292", "BT_292"},
        {"CORE_DRG_256", "DRG_256"},
        {"CORE_EX1_014", "EX1_014"},
        {"CORE_EX1_189", "EX1_189"},
        {"CORE_EX1_198", "EX1_198"},
        {"CORE_EX1_310", "EX1_310"},
        {"CORE_KAR_077", "KAR_077"},
        {"CORE_NEW1_022", "NEW1_022"},
        {"CORE_SCH_713", "SCH_713"},
        {"CORE_SW_072", "SW_072"},
        {"CORE_SW_429", "SW_429"},
        {"CORE_TRL_111", "TRL_111"},
        {"CORE_ULD_178", "ULD_178"},
        {"CORE_UNG_809", "UNG_809"},
        {"CORE_UNG_912", "UNG_912"},
        {"CORE_SW_442", "SW_442"},
        {"CORE_AT_052", "EX1_247"},
        {"CORE_WON_096", "LOE_023"},
        {"CORE_WON_337", "KAR_091"},
        {"Core_LOE_115", "LOE_115"},
        {"Core_UNG_072", "ULD_195"},
        {"TIME_603", "REV_251"},
        {"TIME_720", "ULD_189"},
        {"TLC_483", "ULD_309"},
        {"CORE_EX1_002", "EX1_002"},
    }};

    for (const auto& [coreId, baseId] : aliases)
    {
        CAPTURE(coreId);
        CAPTURE(baseId);
        REQUIRE(Cards::FindCardByID(coreId) != nullptr);
        REQUIRE(Cards::FindCardByID(baseId) != nullptr);
        REQUIRE(CardDefs::HasCardDefByID(coreId));

        auto coreDef = CardDefs::FindCardDefByID(coreId);
        auto baseDef = CardDefs::FindCardDefByID(baseId);
        CHECK(coreDef.property.playReqs == baseDef.property.playReqs);
        CHECK(coreDef.property.chooseCardIDs == baseDef.property.chooseCardIDs);
        CHECK(coreDef.property.entourages == baseDef.property.entourages);
        CHECK(coreDef.property.appendages == baseDef.property.appendages);
        CHECK(coreDef.property.corruptCardID == baseDef.property.corruptCardID);
        CHECK(coreDef.property.infusedCardID == baseDef.property.infusedCardID);
        CHECK(coreDef.power.GetPowerTask().size() == baseDef.power.GetPowerTask().size());
        CHECK(coreDef.power.GetDeathrattleTask().size() == baseDef.power.GetDeathrattleTask().size());
        CHECK((coreDef.power.GetTrigger() != nullptr) == (baseDef.power.GetTrigger() != nullptr));
    }
}

TEST_CASE("[ManaMind Core aliases] - CORE_ULD_133 Crystal Merchant draws only with unspent mana")
{
    GameConfig config;
    config.player1Class = CardClass::DRUID;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);

    auto& currentHand = *current->GetHandZone();
    auto* merchantCard = Cards::FindCardByID("CORE_ULD_133");
    REQUIRE(merchantCard != nullptr);
    const auto merchant = Generic::DrawCard(current, merchantCard);
    game.Process(current, PlayCardTask::Minion(merchant));

    const auto beforeEndTurn = currentHand.GetCount();
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    // The trigger draws one additional card at end of turn.
    CHECK_EQ(currentHand.GetCount(), beforeEndTurn + 1);

    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(10);
    const auto beforeSpentTurn = currentHand.GetCount();
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    // The trigger does not draw when all mana has been spent; only the next turn draw occurs.
    CHECK_EQ(currentHand.GetCount(), beforeSpentTurn);
}

TEST_CASE("[ManaMind Core aliases] - CORE_SW_108 First Flame preserves damage and generated card")
{
    GameConfig config;
    config.player1Class = CardClass::MAGE;
    config.player2Class = CardClass::PALADIN;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);

    auto& currentHand = *current->GetHandZone();
    auto& opponentField = *opponent->GetFieldZone();
    auto* firstFlameCard = Cards::FindCardByID("CORE_SW_108");
    auto* yetiCard = Cards::FindCardByName("Chillwind Yeti");
    REQUIRE(firstFlameCard != nullptr);
    REQUIRE(yetiCard != nullptr);
    const auto firstFlame = Generic::DrawCard(current, firstFlameCard);
    const auto target = Generic::DrawCard(opponent, yetiCard);

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, PlayCardTask::Minion(target));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(current, PlayCardTask::SpellTarget(firstFlame, target));

    CHECK_EQ(opponentField[0]->GetHealth(), 3);
    REQUIRE_EQ(currentHand.GetCount(), 1);
    CHECK_EQ(currentHand[0]->card->id, "SW_108t");
}

TEST_CASE("[ManaMind Core aliases] - CORE_BAR_801 Wound Prey preserves damage and Rush token")
{
    GameConfig config;
    config.player1Class = CardClass::HUNTER;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);

    auto& currentField = *current->GetFieldZone();
    auto& opponentField = *opponent->GetFieldZone();
    auto* woundPreyCard = Cards::FindCardByID("CORE_BAR_801");
    auto* malygosCard = Cards::FindCardByName("Malygos");
    REQUIRE(woundPreyCard != nullptr);
    REQUIRE(malygosCard != nullptr);
    const auto woundPrey = Generic::DrawCard(current, woundPreyCard);
    const auto target = Generic::DrawCard(opponent, malygosCard);

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, PlayCardTask::Minion(target));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(current, PlayCardTask::SpellTarget(woundPrey, target));

    REQUIRE_EQ(currentField.GetCount(), 1);
    CHECK_EQ(currentField[0]->card->id, "BAR_035t");
    CHECK_EQ(currentField[0]->GetAttack(), 1);
    CHECK_EQ(currentField[0]->GetHealth(), 1);
    CHECK(currentField[0]->HasRush());
    CHECK_EQ(opponentField[0]->GetHealth(), 11);
}

TEST_CASE("[ManaMind Core aliases] - CORE_SW_088 Demonic Assault deals damage and summons Taunt tokens")
{
    GameConfig config;
    config.player1Class = CardClass::WARLOCK;
    config.player2Class = CardClass::HUNTER;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);

    auto& currentField = *current->GetFieldZone();
    const auto assaultCard = Cards::FindCardByID("CORE_SW_088");
    REQUIRE(assaultCard != nullptr);
    const auto assault = Generic::DrawCard(current, assaultCard);

    game.Process(current, PlayCardTask::SpellTarget(assault, opponent->GetHero()));

    CHECK_EQ(opponent->GetHero()->GetHealth(), 27);
    REQUIRE_EQ(currentField.GetCount(), 2);
    for (int i = 0; i < currentField.GetCount(); ++i)
    {
        const auto minion = currentField[i];
        CHECK_EQ(minion->card->id, "CS2_065");
        CHECK_EQ(minion->GetAttack(), 1);
        CHECK_EQ(minion->GetHealth(), 3);
        CHECK(minion->HasTaunt());
    }
}

TEST_CASE("[ManaMind Core aliases] - CORE_EX1_278 Shiv deals damage and draws")
{
    GameConfig config;
    config.player1Class = CardClass::ROGUE;
    config.player2Class = CardClass::PALADIN;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);

    auto& currentHand = *current->GetHandZone();
    auto& opponentField = *opponent->GetFieldZone();
    const auto shivCard = Cards::FindCardByID("CORE_EX1_278");
    auto* yetiCard = Cards::FindCardByName("Chillwind Yeti");
    REQUIRE(shivCard != nullptr);
    REQUIRE(yetiCard != nullptr);
    const auto shiv = Generic::DrawCard(current, shivCard);
    const auto target = Generic::DrawCard(opponent, yetiCard);

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, PlayCardTask::Minion(target));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    const auto handBefore = currentHand.GetCount();
    game.Process(current, PlayCardTask::SpellTarget(shiv, target));

    CHECK_EQ(opponentField[0]->GetHealth(), 4);
    CHECK_EQ(currentHand.GetCount(), handBefore);
}

TEST_CASE("[ManaMind Core aliases] - CORE_EX1_058 Sunfury Protector gives Taunt to adjacent minions")
{
    GameConfig config;
    config.player1Class = CardClass::MAGE;
    config.player2Class = CardClass::PALADIN;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);

    auto& currentField = *current->GetFieldZone();
    const auto yetiCard = Cards::FindCardByName("Chillwind Yeti");
    const auto sunfuryCard = Cards::FindCardByID("CORE_EX1_058");
    REQUIRE(yetiCard != nullptr);
    REQUIRE(sunfuryCard != nullptr);
    const auto yeti = Generic::DrawCard(current, yetiCard);
    const auto sunfury = Generic::DrawCard(current, sunfuryCard);

    game.Process(current, PlayCardTask::Minion(yeti));
    game.Process(current, PlayCardTask::Minion(sunfury));

    REQUIRE_EQ(currentField.GetCount(), 2);
    CHECK_EQ(currentField[0]->card->id, "CORE_CS2_182");
    CHECK(currentField[0]->HasTaunt());
}
