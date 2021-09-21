#pragma once

#include "Agent/SteeringAgent.h"
#include <IExamInterface.h>

//Update the steeringAgent with information about the Agent with the information found in AgentInfo, which is provided by the plugin
void UpdateSteeringAgentWithAgentInfo(const AgentInfo& agentInfo, SteeringAgent* SteeringAgent);

//The SteeringOutput_Plugin also requires the AutoOrientate, located in the SteerinAgent and it also requires the bool RunMode, specific to the plugin
SteeringPlugin_Output SteeringOutput_To_SteeringPlugin_Output(const SteeringOutput& steeringOutput, bool AutoOrientate = true, bool RunMode = false);

//Get the next pathPoint on th way to the final destination
void GotoPoint(const Elite::Vector2& target, ISteeringBehavior* steeringBehavior, IExamInterface* examInterface);

//Comparing floats if they are the same
//https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
bool AlmostEqualRelative(float A, float B, float maxRelDiff = FLT_EPSILON);

//Clamp a vector with a max value for x and a max value for y
void AbsoluteClampVectorwitVector(Elite::Vector2& toBeClamped, const Elite::Vector2& limits);

//Check if an object is near a line, using a polarCoordinate and the position of the object
bool IsObjectnearLinePolar(const Elite::Vector2& originPoint, const Elite::Vector2& lineEndPolar, const Elite::Vector2& objectPosition, const float objectRadius);

//A simpler solution mayhaps, justlook at the difference between the end of s line and the object
//the int returns -1 if the object lies to the left, 1 if the object lies to the right, 0 if the object is near
bool IsObjectnearLinePoints(const Elite::Vector2& originPoint, const Elite::Vector2& lineEnd, const Elite::Vector2& objectPosition, const float objectRadius);

//Check if a point is to the left or right from a line
int IsPointToTheLeftOfLine(const Elite::Vector2& originPoint, const Elite::Vector2& lineEnd, const Elite::Vector2& objectPosition);

//Check if a point is to the left or right from a line
bool IsPointinHouseWalls(const Elite::Vector2& houseDimensions, const Elite::Vector2& houseCenter, const Elite::Vector2& point, const float wallThicknes);

//Conversion Polar coordinates to Cartesian
Elite::Vector2 CartesianToPolar(float x, float y);
//Conversion Cartesian coordinates to Polar
Elite::Vector2 PolarToCartesian(float r, float angleRadians);

//See if a float is near to another float, with a maximumdifference
bool FloatIsNearTo(float f1, float f2, float maximumDiference = 1.f);