// This code is based on Sabberstone project.
// Copyright (c) 2017-2019 SabberStone Team, darkfriend77 & rnilva
// RosettaStone is hearthstone simulator using C++ with reinforcement learning.
// Copyright (c) 2019 Chris Ohk, Youngjoong Kim, SeungHyun Jeon

#ifndef ROSETTASTONE_PLAYMODE_CAST_RANDOM_SPELL_TASK_HPP
#define ROSETTASTONE_PLAYMODE_CAST_RANDOM_SPELL_TASK_HPP

#include <Rosetta/PlayMode/Tasks/ITask.hpp>

namespace RosettaStone::PlayMode::SimpleTasks
{
//!
//! \brief CastRandomSpellTask class.
//!
//! This class represents the task for casting random spell.
//!
class CastRandomSpellTask : public ITask
{
 public:
    //! Optionally casts spells of one school up to a mana budget. When
    //! randomEnemyTargets is set, targeted spells must have an enemy target.
    explicit CastRandomSpellTask(SpellSchool spellSchool = SpellSchool::NONE,
                                 int manaBudget = 0,
                                 bool randomEnemyTargets = false);

 private:
    //! Processes task logic internally and returns meta data.
    //! \param player The player to run task.
    //! \return The result of task processing.
    TaskStatus Impl(Player* player) override;

    //! Internal method of Clone().
    //! \return The cloned task.
    std::unique_ptr<ITask> CloneImpl() override;

    SpellSchool m_spellSchool = SpellSchool::NONE;
    int m_manaBudget = 0;
    bool m_randomEnemyTargets = false;
};
}  // namespace RosettaStone::PlayMode::SimpleTasks

#endif  // ROSETTASTONE_PLAYMODE_CAST_RANDOM_SPELL_TASK_HPP
