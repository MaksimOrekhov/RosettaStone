// Copyright (c) 2017-2024 Chris Ohk

#include <doctest/doctest.h>

#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DestroyTask.hpp>

using namespace RosettaStone;
using namespace RosettaStone::PlayMode;

TEST_CASE("[ManaMind effect composition] - CORE_CS2_024 damages and freezes its target")
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
    Player* opponent = game.GetOpponentPlayer();
    const auto frostbolt = Generic::DrawCard(current, Cards::FindCardByID("CORE_CS2_024"));
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(frostbolt != nullptr);
    REQUIRE(yeti != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(frostbolt, yeti));
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 2);
    CHECK(opponent->GetFieldZone()->operator[](0)->IsFrozen());
}

TEST_CASE("[ManaMind effect composition] - CORE_CS2_094 damages its target and replaces itself with a draw")
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
    const auto hammer = Generic::DrawCard(current, Cards::FindCardByID("CORE_CS2_094"));
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(hammer != nullptr);
    REQUIRE(yeti != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto handBefore = current->GetHandZone()->GetCount();
    game.Process(current, PlayCardTask::SpellTarget(hammer, yeti));
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 2);
    CHECK_EQ(current->GetHandZone()->GetCount(), handBefore);
}

TEST_CASE("[ManaMind effect composition] - CORE_EX1_129 damages all enemy minions then draws")
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
    const auto fan = Generic::DrawCard(current, Cards::FindCardByID("CORE_EX1_129"));
    const auto friendly = Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti"));
    const auto enemy1 = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    const auto enemy2 = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(fan != nullptr);
    REQUIRE(friendly != nullptr);
    REQUIRE(enemy1 != nullptr);
    REQUIRE(enemy2 != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(friendly));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(enemy1));
    game.Process(opponent, PlayCardTask::Minion(enemy2));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto handBefore = current->GetHandZone()->GetCount();
    game.Process(current, PlayCardTask::Spell(fan));
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(opponent->GetFieldZone()->operator[](1)->GetHealth(), 4);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 5);
    CHECK_EQ(current->GetHandZone()->GetCount(), handBefore);
}

TEST_CASE("[ManaMind effect composition] - CORE_UNG_084 damages the selected minion with its Battlecry")
{
    GameConfig config;
    config.player1Class = CardClass::HUNTER;
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
    const auto phoenix = Generic::DrawCard(current, Cards::FindCardByID("CORE_UNG_084"));
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(phoenix != nullptr);
    REQUIRE(yeti != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::MinionTarget(phoenix, yeti));
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 2);
    CHECK_EQ(current->GetFieldZone()->GetCount(), 1);
}

TEST_CASE("[ManaMind effect composition] - CORE_OG_149 damages all other minions")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
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
    const auto ghoul = Generic::DrawCard(current, Cards::FindCardByID("CORE_OG_149"));
    const auto friendly = Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti"));
    const auto enemy = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(ghoul != nullptr);
    REQUIRE(friendly != nullptr);
    REQUIRE(enemy != nullptr);
    game.Process(current, PlayCardTask::Minion(friendly));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(enemy));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(ghoul));
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(current->GetFieldZone()->operator[](1)->GetHealth(), 3);
}

TEST_CASE("[ManaMind effect composition] - CORE_TSC_076 summons its three Taunt Statues in order")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
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
    const auto spell = Generic::DrawCard(current, Cards::FindCardByID("CORE_TSC_076"));
    REQUIRE(spell != nullptr);
    game.Process(current, PlayCardTask::Spell(spell));
    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 3);
    CHECK_EQ(field[0]->GetAttack(), 4);
    CHECK_EQ(field[0]->GetHealth(), 8);
    CHECK_EQ(field[1]->GetAttack(), 2);
    CHECK_EQ(field[1]->GetHealth(), 4);
    CHECK_EQ(field[2]->GetAttack(), 1);
    CHECK_EQ(field[2]->GetHealth(), 2);
    for (int i = 0; i < field.GetCount(); ++i)
    {
        CHECK_EQ(field[i]->GetGameTag(GameTag::TAUNT), 1);
    }
}

TEST_CASE("[ManaMind effect composition] - CORE_GVG_061 summons recruits and equips Light's Justice")
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
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto spell = Generic::DrawCard(current, Cards::FindCardByID("CORE_GVG_061"));
    REQUIRE(spell != nullptr);
    game.Process(current, PlayCardTask::Spell(spell));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 3);
    for (int i = 0; i < current->GetFieldZone()->GetCount(); ++i)
    {
        CHECK_EQ(current->GetFieldZone()->operator[](i)->GetAttack(), 1);
        CHECK_EQ(current->GetFieldZone()->operator[](i)->GetHealth(), 1);
    }
    CHECK_EQ(current->GetWeapon().GetAttack(), 1);
    CHECK_EQ(current->GetWeapon().GetDurability(), 4);
}

TEST_CASE("[ManaMind effect composition] - CORE_BOT_222 damages the minion and its caster")
{
    GameConfig config;
    config.player1Class = CardClass::WARLOCK;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    const auto spiritBomb = Generic::DrawCard(current, Cards::FindCardByID("CORE_BOT_222"));
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(spiritBomb != nullptr);
    REQUIRE(yeti != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(spiritBomb, yeti));
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 1);
    CHECK_EQ(current->GetHero()->GetHealth(), 26);
}

TEST_CASE("[ManaMind effect composition] - CORE_BOT_451 summons two Rush Sparks and overloads")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
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
    const auto spell = Generic::DrawCard(current, Cards::FindCardByID("CORE_BOT_451"));
    REQUIRE(spell != nullptr);
    game.Process(current, PlayCardTask::Spell(spell));
    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 2);
    for (int i = 0; i < field.GetCount(); ++i)
    {
        CHECK_EQ(field[i]->GetAttack(), 1);
        CHECK_EQ(field[i]->GetHealth(), 1);
        CHECK_EQ(field[i]->GetGameTag(GameTag::RUSH), 1);
        CHECK_EQ(field[i]->card->GetRace(), Race::ELEMENTAL);
    }
    CHECK_EQ(current->GetOverloadOwed(), 1);
}

