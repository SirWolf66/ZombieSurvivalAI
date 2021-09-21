#pragma once
#include "Steering/SteeringHelpers.h"

class SteeringAgent;
class ISteeringBehavior;

class ZombieSurvivalBehaviorTree final
	{
	public:
		//--- Constructor & Destructor ---
		ZombieSurvivalBehaviorTree(SteeringAgent* pAgent, IExamInterface* pExamInterface);
		~ZombieSurvivalBehaviorTree();

		//--- Functions ---
		void Initialize();
		void Update(float deltaTime, SteeringPlugin_Output& steeringOutputPlugin, const Elite::Vector2& targetPos/*Manual overide*/, const AgentInfo& agentInfo, const vector<HouseInfo>& houseInfoVector, const vector<EntityInfo>& entitiesInfoVector);
		void Render(float deltaTime);

	private:
		//--- Datamembers ---
		SteeringAgent* m_pAgent = nullptr;
		IExamInterface* m_pInterface = nullptr;
		std::vector<ISteeringBehavior*> m_vpBehaviorVec = {};
		Elite::BehaviorTree* m_pBehaviorTree = nullptr;
		TargetData m_Target = {};

		//Private functions
		void UpdateHouses();
};



