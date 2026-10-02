// Independent expectations frozen in minion_set_enchant_v1_expected_semantics.md.
#include <Utils/CardSetHeaders.hpp>
#include <Rosetta/PlayMode/Actions/Summon.hpp>
#include <Rosetta/PlayMode/Actions/Copy.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/AddEnchantmentTask.hpp>
#include <Rosetta/PlayMode/Tasks/SimpleTasks/ReturnHandTask.hpp>
#include <algorithm>

namespace {
struct Fixture {
    static GameConfig Config() {
        GameConfig c;
        c.player1Class = CardClass::PRIEST;
        c.player2Class = CardClass::MAGE;
        c.startPlayer = PlayerType::PLAYER1;
        c.doShuffle = false; c.skipMulligan = true; c.autoRun = false;
        for (int i = 0; i < 30; ++i) {
            c.player1Deck[i] = Cards::FindCardByID("CS2_182");
            c.player2Deck[i] = Cards::FindCardByID("CS2_182");
        }
        return c;
    }
    Game game{Config()};
    Player* own;
    Player* enemy;
    Fixture() {
        game.Start(); game.ProcessUntil(Step::MAIN_ACTION);
        own = game.GetPlayer1(); enemy = game.GetPlayer2(); Mana();
    }
    void Mana() { own->SetTotalMana(10); own->SetUsedMana(0); }
    Minion* Summon(Player* p, const char* id = "CS2_182") {
        auto* m = dynamic_cast<Minion*>(Entity::GetFromCard(p, Cards::FindCardByID(id)));
        REQUIRE(m != nullptr); Generic::Summon(m, -1, nullptr); return m;
    }
    Playable* Draw(const char* id) { Mana(); return Generic::DrawCard(own, Cards::FindCardByID(id)); }
    void Shell() { game.Process(own, PlayCardTask::Spell(Draw("RLK_048"))); }
    Minion* Helper() {
        auto* m = dynamic_cast<Minion*>(Draw("TLC_233"));
        game.Process(own, PlayCardTask::Minion(m)); return m;
    }
    void Barrier(Character* target) { game.Process(own, PlayCardTask::SpellTarget(Draw("TIME_447"), target)); }
};
bool Contains(const std::vector<Character*>& targets, Character* target) {
    return std::find(targets.begin(), targets.end(), target) != targets.end();
}
}

TEST_CASE("[ManaMind minion set enchant] Shell recipients and silence") {
    Fixture f;
    auto* own = f.Summon(f.own); auto* enemy = f.Summon(f.enemy);
    own->SetDamage(1);
    f.Shell();
    CHECK_EQ(own->GetAttack(), 5); CHECK_EQ(own->GetBaseHealth(), 6);
    CHECK_EQ(own->GetHealth(), 5); CHECK_EQ(own->GetDamage(), 1);
    CHECK_EQ(own->card->gameTags.at(GameTag::ATK), 4); CHECK_EQ(own->card->gameTags.at(GameTag::HEALTH), 5);
    CHECK_EQ(enemy->GetAttack(), 4); CHECK_EQ(enemy->GetBaseHealth(), 5);
    CHECK_EQ(f.own->GetHero()->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 0);
    CHECK_EQ(own->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 1);
    CHECK_EQ(own->GetGameTag(GameTag::CANT_BE_TARGETED_BY_HERO_POWERS), 1);
    own->Silence();
    CHECK_EQ(own->GetAttack(), 4); CHECK_EQ(own->GetBaseHealth(), 5);
    CHECK_EQ(own->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 0);
    CHECK_EQ(own->GetGameTag(GameTag::CANT_BE_TARGETED_BY_HERO_POWERS), 0);
}

TEST_CASE("[ManaMind minion set enchant] Helper snapshots current attack once") {
    Fixture f;
    auto* zero = f.Summon(f.own); zero->SetAttack(0);
    auto* two = f.Summon(f.own); two->SetAttack(2);
    auto* three = f.Summon(f.own); three->SetAttack(3);
    auto* enemy = f.Summon(f.enemy); enemy->SetAttack(2);
    auto* source = f.Helper();
    CHECK_EQ(zero->GetAttack(), 1); CHECK(zero->HasTaunt());
    CHECK_EQ(two->GetAttack(), 3); CHECK(two->HasTaunt());
    CHECK_EQ(two->GetBaseHealth(), 6);
    CHECK_EQ(three->GetAttack(), 3); CHECK_FALSE(three->HasTaunt());
    CHECK_EQ(enemy->GetAttack(), 2); CHECK_FALSE(enemy->HasTaunt());
    CHECK_EQ(source->GetAttack(), 2); CHECK_EQ(source->GetBaseHealth(), 3);
    CHECK_FALSE(source->HasTaunt());
    two->Silence(); CHECK_EQ(two->GetAttack(), 4); CHECK_FALSE(two->HasTaunt());
}