TEST_CASE("[ManaMind effect composition] - CORE_RLK_062 summons two copies of its Taunt body")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto swarmguard = Generic::DrawCard(current, Cards::FindCardByID("CORE_RLK_062"));
    REQUIRE(swarmguard != nullptr);
    game.Process(current, PlayCardTask::Minion(swarmguard));
    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 3);
    for (int i = 0; i < field.GetCount(); ++i)
    {
        CHECK_EQ(field[i]->GetAttack(), 1);
        CHECK_EQ(field[i]->GetHealth(), 3);
        CHECK_EQ(field[i]->GetGameTag(GameTag::TAUNT), 1);
    }
}

TEST_CASE("[ManaMind effect composition] - CORE_CS2_004 enchants a minion then draws")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
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

    auto& hand = *current->GetHandZone();
    auto& field = *current->GetFieldZone();
    const auto shieldCard = Cards::FindCardByID("CORE_CS2_004");
    const auto ogreCard = Cards::FindCardByName("Boulderfist Ogre");
    REQUIRE(shieldCard != nullptr);
    REQUIRE(ogreCard != nullptr);
    const auto shield = Generic::DrawCard(current, shieldCard);
    const auto ogre = Generic::DrawCard(current, ogreCard);

    game.Process(current, PlayCardTask::Minion(ogre));
    const auto handBefore = hand.GetCount();
    game.Process(current, PlayCardTask::SpellTarget(shield, ogre));

    CHECK_EQ(field[0]->GetHealth(), 9);
    CHECK_EQ(hand.GetCount(), handBefore);
}

TEST_CASE("[ManaMind effect composition] - END_007 resolves damage, attack enchant, draw, and armor in order")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::WARLOCK;
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
    const auto advantageCard = Cards::FindCardByID("END_007");
    REQUIRE(advantageCard != nullptr);
    CHECK_FALSE(advantageCard->playRequirements.contains(PlayReq::REQ_MINION_TARGET));
    const auto advantage = Generic::DrawCard(current, advantageCard);
    const auto handBefore = hand.GetCount();

    game.Process(current, PlayCardTask::SpellTarget(advantage, opponent->GetHero()));

    CHECK_EQ(opponent->GetHero()->GetHealth(), 29);
    CHECK_EQ(current->GetHero()->GetAttack(), 1);
    CHECK_EQ(current->GetHero()->GetArmor(), 1);
    CHECK_EQ(hand.GetCount(), handBefore);

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    CHECK_EQ(current->GetHero()->GetAttack(), 0);
}

TEST_CASE("[ManaMind effect composition] - CAP_801 applies Taunt, stats, and Reborn")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
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

    auto& field = *current->GetFieldZone();
    const auto hauntCard = Cards::FindCardByID("CAP_801");
    const auto ogreCard = Cards::FindCardByName("Boulderfist Ogre");
    REQUIRE(hauntCard != nullptr);
    REQUIRE(ogreCard != nullptr);
    const auto haunt = Generic::DrawCard(current, hauntCard);
    const auto ogre = Generic::DrawCard(current, ogreCard);

    game.Process(current, PlayCardTask::Minion(ogre));
    game.Process(current, PlayCardTask::SpellTarget(haunt, ogre));

    REQUIRE_EQ(field.GetCount(), 1);
    CHECK_EQ(field[0]->GetAttack(), 8);
    CHECK_EQ(field[0]->GetHealth(), 10);
    CHECK(field[0]->HasTaunt());
    CHECK_EQ(field[0]->GetGameTag(GameTag::REBORN), 1);
}

TEST_CASE("[ManaMind effect composition] - CORE_SW_066 silences the selected minion")
{
    GameConfig config;
    config.player1Class = CardClass::MAGE;
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

    auto& opponentField = *opponent->GetFieldZone();
    const auto librarianCard = Cards::FindCardByID("CORE_SW_066");
    const auto yetiCard = Cards::FindCardByName("Chillwind Yeti");
    REQUIRE(librarianCard != nullptr);
    REQUIRE(yetiCard != nullptr);
    const auto librarian = Generic::DrawCard(current, librarianCard);
    const auto yeti = Generic::DrawCard(opponent, yetiCard);
    const auto taunt = Cards::FindCardByName("Mark of the Wild");
    REQUIRE(taunt != nullptr);
    const auto mark = Generic::DrawCard(opponent, taunt);

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, PlayCardTask::SpellTarget(mark, yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    game.Process(current, PlayCardTask::MinionTarget(librarian, opponentField[0]));

    CHECK_EQ(opponentField[0]->GetGameTag(GameTag::SILENCED), 1);
}

TEST_CASE("[ManaMind effect composition] - CORE_LOOT_013 damages its controller's hero")
{
    GameConfig config;
    config.player1Class = CardClass::WARLOCK;
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
    const auto homunculus = Generic::DrawCard(current, Cards::FindCardByID("CORE_LOOT_013"));
    REQUIRE(homunculus != nullptr);
    const auto healthBefore = current->GetHero()->GetHealth();
    game.Process(current, PlayCardTask::Minion(homunculus));
    CHECK_EQ(current->GetHero()->GetHealth(), healthBefore - 2);
}

TEST_CASE("[ManaMind effect composition] - RLK_024 damages its target and Lifesteals")
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
    const auto deathStrike = Generic::DrawCard(current, Cards::FindCardByID("RLK_024"));
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(deathStrike != nullptr);
    REQUIRE(yeti != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->GetHero()->SetDamage(10);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto healthBefore = current->GetHero()->GetHealth();
    game.Process(current, PlayCardTask::SpellTarget(deathStrike, yeti));
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 1);
    CHECK_EQ(current->GetHero()->GetHealth(), healthBefore + 6);
}

TEST_CASE("[ManaMind effect composition] - CORE_LOOT_368 summons three Taunt Voidwalkers on death")
{
    GameConfig config;
    config.player1Class = CardClass::WARLOCK;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto voidlord = Generic::DrawCard(current, Cards::FindCardByID("CORE_LOOT_368"));
    REQUIRE(voidlord != nullptr);
    game.Process(current, PlayCardTask::Minion(voidlord));
    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 1);
    CHECK(field[0]->HasTaunt());

    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(field[0]);
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();

    REQUIRE_EQ(field.GetCount(), 3);
    for (int index = 0; index < field.GetCount(); ++index)
    {
        CHECK_EQ(field[index]->GetAttack(), 1);
        CHECK_EQ(field[index]->GetHealth(), 3);
        CHECK(field[index]->HasTaunt());
    }
}

