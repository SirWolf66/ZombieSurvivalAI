#pragma once
#include "Exam_HelperStructs.h"
#include "Steering/SteeringBehaviors.h"

struct SteeringPlugin_Output_Extended : SteeringPlugin_Output
{
	//Variables
	bool IsValid = true;
};

struct AgentInfo_Extended : AgentInfo
{
	//Constructor
	AgentInfo_Extended() : AgentInfo()
	{
	}

	AgentInfo_Extended(AgentInfo agentInfo) : AgentInfo(agentInfo)
	{
	}

	//Variables
	ISteeringBehavior* SteeringBehavior = nullptr;
	bool IsValid = true;

	//Functions
	void Update(AgentInfo agentInfo)
	{
		Stamina = agentInfo.Stamina;
		Health = agentInfo.Health;
		Energy = agentInfo.Energy;
		RunMode = agentInfo.RunMode;
		IsInHouse = agentInfo.IsInHouse;
		Bitten = agentInfo.Bitten;
		WasBitten = agentInfo.WasBitten;
		Death = agentInfo.Death;

		FOV_Angle = agentInfo.FOV_Angle;
		FOV_Range = agentInfo.FOV_Range;

		LinearVelocity = agentInfo.LinearVelocity;
		AngularVelocity = agentInfo.AngularVelocity;
		CurrentLinearSpeed = agentInfo.CurrentLinearSpeed;
		Position = agentInfo.Position;
		Orientation = agentInfo.Orientation;
		MaxLinearSpeed = agentInfo.MaxLinearSpeed;
		MaxAngularSpeed = agentInfo.MaxAngularSpeed;
		GrabRange = agentInfo.GrabRange;	
		AgentSize = agentInfo.AgentSize;
	}
	//SteeringOutput CalculateSteeringOutput(float dt)
	//{
	//	return SteeringBehavior->CalculateSteering(dt, this);
	//}
};

