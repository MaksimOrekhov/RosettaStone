// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeHyun Jeon

#include <doctest/doctest.h>

#include <Rosetta/PlayMode/Cards/Cards.hpp>
#include <Rosetta/PlayMode/Cards/CardDefs.hpp>
#include <Rosetta/PlayMode/Actions/Choose.hpp>
#include <Rosetta/PlayMode/Actions/Draw.hpp>
#include <Rosetta/PlayMode/Actions/Generic.hpp>
#include <Rosetta/PlayMode/Actions/PlayCard.hpp>
#include <Rosetta/PlayMode/Actions/Summon.hpp>
#include <Rosetta/PlayMode/Tasks/PlayerTasks/PlayCardTask.hpp>
#include <Rosetta/PlayMode/Games/Game.hpp>
#include <Rosetta/PlayMode/Enchants/Effects.hpp>
#include <Rosetta/PlayMode/Models/Enchantment.hpp>
#include <Rosetta/PlayMode/Models/DarkGift.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/DarkGiftDiscoverTask.hpp>
#include <Rosetta/PlayMode/Zones/DeckZone.hpp>
#include <Rosetta/PlayMode/Zones/FieldZone.hpp>
#include <Rosetta/PlayMode/Zones/GraveyardZone.hpp>
#include <Rosetta/PlayMode/Zones/HandZone.hpp>
#include <Rosetta/PlayMode/Zones/SetasideZone.hpp>

#include <algorithm>
#include <random>
#include <unordered_set>

using namespace RosettaStone;
using namespace RosettaStone::PlayMode;

TEST_CASE("[ManaMind Dark Gift] - eligibility follows printed minion properties")
{
    Card minion;
    minion.gameTags[GameTag::CARDTYPE] = static_cast<int>(CardType::MINION);
    minion.gameTags[GameTag::ATK] = 3;
    minion.gameTags[GameTag::HEALTH] = 2;
    minion.gameTags[GameTag::BATTLECRY] = 1;

    const auto gifts = GetEligibleDarkGifts(MakeDarkGiftCandidate(minion));
    REQUIRE_EQ(gifts.size(), 10);
    CHECK(std::ranges::find(gifts, DarkGift::DISCOUNT_ATTACK) != gifts.end());
}

TEST_CASE("[ManaMind Dark Gift] - keyword gifts are excluded when redundant")
{
    Card minion;
    minion.gameTags[GameTag::CARDTYPE] = static_cast<int>(CardType::MINION);
    minion.gameTags[GameTag::ATK] = 2;
    minion.gameTags[GameTag::HEALTH] = 3;
    minion.gameTags[GameTag::LIFESTEAL] = 1;
    minion.gameTags[GameTag::CANT_BE_TARGETED_BY_OPPONENTS] = 1;
    minion.gameTags[GameTag::CHARGE] = 1;
    minion.gameTags[GameTag::TAUNT] = 1;
    minion.gameTags[GameTag::REBORN] = 1;
    minion.gameTags[GameTag::DIVINE_SHIELD] = 1;

    const auto gifts = GetEligibleDarkGifts(MakeDarkGiftCandidate(minion));
    CHECK_EQ(gifts.size(), 2);
    CHECK(std::ranges::find(gifts, DarkGift::SUMMON_COPY) != gifts.end());
    CHECK(std::ranges::find(gifts, DarkGift::STATS_TOP_DECK) != gifts.end());
}

TEST_CASE("[ManaMind Dark Gift] - non-minions cannot receive a gift")
{
    Card spell;
    spell.gameTags[GameTag::CARDTYPE] = static_cast<int>(CardType::SPELL);
    spell.gameTags[GameTag::ATK] = 0;

    CHECK(GetEligibleDarkGifts(MakeDarkGiftCandidate(spell)).empty());
}

TEST_CASE("[ManaMind Dark Gift] - current Elusive metadata maps to targeting rules")
{
    const auto* flitterwing = Cards::FindCardByID("CATA_133");
    REQUIRE(flitterwing != nullptr);
    CHECK(flitterwing->HasGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS));
    CHECK(flitterwing->HasGameTag(GameTag::CANT_BE_TARGETED_BY_HERO_POWERS));
    const auto gifts = GetEligibleDarkGifts(MakeDarkGiftCandidate(*flitterwing));
    CHECK(std::ranges::find(gifts, DarkGift::STATS_ELUSIVE) == gifts.end());
}

