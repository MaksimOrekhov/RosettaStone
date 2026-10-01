// Copyright (c) 2026 OpenAI

#include "doctest_proxy.hpp"

#include <Rosetta/PlayMode/Actions/Draw.hpp>
#include <Rosetta/PlayMode/Cards/Cards.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Models/Minion.hpp>
#include <Rosetta/PlayMode/Models/Spell.hpp>
#include <Rosetta/PlayMode/Models/Weapon.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawMinionTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawRaceMinionTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawSpellTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DrawWeaponTask.hpp>
#include <Rosetta/PlayMode/Tasks/PlayerTasks/PlayCardTask.hpp>
#include <Rosetta/PlayMode/Zones/DeckZone.hpp>
#include <Rosetta/PlayMode/Zones/HandZone.hpp>

using namespace RosettaStone;
using namespace PlayMode;
using namespace SimpleTasks;
using namespace PlayerTasks;

namespace
{
GameConfig MakeConfig()
{
    GameConfig config;
    config.startPlayer = PlayerType::PLAYER1;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::MAGE;
    return config;
}

Card* MakeCard(std::vector<Card*>& cards, const std::string& id,
               CardType type, Race race = Race::INVALID,
               SpellSchool school = SpellSchool::NONE)
{
    auto* card = new Card();
    card->id = id;
    card->gameTags[GameTag::CARDTYPE] = std::to_underlying(type);
    card->gameTags[GameTag::CARDRACE] = std::to_underlying(race);
    card->gameTags[GameTag::SPELL_SCHOOL] = std::to_underlying(school);
    card->gameTags[GameTag::COST] = 1;
    cards.emplace_back(card);
    return card;
}
}  // namespace

TEST_CASE("[FilteredDrawTask] - DrawMinion caps amount to eligible minions")
{
    std::vector<Card*> cards;
    Game game(MakeConfig());
    Player* player = game.GetPlayer1();

    Card* minionCard = MakeCard(cards, "test_minion", CardType::MINION,
                                Race::DRAGON);
    player->GetDeckZone()->Add(new Minion(player, minionCard, {}));
    Card* spellCard = MakeCard(cards, "test_spell", CardType::SPELL);
    player->GetDeckZone()->Add(new Spell(player, spellCard, {}));

    DrawMinionTask draw(2, false);
    draw.SetPlayer(player);
    CHECK_EQ(draw.Run(), TaskStatus::COMPLETE);
    CHECK_EQ(player->GetHandZone()->GetCount(), 1);
    CHECK_EQ(player->GetHandZone()->GetAll()[0]->card->id, "test_minion");
    CHECK_EQ(player->GetDeckZone()->GetCount(), 1);

    for (Card* card : cards)
    {
        delete card;
    }
}

TEST_CASE("[FilteredDrawTask] - DrawMinion stops when Deathrattle filter is empty")
{
    std::vector<Card*> cards;
    Game game(MakeConfig());
    Player* player = game.GetPlayer1();

    Card* minionCard = MakeCard(cards, "no_deathrattle", CardType::MINION,
                                Race::DRAGON);
    player->GetDeckZone()->Add(new Minion(player, minionCard, {}));

    DrawMinionTask draw(DrawMinionType::DEATHRATTLE, 1, false);
    draw.SetPlayer(player);
    CHECK_EQ(draw.Run(), TaskStatus::STOP);
    CHECK_EQ(player->GetHandZone()->GetCount(), 0);
    CHECK_EQ(player->GetDeckZone()->GetCount(), 1);

    for (Card* card : cards)
    {
        delete card;
    }
}

