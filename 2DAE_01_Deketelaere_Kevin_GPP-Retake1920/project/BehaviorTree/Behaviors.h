
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "Steering/SteeringBehaviors.h"
#include "PluginStructsExpanded.h"
#include <chrono>

#pragma region Forward declerations
BehaviorState Seeking(Elite::Blackboard* pBlackBoard);
float itemInventoryPercentage(Elite::Blackboard* pBlackBoard, const eItemType& itemType, IExamInterface* pInterface, const std::vector<ItemInfo>& agentInventory);
bool NeedItemCheck(Elite::Blackboard* pBlackBoard, const eItemType& itemType, float itemIdealPercentage, const std::vector<ItemInfo>& agentInventory);
float GetIdealItemPercentage(Elite::Blackboard* pBlackBoard, const eItemType& item);
Elite::Vector2 MapBorderEncountered(Elite::Blackboard* pBlackBoard, const Elite::Vector2& point);
BehaviorState FindNextMapCorner(Elite::Blackboard* pBlackBoard, const Elite::Vector2& currenPoint, const Elite::Vector2& previousPoint);
EnemyInfo GetClosestEnemy(const std::vector<EntityInfo>& enemiesInPOV, IExamInterface* pInterface, AgentInfo agentInfo);
int HasItem(IExamInterface* pInterface, eItemType itemType, const std::vector<ItemInfo>& agentInventory);
bool AddItemToInventory(IExamInterface* pInterface, std::vector<ItemInfo>& agentInventory, ItemInfo item);
bool UseItem(int itemSlot, IExamInterface* pInterface, std::vector<ItemInfo>& agentInventory);
bool RemoveItemFromInventory(int itemSlot, IExamInterface* pInterface, std::vector<ItemInfo>& agentInventory);

void ResetHouseVariables(Elite::Blackboard* pBlackBoard);
void ResetItemregistrationVariables(Elite::Blackboard* pBlackBoard);
void ResetItemPickupVariables(Elite::Blackboard* pBlackBoard);
void ResetMapExplorationVariables(Elite::Blackboard* pBlackBoard);
void ResetPurgeZoneAvoidanceVariables(Elite::Blackboard* pBlackBoard);
#pragma endregion

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

//Conditionals
#pragma region Conditionals

#pragma region House exploration
//Is there a house in the agent POV?
inline bool HouseInPOV(Elite::Blackboard* pBlackBoard)
{
	std::vector<HouseInfo> housesInPOV{};
	HouseInfo currenthouse{};

	bool dataAvailable = pBlackBoard->GetData("HousesInPOV", housesInPOV)
						 && pBlackBoard->GetData("CurrentHouse", currenthouse);
	if (!dataAvailable)
	{
		return false;
	}

	if (housesInPOV.empty() && (currenthouse.Center == ZeroVector2) )
	{
		return false;
	}
	else
	{
		return true;
	}
}

//Is the house searched?
inline bool HouseSearched(Elite::Blackboard* pBlackBoard)
{
	std::vector<std::pair<Elite::Vector2, std::chrono::steady_clock::time_point>> searchedHouseVector{};

	std::vector<HouseInfo> houseInfoVector{};

	bool dataAvailable = pBlackBoard->GetData("HousesInPOV", houseInfoVector)
						 && pBlackBoard->GetData("SearchedHousesVector", searchedHouseVector);

	if (!dataAvailable)
	{
		return false;
	}

	if (int(houseInfoVector.size() == 0))
	{
		return false;
	}

	for (int i{}; i < int( searchedHouseVector.size()) ; i++)
	{
		if (searchedHouseVector[i].first == houseInfoVector[0].Center && searchedHouseVector[i].second != std::chrono::steady_clock::time_point{})
		{
			return true;
		}
	}
	return false;
}

//Is the agent in a house?
inline bool AgentInside(Elite::Blackboard* pBlackBoard)
{
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo);
	if (!dataAvailable)
	{
		return false;
	}

	if (agentInfo.IsInHouse)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Is the search path in a house calculated?
inline bool SearchPathCalculated(Elite::Blackboard* pBlackBoard)
{
	bool searchPathCalculated{ false };

	bool dataAvailable = pBlackBoard->GetData("SearchPathCalculated", searchPathCalculated);
	if (!dataAvailable)
	{
		return false;
	}

	return searchPathCalculated;
}

//Is the agent in a house?
inline bool PathPointFound(Elite::Blackboard* pBlackBoard)
{
	Elite::Vector2 pathFound{};

	bool dataAvailable = pBlackBoard->GetData("PathPoint", pathFound);
	if (!dataAvailable)
	{
		return false;
	}

	if (pathFound == ZeroVector2)
	{
		return false;
	}
	else
	{
		return true;
	}
}
#pragma endregion

#pragma region Item registration
//Are there any items we know the location of, but not know what kind of item it is?
inline bool ItemsNeedRegistration(Elite::Blackboard* pBlackBoard)
{
	std::vector<EntityInfo> foundItemsInHouse{};

	bool dataAvailable = pBlackBoard->GetData("FoundItemsInHouse", foundItemsInHouse);
	if (!dataAvailable)
	{
		return false;
	}

	if (foundItemsInHouse.empty())
	{
		return false;
	}
	else
	{
		return true;
	}
}

//Is the ItemTarget set?
inline bool ItemTargetSet(Elite::Blackboard* pBlackBoard)
{
	Elite::Vector2 itemTargetPosition{};

	bool dataAvailable = pBlackBoard->GetData("ItemTargetPosition", itemTargetPosition);
	if (!dataAvailable)
	{
		return false;
	}

	if (itemTargetPosition == ZeroVector2)
	{
		return false;
	}
	else
	{
		return true;
	}
}
#pragma endregion

#pragma region Item pickup
//Need an item?
inline bool NeedItem(Elite::Blackboard* pBlackBoard)
{
	float GunIdealPercentage{};
	float MedkitIdealPercentage{};
	float FoodIdealPercentage{};
	std::vector<ItemInfo> agentInventory{};
	std::vector <ItemInfo> registeredItems{};
	std::vector<eItemType> neededItems{};

	bool dataAvailable = pBlackBoard->GetData("IdealGunsPercentage", GunIdealPercentage)
						 && pBlackBoard->GetData("IdealMedkitPercentage", MedkitIdealPercentage)
						 && pBlackBoard->GetData("IdealFoodAPercentage", FoodIdealPercentage)
						 && pBlackBoard->GetData("RegisteredItems", registeredItems)
						 && pBlackBoard->GetData("AgentInventory", agentInventory);

	if (!dataAvailable)
	{
		return false;
	}

	if (registeredItems.empty())
	{
		return false;
	}

	if (NeedItemCheck(pBlackBoard, eItemType::PISTOL, GunIdealPercentage, agentInventory))
	{
		neededItems.push_back(eItemType::PISTOL);
	}

	if (NeedItemCheck(pBlackBoard, eItemType::MEDKIT, MedkitIdealPercentage, agentInventory))
	{
		neededItems.push_back(eItemType::MEDKIT);
	}

	if (NeedItemCheck(pBlackBoard, eItemType::FOOD, FoodIdealPercentage, agentInventory))
	{
		neededItems.push_back(eItemType::FOOD);
	}

	pBlackBoard->ChangeData("NeededItems", neededItems);

	if (!neededItems.empty())
	{
		return true;
	}
	
	return false;
}

//Does the agent know a location of the needed item?
inline bool DicoveredNeededItem(Elite::Blackboard* pBlackBoard)
{
	std::vector <ItemInfo> registeredItems{};
	std::vector<eItemType> neededItems{};
	std::vector<ItemInfo>::iterator registeredItemsIterator{};
	Elite::Vector2 itemTargetPosition{};

	bool dataAvailable = pBlackBoard->GetData("RegisteredItems", registeredItems)
						 && pBlackBoard->GetData("NeededItems", neededItems)
						 && pBlackBoard->GetData("ItemTargetPosition", itemTargetPosition);

	if (!dataAvailable)
	{
		return false;
	}

	if (itemTargetPosition != ZeroVector2)
	{
		return true;
	}

	for (int i{}; i < int(neededItems.size()); i++)
	{
		registeredItemsIterator = std::find_if(registeredItems.begin(), registeredItems.end(), ItemTypeCompareFunctor(neededItems[i]));
		if (registeredItemsIterator != registeredItems.end())
		{
			pBlackBoard->ChangeData("ItemTargetPosition", registeredItemsIterator->Location);
			return true;
		}
	}
	return false;
}
#pragma endregion