TEST_CASE("[ManaMind Dark Gift] - assignment gives each option a distinct compatible gift")
{
    Card ordinaryMinion;
    ordinaryMinion.gameTags[GameTag::CARDTYPE] =
        static_cast<int>(CardType::MINION);
    ordinaryMinion.gameTags[GameTag::ATK] = 2;
    ordinaryMinion.gameTags[GameTag::HEALTH] = 2;

    Card battlecryMinion = ordinaryMinion;
    battlecryMinion.gameTags[GameTag::ATK] = 3;
    battlecryMinion.gameTags[GameTag::BATTLECRY] = 1;

    const std::vector<DarkGiftCandidate> candidates = {
        MakeDarkGiftCandidate(ordinaryMinion),
        MakeDarkGiftCandidate(battlecryMinion),
        MakeDarkGiftCandidate(ordinaryMinion) };
    std::mt19937 randomEngine(7821);
    const auto gifts = AssignDistinctDarkGifts(candidates, randomEngine);

    REQUIRE_EQ(gifts.size(), candidates.size());
    auto sortedGifts = gifts;
    std::ranges::sort(sortedGifts);
    CHECK(std::ranges::adjacent_find(sortedGifts) == sortedGifts.end());
    for (std::size_t i = 0; i < candidates.size(); ++i)
    {
        const auto eligible = GetEligibleDarkGifts(candidates[i]);
        CHECK(std::ranges::find(eligible, gifts[i]) != eligible.end());
    }
}

TEST_CASE("[ManaMind Dark Gift] - assignment reports when no distinct matching exists")
{
    Card restricted;
    restricted.gameTags[GameTag::CARDTYPE] = static_cast<int>(CardType::MINION);
    restricted.gameTags[GameTag::ATK] = 2;
    restricted.gameTags[GameTag::LIFESTEAL] = 1;
    restricted.gameTags[GameTag::CANT_BE_TARGETED_BY_OPPONENTS] = 1;
    restricted.gameTags[GameTag::CHARGE] = 1;
    restricted.gameTags[GameTag::TAUNT] = 1;
    restricted.gameTags[GameTag::REBORN] = 1;
    restricted.gameTags[GameTag::DIVINE_SHIELD] = 1;

    std::mt19937 randomEngine(7822);
    const auto restrictedCandidate = MakeDarkGiftCandidate(restricted);
    const std::vector<DarkGiftCandidate> candidates = {
        restrictedCandidate, restrictedCandidate, restrictedCandidate };
    CHECK(AssignDistinctDarkGifts(candidates, randomEngine).empty());
}

TEST_CASE("[ManaMind Dark Gift] - eligibility reads the current entity state")
{
    GameConfig config;
    config.player1Class = CardClass::DRUID;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;

    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* flitterwingCard = Cards::FindCardByID("CATA_133");
    REQUIRE(flitterwingCard != nullptr);
    auto* flitterwing = Generic::DrawCard(player, flitterwingCard);
    flitterwing->SetGameTag(GameTag::ATK, 2);
    flitterwing->SetGameTag(GameTag::CHARGE, 1);

    const auto gifts = GetEligibleDarkGifts(MakeDarkGiftCandidate(*flitterwing));
    CHECK(std::ranges::find(gifts, DarkGift::DISCOUNT_ATTACK) == gifts.end());
    CHECK(std::ranges::find(gifts, DarkGift::CHARGE) == gifts.end());
    CHECK(std::ranges::find(gifts, DarkGift::STATS_ELUSIVE) == gifts.end());
}