TEST_CASE("[ManaMind effect composition] - CORE_OG_031 summons a Twilight Elemental when its weapon breaks")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const auto hammer = Generic::DrawCard(current, Cards::FindCardByID("CORE_OG_031"));
    REQUIRE(hammer != nullptr);
    game.Process(current, PlayCardTask::Weapon(hammer));
    REQUIRE(current->GetHero()->HasWeapon());
    current->GetWeapon().SetDurability(1);
    game.Process(current, AttackTask(current->GetHero(), game.GetOpponentPlayer()->GetHero()));

    auto& field = *current->GetFieldZone();
    REQUIRE_EQ(field.GetCount(), 1);
    CHECK_EQ(field[0]->GetAttack(), 4);
    CHECK_EQ(field[0]->GetHealth(), 2);
}

TEST_CASE("[ManaMind effect composition] - FIR_778 deals nine damage to enemy minions on death")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(yeti != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto avatar = Generic::DrawCard(current, Cards::FindCardByID("FIR_778"));
    REQUIRE(avatar != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(avatar));
    REQUIRE(current->GetFieldZone()->GetCount() == 1);

    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(current->GetFieldZone()->operator[](0));
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();

    CHECK_EQ(opponent->GetFieldZone()->GetCount(), 0);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 30);
}

TEST_CASE("[ManaMind effect composition] - JAIL_007 damages enemy minions and hero on death")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(yeti != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto sewerImp = Generic::DrawCard(current, Cards::FindCardByID("JAIL_007"));
    REQUIRE(sewerImp != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(sewerImp));
    REQUIRE(current->GetFieldZone()->GetCount() == 1);
    CHECK(current->GetFieldZone()->operator[](0)->HasTaunt());

    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(current->GetFieldZone()->operator[](0));
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();

    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 3);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 28);
}

TEST_CASE("[ManaMind effect composition] - CATA_475 damages all enemies at its controller's turn end")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(yeti != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    const auto bulwark = Generic::DrawCard(current, Cards::FindCardByID("CATA_475"));
    REQUIRE(bulwark != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(bulwark));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    game.Process(current, EndTurnTask());

    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 3);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 28);
    CHECK_EQ(current->GetHero()->GetHealth(), 30);
}

TEST_CASE("[ManaMind effect composition] - EDR_459 has distinct Battlecry and Deathrattle damage")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto devastator = Generic::DrawCard(current, Cards::FindCardByID("EDR_459"));
    const auto friendlyOgre = Generic::DrawCard(current, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(devastator != nullptr);
    REQUIRE(friendlyOgre != nullptr);

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto enemyOgre = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(enemyOgre != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(enemyOgre));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(friendlyOgre));
    game.Process(current, PlayCardTask::Minion(devastator));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 2);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(current->GetFieldZone()->operator[](1)->GetHealth(), 6);

    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(current->GetFieldZone()->operator[](1));
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();

    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 4);
}

TEST_CASE("[ManaMind effect composition] - CATA_304 damages itself and Lifesteals for four")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto attendant = Generic::DrawCard(current, Cards::FindCardByID("CATA_304"));
    REQUIRE(attendant != nullptr);
    current->GetHero()->SetDamage(10);
    const int heroHealthBefore = current->GetHero()->GetHealth();
    const int opponentHealthBefore = opponent->GetHero()->GetHealth();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(attendant));

    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(current->GetHero()->GetHealth(), heroHealthBefore + 4);
    CHECK_EQ(opponent->GetHero()->GetHealth(), opponentHealthBefore);
}

TEST_CASE("[ManaMind effect composition] - CORE_CFM_604 heals a friendly hero and draws")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    const auto potion = Generic::DrawCard(current, Cards::FindCardByID("CORE_CFM_604"));
    REQUIRE(potion != nullptr);
    current->GetHero()->SetDamage(10);
    const int deckCountBefore = current->GetDeckZone()->GetCount();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(potion, current->GetHero()));

    CHECK_EQ(current->GetHero()->GetHealth(), 30);
    CHECK_EQ(current->GetDeckZone()->GetCount(), deckCountBefore - 1);
}

TEST_CASE("[ManaMind effect composition] - EDR_476 damages enemy characters and heals friendly ones")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto moonwell = Generic::DrawCard(current, Cards::FindCardByID("EDR_476"));
    const auto friendlyYeti = Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(moonwell != nullptr);
    REQUIRE(friendlyYeti != nullptr);
    current->GetHero()->SetDamage(10);
    opponent->GetHero()->SetDamage(10);

    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto enemyYeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(enemyYeti != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(enemyYeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(friendlyYeti));
    dynamic_cast<Character*>(friendlyYeti)->SetDamage(4);
    game.Process(current, PlayCardTask::Spell(moonwell));

    CHECK_EQ(current->GetHero()->GetHealth(), 24);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 16);
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 5);
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 1);
}

TEST_CASE("[ManaMind effect composition] - EDR_971 heals both heroes at its controller's turn end")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto caretaker = Generic::DrawCard(current, Cards::FindCardByID("EDR_971"));
    REQUIRE(caretaker != nullptr);
    current->GetHero()->SetDamage(10);
    opponent->GetHero()->SetDamage(10);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(caretaker));
    game.Process(current, EndTurnTask());

    CHECK_EQ(current->GetHero()->GetHealth(), 23);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 23);
}

TEST_CASE("[ManaMind effect composition] - RLK_709 damages all enemies and draws")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto winter = Generic::DrawCard(current, Cards::FindCardByID("RLK_709"));
    REQUIRE(winter != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(yeti != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    const int deckCountBefore = current->GetDeckZone()->GetCount();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Spell(winter));

    CHECK_EQ(opponent->GetHero()->GetHealth(), 28);
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 3);
    CHECK_EQ(current->GetDeckZone()->GetCount(), deckCountBefore - 1);
}

TEST_CASE("[ManaMind effect composition] - CATA_612 freezes only its played minion")
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
    Player* current = game.GetPlayer1();
    const auto imp = Generic::DrawCard(current, Cards::FindCardByID("CATA_612"));
    REQUIRE(imp != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(imp));

    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK(current->GetFieldZone()->operator[](0)->IsFrozen());
    CHECK_FALSE(game.GetOpponentPlayer()->GetHero()->IsFrozen());
}