#pragma region Find map dimensions
//Is a border of the map found? In this case the left most border
inline bool MapBorderFound(Elite::Blackboard * pBlackBoard)
{
	Elite::Vector2 mapBorder{};
	std::vector<Elite::Vector2> mapBorderCorners{};
	std::vector<Elite::Vector2>::iterator mapBorderCornersIterator{};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("MapBorderCorners", mapBorderCorners)
						 && pBlackBoard->GetData("AgentInfo", agentInfo);				
	if (!dataAvailable)
	{
		return false;
	}

	mapBorder = MapBorderEncountered(pBlackBoard, agentInfo.Position);
	mapBorderCornersIterator = std::find(mapBorderCorners.begin(), mapBorderCorners.end(), mapBorder);
	if (mapBorder != ZeroVector2)
	{
		if ((int(mapBorderCorners.size()) < 2) || (mapBorderCornersIterator != mapBorderCorners.end()))
		{
			mapBorderCorners.push_back(mapBorder);
			pBlackBoard->ChangeData("MapBorderCorners", mapBorderCorners);
			std::cout << "Border found!";
			return true;
		}
	}
	return false;
}

//Are the dimensions of the map found?
inline bool MapDimensionsFound(Elite::Blackboard* pBlackBoard)
{
	bool mapDimensionsFound{};

	bool dataAvailable = pBlackBoard->GetData("MapDimensionsFound", mapDimensionsFound);
	if (!dataAvailable)
	{
		return false; 
	}

	return mapDimensionsFound;
}

//---
//Have we reached the explorationPoint?
inline bool ExplorationPointSet(Elite::Blackboard* pBlackBoard)
{
	Elite::Vector2 mapSearchtarget{};

	bool dataAvailable = pBlackBoard->GetData("MapSearchtarget", mapSearchtarget);
					
	if (!dataAvailable)
	{
		return false;
	}

	if (mapSearchtarget != ZeroVector2)
	{
		return true;
	}

	return false;
}

inline bool PastMaximumSearchNodes(Elite::Blackboard* pBlackBoard)
{
	float searchTimeMax{ 10.f };
	int mapSearchTargetsVisited{8};
	std::chrono::steady_clock::time_point now{ std::chrono::high_resolution_clock::now() };
	std::chrono::steady_clock::time_point searchStartTime{};
	std::vector<Elite::Vector2> mapSearchTargets{};

	bool dataAvailable = pBlackBoard->GetData("SearchStartTime", searchStartTime)
						 && pBlackBoard->GetData("MapSearchTargets", mapSearchTargets);

	if (!dataAvailable)
	{
		return false;
	}

	if (int(mapSearchTargets.size()) >= mapSearchTargetsVisited)
	{
		return true;
		std::cout << "Time to move on";
	}

	return false;
}

//---
#pragma endregion

#pragma region Item usage
//Does the agent need to use a medkit, cause low health?
inline bool NeedToUseMedkit(Elite::Blackboard * pBlackBoard)
{
	float lowHealth{ 8.f };
	IExamInterface* pInterface{};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInfo", agentInfo);
	if (!dataAvailable)
	{
		return false;
	}

	if (agentInfo.Health < lowHealth)
	{
		return true;
	}

	return false;
}

//Does the agent need to eat food, cause low energy?
inline bool NeedToUseFood(Elite::Blackboard* pBlackBoard)
{
	float lowEnergy{2.f};
	IExamInterface* pInterface{};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInfo", agentInfo);
	if (!dataAvailable)
	{
		return false;
	}

	if (agentInfo.Energy < lowEnergy)
	{
		return true;
	}

	return false;
}

