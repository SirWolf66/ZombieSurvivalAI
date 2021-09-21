//=== General Includes ===
#include "stdafx.h"
#include "ZombieSurvivalBehaviorTree.h"
#include "Steering/SteeringHelpers.h"
#include "Steering/SteeringBehaviors.h"
#include "Behaviors.h"
#include <chrono>

//=== Constructor & Destructor ===
ZombieSurvivalBehaviorTree::ZombieSurvivalBehaviorTree(SteeringAgent* pAgent, IExamInterface* pExamInterface)
	:m_pAgent(pAgent)
	, m_pInterface(pExamInterface)
{
	Initialize();
}

ZombieSurvivalBehaviorTree::~ZombieSurvivalBehaviorTree()
{
	for (auto pb : m_vpBehaviorVec)
		SAFE_DELETE(pb);
	m_vpBehaviorVec.clear();

	SAFE_DELETE(m_pBehaviorTree);
}

//=== Functions ===
void ZombieSurvivalBehaviorTree::Initialize()
{
	//Create behaviors
	float seekBehaviorWeight{ 0.25f };
	float enemyAvoidanceWeight{ 0.25f };
	float purgeZoneAvoidanceWeight{ 0.5f };

	//Seek behavior
	Seek* pSeekBehavior = new Seek();
	m_vpBehaviorVec.push_back(pSeekBehavior);

	//EnemyAvoidance behavior
	CollisionAvoidance* pEnemyAvoidanceBehavior = new CollisionAvoidance();
	m_vpBehaviorVec.push_back(pEnemyAvoidanceBehavior);

	//Blended steering
	auto pSeekAvoid = new BlendedSteering({ {pSeekBehavior, seekBehaviorWeight}, {pEnemyAvoidanceBehavior, enemyAvoidanceWeight} });
	m_vpBehaviorVec.push_back(pSeekAvoid);

	//Set the agent SteeringBehavior
	m_pAgent->SetSteeringBehavior(pSeekAvoid);

	Blackboard* pBlackBoard = new Blackboard();

	//The interface
	pBlackBoard->AddData("Interface", m_pInterface);

	//Behaviors
	pBlackBoard->AddData("SeekBehavior", static_cast<ISteeringBehavior*>(pSeekBehavior));
	pBlackBoard->AddData("EnemyAvoidanceBehavior", static_cast<ISteeringBehavior*>(pEnemyAvoidanceBehavior));

	//Agent info
	pBlackBoard->AddData("Agent", m_pAgent);
	pBlackBoard->AddData("AgentInfo", AgentInfo{});
	pBlackBoard->AddData("AutOrientate", true);
	pBlackBoard->AddData("RunMode", false);

	//Inventory
	pBlackBoard->AddData("AgentInventory", std::vector<ItemInfo>{});
	pBlackBoard->AddData("NeededItems", std::vector<eItemType>{});
	pBlackBoard->AddData("IdealGunsPercentage", 40.f);
	pBlackBoard->AddData("IdealMedkitPercentage", 40.f);
	pBlackBoard->AddData("IdealFoodAPercentage", 20.f);

	//Target info
	pBlackBoard->AddData("TargetSet", false);
	pBlackBoard->AddData("Target", Elite::Vector2{});
	pBlackBoard->AddData("PathPoint", Elite::Vector2{});
	pBlackBoard->AddData("MinimumDistance", 2.f);

	//Objects in FOV
	pBlackBoard->AddData("PurgeZoneInPOV", PurgeZoneInfo{});
	pBlackBoard->AddData("EnemiesInPOV", std::vector<EntityInfo>{});
	pBlackBoard->AddData("ItemsInPOV", std::vector<EntityInfo>{});
	pBlackBoard->AddData("HousesInPOV", std::vector<HouseInfo>{});

	//House info
	pBlackBoard->AddData("HouseWallThickness", 3.f);
	pBlackBoard->AddData("CurrentHouse", HouseInfo{});
	pBlackBoard->AddData("HouseSearchPathPoints", std::vector <Elite::Vector2> {});
	pBlackBoard->AddData("SearchedHousesVector", std::vector<std::pair<Elite::Vector2, std::chrono::steady_clock::time_point>>{}); //Vector with searched houses and the items in the houses
	pBlackBoard->AddData("SearchPathCalculated", false);

	//Iteminfo
	pBlackBoard->AddData("RegisteredItems", std::vector <ItemInfo> {});
	pBlackBoard->AddData("FoundItemsInHouse", std::vector<EntityInfo>{});
	pBlackBoard->AddData("ItemTargetPosition", Elite::Vector2{});

	//MapExploration
	pBlackBoard->AddData("MapDimensionsFound", false);
	pBlackBoard->AddData("LastHitBorderIsX", false);
	pBlackBoard->AddData("MapCornersAmount", 4);
	pBlackBoard->AddData("DistanceBetweenSearchCenters", 64.f); //66.f
	pBlackBoard->AddData("CurrentSearchCenter", Elite::Vector2{});
	pBlackBoard->AddData("xySign", Elite::Vector2{-1.f, 0.f});
	pBlackBoard->AddData("MapSearchtarget", Elite::Vector2{});
	pBlackBoard->AddData("ExploringDistance", Elite::Vector2{});
	pBlackBoard->AddData("MapSearchTargets", std::vector<Elite::Vector2>{});
	pBlackBoard->AddData("MapBorderCorners", std::vector<Elite::Vector2>{});
	pBlackBoard->AddData("SearchStartTime", std::chrono::high_resolution_clock::now());

	//PurgeZone avoidance
	pBlackBoard->AddData("EvadePath", std::vector<Elite::Vector2>{});
	pBlackBoard->AddData("EvadePathPoint", Elite::Vector2{});
	pBlackBoard->AddData("EvadePathPointLast2Points", std::array<Elite::Vector2, 2>{});

	//EnemyInfo
	pBlackBoard->AddData("IsBitten", false);
	pBlackBoard->AddData("ClosestEnemy", EnemyInfo{});
	pBlackBoard->AddData("TimeSinceEnemySighted", 0.f);
	pBlackBoard->AddData("RemeberEnemyTime", 1.f);

	//misc
	pBlackBoard->AddData("deltaTime", 0.f);

	m_pBehaviorTree = new BehaviorTree(pBlackBoard,
		new BehaviorSequence
		({
			new BehaviorSelector	//Everything related to setting the target for the Seek
			({
				#pragma region PurgeZoneAvoidance
				new BehaviorSequence
				({
					new BehaviorConditional(PurgeZoneInPOV),
					new BehaviorForceSucces(new BehaviorSequence
					({
						new BehaviorInverter(new BehaviorConditional(EvasionPathCalculated)),
						new BehaviorAction(CalculateEvadingPath)
					})),
					new BehaviorAction(EvadePurgeZoneCircumNavigate)
				}),
				#pragma endregion

				#pragma region House search
				new BehaviorSequence
				({
					new BehaviorConditional(HouseInPOV),
					new BehaviorInverter(new BehaviorConditional(HouseSearched)),
					new BehaviorSelector
					({
						new BehaviorSequence
						({
							new BehaviorInverter(new BehaviorConditional(AgentInside)),
							new BehaviorAction(GoInside)
						}),
						new BehaviorSequence
						({
							new BehaviorInverter(new BehaviorConditional(SearchPathCalculated)),
							new BehaviorAction(CalculateSearchPathPoints)
						}),
						new BehaviorSequence
						({
							new BehaviorInverter(new BehaviorConditional(PathPointFound)),
							new BehaviorAction(FindClosestPathPoint)
						}),
						new BehaviorAction(GoToPathPoint)
					}),
					new BehaviorAction(RegisterItemLocation)
				}),
			#pragma endregion

				#pragma region Item registration
				new BehaviorSequence
				({
					new BehaviorConditional(ItemsNeedRegistration),
					new BehaviorSelector
					({
						new BehaviorSequence
						({
							new BehaviorInverter(new BehaviorConditional(ItemTargetSet)),
							new BehaviorAction(FindClosestItemInHouse),
						}),
						new BehaviorAction(GoToItemAndRegister)
					}),
					new BehaviorAction(RegisterItemLocation)
				}),
				#pragma endregion

				#pragma region Item pickup
				new BehaviorSequence
				({
					new BehaviorConditional(NeedItem),
					new BehaviorConditional(DicoveredNeededItem),
					new BehaviorAction(GoToItemAndPickItUp)
				}),
				#pragma endregion

				#pragma region Find map dimensions
				new BehaviorSequence
				({
					new BehaviorForceSucces (new BehaviorSelector
					({
						new BehaviorSequence
						({
							new BehaviorConditional(MapBorderFound),
							new BehaviorAction(AdjustSearchCenter)
						}),
						new BehaviorSequence
						({
							new BehaviorConditional(PastMaximumSearchNodes),
							new BehaviorAction(AddNewSearchCenter)
						}),
						new BehaviorSequence
						({
							new BehaviorInverter(new BehaviorConditional(ExplorationPointSet)),
							new BehaviorAction(SetNewExplorationPoint),
						})
					})),			
					new BehaviorAction(GoToExplorationPoint)
				}),
				#pragma endregion

				#pragma region Manual target setting
				//Manual overide
				new BehaviorSequence
				({
					new BehaviorConditional(HasTarget),
					new BehaviorSelector
					({

						new BehaviorSequence
						({
							new BehaviorConditional(NotCloseToTarget),
							new BehaviorAction(ChangeToSeek)
						}),
					})
				}),
				#pragma endregion
			}),

			#pragma region Item usage
			new BehaviorForceSucces( new BehaviorSelector
			({
				new BehaviorSequence
				({
					new BehaviorConditional(NeedToUseMedkit),
					new BehaviorConditional(HasMedkit),
					new BehaviorAction(UseMedkit)
				}),
				new BehaviorSequence
				({
					new BehaviorConditional(NeedToUseFood),
					new BehaviorConditional(HasFood),
					new BehaviorAction(UseFood)
				})
			})),
			#pragma endregion

			#pragma region Enemy handeling
			new BehaviorSequence						
			({
			 	new BehaviorForceSucces( new BehaviorSequence
				({
					new BehaviorConditional(IsBitten),
					new BehaviorSelector
					({
						new BehaviorSequence
						({
							new BehaviorConditional(HasAmmo),
							new BehaviorAction(TurnAround)
						}),
						new BehaviorAction(Run)
					}),
					//new BehaviorConditional(HasAmmo),
					//new BehaviorAction(TurnAround)
				})),
				new BehaviorConditional(EnemiesInFOV),
				new BehaviorSelector
				({
					new BehaviorSequence
					({
						new BehaviorConditional(HasAmmo),
						new BehaviorAction(ShootEnemy)
					}),
					new BehaviorAction(EvadeEnemy)
				})
			})
			#pragma endregion
		}));
}

