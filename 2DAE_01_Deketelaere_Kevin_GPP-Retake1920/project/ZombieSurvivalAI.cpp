#include "stdafx.h"
#include "ZombieSurvivalAI.h"

#include "Steering/SteeringBehaviors.h"
#include "BehaviorTree/ZombieSurvivalBehaviorTree.h"

#pragma region Constructor and destructor
ZombieSurvivalAI::ZombieSurvivalAI(IExamInterface* m_pInterface)
	:m_pAgent{new SteeringAgent()}
	, m_pSteeringBehavior{new Seek()}
	, m_pInterface{ m_pInterface }
	, m_pAgentinfo{ new AgentInfo() }
{
	//Set the steeringBehavior of the SteeringAgent
	m_pAgent->SetSteeringBehavior(m_pSteeringBehavior);
	m_pbehaviorTree = new ZombieSurvivalBehaviorTree(m_pAgent, m_pInterface);
}

ZombieSurvivalAI::~ZombieSurvivalAI()
{
	SAFE_DELETE(m_pAgent);
	SAFE_DELETE(m_pSteeringBehavior);
	SAFE_DELETE(m_pbehaviorTree);
}
#pragma endregion

#pragma region Public functions
void ZombieSurvivalAI::Update(float deltaTime, SteeringPlugin_Output& steeringBehaviorOutout, const AgentInfo& agentInfo, const vector<HouseInfo>& houseInfoVector, const vector<EntityInfo>& entitiesInfoVector)
{
	//Update the steeringAgent with the information from AgentInfo
	UpdateSteeringAgentWithAgentInfo(agentInfo, m_pAgent);												

	//Pass data about the agent to the Decision making structure (Behaviortree)
	m_pbehaviorTree->Update(deltaTime, steeringBehaviorOutout, m_FinalDestination, agentInfo, houseInfoVector, entitiesInfoVector);
}

void ZombieSurvivalAI::render(float deltaTime)
{
	m_pbehaviorTree->Render(deltaTime);
}

//Debug only
void ZombieSurvivalAI::UpdateDebug(float deltaTime)
{
	#pragma region Manual Target - Input
	//Demo Event Code
	//In the end your AI should be able to walk around without external input
	if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
	{
		//Update target based on input
		Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eLeft);
		const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
		m_FinalDestination = m_pInterface->Debug_ConvertScreenToWorld(pos);
	}
	#pragma endregion
}
#pragma endregion

#pragma region Private functions
void ZombieSurvivalAI::UpdateSteeringAgentWithAgentInfo(const AgentInfo& agentInfo, SteeringAgent* SteeringAgent)
{
	SteeringAgent->SetPosition(agentInfo.Position);
	SteeringAgent->SetLinearVelocity(agentInfo.LinearVelocity);
	SteeringAgent->SetAngularVelocity(agentInfo.AngularVelocity);
	SteeringAgent->SetMaxLinearSpeed(agentInfo.MaxLinearSpeed);
	SteeringAgent->SetMaxAngularSpeed(agentInfo.AngularVelocity);
}

SteeringPlugin_Output ZombieSurvivalAI::SteeringOutput_To_SteeringPlugin_Output(const SteeringOutput& steeringOutput, bool AutoOrientate, bool RunMode)
{
	SteeringPlugin_Output steeringPluginOutput{};
	steeringPluginOutput.LinearVelocity = steeringOutput.LinearVelocity;
	steeringPluginOutput.AngularVelocity = steeringOutput.AngularVelocity;
	steeringPluginOutput.AutoOrientate = AutoOrientate;

	steeringPluginOutput.RunMode;

	//IsVald is not being used in the plugin

	return steeringPluginOutput;
}

void ZombieSurvivalAI::GotoPoint( ISteeringBehavior* steeringBehavior)
{	
	Elite::Vector2 nextPathPoint{ m_pInterface->NavMesh_GetClosestPathPoint(m_FinalDestination) };
	steeringBehavior->SetTarget(nextPathPoint);

	m_pInterface->Draw_SolidCircle(nextPathPoint, .7f, { 0,0 }, { 0, 0, 1 });				//Closest Navigation mesh node
}
#pragma endregion