TEST_CASE("[ManaMind Dark Gift] - resolver applies all ten immediate gifts")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();

    Card card;
    card.gameTags[GameTag::CARDTYPE] = static_cast<int>(CardType::MINION);
    card.gameTags[GameTag::ATK] = 4;
    card.gameTags[GameTag::HEALTH] = 5;
    card.gameTags[GameTag::COST] = 5;
    card.gameTags[GameTag::BATTLECRY] = 1;

    const auto createMinion = [&]() {
        return dynamic_cast<Minion*>(Entity::GetFromCard(player, &card));
    };
    auto* lifesteal = createMinion();
    ApplyDarkGift(*lifesteal, DarkGift::ATTACK_LIFESTEAL);
    CHECK_EQ(lifesteal->GetAttack(), 7);
    CHECK_EQ(lifesteal->GetGameTag(GameTag::LIFESTEAL), 1);

    auto* elusive = createMinion();
    ApplyDarkGift(*elusive, DarkGift::STATS_ELUSIVE);
    CHECK_EQ(elusive->GetAttack(), 6);
    CHECK_EQ(elusive->GetBaseHealth(), 7);
    CHECK_EQ(elusive->GetGameTag(GameTag::CANT_BE_TARGETED_BY_OPPONENTS), 1);
    CHECK_EQ(elusive->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 1);
    CHECK_EQ(elusive->GetGameTag(GameTag::CANT_BE_TARGETED_BY_HERO_POWERS), 1);

    auto* discount = createMinion();
    ApplyDarkGift(*discount, DarkGift::DISCOUNT_ATTACK);
    CHECK_EQ(discount->GetGameTag(GameTag::COST), 3);
    CHECK_EQ(discount->GetAttack(), 2);

    auto* charge = createMinion();
    ApplyDarkGift(*charge, DarkGift::CHARGE);
    CHECK_EQ(charge->GetGameTag(GameTag::CHARGE), 1);

    auto* copy = createMinion();
    ApplyDarkGift(*copy, DarkGift::SUMMON_COPY);
    CHECK_EQ(copy->GetGameTag(GameTag::MANAMIND_DARK_GIFT_ID),
             static_cast<int>(DarkGift::SUMMON_COPY));

    auto* doubleBattlecry = createMinion();
    ApplyDarkGift(*doubleBattlecry, DarkGift::DOUBLE_BATTLECRY);
    CHECK_EQ(doubleBattlecry->GetGameTag(GameTag::MANAMIND_DARK_GIFT_ID),
             static_cast<int>(DarkGift::DOUBLE_BATTLECRY));

    auto* taunt = createMinion();
    ApplyDarkGift(*taunt, DarkGift::HEALTH_TAUNT);
    CHECK_EQ(taunt->GetBaseHealth(), 9);
    CHECK_EQ(taunt->GetGameTag(GameTag::TAUNT), 1);

    auto* reborn = createMinion();
    ApplyDarkGift(*reborn, DarkGift::REBORN_FULL_HEALTH);
    CHECK_EQ(reborn->GetGameTag(GameTag::REBORN), 1);

    auto* topDeck = createMinion();
    ApplyDarkGift(*topDeck, DarkGift::STATS_TOP_DECK);
    CHECK_EQ(topDeck->GetAttack(), 8);
    CHECK_EQ(topDeck->GetBaseHealth(), 10);

    auto* shield = createMinion();
    ApplyDarkGift(*shield, DarkGift::DIVINE_SHIELD_WINDFURY);
    CHECK_EQ(shield->GetGameTag(GameTag::DIVINE_SHIELD), 1);
    CHECK_EQ(shield->GetGameTag(GameTag::WINDFURY), 1);
}

TEST_CASE("[ManaMind Dark Gift] - gifted Battlecry resolves twice")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    player->SetTotalMana(10);
    player->SetUsedMana(0);
    Card* printedCard = Cards::FindCardByID("CS2_117");
    REQUIRE(printedCard != nullptr);
    auto* minion = dynamic_cast<Minion*>(
        Generic::DrawCard(player, printedCard));
    REQUIRE(minion != nullptr);
    player->GetHero()->SetDamage(10);
    ApplyDarkGift(*minion, DarkGift::DOUBLE_BATTLECRY);

    game.Process(player,
                 PlayerTasks::PlayCardTask::MinionTarget(
                     minion, player->GetHero()));

    CHECK_EQ(player->GetHero()->GetHealth(), 26);
}

TEST_CASE("[ManaMind Dark Gift] - gifted minion summons a 2/2 copy when played")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    player->SetTotalMana(10);
    player->SetUsedMana(0);
    Card* printedCard = Cards::FindCardByID("CS2_182");
    REQUIRE(printedCard != nullptr);
    auto* minion = dynamic_cast<Minion*>(
        Generic::DrawCard(player, printedCard));
    REQUIRE(minion != nullptr);
    ApplyDarkGift(*minion, DarkGift::SUMMON_COPY);

    game.Process(player, PlayerTasks::PlayCardTask::Minion(minion));

    const auto minions = player->GetFieldZone()->GetMinions();
    REQUIRE_EQ(minions.size(), 2);
    const auto* copy = minions.back();
    CHECK_EQ(copy->GetAttack(), 2);
    CHECK_EQ(copy->GetBaseHealth(), 2);
    CHECK_EQ(copy->GetGameTag(GameTag::MANAMIND_DARK_GIFT_ID), 0);
}

