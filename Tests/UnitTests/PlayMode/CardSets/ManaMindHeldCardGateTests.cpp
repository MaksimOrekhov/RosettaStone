// Independent expectations frozen in docs/proposals/20261002_held_card_gate_fixed_battlecry_v1.md.
#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Actions/Summon.hpp>
#include <algorithm>

namespace {
struct GateFixture {
    static GameConfig Config() {
        GameConfig c; c.player1Class = CardClass::PRIEST; c.player2Class = CardClass::MAGE;
        c.startPlayer = PlayerType::PLAYER1; c.doShuffle = false; c.skipMulligan = true; c.autoRun = false;
        for (int i=0;i<30;++i) { c.player1Deck[i]=Cards::FindCardByID("CS2_182"); c.player2Deck[i]=Cards::FindCardByID("CS2_182"); }
        return c;
    }
    Game game{Config()}; Player* own{}; Player* enemy{};
    GateFixture() { game.Start(); game.ProcessUntil(Step::MAIN_ACTION); own=game.GetPlayer1(); enemy=game.GetPlayer2(); own->SetTotalMana(10); own->SetUsedMana(0); }
    Playable* Draw(const char* id) { auto* card=Cards::FindCardByID(id); INFO("drawing " << id << " type=" << static_cast<int>(card->GetCardType())); return Generic::DrawCard(own, card); }
    Minion* Play(const char* id) { auto* m=dynamic_cast<Minion*>(Draw(id)); REQUIRE(m); game.Process(own, PlayCardTask::Minion(m)); return m; }
};
bool HasTarget(Card* card, Player* player, Character* target) {
    const auto targets=card->GetValidPlayTargets(player);
    return std::find(targets.begin(),targets.end(),target)!=targets.end();
}
}

TEST_CASE("[ManaMind held card gate] primary and secondary race membership") {
    GateFixture f;
    auto* primary=Cards::FindCardByID("EX1_561");
    auto* secondary=Cards::FindCardByID("EDR_818");
    CHECK(primary->HasRace(Race::DRAGON));
    CHECK(secondary->HasRace(Race::DRAGON));
    CHECK_FALSE(Cards::FindCardByID("CS2_182")->HasRace(Race::DRAGON));
    INFO("secondary type=" << static_cast<int>(secondary->GetCardType()));
    auto* held=Generic::DrawCard(f.own, secondary);
    auto* mother=dynamic_cast<Minion*>(f.Draw("CATA_111")); REQUIRE(mother);
    f.game.Process(f.own, PlayCardTask::Minion(mother));
    CHECK_EQ(f.own->GetUsedMana(), 1); // 3-cost source, then refresh 2 after leaving hand.
    GateFixture selfOnly; auto* source=selfOnly.Play("CATA_111");
    CHECK_EQ(selfOnly.own->GetUsedMana(), 3); // Its own Dragon race is no longer in hand.
    CHECK(source != nullptr);
}

TEST_CASE("[ManaMind held card gate] Dragon gated self keywords resolve from remaining hand") {
    GateFixture f; Generic::DrawCard(f.own, Cards::FindCardByID("EX1_561"));
    auto* keeper=f.Play("TIME_062");
    CHECK(keeper->HasTaunt()); CHECK(keeper->HasDivineShield());
    GateFixture falseCase; Generic::DrawCard(falseCase.own, Cards::FindCardByID("CS2_182"));
    auto* noGate=falseCase.Play("TIME_062");
    CHECK_FALSE(noGate->HasTaunt()); CHECK_FALSE(noGate->HasDivineShield());
}

TEST_CASE("[ManaMind held card gate] school and current-cost predicates gate fixed effects") {
    GateFixture shadow; Generic::DrawCard(shadow.own, Cards::FindCardByID("CS2_057"));
    auto* cultist=shadow.Play("CORE_RLK_814"); CHECK_EQ(cultist->GetAttack(), 2); CHECK_EQ(cultist->GetBaseHealth(), 3);
    GateFixture noShadow; Generic::DrawCard(noShadow.own, Cards::FindCardByID("CS2_029"));
    auto* noCultist=noShadow.Play("CORE_RLK_814"); CHECK_EQ(noCultist->GetAttack(), 1); CHECK_EQ(noCultist->GetBaseHealth(), 2);
    GateFixture cost; auto* held=cost.Draw("CS2_029"); held->SetCost(5);
    auto* pixie=cost.Play("FIR_961"); CHECK(pixie->HasDivineShield()); CHECK(pixie->HasLifesteal());
    GateFixture reduced; auto* cheap=reduced.Draw("CS2_029"); cheap->SetCost(4);
    auto* plain=reduced.Play("FIR_961"); CHECK_FALSE(plain->HasDivineShield()); CHECK_FALSE(plain->HasLifesteal());
}

TEST_CASE("[ManaMind held card gate] optional target list shares current-cost gate") {
    GateFixture f;
    auto* held=f.Draw("CS2_029"); held->SetCost(4);
    auto* untargeted=Cards::FindCardByID("EDR_472");
    CHECK(untargeted->GetValidPlayTargets(f.own).empty());
    held->SetCost(5);
    auto* friendly=dynamic_cast<Minion*>(Entity::GetFromCard(f.own, Cards::FindCardByID("CS2_182")));
    auto* opposing=dynamic_cast<Minion*>(Entity::GetFromCard(f.enemy, Cards::FindCardByID("CS2_182")));
    Generic::Summon(friendly,-1,nullptr); Generic::Summon(opposing,-1,nullptr);
    CHECK(HasTarget(untargeted,f.own,friendly)); CHECK(HasTarget(untargeted,f.own,opposing));
    CHECK(HasTarget(untargeted,f.own,f.own->GetHero())); CHECK(HasTarget(untargeted,f.own,f.enemy->GetHero()));
    opposing->SetGameTag(GameTag::STEALTH, 1);
    CHECK_FALSE(HasTarget(untargeted,f.own,opposing));
    opposing->SetGameTag(GameTag::STEALTH, 0);
    auto* consumer=dynamic_cast<Minion*>(f.Draw("EDR_472")); REQUIRE(consumer);
    f.game.Process(f.own, PlayCardTask::MinionTarget(consumer, opposing));
    CHECK_EQ(opposing->GetHealth(), 2);
    GateFixture optional; auto* below=optional.Draw("CS2_029"); below->SetCost(4);
    auto* blocker=dynamic_cast<Minion*>(Entity::GetFromCard(optional.enemy, Cards::FindCardByID("CS2_182")));
    Generic::Summon(blocker,-1,nullptr);
    auto* noTarget=dynamic_cast<Minion*>(optional.Draw("EDR_472")); REQUIRE(noTarget);
    CHECK(Cards::FindCardByID("EDR_472")->GetValidPlayTargets(optional.own).empty());
    optional.game.Process(optional.own, PlayCardTask::Minion(noTarget));
    CHECK_EQ(blocker->GetHealth(), 5);
}