TEST_CASE("[ManaMind minion set enchant] Helper aura uses current rather than printed attack") {
    Fixture f;
    auto* low = f.Summon(f.own, "CS2_171"); // Stonetusk Boar: printed 1/1.
    auto* leader = f.Summon(f.own, "CS2_122"); // Raid Leader gives other minions +1 Attack.
    f.game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(low->GetAttack(), 2);
    f.Helper();
    CHECK_EQ(low->GetAttack(), 3); CHECK(low->HasTaunt());
    CHECK_EQ(leader->GetAttack(), 3); CHECK(leader->HasTaunt());
}

TEST_CASE("[ManaMind minion set enchant] repeated selection replaces previous stack") {
    Fixture f; auto* m = f.Summon(f.own); m->SetAttack(2);
    f.Helper(); CHECK_EQ(m->GetAttack(), 3); CHECK_EQ(m->GetBaseHealth(), 6);
    auto* firstSource = f.own->GetFieldZone()->GetMinions().back();
    f.Helper();
    CHECK_EQ(m->GetAttack(), 3); CHECK_EQ(m->GetBaseHealth(), 6);
    CHECK_EQ(firstSource->GetAttack(), 3); CHECK(firstSource->HasTaunt());
}

TEST_CASE("[ManaMind minion set enchant] fixed stats do not bake in attack or health auras") {
    Fixture f;
    auto* m = f.Summon(f.own);
    auto* champion = f.Summon(f.own, "CS2_222"); // Stormwind Champion: other +1/+1.
    f.game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(m->GetAttack(), 5); CHECK_EQ(m->GetBaseHealth(), 6);
    m->SetDamage(2); f.Shell();
    CHECK_EQ(m->GetAttack(), 6); CHECK_EQ(m->GetBaseHealth(), 7); CHECK_EQ(m->GetHealth(), 5);
    champion->Silence(); f.game.ProcessDestroyAndUpdateAura();
    CHECK_EQ(m->GetAttack(), 5); CHECK_EQ(m->GetBaseHealth(), 6);
}

TEST_CASE("[ManaMind minion set enchant] Shell repeat copy and bounce reset") {
    Fixture f; auto* m = f.Summon(f.own);
    f.Shell(); f.Shell(); CHECK_EQ(m->GetAttack(), 6); CHECK_EQ(m->GetBaseHealth(), 7);
    auto* copy = dynamic_cast<Minion*>(Generic::Copy(f.own, m, ZoneType::PLAY));
    REQUIRE(copy != nullptr);
    CHECK_EQ(copy->GetAttack(), 6); CHECK_EQ(copy->GetBaseHealth(), 7);
    CHECK_EQ(copy->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 1);
    copy->Silence(); CHECK_EQ(copy->GetAttack(), 4); CHECK_EQ(m->GetAttack(), 6);
    ReturnHandTask bounce(EntityType::TARGET); bounce.SetPlayer(f.own); bounce.SetTarget(m);
    CHECK_EQ(bounce.Run(), TaskStatus::COMPLETE);
    CHECK_EQ(m->GetAttack(), 4); CHECK_EQ(m->GetBaseHealth(), 5);
    CHECK_EQ(m->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 0);
    f.Mana(); f.game.Process(f.own, PlayCardTask::Minion(m));
    CHECK_EQ(m->GetAttack(), 4); CHECK_EQ(m->GetBaseHealth(), 5);
}

TEST_CASE("[ManaMind minion set enchant] Barrier all four character target kinds") {
    for (int kind = 0; kind < 4; ++kind) {
        Fixture f; auto* own = f.Summon(f.own); auto* enemy = f.Summon(f.enemy);
        Character* target = kind == 0 ? static_cast<Character*>(own) : kind == 1 ? static_cast<Character*>(enemy)
                            : kind == 2 ? static_cast<Character*>(f.own->GetHero()) : static_cast<Character*>(f.enemy->GetHero());
        Card* barrier = Cards::FindCardByID("TIME_447");
        CHECK(Contains(barrier->GetValidPlayTargets(f.own), target));
        f.Barrier(target); CHECK_EQ(target->GetGameTag(GameTag::DIVINE_SHIELD), 1);
        CHECK(Contains(barrier->GetValidPlayTargets(f.own), target)); // Already shielded is legal.
        const int hp = target->GetHealth();
        CHECK_EQ(target->TakeDamage(own, 3), 0); CHECK_EQ(target->GetHealth(), hp);
        CHECK_EQ(target->GetGameTag(GameTag::DIVINE_SHIELD), 0);
    }
}