TEST_CASE("[ManaMind Dark Gift] - ChoicePick applies gift and moves selected option")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* source = Generic::DrawCard(player, Cards::FindCardByID("EDR_456"));
    auto* candidateCard = Cards::FindCardByID("CAP_804");
    REQUIRE(candidateCard != nullptr);
    auto* candidate = dynamic_cast<Minion*>(Entity::GetFromCard(
        player, candidateCard, std::nullopt, player->GetSetasideZone()));
    REQUIRE(candidate != nullptr);
    player->GetSetasideZone()->Add(candidate);

    auto choice = std::make_unique<Choice>(player);
    choice->choiceType = ChoiceType::GENERAL;
    choice->choiceAction = ChoiceAction::HAND;
    choice->source = source;
    const int candidateID = candidate->GetGameTag(GameTag::ENTITY_ID);
    choice->choices.emplace_back(candidateID);
    choice->darkGiftOptionsByEntityID.emplace_back(
        candidateID, DarkGift::ATTACK_LIFESTEAL);
    player->choice = std::move(choice);

    CHECK(Generic::ChoicePick(player, candidateID));
    CHECK(candidate->zone == player->GetHandZone());
    CHECK_EQ(candidate->GetAttack(),
             candidateCard->gameTags.at(GameTag::ATK) + 3);
    CHECK_EQ(candidate->GetGameTag(GameTag::LIFESTEAL), 1);
}

TEST_CASE("[ManaMind Dark Gift] - Reborn returns with full gifted health")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* original = dynamic_cast<Minion*>(Entity::GetFromCard(
        player, Cards::FindCardByID("CS2_182")));
    REQUIRE(original != nullptr);
    original->SetAttack(8);
    original->SetBaseHealth(9);
    original->SetDamage(4);
    ApplyDarkGift(*original, DarkGift::REBORN_FULL_HEALTH);
    Card* enchantmentCard = Cards::FindCardByID("BOT_083e");
    REQUIRE(enchantmentCard != nullptr);
    Enchantment::GetInstance(original, enchantmentCard, original);
    Effects::AttackN(1)->ApplyTo(original);
    player->GetFieldZone()->Add(original);
    player->GetFieldZone()->Remove(original);
    original->isDestroyed = true;
    player->GetGraveyardZone()->Add(original);

    Generic::SummonReborn(original);

    REQUIRE_EQ(player->GetFieldZone()->GetMinionCount(), 1);
    const auto* reborn = player->GetFieldZone()->GetMinions().front();
    CHECK_EQ(reborn->GetAttack(), 9);
    CHECK_EQ(reborn->GetBaseHealth(), 9);
    CHECK_EQ(reborn->GetHealth(), 9);
    CHECK_EQ(reborn->GetGameTag(GameTag::MANAMIND_REBORN_THIS_GAME), 0);
    REQUIRE_EQ(reborn->appliedEnchantments.size(), 1);
}

TEST_CASE("[ManaMind Dark Gift] - top deck gift returns the chosen deck entity to top")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* source = Generic::DrawCard(player, Cards::FindCardByID("EDR_856"));
    auto* candidateCard = Cards::FindCardByID("CAP_804");
    REQUIRE(candidateCard != nullptr);
    auto* candidate =
        dynamic_cast<Minion*>(Entity::GetFromCard(player, candidateCard));
    REQUIRE(candidate != nullptr);
    player->GetDeckZone()->Add(candidate);

    auto choice = std::make_unique<Choice>(player);
    choice->choiceType = ChoiceType::GENERAL;
    choice->choiceAction = ChoiceAction::DRAW_FROM_DECK;
    choice->source = source;
    const int candidateID = candidate->GetGameTag(GameTag::ENTITY_ID);
    choice->choices.emplace_back(candidateID);
    choice->darkGiftOptionsByEntityID.emplace_back(
        candidateID, DarkGift::STATS_TOP_DECK);
    player->choice = std::move(choice);

    CHECK(Generic::ChoicePick(player, candidateID));
    CHECK(candidate->zone == player->GetDeckZone());
    CHECK(player->GetDeckZone()->GetTopCard() == candidate);
    CHECK_EQ(candidate->GetAttack(),
             candidateCard->gameTags.at(GameTag::ATK) + 4);
    CHECK_EQ(candidate->GetBaseHealth(),
             candidateCard->gameTags.at(GameTag::HEALTH) + 5);
}

