#pragma once
#include "_EliteFrameWork/EliteAI/EliteBehaviorTree/EBehaviorTree.h"

namespace Elite
{

	//-----------------------------------------------------------------
	// BEHAVIOR TREE COMPOSITES (IBehavior)
	//-----------------------------------------------------------------
#pragma region DECORATOR
//--- DECORATOR BASE ---
	class BehaviorDecorator : public IBehavior
	{
	public:
		explicit BehaviorDecorator(IBehavior* childrenBehavior)
		{
			m_ChildBehavior = childrenBehavior;
		}
		virtual ~BehaviorDecorator()
		{
			SAFE_DELETE(m_ChildBehavior);
		}

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override = 0;

	protected:
		IBehavior* m_ChildBehavior = {};
	};

	//--- Inverter ---
	class BehaviorInverter : public BehaviorDecorator
	{
	public:
		explicit BehaviorInverter(IBehavior* childrenBehavior) :
			BehaviorDecorator(childrenBehavior) {}
		virtual ~BehaviorInverter() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};


	//--- ForceSucces ---
	class BehaviorForceSucces : public BehaviorDecorator
	{
	public:
		explicit BehaviorForceSucces(IBehavior* childrenBehavior) :
			BehaviorDecorator(childrenBehavior) {}
		virtual ~BehaviorForceSucces() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	//--- ForceFailure ---
	class BehaviorForceFailure : public BehaviorDecorator
	{
	public:
		explicit BehaviorForceFailure(IBehavior* childrenBehavior) :
			BehaviorDecorator(childrenBehavior) {}
		virtual ~BehaviorForceFailure() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;
	};

	//--- Repeat ---
	class BehaviorRepeat : public BehaviorDecorator
	{
	public:
		explicit BehaviorRepeat(IBehavior* childrenBehavior, int repeatAmount) 
			: m_RepeatAmount{ repeatAmount }
			, BehaviorDecorator(childrenBehavior) {}
		virtual ~BehaviorRepeat() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		int m_RepeatAmount;
	};

	//--- Retry ---
	class BehaviorRetry : public BehaviorDecorator
	{
	public:
		explicit BehaviorRetry(IBehavior* childrenBehavior, int retryAmount)
			: m_RepeatAmount( retryAmount)
			, BehaviorDecorator(childrenBehavior) {}
		virtual ~BehaviorRetry() = default;

		virtual BehaviorState Execute(Blackboard* pBlackBoard) override;

	private:
		int m_RepeatAmount;
	};
}