TEST_CASE("[ManaMind effect composition] - TIME_218 damages a minion and gives the hero +1 Attack")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    const auto shock = Generic::DrawCard(current, Cards::FindCardByID("TIME_218"));
    const auto shockCard = Cards::FindCardByID("TIME_218");
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(shock != nullptr);
    REQUIRE(shockCard != nullptr);
    REQUIRE(yeti != nullptr);
    CHECK(shockCard->playRequirements.contains(PlayReq::REQ_MINION_TARGET));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(shock, yeti));
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(current->GetHero()->GetAttack(), 1);
}

TEST_CASE("[ManaMind profile deck minion] - JAIL_516 summons two current-cost eligible minions with Rush")
{
    GameConfig config;
    config.player1Class = CardClass::PALADIN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);

    for (const auto& cardID : { "CORE_ICC_038", "TLC_438", "CORE_CS2_182" })
    {
        const auto card = Cards::FindCardByID(cardID);
        REQUIRE(card != nullptr);
        current->GetDeckZone()->Add(Entity::GetFromCard(current, card));
    }
    const auto recruiterCard = Cards::FindCardByID("JAIL_516");
    REQUIRE(recruiterCard != nullptr);
    const auto recruiter = Generic::DrawCard(current, recruiterCard);
    REQUIRE(recruiter != nullptr);

    game.Process(current, PlayCardTask::Minion(recruiter));

    CHECK_EQ(current->GetDeckZone()->GetCount(), 1);
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 3);
    int summonedCount = 0;
    for (const auto minion : current->GetFieldZone()->GetMinions())
    {
        if (minion->card->id == "JAIL_516")
        {
            continue;
        }
        CHECK(minion->HasRush());
        const bool isProfileEligibleMinion =
            minion->card->id == "CORE_ICC_038" || minion->card->id == "TLC_438";
        CHECK(isProfileEligibleMinion);
        ++summonedCount;
    }
    CHECK_EQ(summonedCount, 2);
}

TEST_CASE("[ManaMind profile deck minion] - JAIL_327 summons one cost-qualified minion at each of three end steps")
{
    GameConfig config;
    config.player1Class = CardClass::PALADIN;
    config.player2Class = CardClass::WARRIOR;
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

    for (const auto& cardID : { "CORE_ICC_038", "TLC_438", "CORE_ICC_038",
                                 "TLC_438", "CORE_ICC_038", "TLC_438",
                                 "CORE_ICC_038", "TLC_438", "CORE_CS2_182",
                                 "CORE_CS2_182", "CORE_CS2_182" })
    {
        const auto card = Cards::FindCardByID(cardID);
        REQUIRE(card != nullptr);
        current->GetDeckZone()->Add(Entity::GetFromCard(current, card));
    }
    const auto auraCard = Cards::FindCardByID("JAIL_327");
    REQUIRE(auraCard != nullptr);
    const auto aura = Generic::DrawCard(current, auraCard);
    REQUIRE(aura != nullptr);
    game.Process(current, PlayCardTask::Spell(aura));

    for (int trigger = 1; trigger <= 4; ++trigger)
    {
        game.Process(current, EndTurnTask());
        game.ProcessUntil(Step::MAIN_ACTION);
        CHECK_EQ(current->GetFieldZone()->GetCount(), trigger <= 3 ? trigger : 3);
        for (const auto minion : current->GetFieldZone()->GetMinions())
        {
            const bool isProfileEligibleMinion =
                minion->card->id == "CORE_ICC_038" || minion->card->id == "TLC_438";
            CHECK(isProfileEligibleMinion);
        }
        if (trigger < 4)
        {
            game.Process(opponent, EndTurnTask());
            game.ProcessUntil(Step::MAIN_ACTION);
        }
    }
    CHECK_EQ(current->GetDeckZone()->GetCount(), 5);
}

TEST_CASE("[ManaMind profile deck minion] - JAIL_516 uses live cost, summons fewer than requested, and handles empty selection")
{
    GameConfig config;
    config.player1Class = CardClass::PALADIN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    current->SetTotalMana(10);
    current->SetUsedMana(0);

    const auto protectorCard = Cards::FindCardByID("CORE_ICC_038");
    const auto yetiCard = Cards::FindCardByID("CORE_CS2_182");
    REQUIRE(protectorCard != nullptr);
    REQUIRE(yetiCard != nullptr);
    auto* protector = Entity::GetFromCard(current, protectorCard);
    auto* discountedYeti = Entity::GetFromCard(current, yetiCard);
    auto* normalYeti = Entity::GetFromCard(current, yetiCard);
    REQUIRE(protector != nullptr);
    REQUIRE(discountedYeti != nullptr);
    REQUIRE(normalYeti != nullptr);
    protector->SetCost(3);
    discountedYeti->SetCost(2);
    current->GetDeckZone()->Add(protector);
    current->GetDeckZone()->Add(discountedYeti);
    current->GetDeckZone()->Add(normalYeti);

    const auto recruiterCard = Cards::FindCardByID("JAIL_516");
    REQUIRE(recruiterCard != nullptr);
    const auto firstRecruiter = Generic::DrawCard(current, recruiterCard);
    const auto secondRecruiter = Generic::DrawCard(current, recruiterCard);
    REQUIRE(firstRecruiter != nullptr);
    REQUIRE(secondRecruiter != nullptr);
    game.Process(current, PlayCardTask::Minion(firstRecruiter));

    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 2);
    Minion* rushedYeti = nullptr;
    for (const auto minion : current->GetFieldZone()->GetMinions())
    {
        if (minion->card->id == "CORE_CS2_182")
        {
            rushedYeti = minion;
        }
    }
    REQUIRE(rushedYeti != nullptr);
    CHECK(rushedYeti->HasRush());
    CHECK_EQ(rushedYeti->GetCost(), 2);
    CHECK_EQ(current->GetDeckZone()->GetCount(), 2);

    game.Process(current, PlayCardTask::Minion(secondRecruiter));
    CHECK_EQ(current->GetFieldZone()->GetCount(), 3);
    CHECK_EQ(current->GetDeckZone()->GetCount(), 2);
}

