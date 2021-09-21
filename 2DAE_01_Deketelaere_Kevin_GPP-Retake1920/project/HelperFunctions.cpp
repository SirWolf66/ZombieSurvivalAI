#include "stdafx.h"
#include "HelperFunctions.h"

void UpdateSteeringAgentWithAgentInfo(const AgentInfo& agentInfo, SteeringAgent* SteeringAgent)
{
	SteeringAgent->SetPosition(agentInfo.Position);
	SteeringAgent->SetLinearVelocity(agentInfo.LinearVelocity);

	//SteeringAgent->SetRotation = agentInfo->;
	SteeringAgent->SetAngularVelocity(agentInfo.AngularVelocity);
	//SteeringAgent->SetMass; //There is no mass

	SteeringAgent->SetMaxLinearSpeed(agentInfo.MaxLinearSpeed);
	SteeringAgent->SetMaxAngularSpeed(agentInfo.MaxAngularSpeed);
}

SteeringPlugin_Output SteeringOutput_To_SteeringPlugin_Output(const SteeringOutput& steeringOutput, bool AutoOrientate, bool RunMode)
{
	SteeringPlugin_Output steeringPluginOutput{};
	steeringPluginOutput.LinearVelocity = steeringOutput.LinearVelocity;
	steeringPluginOutput.AngularVelocity = steeringOutput.AngularVelocity;
	steeringPluginOutput.AutoOrientate = AutoOrientate;

	steeringPluginOutput.RunMode = RunMode;

	//IsVald is not being used in the plugin

	return steeringPluginOutput;
}

void GotoPoint(const Elite::Vector2& target, ISteeringBehavior* steeringBehavior, IExamInterface* examInterface)
{
	Elite::Vector2 nextPathPoint{examInterface->NavMesh_GetClosestPathPoint(target)};
	steeringBehavior->SetTarget(nextPathPoint);

	examInterface->Draw_SolidCircle(nextPathPoint, .7f, { 0,0 }, { 0, 0, 1 });				//Closest Navigation mesh node
}

bool AlmostEqualRelative(float A, float B,float maxRelDiff /*= FLT_EPSILON*/)
{
	// Calculate the difference.
	float diff = fabs(A - B);
	A = fabs(A);
	B = fabs(B);
	// Find the largest
	float largest = (B > A) ? B : A;

	if (diff <= largest * maxRelDiff)
		return true;
	return false;
}

void AbsoluteClampVectorwitVector(Elite::Vector2& toBeClamped, const Elite::Vector2& limits)
{
	if (fabs(toBeClamped.x) > limits.x)
	{
		toBeClamped.x = toBeClamped.x / fabs(toBeClamped.x) * limits.x;
	}

	if (fabs(toBeClamped.y) > limits.y)
	{
		toBeClamped.y = toBeClamped.y / fabs(toBeClamped.y) * limits.y;
	}
}

bool IsObjectnearLinePolar(const Elite::Vector2& originPoint, const Elite::Vector2& lineEndPolar, const Elite::Vector2& objectPosition, const float objectRadius)
{	
	float distanceToObject{ originPoint.Distance(objectPosition) };
	Elite::Vector2 lineVecetorObjectAdjustedCartesian{ PolarToCartesian( objectPosition.x, lineEndPolar.y )};

	if (lineVecetorObjectAdjustedCartesian.Distance(objectPosition) < objectRadius)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool IsObjectnearLinePoints(const Elite::Vector2& originPoint, const Elite::Vector2& lineEnd, const Elite::Vector2& objectCenter, const float objectRadius)
{
	Elite::Vector2 leftOfLineEnd{};
	Elite::Vector2 righttOfLineEnd{};

	float lineLength{originPoint.Distance(lineEnd)};

	float distance = (originPoint.Distance(objectCenter) + lineEnd.Distance(objectCenter));

	if (distance <= lineLength + 0.1f)
	{
		return true;
	}

	return false;
}

int IsPointToTheLeftOfLine(const Elite::Vector2& originPoint, const Elite::Vector2& lineEnd, const Elite::Vector2& objectPosition)
{
	Elite::Vector3 CrossProduct = (Cross(Elite::Vector3(lineEnd - originPoint), Elite::Vector3(objectPosition - originPoint)));
	return CrossProduct.z > 0;
}

bool IsPointinHouseWalls(const Elite::Vector2& houseDimensions, const Elite::Vector2& houseCenter, const Elite::Vector2& point, const float wallThickness)
{
	float xLeft{ houseCenter.x - (houseDimensions.x / 2) };
	float xRight{ houseCenter.x + (houseDimensions.x / 2) };
	float yUp{ houseCenter.y + (houseDimensions.y / 2) };
	float yDown{ houseCenter.y - (houseDimensions.y / 2) };

	//First check if the point is simply outside of the house
	if (point.x < xLeft || point.x > xRight )
	{
		return false;
	}
	else if (point.y < yDown || point.y > yUp)
	{
		return false;
	}

	//Then check if the point is behind the walls
	if (point.x < (xLeft + wallThickness) || point.x > (xRight - wallThickness ))
	{
		return true;
	}
	else if (point.y < (yDown + wallThickness) || point.y > (yUp - wallThickness ))
	{
		return true;
	}

	return false;
}

Elite::Vector2 CartesianToPolar(float x, float y)
{
	Elite::Vector2 polar{};

	polar.x = sqrtf(x + y);
	polar.y = ToDegrees(atanf(x + y));

	return polar;
}

Elite::Vector2 PolarToCartesian(float r, float angleRadians)
{
	Elite::Vector2 cartesian{};

	cartesian.x = r * cos(angleRadians);
	cartesian.y = r * sin(angleRadians);

	return cartesian;
}


bool FloatIsNearTo(float f1, float f2, float maximumDiference /*= 1.f*/)
{
	return fabs(f1 - f2) < maximumDiference;
}

