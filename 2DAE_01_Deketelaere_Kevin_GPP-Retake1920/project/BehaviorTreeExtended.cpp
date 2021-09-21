#include "stdafx.h"
//#include "BehaviorTreeExtended.h"

using namespace Elite;

//Inverter
BehaviorState BehaviorInverter::Execute(Blackboard* pBlackBoard)
{
	m_CurrentState = m_ChildBehavior->Execute(pBlackBoard);
	switch (m_CurrentState)
	{
	case BehaviorState::Failure:
		return m_CurrentState = BehaviorState::Success; 
		break;
	case BehaviorState::Success:
		return m_CurrentState = BehaviorState::Failure;
		break;
	case BehaviorState::Running:
		return m_CurrentState; 
		break;
	default:
		break;
	}
	return m_CurrentState = BehaviorState::Failure;
}

//Force Succes
BehaviorState BehaviorForceSucces::Execute(Blackboard * pBlackBoard)
{
	m_CurrentState = m_ChildBehavior->Execute(pBlackBoard);
	switch (m_CurrentState)
	{
	case BehaviorState::Failure:
		break;
	case BehaviorState::Success:
		return m_CurrentState;
		break;
	case BehaviorState::Running:
		return m_CurrentState; 
		break;
	default:
		break;
	}
	return m_CurrentState = BehaviorState::Success;
}

//Force Faillure
BehaviorState BehaviorForceFailure::Execute(Blackboard* pBlackBoard)
{
	m_CurrentState = m_ChildBehavior->Execute(pBlackBoard);
	switch (m_CurrentState)
	{
	case BehaviorState::Failure:
		return m_CurrentState;
		break;
	case BehaviorState::Success:
		return m_CurrentState = BehaviorState::Failure;
		break;
	case BehaviorState::Running:
		return m_CurrentState; 
		break;
	default:
		break;
	}
	return m_CurrentState = BehaviorState::Failure;
}

//Repeat
BehaviorState BehaviorRepeat::Execute(Blackboard* pBlackBoard)
{
	for (int i{ 0 }; i < m_RepeatAmount; i++)
	{
		m_CurrentState = m_ChildBehavior->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			return m_CurrentState;
			break;
		case BehaviorState::Success:
			continue;  break;
		case BehaviorState::Running:
			return m_CurrentState; 
			break;
		default:
			break;
		}
	}
	return m_CurrentState = BehaviorState::Success;
}

//Retry
BehaviorState BehaviorRetry::Execute(Blackboard * pBlackBoard)
{
	for (int i{}; i < m_RepeatAmount; i++)
	{
		m_CurrentState = m_ChildBehavior->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case BehaviorState::Failure:
			continue; break;
		case BehaviorState::Success:
			return m_CurrentState;
			break;
		case BehaviorState::Running:
			return m_CurrentState; 
			break;
		default:
			break;
		}
	}
	return m_CurrentState = BehaviorState::Failure;
}