TEST_CASE("[ManaMind effect composition] - TIME_215 damages every minion and adds Static Shock")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    const auto thunderquake = Generic::DrawCard(current, Cards::FindCardByID("TIME_215"));
    const auto friendly = Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti"));
    const auto enemy = Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti"));
    REQUIRE(thunderquake != nullptr);
    REQUIRE(friendly != nullptr);
    REQUIRE(enemy != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(friendly));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(enemy));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const int handCountBefore = current->GetHandZone()->GetCount();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Spell(thunderquake));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 4);
    CHECK_EQ(current->GetHandZone()->GetCount(), handCountBefore);
    bool hasStaticShock = false;
    for (int i = 0; i < current->GetHandZone()->GetCount(); ++i)
    {
        const Playable* handCard = current->GetHandZone()->operator[](i);
        if (handCard != nullptr && handCard->card != nullptr && handCard->card->id == "TIME_218")
        {
            hasStaticShock = true;
        }
    }
    CHECK(hasStaticShock);
}

TEST_CASE("[ManaMind effect composition] - TLC_620 gains armor then damages an enemy by current armor")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetCurrentPlayer();
    Player* opponent = game.GetOpponentPlayer();
    const auto fortify = Generic::DrawCard(current, Cards::FindCardByID("TLC_620"));
    const auto ogre = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(fortify != nullptr);
    REQUIRE(ogre != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->GetHero()->SetArmor(2);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(fortify, ogre));
    CHECK_EQ(current->GetHero()->GetArmor(), 5);
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 2);
}

TEST_CASE("[ManaMind effect composition] - TLC_225 summons a TLC_249 whose Deathrattle deals two random pings")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto cinderfin = Generic::DrawCard(current, Cards::FindCardByID("TLC_225"));
    const auto yeti = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(cinderfin != nullptr);
    REQUIRE(yeti != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(yeti));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(cinderfin));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);

    DestroyTask destroyFin(EntityType::SOURCE);
    destroyFin.SetPlayer(current);
    destroyFin.SetSource(current->GetFieldZone()->operator[](0));
    REQUIRE_EQ(destroyFin.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    auto* cinder = current->GetFieldZone()->operator[](0);
    REQUIRE_EQ(cinder->card->id, "TLC_249");
    CHECK_EQ(cinder->GetAttack(), 2);
    CHECK_EQ(cinder->GetHealth(), 1);

    const int enemyTotalBefore = opponent->GetHero()->GetHealth() +
                                 opponent->GetFieldZone()->operator[](0)->GetHealth();
    DestroyTask destroyCinder(EntityType::SOURCE);
    destroyCinder.SetPlayer(current);
    destroyCinder.SetSource(cinder);
    REQUIRE_EQ(destroyCinder.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    const int enemyTotalAfter = opponent->GetHero()->GetHealth() +
                                opponent->GetFieldZone()->operator[](0)->GetHealth();
    CHECK_EQ(enemyTotalBefore - enemyTotalAfter, 2);
}

TEST_CASE("[ManaMind effect composition] - FIR_909 deals three independent spell-damage hits to random enemies")
{
    GameConfig config;
    config.player1Class = CardClass::HUNTER;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto burst = Generic::DrawCard(current, Cards::FindCardByID("FIR_909"));
    REQUIRE(burst != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto ogre1 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    const auto ogre2 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(ogre1 != nullptr);
    REQUIRE(ogre2 != nullptr);
    opponent->SetTotalMana(20);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre1));
    game.Process(opponent, PlayCardTask::Minion(ogre2));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    const int enemyHealthBefore = opponent->GetHero()->GetHealth() +
                                  opponent->GetFieldZone()->operator[](0)->GetHealth() +
                                  opponent->GetFieldZone()->operator[](1)->GetHealth();
    const int ownHealthBefore = current->GetHero()->GetHealth();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Spell(burst));
    int enemyHealthAfter = opponent->GetHero()->GetHealth();
    for (int i = 0; i < opponent->GetFieldZone()->GetCount(); ++i)
    {
        enemyHealthAfter += opponent->GetFieldZone()->operator[](i)->GetHealth();
    }
    CHECK_EQ(enemyHealthBefore - enemyHealthAfter, 6);
    CHECK_EQ(current->GetHero()->GetHealth(), ownHealthBefore);
    CHECK_EQ(opponent->GetFieldZone()->GetCount(), 2);
}

TEST_CASE("[ManaMind effect composition] - CORE_YOP_034 damages one random enemy minion at turn end")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto blackwing = Generic::DrawCard(current, Cards::FindCardByID("CORE_YOP_034"));
    REQUIRE(blackwing != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto ogre1 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    const auto ogre2 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(ogre1 != nullptr);
    REQUIRE(ogre2 != nullptr);
    opponent->SetTotalMana(20);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre1));
    game.Process(opponent, PlayCardTask::Minion(ogre2));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(blackwing));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    game.Process(current, EndTurnTask());

    CHECK_EQ(opponent->GetFieldZone()->GetCount(), 1);
    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), 7);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 30);
}

TEST_CASE("[ManaMind effect composition] - CORE_ICC_210 buffs one other friendly minion at turn end")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    auto* ascendant = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByID("CORE_ICC_210")));
    auto* yeti = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(ascendant != nullptr);
    REQUIRE(yeti != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(ascendant));
    game.Process(current, PlayCardTask::Minion(yeti));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 2);
    game.Process(current, EndTurnTask());
    CHECK_EQ(ascendant->GetAttack(), 2);
    CHECK_EQ(ascendant->GetHealth(), 3);
    CHECK_EQ(yeti->GetAttack(), 5);
    CHECK_EQ(yeti->GetHealth(), 6);
}

TEST_CASE("[ManaMind effect composition] - CORE_AT_062 summons three Webspinners")
{
    GameConfig config;
    config.player1Class = CardClass::HUNTER;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* hunter = game.GetPlayer1();
    const auto ball = Generic::DrawCard(hunter, Cards::FindCardByID("CORE_AT_062"));
    REQUIRE(ball != nullptr);
    hunter->SetTotalMana(10);
    hunter->SetUsedMana(0);
    game.Process(hunter, PlayCardTask::Spell(ball));
    REQUIRE_EQ(hunter->GetFieldZone()->GetCount(), 3);
    for (const auto& webspinner : hunter->GetFieldZone()->GetMinions())
    {
        CHECK_EQ(webspinner->GetAttack(), 1);
        CHECK_EQ(webspinner->GetHealth(), 1);
        CHECK(webspinner->HasDeathrattle());
    }
}

