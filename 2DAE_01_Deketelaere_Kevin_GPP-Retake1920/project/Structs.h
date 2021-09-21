#pragma once

//Basic color
struct Color final
{
	//Constructors
	Color() = default;
	Color(float _r, float _g, float _b, float _a = 1.0f)
	{
		r = _r; g = _g; b = _b; a = _a;
	}

	//Datamembers
	float r = 0.f, g = 0.f, b = 0.f, a = 1.f;
};

//Rigifbody
struct RigidBody final
{
	//Constructors
	RigidBody(Elite::Vector2 _position = {0.f, 0.f}, Elite::Vector2 _linearVelocity = { 0.f, 0.f }, float _rotation = 0.f, float _angularVelocity = 0.f, float _mass = 0.f)
		:position(_position),
		linearVelocity(_linearVelocity),
		rotation(_rotation),
		angularVelocity(_angularVelocity),
		mass(_mass)
	{		
	}

	//Datamembers
	Elite::Vector2 position = Elite::Vector2{0.f, 0.f};
	Elite::Vector2 linearVelocity = Elite::Vector2{0.f, 0.f};

	float rotation = 0.f;
	float angularVelocity = 0.f;
	float mass = 0.f;
};

#pragma region Functors
//Compare if 2 entotInfo's are the same
struct EntityInfoCompareFunctor
{
public:
	explicit EntityInfoCompareFunctor(const EntityInfo& otherEntity) :m_otherEntity{ otherEntity } {}
	inline bool operator() (const EntityInfo& currentEntity )
	{
		bool itsTheSame{false};

		itsTheSame = currentEntity.EntityHash == m_otherEntity.EntityHash;

		return itsTheSame;
	}

private:
	EntityInfo m_otherEntity;
};

//Compare if an EntityInfo and an itemInfo are the same, based on location(compare an iteminfo with an entityinfo vector)
struct ItemAndEntityLocationeCompareFunctor
{
public:
	explicit ItemAndEntityLocationeCompareFunctor(const ItemInfo& otherEntity) :m_otherItem{ otherEntity } {}
	inline bool operator() (const EntityInfo& currentItem)
	{
		bool itsTheSame{ false };

		itsTheSame = currentItem.Location == m_otherItem.Location;
		itsTheSame = itsTheSame && currentItem.Type == eEntityType::ITEM;

		return itsTheSame;
	}

private:
	ItemInfo m_otherItem;
};

// Compare if an EntityInfo and an itemInfo are the same, based on location(compare an entityInfo with an itemInfo vector)
struct EntityAndItemLocationeCompareFunctor
{
public:
	explicit EntityAndItemLocationeCompareFunctor(const EntityInfo& otherEntity) :m_otherItem{ otherEntity } {}
	inline bool operator() (const ItemInfo& currentItem)
	{
		bool itsTheSame{ false };

		//itsTheSame = currentItem.ItemHash == m_otherItem.EntityHash;

		itsTheSame = currentItem.Location == m_otherItem.Location;
		//itsTheSame = itsTheSame && currentItem.Type == eEntityType::ITEM;

		return itsTheSame;
	}

private:
	EntityInfo m_otherItem;
};

//Compare if 2 itemInfo's are the same, based on Location and itemType
struct ItemInfoCompareFunctor
{
public:
	explicit ItemInfoCompareFunctor(const ItemInfo& otherEntity) :m_otherItem{ otherEntity } {}
	inline bool operator() (const ItemInfo& currentItem)
	{
		bool itsTheSame{ false };

		itsTheSame = currentItem.Location == m_otherItem.Location;
		itsTheSame = currentItem.Type == m_otherItem.Type;

		return itsTheSame;
	}

private:
	ItemInfo m_otherItem;
};

//Compare if 2 ItemInfo's are the same ItemType
struct ItemTypeCompareFunctor
{
public:
	explicit ItemTypeCompareFunctor(const eItemType& otherEntity) :m_otherItem{ otherEntity } {}
	inline bool operator() (const ItemInfo& currentItem)
	{
		bool itsTheSame{ false };

		itsTheSame = currentItem.Type == m_otherItem;

		return itsTheSame;
	}

private:
	eItemType m_otherItem;
};
#pragma endregion