TEST_CASE("[ManaMind minion set enchant] Barrier hand type controller and transfer") {
    Fixture f;
    auto* held = dynamic_cast<Minion*>(f.Draw("CS2_182"));
    auto* spell = f.Draw("CS2_029"); auto* weapon = f.Draw("CS2_106");
    auto* enemyHeld = Generic::DrawCard(f.enemy, Cards::FindCardByID("CS2_182"));
    f.Barrier(f.enemy->GetHero());
    CHECK_EQ(held->GetAttack(), 4); CHECK_EQ(held->GetBaseHealth(), 7);
    CHECK_EQ(held->card->gameTags.at(GameTag::HEALTH), 5); CHECK_EQ(enemyHeld->GetGameTag(GameTag::HEALTH), 5);
    CHECK_EQ(spell->GetGameTag(GameTag::HEALTH), 0); CHECK_EQ(weapon->GetGameTag(GameTag::ATK), 3);
    f.Mana(); f.game.Process(f.own, PlayCardTask::Minion(held));
    CHECK_EQ(held->GetBaseHealth(), 7); CHECK_EQ(held->GetHealth(), 7);
    held->Silence(); CHECK_EQ(held->GetBaseHealth(), 5);
}

TEST_CASE("[ManaMind minion set enchant] hero Shield protects armor and zero does not spend it") {
    Fixture f;
    auto* source = f.Summon(f.own);
    auto* hero = f.enemy->GetHero(); hero->SetArmor(5);
    f.Barrier(hero);
    CHECK_EQ(hero->TakeDamage(source, 0), 0);
    CHECK_EQ(hero->GetGameTag(GameTag::DIVINE_SHIELD), 1);
    CHECK_EQ(hero->TakeDamage(source, 3), 0);
    CHECK_EQ(hero->GetArmor(), 5); CHECK_EQ(hero->GetHealth(), 30);
    CHECK_EQ(hero->GetGameTag(GameTag::DIVINE_SHIELD), 0);
    hero->TakeDamage(source, 3); CHECK_EQ(hero->GetArmor(), 2);
}

TEST_CASE("[ManaMind minion set enchant] Barrier missing and protected targets fail") {
    Fixture f; auto* own = f.Summon(f.own); auto* enemy = f.Summon(f.enemy);
    f.Shell();
    Card* barrier = Cards::FindCardByID("TIME_447");
    CHECK_FALSE(Contains(barrier->GetValidPlayTargets(f.own), own));
    enemy->SetGameTag(GameTag::STEALTH, 1);
    CHECK_FALSE(Contains(barrier->GetValidPlayTargets(f.own), enemy));
    auto* held = dynamic_cast<Minion*>(f.Draw("CS2_182"));
    auto* card = f.Draw("TIME_447");
    const int count = f.own->GetHandZone()->GetCount();
    f.game.Process(f.own, PlayCardTask::Spell(card));
    CHECK_EQ(f.own->GetHandZone()->GetCount(), count); CHECK_EQ(held->GetBaseHealth(), 5);
    f.game.Process(f.own, PlayCardTask::SpellTarget(card, own));
    CHECK_EQ(f.own->GetHandZone()->GetCount(), count); CHECK_EQ(held->GetBaseHealth(), 5);
    CHECK_EQ(own->GetGameTag(GameTag::DIVINE_SHIELD), 0);
}

TEST_CASE("[ManaMind minion set enchant] Elusive spells powers attacks and Battlecry") {
    Fixture f; auto* m = f.Summon(f.own); auto* enemy = f.Summon(f.enemy);
    f.Shell();
    CHECK_FALSE(Contains(Cards::FindCardByID("CS2_029")->GetValidPlayTargets(f.enemy), m));
    CHECK_FALSE(Contains(f.enemy->GetHero()->heroPower->card->GetValidPlayTargets(f.enemy), m));
    CHECK(Contains(Cards::FindCardByID("EX1_564")->GetValidPlayTargets(f.enemy), m)); // Faceless Battlecry.
    CHECK(Contains(enemy->GetValidAttackTargets(f.own), m));
}

TEST_CASE("[ManaMind minion set enchant] Barrier respects intrinsic Elusive metadata") {
    Fixture f; auto* m = f.Summon(f.own, "CORE_NEW1_023");
    CHECK_EQ(m->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 1);
    CHECK_EQ(m->GetGameTag(GameTag::CANT_BE_TARGETED_BY_HERO_POWERS), 1);
    CHECK_FALSE(Contains(Cards::FindCardByID("TIME_447")->GetValidPlayTargets(f.own), m));
    CHECK_FALSE(Contains(Cards::FindCardByID("CS2_029")->GetValidPlayTargets(f.enemy), m));
    CHECK(Contains(Cards::FindCardByID("EX1_564")->GetValidPlayTargets(f.enemy), m));
}

