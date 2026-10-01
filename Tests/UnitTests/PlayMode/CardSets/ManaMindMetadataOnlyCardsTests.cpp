// Copyright (c) 2017-2024 Chris Ohk

#include <doctest/doctest.h>

#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Cards/CardDefs.hpp>

#include <array>
#include <string_view>
#include <tuple>

using namespace RosettaStone;
using namespace RosettaStone::PlayMode;

TEST_CASE("[ManaMind metadata-only cards] - textless vanilla Standard minions use their printed stats")
{
    GameConfig config;
    config.player1Class = CardClass::MAGE;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(30);
    current->SetUsedMana(0);
    const std::array<std::tuple<std::string_view, int, int>, 3> cases{{
        {"Core_CS2_200", 6, 7},
        {"TIME_053", 7, 2},
        {"TLC_248", 14, 28},
    }};
    for (const auto& [cardId, attack, health] : cases)
    {
        CAPTURE(cardId);
        const auto card = Cards::FindCardByID(cardId);
        REQUIRE(card != nullptr);
        REQUIRE(CardDefs::HasCardDefByID(cardId));
        const auto minion = Generic::DrawCard(current, card);
        REQUIRE(minion != nullptr);
        game.Process(current, PlayCardTask::Minion(minion));
        auto& field = *current->GetFieldZone();
        REQUIRE(field.GetCount() > 0);
        auto* summoned = field[field.GetCount() - 1];
        CHECK_EQ(summoned->GetAttack(), attack);
        CHECK_EQ(summoned->GetHealth(), health);
    }
}
