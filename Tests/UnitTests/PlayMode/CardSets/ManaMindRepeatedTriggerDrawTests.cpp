// Copyright (c) 2017-2024 Chris Ohk

#include <doctest/doctest.h>

#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Cards/CardDefs.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DestroyTask.hpp>

using namespace RosettaStone;
using namespace RosettaStone::PlayMode;
using namespace RosettaStone::PlayMode::SimpleTasks;

TEST_CASE("[ManaMind repeated trigger draw] - both cards register Battlecry and Deathrattle tasks")
{
    for (const auto& [cardId, taskCount] : {
             std::pair{"CORE_DMF_067", 2}, std::pair{"RLK_708", 1},
             std::pair{"CORE_RLK_657", 1}})
    {
        CAPTURE(cardId);
        REQUIRE(Cards::FindCardByID(cardId) != nullptr);
        REQUIRE(CardDefs::HasCardDefByID(cardId));
        auto def = CardDefs::FindCardDefByID(cardId);
        CHECK_EQ(def.power.GetPowerTask().size(), taskCount);
        CHECK_EQ(def.power.GetDeathrattleTask().size(), taskCount);
    }
}

TEST_CASE("[ManaMind repeated trigger draw] - Prize Vendor makes each player draw on play and death")
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
    auto& hand = *current->GetHandZone();
    auto& opponentHand = *opponent->GetHandZone();
    const auto prizeVendorCard = Cards::FindCardByID("CORE_DMF_067");
    REQUIRE(prizeVendorCard != nullptr);
    const auto prizeVendor = Generic::DrawCard(current, prizeVendorCard);
    const int ownHandBeforePlay = hand.GetCount();
    const int opponentHandBeforePlay = opponentHand.GetCount();

    game.Process(current, PlayCardTask::Minion(prizeVendor));
    CHECK_EQ(hand.GetCount(), ownHandBeforePlay);
    CHECK_EQ(opponentHand.GetCount(), opponentHandBeforePlay + 1);

    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 1);
    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(field[0]);
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(field.GetCount(), 0);
    CHECK_EQ(hand.GetCount(), ownHandBeforePlay + 1);
    CHECK_EQ(opponentHand.GetCount(), opponentHandBeforePlay + 2);
}

TEST_CASE("[ManaMind repeated trigger draw] - Chillfallen Baron draws for its controller on both triggers")
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
    auto& hand = *current->GetHandZone();
    auto& opponentHand = *opponent->GetHandZone();
    const auto baronCard = Cards::FindCardByID("RLK_708");
    REQUIRE(baronCard != nullptr);
    const auto baron = Generic::DrawCard(current, baronCard);
    const int ownHandBeforePlay = hand.GetCount();
    const int opponentHandBeforePlay = opponentHand.GetCount();

    game.Process(current, PlayCardTask::Minion(baron));
    CHECK_EQ(hand.GetCount(), ownHandBeforePlay);
    CHECK_EQ(opponentHand.GetCount(), opponentHandBeforePlay);

    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 1);
    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(field[0]);
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(field.GetCount(), 0);
    CHECK_EQ(hand.GetCount(), ownHandBeforePlay + 1);
    CHECK_EQ(opponentHand.GetCount(), opponentHandBeforePlay);
}

TEST_CASE("[ManaMind repeated trigger draw] - Underking gains armor on Battlecry and Deathrattle")
{
    GameConfig config;
    config.player1Class = CardClass::DRUID;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto underkingCard = Cards::FindCardByID("CORE_RLK_657");
    REQUIRE(underkingCard != nullptr);
    const auto underking = Generic::DrawCard(current, underkingCard);
    const int armorBeforePlay = current->GetHero()->GetArmor();

    game.Process(current, PlayCardTask::Minion(underking));
    CHECK_EQ(current->GetHero()->GetArmor(), armorBeforePlay + 6);
    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 1);
    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(field[0]);
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(field.GetCount(), 0);
    CHECK_EQ(current->GetHero()->GetArmor(), armorBeforePlay + 12);
}
