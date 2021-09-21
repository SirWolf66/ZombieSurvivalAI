/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"
#include "Exam_HelperStructs.h"


class SteeringAgent;
class IExamInterface;

using namespace Elite;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }
	TargetData GetTarget() { return m_Target; }

	//Set Interface
	static void Setinterface(IExamInterface* pinterface) { m_pInterface = pinterface; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
	
	static IExamInterface* m_pInterface;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//////////////////////////
//WANDER
//******
class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
	void SetWanderOffset(float offset) { m_Offset = offset; };

protected:
	float m_Offset{6.f};
	const float m_Radius{4.f};
	const float m_MaxJitterOffset{ 1.f };
	Elite::Vector2 m_WanderingTarget{};
};

//////////////////////////
//Flee
//******
class Flee : public Seek
{
public:
	Flee(float evasionRadius = 16.f);
	virtual ~Flee() = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetTargetSet(bool targetSet);

private:
	bool m_TargetSet;
	float m_EvasionRadius;
};

//////////////////////////
//Evade
//******
class Evade : public Seek
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Wander Behavior
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

//https://gamedevelopment.tutsplus.com/tutorials/understanding-steering-behaviors-collision-avoidance--gamedev-7777
//////////////////////////
//Collision Avoidance
//******
class CollisionAvoidance : public Seek
{
public:
	CollisionAvoidance(float maxSeeAhead = 0.f);
	virtual ~CollisionAvoidance() = default;

	//Collision Avoidance Behavior
	SteeringOutput CalculateSteering(float delta, SteeringAgent* pAgent);
	void SetAvoidingParameters(float targetRadius, Elite::Vector2 targetPositions, float maxSeeAhead);

private:
	float m_MaxSeeAhead;
	float m_TargetRadius;
};

#pragma region Combined steering
//*****************
//PRIORITY STEERING
class PrioritySteering final : public ISteeringBehavior
{
public:
	PrioritySteering(vector<ISteeringBehavior*> priorityBehaviors)
		:m_PriorityBehaviors(priorityBehaviors)
	{}

	void AddBehaviour(ISteeringBehavior* pBehavior) { m_PriorityBehaviors.push_back(pBehavior); }
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	vector<ISteeringBehavior*> m_PriorityBehaviors = {};
};

//****************
//BLENDED STEERING
class BlendedSteering final : public ISteeringBehavior
{
	struct WeightedBehavior
	{
		ISteeringBehavior* pBehavior = nullptr;
		float weight = 0.f;

		WeightedBehavior(ISteeringBehavior* pBehavior, float weight) :
			pBehavior(pBehavior),
			weight(weight)
		{};
	};

public:
	BlendedSteering(vector<WeightedBehavior> weightedBehaviors);

	void AddBehaviour(WeightedBehavior weightedBehavior) { m_WeightedBehaviors.push_back(weightedBehavior); }
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	vector<WeightedBehavior> m_WeightedBehaviors = {};
};

#pragma endregion
#endif