TEST_CASE("[ManaMind Dark Gift] - Dragon Discover pairs three cards with distinct gifts")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* source = Generic::DrawCard(player, Cards::FindCardByID("EDR_456"));

    SimpleTasks::DarkGiftDiscoverTask task(SimpleTasks::DarkGiftPool::DRAGON);
    task.SetPlayer(player);
    task.SetSource(source);
    CHECK(task.Run() == TaskStatus::COMPLETE);
    REQUIRE(player->choice != nullptr);
    CHECK(player->choice->choiceAction == ChoiceAction::HAND);
    REQUIRE_EQ(player->choice->choices.size(), 3);
    REQUIRE_EQ(player->choice->darkGiftOptionsByEntityID.size(), 3);

    std::unordered_set<int> giftIDs;
    for (const int entityID : player->choice->choices)
    {
        const auto* option = dynamic_cast<Playable*>(game.entityList.at(entityID));
        REQUIRE(option != nullptr);
        CHECK(option->card->GetRace() == Race::DRAGON);
        const auto giftPair = std::ranges::find(
            player->choice->darkGiftOptionsByEntityID, entityID, &std::pair<int, DarkGift>::first);
        REQUIRE(giftPair != player->choice->darkGiftOptionsByEntityID.end());
        const auto gift = giftPair->second;
        const auto eligible = GetEligibleDarkGifts(MakeDarkGiftCandidate(*option));
        CHECK(std::ranges::find(eligible, gift) != eligible.end());
        giftIDs.insert(static_cast<int>(gift));
    }
    CHECK_EQ(giftIDs.size(), 3);
    player->choice.reset();
}

TEST_CASE("[ManaMind Dark Gift] - deck Discover retains selected entity identity")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* source = Generic::DrawCard(player, Cards::FindCardByID("EDR_856"));
    const std::vector<std::string> deckMinionIDs = {
        "CAP_804", "CAP_806", "JAIL_912" };
    std::vector<int> deckEntityIDs;
    for (const auto& cardID : deckMinionIDs)
    {
        auto* card = Cards::FindCardByID(cardID);
        REQUIRE(card != nullptr);
        auto* entity = Entity::GetFromCard(player, card);
        player->GetDeckZone()->Add(entity);
        deckEntityIDs.emplace_back(entity->GetGameTag(GameTag::ENTITY_ID));
    }

    SimpleTasks::DarkGiftDiscoverTask task(SimpleTasks::DarkGiftPool::DECK_MINION);
    task.SetPlayer(player);
    task.SetSource(source);
    CHECK(task.Run() == TaskStatus::COMPLETE);
    REQUIRE(player->choice != nullptr);
    CHECK(player->choice->choiceAction == ChoiceAction::DRAW_FROM_DECK);
    REQUIRE_EQ(player->choice->choices.size(), 3);
    REQUIRE_EQ(player->choice->darkGiftOptionsByEntityID.size(), 3);
    for (const int entityID : player->choice->choices)
    {
        CHECK(std::ranges::find(deckEntityIDs, entityID) != deckEntityIDs.end());
        CHECK(player->game->entityList.at(entityID)->GetGameTag(GameTag::ZONE) ==
              std::to_underlying(ZoneType::DECK));
    }
}

TEST_CASE("[ManaMind Dark Gift] - selected card definitions are registered")
{
    CHECK(CardDefs::HasCardDefByID("EDR_456"));
    CHECK(CardDefs::HasCardDefByID("FIR_939"));
    CHECK(CardDefs::HasCardDefByID("EDR_856"));
}