TEST_CASE("[FilteredDrawTask] - DrawRaceMinion caps amount to matching race")
{
    std::vector<Card*> cards;
    Game game(MakeConfig());
    Player* player = game.GetPlayer1();

    Card* dragon = MakeCard(cards, "dragon", CardType::MINION, Race::DRAGON);
    player->GetDeckZone()->Add(new Minion(player, dragon, {}));
    Card* beast = MakeCard(cards, "beast", CardType::MINION, Race::BEAST);
    player->GetDeckZone()->Add(new Minion(player, beast, {}));

    DrawRaceMinionTask draw(Race::DRAGON, 2, false);
    draw.SetPlayer(player);
    CHECK_EQ(draw.Run(), TaskStatus::COMPLETE);
    CHECK_EQ(player->GetHandZone()->GetCount(), 1);
    CHECK_EQ(player->GetHandZone()->GetAll()[0]->card->id, "dragon");
    CHECK_EQ(player->GetDeckZone()->GetCount(), 1);

    for (Card* card : cards)
    {
        delete card;
    }
}

TEST_CASE("[FilteredDrawTask] - DrawSpell caps amount to eligible spells")
{
    std::vector<Card*> cards;
    Game game(MakeConfig());
    Player* player = game.GetPlayer1();

    Card* spellCard = MakeCard(cards, "fire_spell", CardType::SPELL,
                               Race::INVALID, SpellSchool::FIRE);
    player->GetDeckZone()->Add(new Spell(player, spellCard, {}));
    Card* minionCard = MakeCard(cards, "minion", CardType::MINION,
                                Race::DRAGON);
    player->GetDeckZone()->Add(new Minion(player, minionCard, {}));

    DrawSpellTask draw(SpellSchool::FIRE, 2);
    draw.SetPlayer(player);
    CHECK_EQ(draw.Run(), TaskStatus::COMPLETE);
    CHECK_EQ(player->GetHandZone()->GetCount(), 1);
    CHECK_EQ(player->GetHandZone()->GetAll()[0]->card->id, "fire_spell");
    CHECK_EQ(player->GetDeckZone()->GetCount(), 1);

    for (Card* card : cards)
    {
        delete card;
    }
}

TEST_CASE("[FilteredDrawTask] - DrawWeapon caps amount to eligible weapons")
{
    std::vector<Card*> cards;
    Game game(MakeConfig());
    Player* player = game.GetPlayer1();

    Card* weaponCard = MakeCard(cards, "weapon", CardType::WEAPON);
    player->GetDeckZone()->Add(new Weapon(player, weaponCard, {}));
    Card* minionCard = MakeCard(cards, "minion", CardType::MINION,
                                Race::DRAGON);
    player->GetDeckZone()->Add(new Minion(player, minionCard, {}));

    DrawWeaponTask draw(2);
    draw.SetPlayer(player);
    CHECK_EQ(draw.Run(), TaskStatus::COMPLETE);
    CHECK_EQ(player->GetHandZone()->GetCount(), 1);
    CHECK_EQ(player->GetHandZone()->GetAll()[0]->card->id, "weapon");
    CHECK_EQ(player->GetDeckZone()->GetCount(), 1);

    for (Card* card : cards)
    {
        delete card;
    }
}

TEST_CASE("[FilteredDrawTask] - Filtered draw triggers draw listeners")
{
    GameConfig config;
    config.player1Class = CardClass::MAGE;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    for (int i = 0; i < 30; ++i)
    {
        config.player1Deck[i] = Cards::FindCardByName("Flamestrike");
    }

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);

    Player* player = game.GetCurrentPlayer();
    player->SetTotalMana(10);
    player->SetUsedMana(0);
    Playable* chromaggus =
        Generic::DrawCard(player, Cards::FindCardByName("Chromaggus"));
    game.Process(player, PlayCardTask::Minion(chromaggus));

    const auto handSizeBefore = player->GetHandZone()->GetCount();
    DrawSpellTask draw(SpellSchool::FIRE, 1);
    draw.SetPlayer(player);
    CHECK_EQ(draw.Run(), TaskStatus::COMPLETE);
    CHECK_EQ(player->GetHandZone()->GetCount(), handSizeBefore + 2);
}