void ZombieSurvivalBehaviorTree::Update(float deltaTime, SteeringPlugin_Output& steeringOutputPlugin, const Elite::Vector2& targetPos, const AgentInfo& agentInfo, const vector<HouseInfo>& houseInfoVector, const vector<EntityInfo>& entitiesInfoVector)
{
	auto pBlackBoard = m_pBehaviorTree->GetBlackboard();
	HouseInfo housinfo{};
	std::vector<EntityInfo> EnemyVector{};
	std::vector<EntityInfo> ItemVector{};
	PurgeZoneInfo purgeZone{};

	pBlackBoard->GetData("CurrentHouse", housinfo);

	if (pBlackBoard)
	{
		//This target setting is only for manual override
		if (housinfo.Center == ZeroVector2)
		{
			pBlackBoard->ChangeData("TargetSet", true);
			pBlackBoard->ChangeData("Target", targetPos);
		}

		pBlackBoard->ChangeData("AgentInfo", agentInfo);
		pBlackBoard->ChangeData("HousesInPOV", houseInfoVector);
		pBlackBoard->ChangeData("deltaTime", deltaTime);

		//Split the entities in the POV in an Enemy and Item vector
		for (int i{}; i < int(entitiesInfoVector.size()); i++)
		{
			if (entitiesInfoVector[i].Type == eEntityType::ENEMY)
			{
				EnemyVector.push_back(entitiesInfoVector[i]);
			}
			else if (entitiesInfoVector[i].Type == eEntityType::ITEM)
			{
				ItemVector.push_back(entitiesInfoVector[i]);
			}
			else if (entitiesInfoVector[i].Type == eEntityType::PURGEZONE)
			{
				m_pInterface->PurgeZone_GetInfo(entitiesInfoVector[i], purgeZone);
			}
		}
		pBlackBoard->ChangeData("ItemsInPOV", ItemVector);
		pBlackBoard->ChangeData("EnemiesInPOV", EnemyVector);
		pBlackBoard->ChangeData("PurgeZoneInPOV", purgeZone);
	}

	UpdateHouses();
	m_pBehaviorTree->Update(); 

	//Calculate the steering and pass it back
	//If we run out of stammina, stop running
	if (AlmostEqualRelative(agentInfo.Stamina, 0.f))
	{
		pBlackBoard->ChangeData("RunMode", false);
	}

	bool autoOrientate{};
	bool runMode{};
	Elite::Vector2 targetPosition{};
	EntityInfo purgeZoneInFOV{};
	EnemyInfo enemyInfo{};
	PurgeZoneInfo purgeZoneInfo{};
	ISteeringBehavior* pSeek{};
	ISteeringBehavior* pEnemyAvoidanceBehavior{};
	ISteeringBehavior* pPurgeZoneAvoidanceBehavior{};

	pBlackBoard->GetData("Target", targetPosition);
	pBlackBoard->GetData("SeekBehavior", pSeek);
	pBlackBoard->GetData("EnemyAvoidanceBehavior", pEnemyAvoidanceBehavior);
	pBlackBoard->GetData("ClosestEnemy", enemyInfo);
	pBlackBoard->GetData("AutOrientate", autoOrientate);
	pBlackBoard->GetData("RunMode", runMode);

	//Set the target of the steering behavior as the next Pathpoint on route to the target
	GotoPoint(targetPosition, pSeek, m_pInterface);
	//Set the parameters needed for avoiding an enemy
	dynamic_cast<CollisionAvoidance*>(pEnemyAvoidanceBehavior)->SetAvoidingParameters(enemyInfo.Size, enemyInfo.Location, agentInfo.FOV_Range);

	//Calculate the steeringBehavior
	SteeringOutput steeringOutput{m_pAgent->CalculateSteering(deltaTime)};

	//Convert steeringBehavior to the Plugin steeringbehavior
	steeringOutputPlugin = SteeringOutput_To_SteeringPlugin_Output(steeringOutput, autoOrientate, runMode);	
}

