//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "SteeringHelpers.h"
#include "PluginStructsExpanded.h"

#include "Agent/SteeringAgent.h"
//#include <IExamInterface.h>

//ISteeringBehavior static initialization
IExamInterface* ISteeringBehavior::m_pInterface = nullptr;

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};


	//steering.LinearVelocity = m_pInterface->NavMesh_GetClosestPathPoint(m_Target.Position) - pAgent->GetPosition();
	
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	steering.AngularVelocity = pAgent->GetAngularVelocity();

	/*if (Distance(m_Target.Position, pAgent->GetPosition() ) < 2.f)*/
	if (m_Target.Position == pAgent->GetPosition())
	{
		steering.LinearVelocity = Elite::ZeroVector2;
	}
	
	//std::vector<HouseInfo> houseInfovector = GetHousesInFOV();


	//Debug rendering
	//m_pInterface->Draw_Direction(pAgent->GetPosition, steering.LinearVelocity, pAgent->FOV_Range, { 0.1f, 1.f, 0.f }, 0.4f);	//Direction
	//m_pInterface->Draw_SolidCircle(m_pInterface->NavMesh_GetClosestPathPoint(m_Target.Position), .7f, { 0,0 }, { 0, 0, 1 });				//Closest Navigation mesh node

	return steering;
}

//WANDER (base > SEEK)
//******
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	const float halfJitter = m_MaxJitterOffset / 2;
	const Elite::Vector2 randomOffset = Elite::Vector2{ randomFloat(-halfJitter, halfJitter), randomFloat(-halfJitter, halfJitter) };
	m_WanderingTarget += randomOffset;
	m_WanderingTarget.Normalize();
	m_WanderingTarget *= m_Radius;

	//Add offset
	Elite::Vector2 offset{ pAgent->GetLinearVelocity() };
	offset.Normalize();
	offset *= m_Offset;

	m_Target = TargetData{ pAgent->GetPosition() + offset + m_WanderingTarget };

	//if (pAgent->CanRenderBehavior())
	//{
	//	Elite::Vector2 pos{ pAgent->Position };
	//	//DEBUGRENDERER2D->DrawSegment(pos, pos + offset, { 0.f, 0.f, 1.f, 0.5f }, 0.4f);
	//	//DEBUGRENDERER2D->DrawCircle(pos + offset, m_Radius, { 0.f, 0.f, 1.f, 0.5f }, { 0.4f });
	//	//DEBUGRENDERER2D->DrawSolidCircle(pos + offset + m_WanderingTarget, 0.5f, { 0,0 }, { 0.f, 1.f, 0.f, 0.5f }, 0.3f);

	//};

	return Seek::CalculateSteering(deltaT, pAgent);
}

//Collision Avoidance
//******
CollisionAvoidance::CollisionAvoidance(float maxSeeAhead)
	:m_MaxSeeAhead(maxSeeAhead)
	, m_TargetRadius(0.f)
{

}

SteeringOutput CollisionAvoidance::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	float distanceToTarget = Elite::Distance(pAgent->GetPosition(), m_Target.Position);
	Elite::Vector2 ahead{};
	SteeringOutput steering{};

	if (m_Target.Position != ZeroVector2)
	{
		//Ahead vectors
		ahead = pAgent->GetPosition() + pAgent->GetLinearVelocity().GetNormalized() * pAgent->GetPosition().Distance(m_Target.Position);
		//steering.LinearVelocity = pAgent->GetDirection() + pAgent->GetLinearVelocity().GetNormalized() * m_MaxSeeAhead / 2;

		steering.LinearVelocity = ahead - m_Target.Position;

		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed() *4;
		steering.IsValid = true;
		m_Target = TargetData{};
	}
	else
	{
		steering.IsValid = false;
	}

	//Debug rendering
	//if (pAgent->CanRenderBehavior())
	//{
	//	//DEBUGRENDERER2D->DrawDirection(pAgent->Position, steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.1f, 1.f, 0.f, 0.5f }, 0.4f);
	//}

	return steering;
}

//The targetPosition here should be set with the object/entity that you want to avoid the most
void CollisionAvoidance::SetAvoidingParameters(float targetRadius, Elite::Vector2 targetPosition, float maxSeeAhead)
{
	m_TargetRadius = targetRadius;
	m_Target = targetPosition;	
	m_MaxSeeAhead = maxSeeAhead;
}


#pragma region Not needed (yet)
//Flee 
//******
Flee::Flee(float evasionRadius /*15.f*/)
	:m_EvasionRadius(evasionRadius)
	, m_TargetSet(false)
{

}

SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	auto distanceToTarget = Elite::Distance(pAgent->GetPosition(), m_Target.Position);
	SteeringOutput steering{};

	if (m_TargetSet && distanceToTarget < m_EvasionRadius)
	{
		steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();
	}
	else
	{
		steering.IsValid = false;
	}

	//Debug rendering
	//if (pAgent->CanRenderBehavior())
	//{
	//	//DEBUGRENDERER2D->DrawDirection(pAgent->Position, steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.1f, 1.f, 0.f, 0.5f }, 0.4f);
	//}

	return steering;
}

void Flee::SetTargetSet(bool targetSet)
{
	m_TargetSet = targetSet;
}

//Evade 
//******
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering{};
	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= 0.000000001f;

	//Debug rendering
	//if (pAgent->CanRenderBehavior())
	//{
	//	//DEBUGRENDERER2D->DrawDirection(pAgent->Position, steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0.1f, 1.f, 0.f, 0.5f }, 0.4f);
	//}

	return steering;
}

#pragma endregion

#pragma region Combined steering
//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, pAgent);

		if (steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}

//****************
//BLENDED STEERING
BlendedSteering::BlendedSteering(vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{

};

SteeringOutput BlendedSteering::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	float totalWeight = 0.f;

	for (WeightedBehavior pair : m_WeightedBehaviors)
	{
		SteeringOutput retSteering = pair.pBehavior->CalculateSteering(deltaT, pAgent);
		if (retSteering.IsValid)
		{
			//SteeringOutput retSteering = pair.pBehavior->CalculateSteering(deltaT, pAgent);
			steering.LinearVelocity += pair.weight * retSteering.LinearVelocity;
			steering.AngularVelocity += pair.weight * retSteering.AngularVelocity;

			totalWeight += pair.weight;
		}
	}

	if (totalWeight > 0.f)
	{
		float scale = 1.f / totalWeight;
		steering *= scale;
	}

	return steering;
}
#pragma endregion