TEST_CASE("[ManaMind minion set enchant] two fixed dependencies damaged apply remove") {
    for (const char* id : {"ICC_210e", "ULD_191e"}) {
        Fixture f; auto* m = f.Summon(f.own); m->SetDamage(2);
        CHECK_EQ(Cards::FindCardByID(id)->id, id);
        AddEnchantmentTask task(id, EntityType::TARGET); task.SetPlayer(f.own); task.SetTarget(m);
        CHECK_EQ(task.Run(), TaskStatus::COMPLETE);
        if (std::string(id) == "ICC_210e") {
            CHECK_EQ(m->GetAttack(), 5); CHECK_EQ(m->GetBaseHealth(), 6); CHECK_EQ(m->GetHealth(), 4);
        } else {
            CHECK_EQ(m->GetAttack(), 4); CHECK_EQ(m->GetBaseHealth(), 7); CHECK_EQ(m->GetHealth(), 5);
        }
        CHECK_EQ(m->GetDamage(), 2); m->Silence();
        CHECK_EQ(m->GetAttack(), 4); CHECK_EQ(m->GetBaseHealth(), 5);
    }
}

TEST_CASE("[ManaMind minion set enchant] empty sets and Location exclusion") {
    Fixture f; f.Shell(); CHECK_EQ(f.own->GetHero()->GetBaseHealth(), 30);
    auto* location = dynamic_cast<Location*>(Entity::GetFromCard(f.own, Cards::FindCardByID("REV_990")));
    REQUIRE(location != nullptr); f.own->GetFieldZone()->Add(location);
    const int hp = location->GetBaseHealth();
    f.Helper(); f.Shell(); CHECK_EQ(location->GetBaseHealth(), hp);
    CHECK_EQ(location->GetGameTag(GameTag::TAUNT), 0);
    CHECK_EQ(location->GetGameTag(GameTag::CANT_BE_TARGETED_BY_SPELLS), 0);
}

TEST_CASE("[ManaMind minion set enchant] empty hand does not remove character targets") {
    Fixture f;
    const auto initial = f.own->GetHandZone()->GetAll();
    for (auto* card : initial) f.own->GetHandZone()->Remove(card);
    auto* barrier = f.Draw("TIME_447");
    CHECK_EQ(f.own->GetHandZone()->GetCount(), 1);
    CHECK(Contains(barrier->GetValidPlayTargets(), f.enemy->GetHero()));
    f.game.Process(f.own, PlayCardTask::SpellTarget(barrier, f.enemy->GetHero()));
    CHECK_EQ(f.own->GetHandZone()->GetCount(), 0);
    CHECK_EQ(f.enemy->GetHero()->GetGameTag(GameTag::DIVINE_SHIELD), 1);
}

TEST_CASE("[ManaMind minion set enchant] repeated Barrier stacks only hand Health") {
    Fixture f; auto* held = dynamic_cast<Minion*>(f.Draw("CS2_182"));
    f.Barrier(f.own->GetHero()); f.Barrier(f.own->GetHero());
    CHECK_EQ(held->GetAttack(), 4); CHECK_EQ(held->GetBaseHealth(), 9);
    CHECK_EQ(f.own->GetHero()->GetGameTag(GameTag::DIVINE_SHIELD), 1);
    CHECK_EQ(f.own->GetHero()->TakeDamage(held, 1), 0);
    CHECK_EQ(f.own->GetHero()->GetGameTag(GameTag::DIVINE_SHIELD), 0);
    CHECK_EQ(f.own->GetHero()->TakeDamage(held, 1), 1);
}

TEST_CASE("[ManaMind minion set enchant] existing controls keep their distinct effects") {
    for (const char* id : {"CAP_801", "CORE_CS2_009", "CORE_ULD_191", "CORE_CFM_753"}) {
        Fixture f; auto* m = f.Summon(f.own);
        auto* held = dynamic_cast<Minion*>(f.Draw("CS2_182"));
        auto* card = f.Draw(id);
        if (std::string(id) == "CORE_CFM_753") {
            f.game.Process(f.own, PlayCardTask::Minion(card));
            CHECK_EQ(held->GetAttack(), 5); CHECK_EQ(held->GetBaseHealth(), 6);
            CHECK_EQ(m->GetAttack(), 4); CHECK_EQ(m->GetBaseHealth(), 5);
        } else {
            f.game.Process(f.own, PlayCardTask::SpellTarget(card, m));
            if (std::string(id) == "CORE_ULD_191") {
                CHECK_EQ(m->GetAttack(), 4); CHECK_EQ(m->GetBaseHealth(), 7);
                CHECK_FALSE(m->HasTaunt());
            } else {
                CHECK_EQ(m->GetAttack(), 6); CHECK_EQ(m->GetBaseHealth(), 8); CHECK(m->HasTaunt());
                CHECK_EQ(m->HasReborn(), std::string(id) == "CAP_801");
            }
        }
    }
}