TEST_CASE("[ManaMind effect composition] - EDR_942 gives its hero Divine Shield at turn end")
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
    Player* current = game.GetPlayer1();
    const auto cumulus = Generic::DrawCard(current, Cards::FindCardByID("EDR_942"));
    REQUIRE(cumulus != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(cumulus));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetHero()->GetGameTag(GameTag::DIVINE_SHIELD), 0);
    game.Process(current, EndTurnTask());
    CHECK_EQ(current->GetHero()->GetGameTag(GameTag::DIVINE_SHIELD), 1);
}

TEST_CASE("[ManaMind effect composition] - TIME_100 buffs minions remaining in its controller's hand")
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
    Player* current = game.GetPlayer1();
    auto* attendant = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByID("TIME_100")));
    auto* yeti = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(attendant != nullptr);
    REQUIRE(yeti != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(attendant));
    CHECK_EQ(attendant->GetGameTag(GameTag::DIVINE_SHIELD), 1);
    CHECK_EQ(yeti->GetAttack(), 4);
    CHECK_EQ(yeti->GetHealth(), 5);
    game.Process(current, EndTurnTask());
    CHECK_EQ(yeti->GetAttack(), 5);
    CHECK_EQ(yeti->GetHealth(), 6);
}

TEST_CASE("[ManaMind effect composition] - EDR_889 buffs another friendly Dragon at turn end")
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
    Player* current = game.GetPlayer1();
    auto* peddler = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByID("EDR_889")));
    auto* dragon = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByName("Faerie Dragon")));
    auto* yeti = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(peddler != nullptr);
    REQUIRE(dragon != nullptr);
    REQUIRE(yeti != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(peddler));
    game.Process(current, PlayCardTask::Minion(dragon));
    game.Process(current, PlayCardTask::Minion(yeti));
    game.Process(current, EndTurnTask());
    CHECK_EQ(peddler->GetAttack(), 1);
    CHECK_EQ(peddler->GetHealth(), 4);
    CHECK_EQ(dragon->GetAttack(), 4);
    CHECK_EQ(dragon->GetHealth(), 3);
    CHECK_EQ(yeti->GetAttack(), 4);
    CHECK_EQ(yeti->GetHealth(), 5);
}

TEST_CASE("[ManaMind effect composition] - CORE_GVG_059 gives one friendly minion Divine Shield and Taunt")
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
    Player* current = game.GetPlayer1();
    const auto coghammer = Generic::DrawCard(current, Cards::FindCardByID("CORE_GVG_059"));
    auto* yeti = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(coghammer != nullptr);
    REQUIRE(yeti != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(yeti));
    game.Process(current, PlayCardTask::Weapon(coghammer));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(yeti->GetGameTag(GameTag::DIVINE_SHIELD), 1);
    CHECK_EQ(yeti->GetGameTag(GameTag::TAUNT), 1);
}

TEST_CASE("[ManaMind effect composition] - DINO_132 deals five damage to one random enemy minion at turn end")
{
    GameConfig config;
    config.player1Class = CardClass::WARLOCK;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto asphyxiodon = Generic::DrawCard(current, Cards::FindCardByID("DINO_132"));
    REQUIRE(asphyxiodon != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto ogre1 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    const auto ogre2 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(ogre1 != nullptr);
    REQUIRE(ogre2 != nullptr);
    opponent->SetTotalMana(20);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre1));
    game.Process(opponent, PlayCardTask::Minion(ogre2));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* enemyMinion1 = opponent->GetFieldZone()->operator[](0);
    auto* enemyMinion2 = opponent->GetFieldZone()->operator[](1);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(asphyxiodon));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    const int health1 = enemyMinion1->GetHealth();
    const int health2 = enemyMinion2->GetHealth();
    game.Process(current, EndTurnTask());
    const int damage1 = health1 - enemyMinion1->GetHealth();
    const int damage2 = health2 - enemyMinion2->GetHealth();
    CHECK((damage1 == 5) != (damage2 == 5));
    CHECK_EQ(damage1 + damage2, 5);
}

TEST_CASE("[ManaMind effect composition] - CORE_RLK_083 deals two one-damage hits to enemies after a friendly spell")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto deathchiller = Generic::DrawCard(current, Cards::FindCardByID("CORE_RLK_083"));
    const auto frostbolt = Generic::DrawCard(current, Cards::FindCardByID("CORE_CS2_024"));
    REQUIRE(deathchiller != nullptr);
    REQUIRE(frostbolt != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto ogre1 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    const auto ogre2 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(ogre1 != nullptr);
    REQUIRE(ogre2 != nullptr);
    opponent->SetTotalMana(20);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre1));
    game.Process(opponent, PlayCardTask::Minion(ogre2));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(deathchiller));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    auto* enemyMinion = opponent->GetFieldZone()->operator[](0);
    const int enemyHealthBefore = opponent->GetHero()->GetHealth() +
                                  enemyMinion->GetHealth() +
                                  opponent->GetFieldZone()->operator[](1)->GetHealth();
    game.Process(current, PlayCardTask::SpellTarget(frostbolt, enemyMinion));
    const int enemyHealthAfter = opponent->GetHero()->GetHealth() +
                                 enemyMinion->GetHealth() +
                                 opponent->GetFieldZone()->operator[](1)->GetHealth();
    CHECK_EQ(enemyHealthBefore - enemyHealthAfter, 5);
    CHECK_EQ(current->GetHero()->GetHealth(), 30);
}