TEST_CASE("[ManaMind Dark Gift] - Darkrider requires a Dragon in hand")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* source = Generic::DrawCard(player, Cards::FindCardByID("EDR_456"));

    SimpleTasks::DarkGiftDiscoverTask withoutDragon(
        SimpleTasks::DarkGiftPool::DRAGON, true);
    withoutDragon.SetPlayer(player);
    withoutDragon.SetSource(source);
    CHECK(withoutDragon.Run() == TaskStatus::STOP);
    CHECK(player->choice == nullptr);

    REQUIRE(Cards::FindCardByID("CATA_111") != nullptr);
    Generic::DrawCard(player, Cards::FindCardByID("CATA_111"));
    SimpleTasks::DarkGiftDiscoverTask withDragon(
        SimpleTasks::DarkGiftPool::DRAGON, true);
    withDragon.SetPlayer(player);
    withDragon.SetSource(source);
    CHECK(withDragon.Run() == TaskStatus::COMPLETE);
    REQUIRE(player->choice != nullptr);
    CHECK_EQ(player->choice->choices.size(), 3);
    player->choice.reset();
}

TEST_CASE("[ManaMind Dark Gift] - Shadowflame Suffusion damages before its gift choice")
{
    GameConfig config;
    config.player1Class = CardClass::WARRIOR;
    config.player2Class = CardClass::PRIEST;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    auto* opponent = game.GetOpponentPlayer();
    player->SetTotalMana(10);
    player->SetUsedMana(0);
    opponent->GetHero()->SetDamage(0);
    auto* spell = Generic::DrawCard(player, Cards::FindCardByID("FIR_939"));
    REQUIRE(spell != nullptr);

    game.Process(player, PlayerTasks::PlayCardTask::Spell(spell));

    CHECK_EQ(opponent->GetHero()->GetHealth(), 28);
    REQUIRE(player->choice != nullptr);
    CHECK_EQ(player->choice->choices.size(), 3);
    const int selected = player->choice->choices.front();
    CHECK(Generic::ChoicePick(player, selected));
    const auto* gifted = dynamic_cast<Playable*>(game.entityList.at(selected));
    REQUIRE(gifted != nullptr);
    REQUIRE(gifted->zone == player->GetHandZone());
    CHECK(gifted->GetGameTag(GameTag::MANAMIND_DARK_GIFT_ID) > 0);
}

TEST_CASE("[ManaMind Dark Gift] - Xavius chooses an original minion entity from deck")
{
    GameConfig config;
    config.player1Class = CardClass::PRIEST;
    config.player2Class = CardClass::WARRIOR;
    config.startPlayer = PlayerType::PLAYER1;
    config.doFillDecks = false;
    config.autoRun = false;
    Game game(config);
    game.Start();
    game.ProcessUntil(Step::MAIN_ACTION);
    auto* player = game.GetCurrentPlayer();
    player->SetTotalMana(10);
    player->SetUsedMana(0);
    const std::vector<std::string> deckMinionIDs = {
        "CAP_804", "CAP_806", "JAIL_912" };
    for (const auto& cardID : deckMinionIDs)
    {
        auto* card = Cards::FindCardByID(cardID);
        REQUIRE(card != nullptr);
        player->GetDeckZone()->Add(Entity::GetFromCard(player, card));
    }
    auto* xavius = Generic::DrawCard(player, Cards::FindCardByID("EDR_856"));
    REQUIRE(xavius != nullptr);

    game.Process(player, PlayerTasks::PlayCardTask::Minion(xavius));

    REQUIRE(player->choice != nullptr);
    CHECK(player->choice->choiceAction == ChoiceAction::DRAW_FROM_DECK);
    REQUIRE_EQ(player->choice->choices.size(), 3);
    const int selected = player->choice->choices.front();
    CHECK(Generic::ChoicePick(player, selected));
    const auto* gifted = dynamic_cast<Playable*>(game.entityList.at(selected));
    if (gifted->zone == player->GetHandZone())
    {
        CHECK(gifted->GetGameTag(GameTag::MANAMIND_DARK_GIFT_ID) > 0);
    }
    else
    {
        CHECK(player->GetDeckZone()->GetTopCard()->GetGameTag(
                  GameTag::ENTITY_ID) == selected);
        CHECK(player->GetDeckZone()->GetTopCard()->GetGameTag(
                  GameTag::MANAMIND_DARK_GIFT_ID) ==
              static_cast<int>(DarkGift::STATS_TOP_DECK));
    }
}
