#pragma once
#include "Structs.h"

class BaseAgent
{
public:
	BaseAgent(float radius = 1.f);
	virtual ~BaseAgent();

	virtual void Update(float dt);
	virtual void Render(float dt);

	//Functions
	void TrimToWorld(float worldBounds) const { TrimToWorld({ worldBounds,worldBounds }); }
	void TrimToWorld(const Elite::Vector2& bounds) const;
	void TrimToWorld(const Elite::Vector2& bottomLeft, const Elite::Vector2& topRight) const;

	//Get - Set
	Elite::Vector2 GetPosition() const { return m_pRigidBody->position; }
	void SetPosition(const Elite::Vector2& pos) const { m_pRigidBody->position = pos; }

	float GetRotation() const { return m_pRigidBody->rotation; }
	void SetRotation(float rot) const { m_pRigidBody->rotation = rot; }

	Elite::Vector2 GetLinearVelocity() const { return m_pRigidBody->linearVelocity; }
	void SetLinearVelocity(const Elite::Vector2& linVel) const { m_pRigidBody->linearVelocity =linVel; }

	float GetAngularVelocity() const { return m_pRigidBody->angularVelocity; }
	void SetAngularVelocity(float angVel) const { m_pRigidBody->angularVelocity = angVel; }
	
	float GetMass() const { return m_pRigidBody->mass; }
	void SetMass(float mass) const { m_pRigidBody->mass = mass; }

	/*const Color& GetBodyColor() const { return m_BodyColor; }
	void SetBodyColor(const Color& col) { m_BodyColor = col; }*/

	float GetRadius() const { return m_Radius; }

protected:
	RigidBody* m_pRigidBody = nullptr;
	float m_Radius = 1.f;
	//Color m_BodyColor = { 1,1,0,1 };

private:
	//float m_Radius = 1.f;
	//Elite::Color m_BodyColor = { 1,1,0,1 };

	//C++ make the class non-copyable
	BaseAgent(const BaseAgent&) {};
	BaseAgent& operator=(const BaseAgent&) {};

};