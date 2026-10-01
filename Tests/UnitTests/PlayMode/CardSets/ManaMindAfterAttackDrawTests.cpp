// Copyright (c) 2017-2024 Chris Ohk

#include <doctest/doctest.h>

#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Cards/CardDefs.hpp>
#include <Rosetta/PlayMode/Triggers/Trigger.hpp>

#include <tuple>

using namespace RosettaStone;
using namespace RosettaStone::PlayMode;

TEST_CASE("[ManaMind after attack draw] - generated cards have reviewed trigger sources and tasks")
{
    for (const auto& [cardId, source, taskCount] : {
             std::tuple{"CAP_003", TriggerSource::SELF, 1},
             std::tuple{"EDR_253", TriggerSource::HERO, 1},
             std::tuple{"CORE_NX2_028", TriggerSource::HERO, 2},
             std::tuple{"TLC_478", TriggerSource::HERO, 1},
             std::tuple{"TLC_840", TriggerSource::SELF, 1}})
    {
        CAPTURE(cardId);
        REQUIRE(Cards::FindCardByID(cardId) != nullptr);
        REQUIRE(CardDefs::HasCardDefByID(cardId));
        const auto def = CardDefs::FindCardDefByID(cardId);
        REQUIRE(def.power.GetTrigger() != nullptr);
        CHECK(def.power.GetTrigger()->triggerSource == source);
        CHECK(def.power.GetTrigger()->tasks.size() == taskCount);
    }
}

TEST_CASE("[ManaMind after attack draw] - SI:7 Supplier draws after it attacks, not after another minion")
{
    GameConfig config;
    config.player1Class = CardClass::ROGUE;
    config.player2Class = CardClass::WARRIOR;
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

    const auto supplierCard = Cards::FindCardByID("CAP_003");
    const auto yetiCard = Cards::FindCardByName("Chillwind Yeti");
    REQUIRE(supplierCard != nullptr);
    REQUIRE(yetiCard != nullptr);
    auto supplier = Generic::DrawCard(current, supplierCard);
    auto yeti = Generic::DrawCard(current, yetiCard);
    game.Process(current, PlayCardTask::Minion(supplier));
    game.Process(current, PlayCardTask::Minion(yeti));

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    auto& hand = *current->GetHandZone();
    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 2);
    const int handBeforeOtherAttacks = hand.GetCount();
    game.Process(current, AttackTask(field[1], opponent->GetHero()));
    CHECK_EQ(hand.GetCount(), handBeforeOtherAttacks);

    const int handBeforeSupplierAttack = hand.GetCount();
    game.Process(current, AttackTask(field[0], opponent->GetHero()));
    CHECK_EQ(hand.GetCount(), handBeforeSupplierAttack + 1);
}

TEST_CASE("[ManaMind after attack draw] - Ursine Maul and Hookfist trigger on hero attacks")
{
    GameConfig config;
    config.player1Class = CardClass::PALADIN;
    config.player2Class = CardClass::WARRIOR;
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

    const auto maulCard = Cards::FindCardByID("EDR_253");
    const auto hookfistCard = Cards::FindCardByID("CORE_NX2_028");
    REQUIRE(maulCard != nullptr);
    REQUIRE(hookfistCard != nullptr);
    auto maul = Generic::DrawCard(current, maulCard);
    auto hookfist = Generic::DrawCard(current, hookfistCard);
    game.Process(current, PlayCardTask::Weapon(maul));
    game.Process(current, PlayCardTask::Minion(hookfist));

    auto& hand = *current->GetHandZone();
    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 1);
    const int armorBefore = current->GetHero()->GetArmor();
    const int handBefore = hand.GetCount();
    game.Process(current, AttackTask(current->GetHero(), opponent->GetHero()));
    CHECK_EQ(current->GetHero()->GetArmor(), armorBefore + 4);
    CHECK_EQ(hand.GetCount(), handBefore + 2);
}

TEST_CASE("[ManaMind after attack draw] - Axe of the Forefathers damages all minions after a hero attack")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::WARRIOR;
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

    const auto axe = Generic::DrawCard(current, Cards::FindCardByID("TLC_478"));
    const auto friendly = Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti"));
    const auto enemy = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(axe != nullptr);
    REQUIRE(friendly != nullptr);
    REQUIRE(enemy != nullptr);
    game.Process(current, PlayCardTask::Weapon(axe));
    game.Process(current, PlayCardTask::Minion(friendly));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, PlayCardTask::Minion(enemy));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(current, AttackTask(current->GetHero(), opponent->GetHero()));

    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 6);
}

TEST_CASE("[ManaMind after attack draw] - Gorishi Tunneler damages the enemy hero after it attacks")
{
    GameConfig config;
    config.player1Class = CardClass::DEMONHUNTER;
    config.player2Class = CardClass::WARRIOR;
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
    const auto tunneler = Generic::DrawCard(current, Cards::FindCardByID("TLC_840"));
    REQUIRE(tunneler != nullptr);
    game.Process(current, PlayCardTask::Minion(tunneler));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto healthBefore = opponent->GetHero()->GetHealth();
    game.Process(current, AttackTask(current->GetFieldZone()->operator[](0), opponent->GetHero()));

    CHECK_EQ(opponent->GetHero()->GetHealth(), healthBefore - 4);
}
