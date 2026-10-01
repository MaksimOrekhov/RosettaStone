#include <doctest/doctest.h>

#include <Rosetta/PlayMode/Cards/CardDefs.hpp>

using namespace RosettaStone::PlayMode;

TEST_CASE("[ManaMind keyword-only cards] - supported keyword cards have CardDefs")
{
    CHECK(CardDefs::HasCardDefByID("CORE_GIL_558"));
    CHECK(CardDefs::HasCardDefByID("CORE_ULD_723"));
    CHECK(CardDefs::HasCardDefByID("EDR_486"));
    CHECK(CardDefs::HasCardDefByID("END_031"));
    CHECK(CardDefs::HasCardDefByID("RLK_067"));
    CHECK(CardDefs::HasCardDefByID("CORE_EX1_250"));
    CHECK_FALSE(CardDefs::HasCardDefByID("CORE_DRG_079"));
}
