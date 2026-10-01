// Copyright (c) 2026 OpenAI

#include <doctest/doctest.h>

#include <Rosetta/PlayMode/Actions/Draw.hpp>
#include <Rosetta/PlayMode/Cards/Cards.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Tasks/PlayerTasks/PlayCardTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DestroyTask.hpp>
#include <Rosetta/PlayMode/Zones/DeckZone.hpp>
#include <Rosetta/PlayMode/Zones/FieldZone.hpp>
#include <Rosetta/PlayMode/Zones/HandZone.hpp>

#include <algorithm>

using namespace RosettaStone;
using namespace PlayMode;
using namespace PlayerTasks;
using namespace SimpleTasks;

namespace
{
int CountSchool(const std::vector<Playable*>& cards, SpellSchool school)
{
    return static_cast<int>(std::count_if(cards.begin(), cards.end(),
        [school](const Playable* card) {
            return card->card->GetCardType() == CardType::SPELL &&
                   card->card->GetSpellSchool() == school;
        }));
}

void VerifyDeathrattleSchoolDraw(const std::string& minionId,
                                 const std::string& deckSpellId,
                                 const std::string& opponentSpellId,
                                 SpellSchool school, bool hasMatchingCard)
{
    GameConfig config;
    config.player1Class = CardClass::MAGE;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Card* deckSpell = Cards::FindCardByID(deckSpellId);
    Card* opponentSpell = Cards::FindCardByID(opponentSpellId);
    REQUIRE(deckSpell != nullptr);
    REQUIRE(opponentSpell != nullptr);
    for (int i = 0; i < 30; ++i)
    {
        config.player1Deck[i] = deckSpell;
        config.player2Deck[i] = opponentSpell;
    }

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* player = game.GetCurrentPlayer();
    player->SetTotalMana(10);
    player->SetUsedMana(0);
    Card* minionCard = Cards::FindCardByID(minionId);
    REQUIRE(minionCard != nullptr);
    Playable* minion = Generic::DrawCard(player, minionCard);
    game.Process(player, PlayCardTask::Minion(minion));

    auto& field = *player->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 1);
    auto& deck = *player->GetDeckZone();
    auto& hand = *player->GetHandZone();
    const int handCountBefore = hand.GetCount();
    const int matchingHandBefore = CountSchool(hand.GetAll(), school);
    const int deckCountBefore = deck.GetCount();

    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(player);
    destroy.SetSource(field[0]);
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();

    CHECK_EQ(field.GetCount(), 0);
    CHECK_EQ(hand.GetCount(), handCountBefore + (hasMatchingCard ? 1 : 0));
    CHECK_EQ(deck.GetCount(), deckCountBefore - (hasMatchingCard ? 1 : 0));
    CHECK_EQ(CountSchool(hand.GetAll(), school), matchingHandBefore + (hasMatchingCard ? 1 : 0));
}
}  // namespace

TEST_CASE("[ManaMind filtered school draw] - Living Flame draws a Fire spell")
{
    VerifyDeathrattleSchoolDraw("FIR_929", "CORE_CS2_029", "CORE_CS2_024",
                                SpellSchool::FIRE, true);
}

TEST_CASE("[ManaMind filtered school draw] - Harbinger of Winter draws a Frost spell")
{
    VerifyDeathrattleSchoolDraw("RLK_511", "CORE_CS2_024", "CORE_CS2_029",
                                SpellSchool::FROST, true);
}

TEST_CASE("[ManaMind filtered school draw] - no eligible spell does not draw")
{
    VerifyDeathrattleSchoolDraw("FIR_929", "CORE_CS2_024", "CORE_CS2_029",
                                SpellSchool::FIRE, false);
}