void ZombieSurvivalBehaviorTree::Render(float deltaTime)
{
	//Debug render
	auto pBlackBoard = m_pBehaviorTree->GetBlackboard();
	Elite::Vector2 targetPosition{};
	pBlackBoard->GetData("Target", targetPosition);

	m_pInterface->Draw_SolidCircle(targetPosition, .7f, { 0,0 }, { 1, 0, 0 });

	//temp debug
	AgentInfo agentInfo{};
	std::vector<Elite::Vector2>  mapSearchTargets{};
	std::vector<Elite::Vector2> evadePath{};
	pBlackBoard->GetData("MapSearchTargets", mapSearchTargets);
	pBlackBoard->GetData("AgentInfo", agentInfo);
	pBlackBoard->GetData("EvadePath", evadePath);

	//Middle FOV
	Elite::Vector2 PolarFOVRangeFromAgent{ agentInfo.FOV_Range, agentInfo.Orientation - agentInfo.FOV_Angle };
	Elite::Vector2 cartesianFOVRangeFromAgent{ agentInfo.Position + PolarToCartesian(PolarFOVRangeFromAgent.x, PolarFOVRangeFromAgent.y) };

	//Left FOV border
	Elite::Vector2 PolarFOVRangeFromAgentLeft{ agentInfo.FOV_Range, agentInfo.Orientation - agentInfo.FOV_Angle / 2 };
	Elite::Vector2 cartesianFOVRangeFromAgentLeft{ agentInfo.Position + PolarToCartesian(PolarFOVRangeFromAgentLeft.x, PolarFOVRangeFromAgentLeft.y) };

	//Right FOV border
	Elite::Vector2 PolarFOVRangeFromAgentRight{ agentInfo.FOV_Range, agentInfo.Orientation - agentInfo.FOV_Angle * 1.5f };
	Elite::Vector2 cartesianFOVRangeFromAgentRight{ agentInfo.Position + PolarToCartesian(PolarFOVRangeFromAgentRight.x, PolarFOVRangeFromAgentRight.y) };

	//Right FOV border
	m_pInterface->Draw_Segment(agentInfo.Position, cartesianFOVRangeFromAgent, { 1, 0, 0 });
	m_pInterface->Draw_Segment(agentInfo.Position, cartesianFOVRangeFromAgentLeft, { 1, 0, 0 });
	m_pInterface->Draw_Segment(agentInfo.Position, cartesianFOVRangeFromAgentRight, { 1, 0, 0 });

	//Print mapSearchtargets
	for (int i{}; i < int(mapSearchTargets.size()); i++)
	{
		if (i >= 2)
		{
			m_pInterface->Draw_Segment(mapSearchTargets[i - 1], mapSearchTargets[i], { 1, 0, 0 });
		}
	}

	//POV evade path
	if (!evadePath.empty())
	{
		for (int i{}; i < int(evadePath.size()); i++)
		{
			m_pInterface->Draw_SolidCircle(evadePath[i], 0.4f, {0, 0}, { 0, 1, 0 });
		}
	}
}
void ZombieSurvivalBehaviorTree::UpdateHouses()
{
	float time{130.f};
	auto pBlackBoard = m_pBehaviorTree->GetBlackboard();
	std::vector<std::pair<Elite::Vector2, std::chrono::steady_clock::time_point>> searchedHouseVector{};
	std::chrono::steady_clock::time_point now{ std::chrono::high_resolution_clock::now() };

	pBlackBoard->GetData("SearchedHousesVector", searchedHouseVector);

	for (int i{}; i < int(searchedHouseVector.size()); i++)
	{
		float dif = std::chrono::duration<float>(now - searchedHouseVector[i].second).count();
		if (searchedHouseVector[i].second != std::chrono::steady_clock::time_point{} 
			&& std::chrono::duration<float>(now - searchedHouseVector[i].second).count() > time)
		{
			searchedHouseVector[i].second = std::chrono::steady_clock::time_point{};	//Or just putting the time it was discovered to the default value of time_point
			pBlackBoard->ChangeData("SearchedHousesVector", searchedHouseVector);
			std::cout << "House has been tossed out";
		}
	}
}


