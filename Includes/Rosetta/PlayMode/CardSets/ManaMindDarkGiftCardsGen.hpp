// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeHyun Jeon

#ifndef ROSETTASTONE_MANAMIND_DARK_GIFT_CARDS_GEN_HPP
#define ROSETTASTONE_MANAMIND_DARK_GIFT_CARDS_GEN_HPP

#include <Rosetta/PlayMode/Cards/CardDef.hpp>

#include <map>
#include <string>

namespace RosettaStone::PlayMode
{
class ManaMindDarkGiftCardsGen
{
 public:
    static void AddAll(std::map<std::string, CardDef>& cards);
};
}  // namespace RosettaStone::PlayMode

#endif  // ROSETTASTONE_MANAMIND_DARK_GIFT_CARDS_GEN_HPP
