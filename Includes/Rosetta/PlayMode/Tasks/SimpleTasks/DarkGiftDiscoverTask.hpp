// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeHyun Jeon

#ifndef ROSETTASTONE_PLAYMODE_DARK_GIFT_DISCOVER_TASK_HPP
#define ROSETTASTONE_PLAYMODE_DARK_GIFT_DISCOVER_TASK_HPP

#include <Rosetta/PlayMode/Models/DarkGift.hpp>
#include <Rosetta/PlayMode/Tasks/ITask.hpp>

namespace RosettaStone::PlayMode::SimpleTasks
{
//! The source from which a Dark Gift Discover draws minion candidates.
enum class DarkGiftPool
{
    DRAGON,
    WARRIOR,
    DECK_MINION,
};

//! Creates paired minion-and-gift choices for current Dark Gift effects.
class DarkGiftDiscoverTask : public ITask
{
 public:
    explicit DarkGiftDiscoverTask(DarkGiftPool pool,
                                  bool requiresDragonInHand = false);

 private:
    TaskStatus Impl(Player* player) override;
    std::unique_ptr<ITask> CloneImpl() override;

    DarkGiftPool m_pool;
    bool m_requiresDragonInHand = false;
};
}  // namespace RosettaStone::PlayMode::SimpleTasks

#endif  // ROSETTASTONE_PLAYMODE_DARK_GIFT_DISCOVER_TASK_HPP