TEST_CASE("[ManaMind effect composition] - END_026 draws only after a friendly spell targets a minion")
{
    GameConfig config;
    config.player1Class = CardClass::WARLOCK;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto fragment = Generic::DrawCard(current, Cards::FindCardByID("END_026"));
    const auto minionSpell = Generic::DrawCard(current, Cards::FindCardByID("CORE_CS2_024"));
    const auto heroSpell = Generic::DrawCard(current, Cards::FindCardByID("CORE_CS2_024"));
    REQUIRE(fragment != nullptr);
    REQUIRE(minionSpell != nullptr);
    REQUIRE(heroSpell != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(fragment));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto ogre = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(ogre != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* enemyMinion = opponent->GetFieldZone()->operator[](0);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    const int handBeforeMinionSpell = current->GetHandZone()->GetCount();
    game.Process(current, PlayCardTask::SpellTarget(minionSpell, enemyMinion));
    CHECK_EQ(current->GetHandZone()->GetCount(), handBeforeMinionSpell);

    const int handBeforeHeroSpell = current->GetHandZone()->GetCount();
    game.Process(current, PlayCardTask::SpellTarget(heroSpell, opponent->GetHero()));
    CHECK_EQ(current->GetHandZone()->GetCount(), handBeforeHeroSpell - 1);
}

TEST_CASE("[ManaMind effect composition] - TLC_256 gains Divine Shield after a friendly spell only")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto thresher = Generic::DrawCard(current, Cards::FindCardByID("TLC_256"));
    const auto friendlySpell = Generic::DrawCard(current, Cards::FindCardByID("CORE_CS2_024"));
    REQUIRE(thresher != nullptr);
    REQUIRE(friendlySpell != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(thresher));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    auto* minion = current->GetFieldZone()->operator[](0);
    CHECK_EQ(minion->GetGameTag(GameTag::DIVINE_SHIELD), 0);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto enemySpell = Generic::DrawCard(opponent, Cards::FindCardByID("CORE_CS2_024"));
    REQUIRE(enemySpell != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::SpellTarget(enemySpell, current->GetHero()));
    CHECK_EQ(minion->GetGameTag(GameTag::DIVINE_SHIELD), 0);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(friendlySpell, opponent->GetHero()));
    CHECK_EQ(minion->GetGameTag(GameTag::DIVINE_SHIELD), 1);
}

TEST_CASE("[ManaMind effect composition] - EDR_110 deals one non-spell ping to a random enemy minion on death")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::MAGE;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto sporegnasher = Generic::DrawCard(current, Cards::FindCardByID("EDR_110"));
    REQUIRE(sporegnasher != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto ogre = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(ogre != nullptr);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(sporegnasher));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetGameTag(GameTag::POISONOUS), 1);
    const auto enemyHealthBefore = opponent->GetFieldZone()->operator[](0)->GetHealth();

    DestroyTask destroySporegnasher(EntityType::SOURCE);
    destroySporegnasher.SetPlayer(current);
    destroySporegnasher.SetSource(current->GetFieldZone()->operator[](0));
    REQUIRE_EQ(destroySporegnasher.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();

    REQUIRE_EQ(opponent->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(opponent->GetFieldZone()->operator[](0)->GetHealth(), enemyHealthBefore - 1);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 30);
}

TEST_CASE("[ManaMind effect composition] - CATA_485 combines targeted and random spell damage")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto sleetStorm = Generic::DrawCard(current, Cards::FindCardByID("CATA_485"));
    REQUIRE(sleetStorm != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    const auto ogre1 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    const auto ogre2 = Generic::DrawCard(opponent, Cards::FindCardByName("Boulderfist Ogre"));
    REQUIRE(ogre1 != nullptr);
    REQUIRE(ogre2 != nullptr);
    opponent->SetTotalMana(20);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(ogre1));
    game.Process(opponent, PlayCardTask::Minion(ogre2));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);

    const int enemyHealthBefore = opponent->GetHero()->GetHealth() +
                                  opponent->GetFieldZone()->operator[](0)->GetHealth() +
                                  opponent->GetFieldZone()->operator[](1)->GetHealth();
    const int ownHealthBefore = current->GetHero()->GetHealth();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(sleetStorm,
                                                    opponent->GetFieldZone()->operator[](0)));
    const int enemyHealthAfter = opponent->GetHero()->GetHealth() +
                                 opponent->GetFieldZone()->operator[](0)->GetHealth() +
                                 opponent->GetFieldZone()->operator[](1)->GetHealth();
    CHECK_EQ(enemyHealthBefore - enemyHealthAfter, 3);
    CHECK_EQ(current->GetHero()->GetHealth(), ownHealthBefore);
}

TEST_CASE("[ManaMind effect composition] - TIME_015 heals hero and grants Divine Shield")
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
    Player* current = game.GetPlayer1();
    current->GetHero()->SetDamage(5);
    const auto protector = Generic::DrawCard(current, Cards::FindCardByID("TIME_015"));
    REQUIRE(protector != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(protector));
    CHECK_EQ(current->GetHero()->GetHealth(), 28);
    CHECK_EQ(current->GetHero()->GetGameTag(GameTag::DIVINE_SHIELD), 1);
}

TEST_CASE("[ManaMind effect composition] - TLC_401 fires three non-spell damage hits")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto stegodon = Generic::DrawCard(current, Cards::FindCardByID("TLC_401"));
    REQUIRE(stegodon != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(stegodon));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);

    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(current->GetFieldZone()->operator[](0));
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(opponent->GetHero()->GetHealth(), 12);
}

TEST_CASE("[ManaMind effect composition] - RLK_223 triggers on Battlecry and Deathrattle with Reborn")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto thassarian = Generic::DrawCard(current, Cards::FindCardByID("RLK_223"));
    REQUIRE(thassarian != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(thassarian));
    CHECK_EQ(opponent->GetHero()->GetHealth(), 28);
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);

    DestroyTask destroy(EntityType::SOURCE);
    destroy.SetPlayer(current);
    destroy.SetSource(current->GetFieldZone()->operator[](0));
    REQUIRE_EQ(destroy.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(opponent->GetHero()->GetHealth(), 26);
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 1);
}

TEST_CASE("[ManaMind keyword-only] - CORE_EX1_250 preserves Taunt and Overload")
{
    GameConfig config;
    config.player1Class = CardClass::SHAMAN;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    const auto elemental = Generic::DrawCard(current, Cards::FindCardByID("CORE_EX1_250"));
    REQUIRE(elemental != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(elemental));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetGameTag(GameTag::TAUNT), 1);
    CHECK_EQ(current->GetOverloadOwed(), 2);
}

TEST_CASE("[ManaMind effect composition] - CATA_303 heals the enemy hero only when its target dies")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto spell = Generic::DrawCard(current, Cards::FindCardByID("CATA_303"));
    auto* target = dynamic_cast<Character*>(Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(spell != nullptr);
    REQUIRE(target != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(target));
    target->SetDamage(4);
    opponent->GetHero()->SetDamage(8);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(spell, target));
    CHECK_EQ(opponent->GetFieldZone()->GetCount(), 0);
    CHECK_EQ(opponent->GetHero()->GetHealth(), 27);
}

