//=== General Includes ===

#include "stdafx.h"

//#include "EBehaviorTree.h"
//#include "EBlackboard.h"

using namespace Elite;

//-----------------------------------------------------------------
// BEHAVIOR TREE COMPOSITES (IBehavior)
//-----------------------------------------------------------------
#pragma region COMPOSITES
//SELECTOR
BehaviorState BehaviorSelector::Execute(Blackboard* pBlackBoard)
{
	for (auto child : m_ChildrenBehaviors)
	{
		m_CurrentState = child->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			continue; break;
		case BehaviorState::Success:
			return m_CurrentState; break;
		case BehaviorState::Running:
			return m_CurrentState; break;
		default:
			continue; break;
		}
	}
	return m_CurrentState = BehaviorState::Failure;
}
//SEQUENCE
BehaviorState BehaviorSequence::Execute(Blackboard* pBlackBoard)
{
	for (auto child : m_ChildrenBehaviors)
	{
		m_CurrentState = child->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			return m_CurrentState; break;
		case BehaviorState::Success:
			continue; break;
		case BehaviorState::Running:
			return m_CurrentState; break;
		default:
			m_CurrentState = BehaviorState::Success;
			return m_CurrentState; break;
		}
	}
	return m_CurrentState = BehaviorState::Success;
}
//PARTIAL SEQUENCE
BehaviorState BehaviorPartialSequence::Execute(Blackboard* pBlackBoard)
{
	while (m_CurrentBehaviorIndex < m_ChildrenBehaviors.size())
	{
		m_CurrentState = m_ChildrenBehaviors[m_CurrentBehaviorIndex]->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			m_CurrentBehaviorIndex = 0;
			return m_CurrentState; break;
		case BehaviorState::Success:
			++m_CurrentBehaviorIndex;
			return m_CurrentState = BehaviorState::Running; break;
		case BehaviorState::Running:
			return m_CurrentState; break;
		}
	}

	m_CurrentBehaviorIndex = 0;
	return m_CurrentState = BehaviorState::Success;
}
#pragma endregion
//-----------------------------------------------------------------
// BEHAVIOR TREE CONDITIONAL (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorConditional::Execute(Blackboard* pBlackBoard)
{
	if (m_fpConditional == nullptr)
		return BehaviorState::Failure;

	switch (m_fpConditional(pBlackBoard))
	{
	case true:
		return m_CurrentState = BehaviorState::Success;
	case false:
		return m_CurrentState = BehaviorState::Failure;
	}
	return m_CurrentState = BehaviorState::Failure;
}
//-----------------------------------------------------------------
// BEHAVIOR TREE ACTION (IBehavior)
//-----------------------------------------------------------------
BehaviorState BehaviorAction::Execute(Blackboard* pBlackBoard)
{
	if (m_fpAction == nullptr)
		return BehaviorState::Failure;

	return m_CurrentState = m_fpAction(pBlackBoard);
}