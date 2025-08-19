#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

extern int DartCounter;

// when a dart hits something, they create a new new dart called CPhysicsDarts and then delete themselves.
// this because for some reason goldsrc doesn`t allow us to change pev->solid after spawning
class CPhysicsDart : public CBaseEntity
{
	void Spawn() override;
	int Classify() override { return 0; };
	void EXPORT DartBounce(CBaseEntity* pOther);
	void EXPORT PickupThink();
	float m_flDieTime;
	string_t ammotype;

public:
	static CPhysicsDart* PhysDartCreate(string_t customammotype);
};

void CPhysicsDart::Spawn()
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	pev->gravity = 0.5;
	SET_MODEL(ENT(pev), "models/bludart.mdl");

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetThink(&CPhysicsDart::PickupThink);
	SetTouch(&CPhysicsDart::DartBounce);
	m_flDieTime = gpGlobals->time + 35;
	pev->nextthink = gpGlobals->time + 0.2;
}

CPhysicsDart* CPhysicsDart::PhysDartCreate(string_t customammotype)
{
	CPhysicsDart* pBolt = GetClassPtr((CPhysicsDart*)NULL);
	pBolt->pev->classname = MAKE_STRING("bolt");
	pBolt->Spawn();

	pBolt->pev->gravity = 0.5;
	pBolt->pev->friction = 0.8;
	pBolt->ammotype = customammotype;

	return pBolt;
}

void CPhysicsDart::PickupThink()
{
	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin + Vector(0, 0, 32), 20)) != NULL)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity->MyMonsterPointer();

		if (pPlayer && pPlayer->IsPlayer() && pPlayer->IsAlive())
		{
			pPlayer->GiveAmmo(1, STRING(ammotype), _9MM_MAX_CARRY);
			EMIT_SOUND(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/Revolver/primeforward.wav", VOL_NORM, ATTN_NORM);
			SUB_Remove();
			DartCounter--;
			return;
		}
	}
	if (m_flDieTime < gpGlobals->time)
	{
		SUB_Remove();
		DartCounter--;
		return;
	}
	if (pev->velocity.Length() < 4)
	{
		pev->angles.x = 0; // lay flat
						   // DROP_TO_FLOOR(ENT(pev));
	}
	pev->nextthink = gpGlobals->time + 0.2;
};

void CPhysicsDart::DartBounce(CBaseEntity* pOther)
{
	pev->velocity = pev->velocity / 5;
}


CDart* CDart::DartCreate(string_t customammotype)
{
	// Create a new entity with CCrossbowBolt private data
	CDart* pBolt = GetClassPtr((CDart*)NULL);
	pBolt->pev->classname = MAKE_STRING("bolt");
	pBolt->Spawn();
	pBolt->ammotype = customammotype;

	return pBolt;
}

void CDart::Spawn()
{
	Precache();
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	SET_MODEL(ENT(pev), "models/bludart.mdl");

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch(&CDart::DartTouch);
	pev->nextthink = gpGlobals->time + 0.2;
	inactive = false;
}


void CDart::Precache()
{
	// PRECACHE_MODEL("models/bludart.mdl"); precache these in mp5
	// PRECACHE_SOUND("weapons/primeforward.wav");
	PRECACHE_SOUND("fvox/beep.wav");
	m_iTrail = PRECACHE_MODEL("sprites/streak.spr");
}


int CDart::Classify()
{
	return CLASS_NONE;
}

void CDart::DartTouch(CBaseEntity* pOther)
{
	if (0 != pOther->pev->takedamage && !inactive)
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		entvars_t* pevOwner;

		pevOwner = VARS(pev->owner);

		// UNDONE: this needs to call TraceAttack instead
		ClearMultiDamage();

		if (pOther->IsPlayer())
		{
			pOther->TraceAttack(pevOwner, 10, pev->velocity.Normalize(), &tr, DMG_NEVERGIB);
		}
		else
		{
			pOther->TraceAttack(pevOwner, 10, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
		}

		ApplyMultiDamage(pev, pevOwner);
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "dart/hit_living.wav", 1, ATTN_NORM);
	}
	if (DartCounter <= 75)
	{
		DartCounter++;
		CPhysicsDart* physdart = CPhysicsDart::PhysDartCreate(ammotype);
		physdart->pev->angles = pev->angles;
		physdart->pev->origin = pev->origin;
		physdart->pev->velocity = pev->velocity;
		physdart->pev->owner = pev->owner;
		pev->nextthink = gpGlobals->time;
		SUB_Remove();
	}
	else
	{
		SUB_Remove();
	}
}

void CDart::PickupThink()
{
	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin + Vector(0, 0, 32), 20)) != NULL)
	{
		CBasePlayer* pPlayer = (CBasePlayer*)pEntity->MyMonsterPointer();

		if (pPlayer && pPlayer->IsPlayer() && pPlayer->IsAlive())
		{
			pPlayer->GiveAmmo(1, "9mm", _9MM_MAX_CARRY);
			EMIT_SOUND(ENT(pPlayer->pev), CHAN_WEAPON, "weapons/Revolver/primeforward.wav", VOL_NORM, ATTN_NORM);
			SUB_Remove();
			DartCounter--;
			return;
		}
	}
	if (m_flDieTime < gpGlobals->time)
	{
		SUB_Remove();
		DartCounter--;
		return;
	}
	if (pev->velocity.Length() < 4)
	{
		pev->angles.x = 0; // lay flat
		//DROP_TO_FLOOR(ENT(pev));
	}
	pev->nextthink = gpGlobals->time + 0.2;
};