//Does the agent have a medkit?
inline bool HasMedkit(Elite::Blackboard* pBlackBoard)
{
	IExamInterface* pInterface{};
	std::vector<ItemInfo> agentInventory{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInventory", agentInventory);
	if (!dataAvailable)
	{
		return false;
	}

	if (HasItem(pInterface, eItemType::MEDKIT, agentInventory) != -1)
	{
		return true;
	}

	return false;
}

//Does the agent have food?
inline bool HasFood(Elite::Blackboard * pBlackBoard)
{
	IExamInterface* pInterface{};
	std::vector<ItemInfo> agentInventory{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
		&& pBlackBoard->GetData("AgentInventory", agentInventory);
	if (!dataAvailable)
	{
		return false;
	}

	if (HasItem(pInterface, eItemType::FOOD, agentInventory) != -1)
	{
		return true;
	}

	return false;
}
#pragma endregion

#pragma region Enemy handeling
//Is there an enemy in the agent's FOV?
inline bool EnemiesInFOV(Elite::Blackboard* pBlackBoard)
{
	bool isBitten{};
	float timeSinceEnemySighted{};
	float rememberEnemyTime{};
	float deltaTime{};
	std::vector<EntityInfo> enemiesInPOV{};

	bool dataAvailable = pBlackBoard->GetData("EnemiesInPOV", enemiesInPOV)
						 && pBlackBoard->GetData("TimeSinceEnemySighted", timeSinceEnemySighted)
						 && pBlackBoard->GetData("RemeberEnemyTime", rememberEnemyTime)
						 && pBlackBoard->GetData("deltaTime", deltaTime)
						 && pBlackBoard->GetData("IsBitten", isBitten);
	if (!dataAvailable)
	{
		return false;
	}

	if (enemiesInPOV.empty())
	{
		if (!isBitten)
		{
			pBlackBoard->ChangeData("AutOrientate", true);
		}

		if (timeSinceEnemySighted < rememberEnemyTime)
		{
			pBlackBoard->ChangeData("TimeSinceEnemySighted", timeSinceEnemySighted + deltaTime);
			return false;
		}
		else
		{
			std::chrono::steady_clock::time_point TimeEnemySighted{ std::chrono::high_resolution_clock::now() };
			pBlackBoard->ChangeData("ClosestEnemy", EnemyInfo{});
			return false;
		}
		pBlackBoard->ChangeData("AutOrientate", true);
	}

	pBlackBoard->ChangeData("IsBitten", false);

	return true;
}

//Does the agents have any guns with ammo?
inline bool HasAmmo(Elite::Blackboard* pBlackBoard)
{
	IExamInterface* pInterface{};
	ItemInfo itemInInventory{};
	std::vector<ItemInfo> agentInventory{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
		&& pBlackBoard->GetData("AgentInventory", agentInventory);
	if (!dataAvailable)
	{
		return false;
	}
	
	if (HasItem(pInterface, eItemType::PISTOL, agentInventory) != -1)
	{
		return true;
	}

	return false;
}

//Has the agent been bitten?
inline bool IsBitten(Elite::Blackboard* pBlackBoard)
{
	bool IsBitten{false};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("IsBitten", IsBitten);
	if (!dataAvailable)
	{
		return false;
	}

	if (agentInfo.Bitten || IsBitten)
	{
		pBlackBoard->ChangeData("IsBitten", true);
		return true;
	}

	return false;
}
#pragma endregion

#pragma region Purge zone avoidance
//Is there a Purgezone in the agent POV?
inline bool PurgeZoneInPOV(Elite::Blackboard* pBlackBoard)
{
	PurgeZoneInfo purgeZoneInPOV{};
	PurgeZoneInfo purgeZone{};

	bool dataAvailable = pBlackBoard->GetData("PurgeZoneInPOV", purgeZoneInPOV);
	if (!dataAvailable)
	{
		return false;
	}

	if (purgeZoneInPOV.ZoneHash != 0)
	{
		return true;
	}

	ResetPurgeZoneAvoidanceVariables(pBlackBoard);
	return false;
}

//Is the evasion path around the purgezone calculated
inline bool EvasionPathCalculated(Elite::Blackboard* pBlackBoard)
{
	std::vector<Elite::Vector2> evadePath{};

	bool dataAvailable = pBlackBoard->GetData("EvadePath", evadePath);
	if (!dataAvailable)
	{
		return false;
	}

	if (!evadePath.empty())
	{
		return true;
	}
	return false;
}
#pragma endregion

#pragma endregion


//Actions
#pragma region Actions

#pragma region House exploration
//Go Inside House
BehaviorState GoInside(Elite::Blackboard* pBlackBoard)
{
	Elite::Vector2 target;
	HouseInfo housinfo;
	std::vector<HouseInfo> housesInPOV{};
	std::vector<std::pair<Elite::Vector2, std::chrono::steady_clock::time_point>> searchedHouseVector{};

	bool dataAvailable = pBlackBoard->GetData("HousesInPOV", housesInPOV)
						 && pBlackBoard->GetData("CurrentHouse", housinfo)
						 && pBlackBoard->GetData("SearchedHousesVector", searchedHouseVector);;
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (housinfo.Size == ZeroVector2)
{
		housinfo = housesInPOV[0];
		pBlackBoard->ChangeData("Target", housinfo.Center);
		pBlackBoard->ChangeData("CurrentHouse", housinfo);

	}
	else
	{
		pBlackBoard->ChangeData("Target", housinfo.Center);
		if (housinfo.Center == ZeroVector2)
		{
			std::cout << "Bollocks!";
		}
	}

	return Seeking(pBlackBoard);						//After the target has been set to the middle of the house, just use the regular seek
}

//Calculate the path points to be used when searching the house
BehaviorState CalculateSearchPathPoints(Elite::Blackboard* pBlackBoard)
{
	bool smallHouse{ false };
	float WallThickness{ 0.f };

	std::vector<Elite::Vector2> pathPoints{};
	AgentInfo agentInfo{};
	HouseInfo currentHouse{};
	std::vector<HouseInfo> housesInPOV{};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("CurrentHouse", currentHouse)
						 && pBlackBoard->GetData("HouseWallThickness", WallThickness)
						 && pBlackBoard->GetData("HousesInPOV", housesInPOV);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	float AgentLineOfSight{ agentInfo.FOV_Range - 2.f };
	//See if the house to be searched is a small house that can be searched with only 2 pathpoints horizontally or vertically
	float PathPointOffset{ WallThickness + AgentLineOfSight };

	if (currentHouse.Center == ZeroVector2)
	{
		currentHouse = housesInPOV[0];
		pBlackBoard->ChangeData("CurrentHouse", housesInPOV[0]);
	}

	if (agentInfo.FOV_Range > currentHouse.Size.x - (WallThickness * 2))		//Vertical small house
	{
		Elite::Vector2 down{ currentHouse.Center.x , (currentHouse.Center.y - (currentHouse.Size.y / 2)) + PathPointOffset };
		Elite::Vector2 up{ currentHouse.Center.x, (currentHouse.Center.y + (currentHouse.Size.y / 2)) - PathPointOffset };

		pathPoints.push_back(down);
		pathPoints.push_back(up);
	}

	else if (agentInfo.FOV_Range > currentHouse.Size.y - (WallThickness * 2))	//Horizontal small house
	{
		Elite::Vector2 left{ (currentHouse.Center.x - (currentHouse.Size.x / 2)) + PathPointOffset, currentHouse.Center.y };
		Elite::Vector2 right{ (currentHouse.Center.x + (currentHouse.Size.x / 2)) - PathPointOffset , currentHouse.Center.y };

		pathPoints.push_back(left);
		pathPoints.push_back(right);
	}

	else
	{
		Elite::Vector2 leftDown{ (currentHouse.Center.x - (currentHouse.Size.x / 2)) + PathPointOffset, (currentHouse.Center.y - (currentHouse.Size.y / 2)) + PathPointOffset };
		Elite::Vector2 rightDown{ (currentHouse.Center.x + (currentHouse.Size.x / 2)) - PathPointOffset , (currentHouse.Center.y - (currentHouse.Size.y / 2)) + PathPointOffset };
		Elite::Vector2 rightUp{ (currentHouse.Center.x + (currentHouse.Size.x / 2)) - PathPointOffset, (currentHouse.Center.y + (currentHouse.Size.y / 2)) - PathPointOffset };
		Elite::Vector2 leftUp{ (currentHouse.Center.x - (currentHouse.Size.x / 2)) + PathPointOffset, (currentHouse.Center.y + (currentHouse.Size.y / 2)) - PathPointOffset };

		pathPoints.push_back(leftDown);
		pathPoints.push_back(rightDown);
		pathPoints.push_back(rightUp);
		pathPoints.push_back(leftUp);
	}

	if (currentHouse.Center == ZeroVector2)
	{
		std::cout << "Bollocks!";
	}

	pBlackBoard->ChangeData("HouseSearchPathPoints", pathPoints);
	pBlackBoard->ChangeData("SearchPathCalculated", true);

	return BehaviorState::Success;
}

//Find the closest path point to the agent
BehaviorState FindClosestPathPoint(Elite::Blackboard* pBlackBoard)
{
	AgentInfo agentInfo{};
	HouseInfo currentHouse{};
	std::vector<Elite::Vector2> houseSearchPathPoints{};
	int pathpointsAmount{ };
	float distancetoPathPoint{ FLT_MAX };
	int pathPointindex{ 0 };
	bool houseFound{};
	bool houseAlreadyVisited{};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("CurrentHouse", currentHouse);

	pBlackBoard->GetData("HouseSearchPathPoints", houseSearchPathPoints);

	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (currentHouse.Center == ZeroVector2)
	{
		ResetHouseVariables(pBlackBoard);
		std::cout << "Bollocks";
	}

	//Seek de closest PathPoint
	pathpointsAmount = int(houseSearchPathPoints.size());
	for (int i{ }; i < pathpointsAmount; i++)
	{
		if (agentInfo.Position.Distance(houseSearchPathPoints[i]) < distancetoPathPoint)
		{
			distancetoPathPoint = agentInfo.Position.Distance(houseSearchPathPoints[i]);
			pathPointindex = i;
		}
	}

	if (houseSearchPathPoints.empty())
	{
		std::pair<Elite::Vector2, std::chrono::steady_clock::time_point> houseAndTimestamp{ currentHouse.Center, std::chrono::high_resolution_clock::now() };
		std::vector<std::pair<Elite::Vector2, std::chrono::steady_clock::time_point>> searchedHouseVector{};
		pBlackBoard->GetData("SearchedHousesVector", searchedHouseVector);

		//check if the house is already been discovered, in this case, just update the time the house was last visited
		for (int i{}; i < int(searchedHouseVector.size()); i++)
		{
			if (searchedHouseVector[i].first == houseAndTimestamp.first)
			{
				searchedHouseVector[i].second = houseAndTimestamp.second;
				houseAlreadyVisited = true;
			}
		}
		if (!(houseAlreadyVisited) && (houseAndTimestamp.first != ZeroVector2))
		{
			searchedHouseVector.push_back(houseAndTimestamp);
		}

		pBlackBoard->ChangeData("SearchedHousesVector", searchedHouseVector);

		ResetHouseVariables(pBlackBoard);
	}
	else
	{
		pBlackBoard->ChangeData("PathPoint", houseSearchPathPoints[pathPointindex]);

		//Erase the PathPoint out of the HouseSearchPathPoints vector
		houseSearchPathPoints.erase(houseSearchPathPoints.begin() + pathPointindex);
		pBlackBoard->ChangeData("HouseSearchPathPoints", houseSearchPathPoints);
	}

	return BehaviorState::Success;
}

//Go to the path point
BehaviorState GoToPathPoint(Elite::Blackboard* pBlackBoard)
{
	float minimumDistance{};
	Elite::Vector2 pathPoint{};
	AgentInfo agentInfo{};

	bool dataAvailable{ pBlackBoard->GetData("PathPoint", pathPoint)
						&& pBlackBoard->GetData("AgentInfo", agentInfo)
						&& pBlackBoard->GetData("MinimumDistance", minimumDistance) };
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (agentInfo.Position.Distance(pathPoint) < minimumDistance)
	{
		pBlackBoard->ChangeData("PathPoint", ZeroVector2);
	}

	pBlackBoard->ChangeData("Target", pathPoint);
	return Seeking(pBlackBoard);
}

//Temporary register item location
BehaviorState RegisterItemLocation(Elite::Blackboard* pBlackBoard)
{
	std::vector<EntityInfo> foundItemsInHouse{};
	std::vector<EntityInfo> itemsInFOV{};
	std::vector<ItemInfo> registeredItems{};
	EntityInfo itemInFOV{};

	bool dataAvailable{ pBlackBoard->GetData("FoundItemsInHouse", foundItemsInHouse)
						&& pBlackBoard->GetData("ItemsInPOV", itemsInFOV) 
						&& pBlackBoard->GetData("RegisteredItems", registeredItems)};
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	for (int i{}; i < int(itemsInFOV.size()); i++)
	{
		itemInFOV = itemsInFOV[i];
		if (std::find_if(registeredItems.begin(), registeredItems.end(), EntityAndItemLocationeCompareFunctor(itemsInFOV[i])) != registeredItems.end())
		{
			std::cout << "Already registered that one, mate";
			continue;
		}
		if (foundItemsInHouse.empty()|| (std::find_if(foundItemsInHouse.begin(), foundItemsInHouse.end(), EntityInfoCompareFunctor(itemsInFOV[i])) == foundItemsInHouse.end()))
		{
			foundItemsInHouse.push_back(itemsInFOV[i]);
			pBlackBoard->ChangeData("FoundItemsInHouse", foundItemsInHouse);
			std::cout << "Item added to Item found in house \n";
		}
	}

	return BehaviorState::Success;
}
#pragma endregion

#pragma region Item registration
//Find the the closest item that needs to be registered
BehaviorState FindClosestItemInHouse(Elite::Blackboard* pBlackBoard)
{
	AgentInfo agentInfo{};
	std::vector<EntityInfo> founditemsinHouse{};
	Elite::Vector2 itemTargetPosition{};
	float distanceToItem{FLT_MAX};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("FoundItemsInHouse", founditemsinHouse);

	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	for (int i{ }; i < int(founditemsinHouse.size()); i++)
	{
		if (agentInfo.Position.Distance(founditemsinHouse[i].Location) < distanceToItem)
		{
			distanceToItem = agentInfo.Position.Distance(founditemsinHouse[i].Location);
			itemTargetPosition = founditemsinHouse[i].Location;
		}
	}

	pBlackBoard->ChangeData("ItemTargetPosition", itemTargetPosition);

	return BehaviorState::Success;						
}

//Go to the item that has been targeted
BehaviorState GoToItemAndRegister(Elite::Blackboard* pBlackBoard)
{
	float minimumDistance{};
	float ItemIdealPercentage{};
	Elite::Vector2 itemTargetPosition{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	ItemInfo item{};
	std::vector<ItemInfo> agentInventory{};
	std::vector <ItemInfo> registeredItems{};
	std::vector <EntityInfo> foundItemsInHouse{};
	std::vector	<EntityInfo> itemsInFOV{};
	std::vector<EntityInfo>::iterator EntityInfoVectorIterator{};
	bool itemAdded{};

	bool dataAvailable = pBlackBoard->GetData("ItemTargetPosition", itemTargetPosition)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("MinimumDistance", minimumDistance)
						 && pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("RegisteredItems", registeredItems)
						 && pBlackBoard->GetData("FoundItemsInHouse", foundItemsInHouse)
						 && pBlackBoard->GetData("ItemsInPOV", itemsInFOV)
						 && pBlackBoard->GetData("AgentInventory", agentInventory);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	float distance = agentInfo.Position.Distance(itemTargetPosition);
	if (agentInfo.Position.Distance(itemTargetPosition) < minimumDistance)
	{
		if(pInterface->Item_Grab({}, item))
		{
			if (itemTargetPosition != item.Location)
			{
				std::cout << "Different item then the one we want to get, NANI? \n";
			}

			//Check if the item to be registered is needed
			if (item.Type == eItemType::GARBAGE || NeedItemCheck(pBlackBoard, item.Type, GetIdealItemPercentage(pBlackBoard, item.Type), agentInventory))
			{
				itemAdded = AddItemToInventory(pInterface, agentInventory, item);
				if (itemAdded && item.Type == eItemType::GARBAGE)
				{
					RemoveItemFromInventory(HasItem(pInterface, eItemType::GARBAGE, agentInventory), pInterface, agentInventory);
				}
				ResetItemPickupVariables(pBlackBoard);						//Reset the variables used in judging if an item is needed, cause a new item is picked up
				pBlackBoard->ChangeData("AgentInventory", agentInventory);
				std::cout << "Item added during register \n";

			}
			else
			{
				//Add item to registeredItems
				registeredItems.push_back(item);
				pBlackBoard->ChangeData("RegisteredItems", registeredItems);

				std::cout << "Item registered! \n";
			}
			//Delete Item out of the foundItemsInHouse
			EntityInfoVectorIterator = std::find_if(foundItemsInHouse.begin(), foundItemsInHouse.end(), ItemAndEntityLocationeCompareFunctor(item));
			if (EntityInfoVectorIterator != foundItemsInHouse.end())
			{
				foundItemsInHouse.erase(EntityInfoVectorIterator);
				pBlackBoard->ChangeData("FoundItemsInHouse", foundItemsInHouse);
			}

			//Delete item out of Items in fov
			EntityInfoVectorIterator = std::find_if(itemsInFOV.begin(), itemsInFOV.end(), ItemAndEntityLocationeCompareFunctor(item));
			if (EntityInfoVectorIterator != itemsInFOV.end())
			{
				itemsInFOV.erase(EntityInfoVectorIterator);
				pBlackBoard->ChangeData("ItemsInPOV", itemsInFOV);
			}

			ResetItemregistrationVariables(pBlackBoard);
		}

		else
		{
			std::cout << "Item failed to registered! \n";
		}
	}

	else
	{
		pBlackBoard->ChangeData("Target", itemTargetPosition);
	}

	return Seeking(pBlackBoard);						
}
#pragma endregion

#pragma region Item pickup
//Go to the desired item location
BehaviorState GoToItemAndPickItUp(Elite::Blackboard* pBlackBoard)
{
	float minimumDistance{};
	Elite::Vector2 itemTargetPosition{};
	AgentInfo agentInfo{};
	ItemInfo item{};
	IExamInterface* pInterface{};
	std::vector<ItemInfo> agentInventory{};
	std::vector < ItemInfo > registeredItems{};
	std::vector<ItemInfo>::iterator registeredItemsIterator{};
	bool itemAdded{};

	bool dataAvailable = pBlackBoard->GetData("ItemTargetPosition", itemTargetPosition)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("MinimumDistance", minimumDistance)
						 && pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("RegisteredItems", registeredItems) 
						 && pBlackBoard->GetData("AgentInventory", agentInventory);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}


	if (agentInfo.Position.Distance(itemTargetPosition) < minimumDistance)
	{
		pInterface->Item_Grab({}, item);
		if (item.ItemHash == 0)
		{
			ResetItemPickupVariables(pBlackBoard);
			return BehaviorState::Failure;
		}

		//Adding item
		itemAdded = AddItemToInventory(pInterface, agentInventory, item);

		if (itemAdded || (item.ItemHash == 0))
		{
			//Deleting item out of registered items
			registeredItemsIterator = std::find_if(registeredItems.begin(), registeredItems.end(), ItemInfoCompareFunctor(item));	//Get the iterator of the item that is picked up
			registeredItems.erase(registeredItemsIterator);																			//Delete the item out of the registered items vector
			pBlackBoard->ChangeData("RegisteredItems", registeredItems);

			ResetItemPickupVariables(pBlackBoard);

			std::cout << "Item addded during specific item pickup \n";
		}
		pBlackBoard->ChangeData("AgentInventory", agentInventory);
	}

	else
	{
		pBlackBoard->ChangeData("Target", itemTargetPosition);
	}

	return Seeking(pBlackBoard);						
}
#pragma endregion

#pragma region Find map dimensions
//Find a border of the map, in this case, we look for the left border
BehaviorState FindMapBorder(Elite::Blackboard* pBlackBoard)
{
	AgentInfo agentInfo{};
	Elite::Vector2 mapSearchTarget{};
	Elite::Vector2 mapBorder{};
	IExamInterface* pInterface{};
	std::vector<Elite::Vector2> mapBorderCorners{};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("MapBorderCorners", mapBorderCorners)
						 && pBlackBoard->GetData("MapSearchtarget", mapSearchTarget);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (mapSearchTarget == ZeroVector2)
	{
		mapSearchTarget = Elite::Vector2(1 - (agentInfo.Position.x + FLT_MAX), agentInfo.Position.y);
		pBlackBoard->ChangeData("MapSearchtarget", mapSearchTarget);
	}

	mapBorder = MapBorderEncountered(pBlackBoard, agentInfo.Position);
	if (mapBorder != ZeroVector2)
	{
		mapBorderCorners.push_back(Elite::Vector2(pInterface->World_GetInfo().Center.x - (pInterface->World_GetInfo().Dimensions.x / 2.f), mapSearchTarget.y));
		pBlackBoard->ChangeData("MapBorderCorners", mapBorderCorners);

		mapSearchTarget = Elite::Vector2(agentInfo.Position.x, agentInfo.Position.y + FLT_MAX);
		pBlackBoard->ChangeData("MapSearchtarget", mapSearchTarget);
		pBlackBoard->ChangeData("LastHitBorderIsX", true);
		return BehaviorState::Success;
	}

	pBlackBoard->ChangeData("Target", mapSearchTarget);

	return Seeking(pBlackBoard);						
}

//Find a corner of the map, in this case after we have found the map border, we shall go upwards
BehaviorState FindMapCorners(Elite::Blackboard* pBlackBoard)
{
	int mapCornerAmount{};
	Elite::Vector2 currentCorner{};
	Elite::Vector2 previousCorner{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	std::vector<Elite::Vector2> mapBorderCorners{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("MapBorderCorners", mapBorderCorners)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("MapCornersAmount", mapCornerAmount);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	currentCorner = mapBorderCorners.back(); 
	if (int(mapBorderCorners.size()) > 1)
	{
		previousCorner = *(mapBorderCorners.end() - 1);
	}

	if (int(mapBorderCorners.size()) > mapCornerAmount + 1)
	{
		mapBorderCorners.erase(mapBorderCorners.begin());
		pBlackBoard->ChangeData("MapDimensionsFound", true);

		ResetMapExplorationVariables(pBlackBoard);
		return BehaviorState::Success;
	}

	return FindNextMapCorner(pBlackBoard, currentCorner, previousCorner);
}

//Find the dimesnions of the map
//Use tracing the map first
BehaviorState FindMapDimensions(Elite::Blackboard* pBlackBoard)
{
	Elite::Vector2 target;
	std::vector<HouseInfo> housesInPOV{};

	bool dataAvailable = pBlackBoard->GetData("HousesInPOV", housesInPOV);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	return Seeking(pBlackBoard);						
}

//---
//Use a zig zagging line to search the map
BehaviorState MapSearchZigZag(Elite::Blackboard* pBlackBoard)
{
	int mapCornerAmount{};
	Elite::Vector2 currentCorner{};
	Elite::Vector2 mapBorder{};
	Elite::Vector2 mapSearchTarget{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	std::vector<Elite::Vector2> mapBorderCorners{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("MapBorderCorners", mapBorderCorners)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("MapCornersAmount", mapCornerAmount)
						 && pBlackBoard->GetData("MapSearchtarget", mapSearchTarget);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	mapBorder = MapBorderEncountered(pBlackBoard, agentInfo.Position);

	if (mapBorder != ZeroVector2)
	{
		if (AlmostEqualRelative(mapBorder.x, 0.f))
		{
			mapBorder.x = mapBorderCorners.back().x;
		}
		else if (AlmostEqualRelative(mapBorder.y, 0.f))
		{
			mapBorder.y = mapBorderCorners.back().y;
		}
		else
		{
			std::cout << "The mapborderFind has fucked up";
		}

		if ((mapBorder != mapBorderCorners.back()) && (mapBorderCorners.size() < 2.f))
		{
			mapSearchTarget = ZeroVector2;
			mapBorderCorners.push_back(mapBorder);
			pBlackBoard->ChangeData("MapBorderCorners", mapBorderCorners);
			pBlackBoard->ChangeData("MapSearchtarget", ZeroVector2);
		}
	}

	if (mapSearchTarget == ZeroVector2)
	{
		currentCorner = mapBorderCorners.back();
		mapSearchTarget = Elite::Vector2(-currentCorner.x, currentCorner.y - agentInfo.FOV_Range * 2.f);
		pBlackBoard->ChangeData("MapSearchtarget", mapSearchTarget);
	}

	if (agentInfo.Position.Distance(mapSearchTarget) < 2.f)
	{
		mapBorderCorners.push_back(mapSearchTarget);
		currentCorner = mapBorderCorners.back();
		mapSearchTarget = Elite::Vector2(-currentCorner.x, currentCorner.y - agentInfo.FOV_Range * 2.f);

		pBlackBoard->ChangeData("MapBorderCorners", mapBorderCorners);
		pBlackBoard->ChangeData("MapSearchtarget", mapSearchTarget);
	}

	//After the first map corner is found go in a zig zag pattern, left to right, from the top legft corner in this case, al the way to the bottom right or left corner
	//After this you could zig zag in the vertical direction, to make a sort of grid search
	//After that, move from house to house till dead

	pBlackBoard->ChangeData("Target", mapSearchTarget);

	return Seeking(pBlackBoard);						
}

//---
//Go in expanding squares
BehaviorState GoToExplorationPoint(Elite::Blackboard* pBlackBoard)
{
	float minimumDistance{};
	Elite::Vector2 mapSearchtarget{};
	Elite::Vector2 mapBorder{};
	std::vector<Elite::Vector2> mapSearchTargets{};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("MapSearchtarget", mapSearchtarget)
						 && pBlackBoard->GetData("MapSearchTargets", mapSearchTargets)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("MinimumDistance", minimumDistance);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (agentInfo.Position.Distance(mapSearchtarget) < 4.f)
	{
		mapSearchTargets.push_back(mapSearchtarget);
		pBlackBoard->ChangeData("MapSearchTargets", mapSearchTargets);
		pBlackBoard->ChangeData("MapSearchtarget", ZeroVector2);
		return BehaviorState::Success;
	}

	pBlackBoard->ChangeData("Target", mapSearchtarget);

	return Seeking(pBlackBoard);						
}

//Go in expanding squares
BehaviorState SetNewExplorationPoint(Elite::Blackboard* pBlackBoard)
{
	int cornerNumber{};
	int sign{};
	float x{};
	float y{};
	Elite::Vector2 exploringDistance{};
	Elite::Vector2 mapSearchTarget{};
	Elite::Vector2 currentSearchCenter{};
	std::vector<Elite::Vector2> mapSearchTargets;
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("MapSearchTargets", mapSearchTargets)
						 && pBlackBoard->GetData("MapSearchtarget", mapSearchTarget)
						 && pBlackBoard->GetData("ExploringDistance", exploringDistance)
						 && pBlackBoard->GetData("CurrentSearchCenter", currentSearchCenter)
						 && pBlackBoard->GetData("AgentInfo", agentInfo);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	//Set the increase of the exploringDistance after the information is got from the blackboard
	float exploringDistanceIncrease{ agentInfo.FOV_Range };

	if (mapSearchTargets.empty())
	{
		mapSearchTargets.push_back(agentInfo.Position);
		currentSearchCenter = agentInfo.Position;
		pBlackBoard->ChangeData("CurrentSearchCenter", currentSearchCenter);
	}

	if ((mapSearchTargets.size()) == 1)
	{
		exploringDistance.x = exploringDistanceIncrease;
		exploringDistance.y = exploringDistanceIncrease;

		mapSearchTarget.x = currentSearchCenter.x - exploringDistance.x; 
		mapSearchTarget.y = currentSearchCenter.y;

		//Add the agent start position to the mapsearch target vector
		pBlackBoard->ChangeData("MapSearchTargets", mapSearchTargets);
		pBlackBoard->ChangeData("SearchStartTime", std::chrono::high_resolution_clock::now());
	}
	else if (int(mapSearchTargets.size()) == 2)
	{
		x = mapSearchTargets[1].x;

		mapSearchTarget.x = currentSearchCenter.x - exploringDistance.x;
		mapSearchTarget.y = currentSearchCenter.y + exploringDistance.y;
	}
	else
	{
		//Settting the x, in comparison to the searchCenter
		cornerNumber = (int(mapSearchTargets.size()) - 1) % 4;
		switch (cornerNumber)
		{
		case 0:
			sign = 1;
			x = exploringDistance.x;
			exploringDistance.x += exploringDistanceIncrease;
			break;
		case 1:
		case 2:
			x = -exploringDistance.x;
			break;
		case 3:
			x = exploringDistance.x;
			break;
		}
		mapSearchTarget.x = currentSearchCenter.x + x;

		//Settting the x, in comparison to the searchCenter
		cornerNumber = (int(mapSearchTargets.size()) - 2) % 4;
		switch (cornerNumber)
		{
		case 0:
			sign = 1;
			y = -exploringDistance.y;
			exploringDistance.y += exploringDistanceIncrease;
			break;
		case 1:
		case 2:
			y = exploringDistance.y;
			break;
		case 3:
			y = -exploringDistance.y;
			break;
		}
		mapSearchTarget.y = currentSearchCenter.y + y;
	}

	pBlackBoard->ChangeData("ExploringDistance", exploringDistance);
	pBlackBoard->ChangeData("MapSearchtarget", mapSearchTarget);

	return Seeking(pBlackBoard);
}

//Adjust the currentSearch center, according to the discovered border
BehaviorState AddNewSearchCenter(Elite::Blackboard* pBlackBoard)
{
	float distanceBetweenSearchCenters{};
	Elite::Vector2 oldSearchCenter{};
	Elite::Vector2 newSearchCenter{};
	Elite::Vector2 mapBorder{};
	Elite::Vector2 xySign{};
	std::vector<Elite::Vector2> mapSearchTargets{};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("CurrentSearchCenter", oldSearchCenter)
						 && pBlackBoard->GetData("MapSearchTargets", mapSearchTargets)
						 && pBlackBoard->GetData("DistanceBetweenSearchCenters", distanceBetweenSearchCenters)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("xySign", xySign);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	newSearchCenter = Elite::Vector2{ oldSearchCenter.x + (distanceBetweenSearchCenters * xySign.x), oldSearchCenter.y + (distanceBetweenSearchCenters * xySign.y) };

	mapSearchTargets.push_back(newSearchCenter);

	pBlackBoard->ChangeData("MapSearchtarget", newSearchCenter);
	pBlackBoard->ChangeData("CurrentSearchCenter", newSearchCenter);

	ResetMapExplorationVariables(pBlackBoard);

	return Seeking(pBlackBoard);					
}

// Set the new center of exploration
BehaviorState AdjustSearchCenter(Elite::Blackboard * pBlackBoard)
{
	float distanceBetweenSearchCenters{};
	Elite::Vector2 oldSearchCenter{};
	Elite::Vector2 newSearchCenter{};
	Elite::Vector2 xySigns{};
	std::vector<Elite::Vector2> mapSearchTargets{};
	std::vector<Elite::Vector2> mapBorderCorners{};
	std::vector<Elite::Vector2>::iterator mapSearchTargetItterator{};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("CurrentSearchCenter", oldSearchCenter)
						 && pBlackBoard->GetData("MapSearchTargets", mapSearchTargets)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("DistanceBetweenSearchCenters", distanceBetweenSearchCenters)
						 && pBlackBoard->GetData("xySign", xySigns)
						 && pBlackBoard->GetData("MapBorderCorners", mapBorderCorners);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (int(mapBorderCorners.back().y) == 0)
	{
		//Set the currentSerachCenter to an adjusted value from the mapBorder
		newSearchCenter.x = mapBorderCorners.back().x + (distanceBetweenSearchCenters * -xySigns.x);

		float gg = mapBorderCorners.back().x / fabs(mapBorderCorners.back().x);
		xySigns.x = 0.f;
		xySigns.y = -(mapBorderCorners.back().x / fabs(mapBorderCorners.back().x));

		newSearchCenter.y = oldSearchCenter.y + (distanceBetweenSearchCenters * xySigns.y);
	}
	else
	{
		//Set the currentSerachCenter to an adjusted value from the mapBorder
		newSearchCenter.y = mapBorderCorners.back().y + (distanceBetweenSearchCenters * -xySigns.y);

		xySigns.x = mapBorderCorners.back().y / fabs(mapBorderCorners.back().y);
		xySigns.y = 0.f;

		newSearchCenter.x = oldSearchCenter.x + (distanceBetweenSearchCenters * xySigns.x);
	}

	//Set the xySign
	pBlackBoard->ChangeData("xySign", xySigns);

	pBlackBoard->ChangeData("MapSearchtarget", newSearchCenter);
	pBlackBoard->ChangeData("CurrentSearchCenter", newSearchCenter);

	ResetMapExplorationVariables(pBlackBoard);


	return Seeking(pBlackBoard);						
}
#pragma endregion

#pragma region Item usage
//Use health
BehaviorState UseMedkit(Elite::Blackboard* pBlackBoard)
{
	int itemSlot{};
	IExamInterface* pInterface{};
	std::vector<ItemInfo> agentInventory{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInventory", agentInventory);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	itemSlot = HasItem(pInterface, eItemType::MEDKIT, agentInventory);

	UseItem(itemSlot, pInterface, agentInventory) ;
	RemoveItemFromInventory(itemSlot, pInterface, agentInventory);

	pBlackBoard->ChangeData("AgentInventory", agentInventory);
	return BehaviorState::Success;
}

//Use food
BehaviorState UseFood(Elite::Blackboard* pBlackBoard)
{

	int itemSlot{};
	IExamInterface* pInterface{};
	std::vector<ItemInfo> agentInventory{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInventory", agentInventory);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	itemSlot = HasItem(pInterface, eItemType::FOOD, agentInventory);

	UseItem(itemSlot, pInterface, agentInventory);
	RemoveItemFromInventory(itemSlot, pInterface, agentInventory);

	pBlackBoard->ChangeData("AgentInventory", agentInventory);
	return BehaviorState::Success;
}
#pragma endregion

#pragma region Enemy handeling
//Evade the sighted enemy(s)
BehaviorState EvadeEnemy(Elite::Blackboard* pBlackBoard)
{
	IExamInterface* pInterface{};
	AgentInfo agentInfo{}; 
	std::vector<EntityInfo> enemiesInPOV{};
	EnemyInfo enemyInfo{};

	bool dataAvailable = pBlackBoard->GetData("EnemiesInPOV", enemiesInPOV)
						 && pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInfo", agentInfo);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}
	enemyInfo = GetClosestEnemy(enemiesInPOV, pInterface, agentInfo);

	pBlackBoard->ChangeData("ClosestEnemy", enemyInfo);

	return BehaviorState::Success;
}

//Shoot the enemy with the gun with the least ammo
BehaviorState ShootEnemy(Elite::Blackboard * pBlackBoard)
{
	int itemSlot{};
	SteeringAgent* pAgent{};
	IExamInterface* pInterface{};
	//ItemInfo itemInInventory{};
	AgentInfo agentInfo{};
	std::vector<ItemInfo> agentInventory{};
	std::vector<EntityInfo> enemiesInPOV{};
	EnemyInfo enemyInfo{};
	Elite::Vector2 polarFOVRangeFromAgent{};
	Elite::Vector2 cartesianFOVRangeFromAgent{};
	Elite::Vector2 cartesianAgentEnemy{};
	Elite::Vector2 polarAgentEnemy{};

	bool dataAvailable = pBlackBoard->GetData("EnemiesInPOV", enemiesInPOV)
						 && pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("Agent", pAgent)	
						 && pBlackBoard->GetData("AgentInventory", agentInventory);

	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	//Before shoot, turn towards the enemy
	enemyInfo = GetClosestEnemy(enemiesInPOV, pInterface, agentInfo);

	//Need the vector of agent-Enemy and agent-End of FOV straight ahead
	polarFOVRangeFromAgent = Elite::Vector2{agentInfo.FOV_Range, agentInfo.Orientation - agentInfo.FOV_Angle };
	cartesianFOVRangeFromAgent = agentInfo.Position + PolarToCartesian(polarFOVRangeFromAgent.x, polarFOVRangeFromAgent.y); 

	if (IsObjectnearLinePoints(agentInfo.Position, cartesianFOVRangeFromAgent, enemyInfo.Location, enemyInfo.Size))
	{
		//Shoot	
		itemSlot = HasItem(pInterface, eItemType::PISTOL, agentInventory);
		if (itemSlot != -1)
		{
			if (!UseItem(itemSlot, pInterface, agentInventory))
			{
				RemoveItemFromInventory(itemSlot, pInterface, agentInventory);
				pBlackBoard->ChangeData("IsBitten", false);
				pBlackBoard->ChangeData("AutOrientate", true);

				pBlackBoard->ChangeData("AgentInventory", agentInventory);
			}
		}
	}
	else
	{
		pBlackBoard->ChangeData("Target", enemyInfo.Location);
		pBlackBoard->ChangeData("AutOrientate", false);
		if (IsPointToTheLeftOfLine(agentInfo.Position, cartesianFOVRangeFromAgent, enemyInfo.Location))
		{
			pAgent->SetAngularVelocity(agentInfo.MaxAngularSpeed);
		}
		else
		{
			pAgent->SetAngularVelocity(-agentInfo.MaxAngularSpeed);
		}

		pAgent->SetMaxLinearSpeed(pAgent->GetMaxLinearSpeed() / 2.f);
		return Seeking(pBlackBoard);
	}
	
	return BehaviorState::Success;
}

//Turn around
BehaviorState TurnAround(Elite::Blackboard* pBlackBoard)
{
	SteeringAgent* pAgent{};
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("Agent", pAgent)
		&& pBlackBoard->GetData("AgentInfo", agentInfo);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	pBlackBoard->ChangeData("AutOrientate", false);
	pAgent->SetAngularVelocity(agentInfo.MaxAngularSpeed);

	return BehaviorState::Success;
}

//Run
BehaviorState Run(Elite::Blackboard* pBlackBoard)
{
	AgentInfo agentInfo{};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (agentInfo.Stamina > 0.f)
	{
		pBlackBoard->ChangeData("RunMode", true);
	}

	return BehaviorState::Success;
}

#pragma endregion

#pragma region Purge zone avoidance
//Go Inside House
BehaviorState EvadePurgeZone(Elite::Blackboard * pBlackBoard)
{
	Elite::Vector2 target;
	EntityInfo purgeZoneInPOV{};
	PurgeZoneInfo purgeZoneInfo{};
	IExamInterface* pInterface{};

	bool dataAvailable = pBlackBoard->GetData("PurgeZoneInPOV", purgeZoneInPOV)
						 && pBlackBoard->GetData("Interface", pInterface);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (purgeZoneInPOV.EntityHash == 0)
	{
		return BehaviorState::Failure;
	}
	
	pInterface->PurgeZone_GetInfo(purgeZoneInPOV, purgeZoneInfo);

	return Seeking(pBlackBoard);						
}

BehaviorState EvadePurgeZoneCircumNavigate(Elite::Blackboard* pBlackBoard)
{
	float distance{};
	float shortestDistance(FLT_MAX);
	float minimumDistance{};
	Elite::Vector2 evadePathPoint;
	Elite::Vector2 previousPathPoint;
	Elite::Vector2 twoPathPointsAgo;
	std::array<Elite::Vector2, 2> evadePathPointLast2Points{};
	std::vector<Elite::Vector2> evadePath{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("EvadePathPoint", evadePathPoint)
						 && pBlackBoard->GetData("EvadePath", evadePath)
						 && pBlackBoard->GetData("EvadePathPointLast2Points", evadePathPointLast2Points)
						 && pBlackBoard->GetData("MinimumDistance", minimumDistance);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (agentInfo.Position.Distance(evadePathPoint) < minimumDistance)
	{
		twoPathPointsAgo = evadePathPointLast2Points[1];
		previousPathPoint = evadePathPoint;
		evadePathPoint = ZeroVector2;
	}

	if (evadePathPoint == ZeroVector2)
	{
		for (int i{}; i < int(evadePath.size()); i++)
		{
			distance = agentInfo.Position.Distance(evadePath[i]);
			if ((shortestDistance > distance) && (evadePath[i] != evadePathPointLast2Points[0]) && (evadePath[i] != evadePathPointLast2Points[1]))
			{
				shortestDistance = distance;
				evadePathPoint = evadePath[i];
			}
		}
		evadePathPointLast2Points[0] = twoPathPointsAgo;
		evadePathPointLast2Points[1] = previousPathPoint;
		pBlackBoard->ChangeData("EvadePathPoint", evadePathPoint);
		pBlackBoard->ChangeData("EvadePathPointLast2Points", evadePathPointLast2Points);
	}
	pBlackBoard->ChangeData("Target", evadePathPoint);

	return Seeking(pBlackBoard);					
}

//Calculate the evasion path
BehaviorState CalculateEvadingPath(Elite::Blackboard* pBlackBoard)
{
	const int circleSides{ 16 };
	float degreeSteps{};
	float degrees{};
	float CircleSpacing{ 3.f };
	Elite::Vector2 pathPoint{};
	std::vector<Elite::Vector2> evadePath{};
	PurgeZoneInfo purgeZoneInPOV{};
	IExamInterface* pInterface{};

	bool dataAvailable = pBlackBoard->GetData("PurgeZoneInPOV", purgeZoneInPOV)
						 && pBlackBoard->GetData("Interface", pInterface);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}
	degreeSteps = 360 / circleSides;
	for (int i{}; i < circleSides; i++)
	{
		pathPoint = PolarToCartesian(purgeZoneInPOV.Radius + CircleSpacing, ToRadians(degrees));
		pathPoint += purgeZoneInPOV.Center;
		evadePath.push_back(pathPoint);

		degrees += degreeSteps;
	}

	pBlackBoard->ChangeData("EvadePath", evadePath);

	return BehaviorState::Success;						//After the target has been set to the middle of the house, just use the regular seek
}
#pragma endregion

#pragma endregion

#pragma region Helper functions
//Sets the behavior to seek and will seek to the coordinate set in "Target"
BehaviorState Seeking(Elite::Blackboard* pBlackBoard)
{
	float houseWallThickness{};
	ISteeringBehavior* pSeekBehavior = nullptr;
	SteeringAgent* pAgent = nullptr;
	Elite::Vector2 target;
	std::vector<HouseInfo>housesInPOV{};
	IExamInterface* pInterface{};

	bool dataAvailable = pBlackBoard->GetData("Agent", pAgent)
						 && pBlackBoard->GetData("SeekBehavior", pSeekBehavior)
						 && pBlackBoard->GetData("Target", target)
						 && pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("HousesInPOV", housesInPOV)
						 && pBlackBoard->GetData("HouseWallThickness", houseWallThickness);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	if (!pAgent || !pSeekBehavior)
	{
		return BehaviorState::Failure;
	}

	AbsoluteClampVectorwitVector(target, pInterface->World_GetInfo().Dimensions / 2);
	pBlackBoard->ChangeData("Target", target);

	return BehaviorState::Success;
}

//Gets the percantage of space an itemkind takes up in your inventory
float itemInventoryPercentage(Elite::Blackboard* pBlackBoard, const eItemType& itemType, IExamInterface* pInterface, const std::vector<ItemInfo>& agentInventory)
{
	ItemInfo itemInInventory{};
	int sameItemKindCount{};
	float itemKindPercentage{};

	itemInInventory.Type = eItemType::_LAST;	//Set the itemtyoe to _LAST, cause it defaults to PISTOL 

	for (int i{}; i < int(agentInventory.size()); i++)
	{
		if (agentInventory[i].Type == itemType && agentInventory[i].ItemHash != 0)
		{
			sameItemKindCount++;
		}
	}

	if (sameItemKindCount == 0)
	{
		return 0.f;
	}

	itemKindPercentage = float( float(sameItemKindCount / float(pInterface->Inventory_GetCapacity()) * 100.f));

	return itemKindPercentage;
}

//Checks if a certain item is needed
bool NeedItemCheck(Elite::Blackboard* pBlackBoard, const eItemType& itemType, float itemIdealPercentage, const std::vector<ItemInfo>& agentInventory)
{
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	float itemPercentageInInventory{};

	bool dataAvailable = pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("Interface", pInterface);
	if (!dataAvailable)
	{
		return false;
	}

	if (int(itemIdealPercentage) == 0)
	{
		return false;
	}

	itemPercentageInInventory = itemInventoryPercentage(pBlackBoard, itemType, pInterface, agentInventory);

	if (AlmostEqualRelative(itemPercentageInInventory, itemIdealPercentage))
	{
		return false;
	}
	else if (itemPercentageInInventory < itemIdealPercentage)
	{
		return true;
	}
	else
	{
		itemPercentageInInventory;
		std::cout << "More than the ideal amount, nani?";
	}

	return false;
}

//Get the IdealItemPercentage
float GetIdealItemPercentage(Elite::Blackboard* pBlackBoard, const eItemType& item)
{
	float itemIdealPercentage{};
	bool dataAvailable{false};

	switch (item)
	{
	case eItemType::PISTOL:
		dataAvailable = pBlackBoard->GetData("IdealGunsPercentage", itemIdealPercentage);
		if (!dataAvailable)
		{
			return 0.f;
		}

		return itemIdealPercentage;
		break;

	case eItemType::MEDKIT:
		dataAvailable = pBlackBoard->GetData("IdealMedkitPercentage", itemIdealPercentage);
		if (!dataAvailable)
		{
			return 0.f;
		}

		return itemIdealPercentage;
		break;

	case eItemType::FOOD:
		dataAvailable = pBlackBoard->GetData("IdealFoodAPercentage", itemIdealPercentage);
		if (!dataAvailable)
		{
			return 0.f;
		}

		return itemIdealPercentage;
		break;

	default:
		std::cout << "Hmm the item is not registered in the wanted item category \n";
		break;

	}

	return 0.f;
}

//Checks if one of the borderPoints of the map is sighted, if one is sighted that borderpoint will be returned, if not then a zerovector will be returned
Elite::Vector2 MapBorderEncountered(Elite::Blackboard* pBlackBoard, const Elite::Vector2& point)
{
	bool lastHitBorderIsX{};
	float borderMinimumDistance{ 3.f };
	float mapBorderYaxisAbsolute{};
	float mapBorderXaxisAbsolute{};
	Elite::Vector2 discoveredMapBorder{};
	IExamInterface* pInterface{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("LastHitBorderIsX", lastHitBorderIsX);
	if (!dataAvailable)
	{
		return ZeroVector2;
	}

	//Calculate the map Borders
	mapBorderXaxisAbsolute = pInterface->World_GetInfo().Center.x + (pInterface->World_GetInfo().Dimensions.x / 2.f);
	mapBorderYaxisAbsolute = pInterface->World_GetInfo().Center.y + (pInterface->World_GetInfo().Dimensions.y / 2.f);

	if (FloatIsNearTo(fabs(point.x), mapBorderXaxisAbsolute, borderMinimumDistance) && !lastHitBorderIsX)
	{
		std::cout << "Found a vertical orientated border \n";
		pBlackBoard->ChangeData("LastHitBorderIsX", true);
		discoveredMapBorder = Elite::Vector2((point.x / fabs(point.x)) * mapBorderXaxisAbsolute, 0.f);
		return discoveredMapBorder;
	}

	else if (FloatIsNearTo(fabs(point.y), mapBorderYaxisAbsolute, borderMinimumDistance) && lastHitBorderIsX)
	{
		std::cout << "Found a horizontal orientated border \n";
		pBlackBoard->ChangeData("LastHitBorderIsX", false);
		discoveredMapBorder = Elite::Vector2(0.f, (point.y / fabs(point.y)) * mapBorderYaxisAbsolute);
		return discoveredMapBorder;
	}

	return ZeroVector2;
}

//Changes to target of the ganet to the direction of the next mapcorner and checks if a corner has been found by the agent
BehaviorState FindNextMapCorner(Elite::Blackboard* pBlackBoard, const Elite::Vector2& currenPoint, const Elite::Vector2& previousPoint)
{
	bool lastHitBorderIsX{};
	Elite::Vector2 mapSearchtarget{};
	Elite::Vector2 mapBorder{};
	Elite::Vector2 mapCorner{};
	AgentInfo agentInfo{};
	IExamInterface* pInterface{};
	std::vector<Elite::Vector2> mapBorderCorners{};

	bool dataAvailable = pBlackBoard->GetData("Interface", pInterface)
						 && pBlackBoard->GetData("MapBorderCorners", mapBorderCorners)
						 && pBlackBoard->GetData("AgentInfo", agentInfo)
						 && pBlackBoard->GetData("MapSearchtarget", mapSearchtarget)
						 && pBlackBoard->GetData("LastHitBorderIsX", lastHitBorderIsX);
	if (!dataAvailable)
	{
		return BehaviorState::Failure;
	}

	//if the target is already set, then there is no need to recalculate the direction the agent needs to go in
	if (mapSearchtarget == ZeroVector2)
	{
		//Set the new target, which will be to the right of the previous found point on the map border, but since the agent can't see that point, put the target on the max value for x or y
		if (!lastHitBorderIsX)
		{
			float x = currenPoint.x / fabs(currenPoint.x) * -FLT_MAX;
			mapSearchtarget = Elite::Vector2(x, agentInfo.Position.y);
		}
		else 
		{
			float y = currenPoint.y / fabs(currenPoint.y) * -FLT_MAX;
			mapSearchtarget = Elite::Vector2(agentInfo.Position.x, y);
		}
		pBlackBoard->ChangeData("MapSearchtarget", mapSearchtarget);
	}

	//Check if a corner of the map has been found
	mapBorder = MapBorderEncountered(pBlackBoard, agentInfo.Position);
	if (mapBorder != ZeroVector2)
	{
		if (AlmostEqualRelative(mapBorder.x, 0.f))
		{
			mapBorder.x = mapBorderCorners.back().x;
		}
		else if(AlmostEqualRelative(mapBorder.y, 0.f))
		{
			mapBorder.y = mapBorderCorners.back().y;
		}
		else
		{
			std::cout << "The mapborderFind has fucked up";
		}

		Elite::Vector2 kk = mapBorderCorners.back();

		if (mapBorder != mapBorderCorners.back())
		{
			mapBorderCorners.push_back(mapBorder);
			pBlackBoard->ChangeData("MapBorderCorners", mapBorderCorners);
			pBlackBoard->ChangeData("MapSearchtarget", ZeroVector2);
			return BehaviorState::Success;
		}
	}

	pBlackBoard->ChangeData("Target", mapSearchtarget);
	
	return Seeking(pBlackBoard);						//After the target has been set to the middle of the house, just use the regular seek
}

//Get closest enemy to the agent
EnemyInfo GetClosestEnemy(const std::vector<EntityInfo>& enemiesInPOV, IExamInterface* pInterface, AgentInfo agentInfo )
{
	float distanceToEnemy{ FLT_MAX };
	EntityInfo ClosestEnemy{};
	EnemyInfo enemyInfo{};

	for (int i{}; i < int(enemiesInPOV.size()); i++)
	{
		pInterface->Enemy_GetInfo(enemiesInPOV[i], enemyInfo);
		if (agentInfo.Position.Distance(enemiesInPOV[i].Location) < distanceToEnemy)
		{
			ClosestEnemy = enemiesInPOV[0];
			pInterface->Enemy_GetInfo(ClosestEnemy, enemyInfo);							//Get the EnemyInfo from the EntityInfo
			distanceToEnemy = agentInfo.Position.Distance(enemiesInPOV[i].Location);
		}
	}

	return enemyInfo;
}

//Does the agnet have a certain item in his inventory?
int HasItem(IExamInterface* pInterface, eItemType itemType, const std::vector<ItemInfo>& agentInventory)
{
	int itemSlot{};
	ItemInfo itemInInventory{};

	for (int i{}; i < int(agentInventory.size()); i++)
	{
		if (agentInventory[i].Type == itemType && agentInventory[i].ItemHash != 0)
		{
			itemSlot = i;
			if (pInterface->Inventory_GetItem(i, itemInInventory))
			{
				return itemSlot;
			}
		}
	}

	return -1;
}


//Add item
bool AddItemToInventory(IExamInterface* pInterface, std::vector<ItemInfo>& agentInventory, ItemInfo item)
{
	bool itemAdded{};
	int freeInventorySlot{};

	//If the agentInventory is not as big as the interface inventory, just pushback the item
	if (int(agentInventory.size()) < pInterface->Inventory_GetCapacity() )
	{
		agentInventory.push_back(item);
		freeInventorySlot = int(agentInventory.size()) - 1;
	}
	else
	{
		//Search for a free spots
		for (int i{}; i < int(agentInventory.size()); i++)
		{
			if (agentInventory[i].ItemHash == 0)
			{
				freeInventorySlot = i;
				break;
			}
		}
	}

	itemAdded = pInterface->Inventory_AddItem(freeInventorySlot, item);
	if (itemAdded)
	{
		agentInventory[freeInventorySlot] = item;
	}

	return itemAdded;
}

bool UseItem(int itemSlot, IExamInterface* pInterface, std::vector<ItemInfo>& agentInventory)
{
	bool itemUsedSuccesfully{};

	itemUsedSuccesfully = pInterface->Inventory_UseItem(itemSlot);
	
	return itemUsedSuccesfully;
}

bool RemoveItemFromInventory(int itemSlot, IExamInterface* pInterface, std::vector<ItemInfo>& agentInventory)
{
	bool itemRemovedSuccesfully{};

	itemRemovedSuccesfully = pInterface->Inventory_RemoveItem(itemSlot);

	if (itemRemovedSuccesfully)
	{
		agentInventory[itemSlot] = ItemInfo{};
	}

	return itemRemovedSuccesfully;
}

//Reseting variables
void ResetHouseVariables(Elite::Blackboard* pBlackBoard)
{
	pBlackBoard->ChangeData("CurrentHouse", HouseInfo{});
	pBlackBoard->ChangeData("HouseSearchPathPoints", std::vector < Elite::Vector2 >{});
	pBlackBoard->ChangeData("SearchPathCalculated", false);
	pBlackBoard->ChangeData("Target", ZeroVector2);
}
void ResetItemregistrationVariables(Elite::Blackboard* pBlackBoard)
{
	pBlackBoard->ChangeData("ItemTargetPosition", ZeroVector2);
	pBlackBoard->ChangeData("Target", ZeroVector2);
}
void ResetItemPickupVariables(Elite::Blackboard* pBlackBoard)
{
	pBlackBoard->ChangeData("ItemTargetPosition", ZeroVector2);
	pBlackBoard->ChangeData("Target", ZeroVector2);
}
void ResetMapExplorationVariables(Elite::Blackboard* pBlackBoard)
{
	pBlackBoard->ChangeData("ExploringDistance", Elite::Vector2{});
	pBlackBoard->ChangeData("MapSearchTargets", std::vector<Elite::Vector2>{});
}
void ResetPurgeZoneAvoidanceVariables(Elite::Blackboard* pBlackBoard)
{
	pBlackBoard->ChangeData("EvadePath", std::vector<Elite::Vector2>{});
	pBlackBoard->ChangeData("EvadePathPoint", Elite::Vector2{});
}

//Manual override related
BehaviorState ChangeToSeek(Elite::Blackboard* pBlackBoard)
{
	return Seeking(pBlackBoard);
}
inline bool HasTarget(Elite::Blackboard* pBlackBoard)
{
	bool hasTarget = false;

	bool dataAvailable = pBlackBoard->GetData("TargetSet", hasTarget);
	if (!dataAvailable)
	{
		return false;
	}

	return hasTarget;
}
inline bool NotCloseToTarget(Elite::Blackboard* pBlackBoard)
{
	SteeringAgent* pAgent = nullptr;
	Elite::Vector2 targetPos = Elite::Vector2();
	float closeToRadius = 1.0f;

	bool dataAvailable = pBlackBoard->GetData("Agent", pAgent)
		&& pBlackBoard->GetData("Target", targetPos);
	;	if (!dataAvailable)
	{
		return false;
	}

	if (!pAgent)
	{
		return false;
	}

	float distance = (targetPos - pAgent->GetPosition()).Magnitude();
	if (distance > closeToRadius)
	{
		return true;
	}

	return false;
}

#pragma endregion