TEST_CASE("[ManaMind effect composition] - END_014 buffs a random friendly minion after a kill")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto spell = Generic::DrawCard(current, Cards::FindCardByID("END_014"));
    const auto friendly = Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti"));
    auto* target = dynamic_cast<Character*>(Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(spell != nullptr);
    REQUIRE(friendly != nullptr);
    REQUIRE(target != nullptr);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(friendly));
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(target));
    target->SetDamage(4);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::SpellTarget(spell, target));
    CHECK_EQ(opponent->GetFieldZone()->GetCount(), 0);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetAttack(), 7);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetHealth(), 8);
}

TEST_CASE("[ManaMind effect composition] - TLC_606 gains Armor when its target dies")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto minion = Generic::DrawCard(current, Cards::FindCardByID("TLC_606"));
    auto* target = dynamic_cast<Character*>(Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(minion != nullptr);
    REQUIRE(target != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(target));
    target->SetDamage(3);
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::MinionTarget(minion, target));
    CHECK_EQ(opponent->GetFieldZone()->GetCount(), 0);
    CHECK_EQ(current->GetHero()->GetArmor(), 5);
}

TEST_CASE("[ManaMind effect composition] - EDR_468 damages a minion and gives it +4 Attack")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto minion = Generic::DrawCard(current, Cards::FindCardByID("EDR_468"));
    auto* target = dynamic_cast<Character*>(Generic::DrawCard(opponent, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(minion != nullptr);
    REQUIRE(target != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(target));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::MinionTarget(minion, target));
    CHECK_EQ(target->GetHealth(), 4);
    CHECK_EQ(target->GetAttack(), 8);
}

TEST_CASE("[ManaMind effect composition] - JAIL_376 buffs only damaged friendly minions on Deathrattle")
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
    Player* current = game.GetPlayer1();
    const auto weapon = Generic::DrawCard(current, Cards::FindCardByID("JAIL_376"));
    auto* damaged = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti")));
    auto* undamaged = dynamic_cast<Character*>(Generic::DrawCard(current, Cards::FindCardByName("Chillwind Yeti")));
    REQUIRE(weapon != nullptr);
    REQUIRE(damaged != nullptr);
    REQUIRE(undamaged != nullptr);
    current->SetTotalMana(20);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(damaged));
    game.Process(current, PlayCardTask::Minion(undamaged));
    damaged->SetDamage(1);
    game.Process(current, PlayCardTask::Weapon(weapon));
    DestroyTask destroyWeapon(EntityType::WEAPON);
    destroyWeapon.SetPlayer(current);
    destroyWeapon.SetSource(current->GetHero());
    REQUIRE_EQ(destroyWeapon.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(damaged->GetAttack(), 5);
    CHECK_EQ(damaged->GetHealth(), 6);
    CHECK_EQ(undamaged->GetAttack(), 4);
    CHECK_EQ(undamaged->GetHealth(), 5);
}

TEST_CASE("[ManaMind effect composition] - JAIL_462 grants Charge after drawing two minions")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = true;
    config.fillCardIDs.fill(Cards::FindCardByName("Chillwind Yeti")->id);
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    Player* current = game.GetPlayer1();
    const auto hogdriver = Generic::DrawCard(current, Cards::FindCardByID("JAIL_462"));
    REQUIRE(hogdriver != nullptr);
    const auto handBefore = current->GetHandZone()->GetCount();
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(hogdriver));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    CHECK_EQ(current->GetFieldZone()->operator[](0)->GetGameTag(GameTag::CHARGE), 1);
    CHECK_EQ(current->GetHandZone()->GetCount(), handBefore + 1);
}

TEST_CASE("[ManaMind effect composition] - EDR_572 draws two Dragons and reduces their Costs")
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
    Player* current = game.GetPlayer1();
    const auto dreadwing = Generic::DrawCard(current, Cards::FindCardByID("EDR_572"));
    const auto handBefore = current->GetHandZone()->GetCount();
    auto* dragonCard = Cards::FindCardByID("DREAM_03");
    REQUIRE(dreadwing != nullptr);
    REQUIRE(dragonCard != nullptr);
    current->GetDeckZone()->Add(Entity::GetFromCard(current, dragonCard));
    current->GetDeckZone()->Add(Entity::GetFromCard(current, dragonCard));
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::Minion(dreadwing));
    REQUIRE_EQ(current->GetFieldZone()->GetCount(), 1);
    DestroyTask destroyMinion(EntityType::SOURCE);
    destroyMinion.SetPlayer(current);
    destroyMinion.SetSource(current->GetFieldZone()->operator[](0));
    REQUIRE_EQ(destroyMinion.Run(), TaskStatus::COMPLETE);
    game.ProcessDestroyAndUpdateAura();
    REQUIRE_EQ(current->GetHandZone()->GetCount(), handBefore + 1);
    int reducedDragons = 0;
    for (const auto* card : current->GetHandZone()->GetAll())
    {
        if (card->card->id == dragonCard->id)
        {
            ++reducedDragons;
            CHECK_EQ(card->GetCost(), dragonCard->GetCost() - 1);
        }
    }
    CHECK_EQ(reducedDragons, 2);
}

TEST_CASE("[ManaMind effect composition] - TLC_633 only targets minions with a minion type")
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
    Player* current = game.GetPlayer1();
    Player* opponent = game.GetPlayer2();
    const auto bugsquasher = Generic::DrawCard(current, Cards::FindCardByID("TLC_633"));
    auto* dragonCard = Cards::FindCardByID("DREAM_03");
    REQUIRE(bugsquasher != nullptr);
    REQUIRE(dragonCard != nullptr);
    const auto dragon = Generic::DrawCard(opponent, dragonCard);
    REQUIRE(dragon != nullptr);
    game.Process(current, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    opponent->SetTotalMana(10);
    opponent->SetUsedMana(0);
    game.Process(opponent, PlayCardTask::Minion(dragon));
    game.Process(opponent, EndTurnTask());
    game.ProcessUntil(Step::MAIN_ACTION);
    current->SetTotalMana(10);
    current->SetUsedMana(0);
    game.Process(current, PlayCardTask::MinionTarget(bugsquasher, dragon));
    CHECK_EQ(opponent->GetFieldZone()->GetCount(), 0);
}
