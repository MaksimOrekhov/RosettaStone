// ManaMind registrations for current Standard Dragon pool outcomes.
#ifndef ROSETTASTONE_MANAMIND_DRAGON_POOL_CARDS_GEN_HPP
#define ROSETTASTONE_MANAMIND_DRAGON_POOL_CARDS_GEN_HPP

#include <Rosetta/PlayMode/Cards/CardDef.hpp>

#include <map>
#include <string>

namespace RosettaStone::PlayMode
{
class ManaMindDragonPoolCardsGen
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::PlayMode

#endif  // ROSETTASTONE_MANAMIND_DRAGON_POOL_CARDS_GEN_HPP
