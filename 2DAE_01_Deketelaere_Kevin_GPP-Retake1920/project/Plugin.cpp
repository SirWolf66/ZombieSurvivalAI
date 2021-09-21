#include "stdafx.h"
#include "Plugin.h"
#include "IExamInterface.h"

//New includes
#include "ZombieSurvivalAI.h"

//Called only once, during initialization
void Plugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	//Retrieving the interface
	//This interface gives you access to certain actions the AI_Framework can perform for you
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	//Bit information about the plugin
	//Please fill this in!!
	info.BotName = "BobTheBot";
	info.Student_FirstName = "Kevin";
	info.Student_LastName = "Deketelaere";
	info.Student_Class = "2DA01";

	//Initializing the ai
	m_pAI = new ZombieSurvivalAI(m_pInterface);
}

//Called only once
void Plugin::DllInit()
{
	//Called when the plugin is loaded
}

//Called only once
void Plugin::DllShutdown()
{
	//Called wheb the plugin gets unloaded
}

//Called only once, during initialization
void Plugin::InitGameDebugParams(GameDebugParams& params)
{
#pragma region Setting level otions
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be usefull to inspect certain behaviours (Default = false)
							//params.LevelFile = "LevelThree.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
#pragma endregion
}

//Only Active in DEBUG Mode
//(=Use only for Debug Purposes)
void Plugin::Update(float dt)
{
	m_pAI->UpdateDebug(dt);
}

//Update
//This function calculates the new SteeringOutput, called once per frame
SteeringPlugin_Output Plugin::UpdateSteering(float dt)
{
	SteeringPlugin_Output steering = SteeringPlugin_Output();
	//Update the position of the Agent
	AgentInfo agentInfo = m_pInterface->Agent_GetInfo();				//Get the AgentInfo from the plugin

	//Call the ai update
	m_pAI->Update(dt, steering, agentInfo, GetHousesInFOV(), GetEntitiesInFOV());

	return steering;
}

//This function should only be used for rendering debug elements
void Plugin::Render(float dt) const
{
	//This Render function should only contain calls to Interface->Draw_... functions
	m_pAI->render(dt);
}

vector<HouseInfo> Plugin::GetHousesInFOV() const
{
	vector<HouseInfo> vHousesInFOV = {};

	HouseInfo hi = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetHouseByIndex(i, hi))
		{
			vHousesInFOV.push_back(hi);
			continue;
		}

		break;
	}

	return vHousesInFOV;
}

vector<EntityInfo> Plugin::GetEntitiesInFOV() const
{
	vector<EntityInfo> vEntitiesInFOV = {};

	EntityInfo ei = {};
	for (int i = 0;; ++i)
	{
		if (m_pInterface->Fov_GetEntityByIndex(i, ei))
		{
			vEntitiesInFOV.push_back(ei);
			continue;
		}

		break;
	}

	return vEntitiesInFOV;
}
