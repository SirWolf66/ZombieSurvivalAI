#pragma once
class SteeringAgent;
struct SteeringOutput;
class ISteeringBehavior;

//Plugin specific structs
struct SteeringPlugin_Output;
struct AgentInfo;

//ZombieSurvival specific classes
class ZombieSurvivalBehaviorTree;

class ZombieSurvivalAI
{
public:
	//Constructor and destructor
	ZombieSurvivalAI(IExamInterface* m_pInterface);
	~ZombieSurvivalAI();

	//Public functions
	void Update(float deltaTime, SteeringPlugin_Output& steeringBehaviorOutout, const AgentInfo& agentInfo, const vector<HouseInfo>& houseInfoVector, const vector<EntityInfo>& entitiesInfoVector);
	void render(float deltaTime);

	void UpdateDebug(float deltaTime);	//The update functions exclusivly used for debug purposes

private:
	//Datamembers
	Elite::Vector2 m_FinalDestination;

	SteeringAgent* m_pAgent;
	ISteeringBehavior* m_pSteeringBehavior;
	ZombieSurvivalBehaviorTree* m_pbehaviorTree;
	IExamInterface* m_pInterface;
	AgentInfo* m_pAgentinfo;
	
	//Private functions
	//Update the steeringAgent with information about the Agent with the information found in AgentInfo, which is provided by the plugin
	void UpdateSteeringAgentWithAgentInfo(const AgentInfo& agentInfo, SteeringAgent* SteeringAgent);

	//The SteeringOutput_Plugin also requires the AutoOrientate, located in the SteerinAgent and it also requires the bool RunMode, specific to the plugin
	SteeringPlugin_Output SteeringOutput_To_SteeringPlugin_Output(const SteeringOutput& steeringOutput, bool AutoOrientate = true, bool RunMode = false);

	//Get the next pathPoint on th way to the final destination
	void GotoPoint(ISteeringBehavior* steeringBehavior);
};

