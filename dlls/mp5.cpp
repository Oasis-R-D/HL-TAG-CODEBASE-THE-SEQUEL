/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
#include "UserMessages.h"

extern int DartCounter; //dart counters are reset in world spawn

LINK_ENTITY_TO_CLASS(weapon_mp5, CReckonMKIII);
LINK_ENTITY_TO_CLASS(weapon_9mmAR, CReckonMKIII);
LINK_ENTITY_TO_CLASS(weapon_reckonmkiii, CReckonMKIII);

//=========================================================
//=========================================================
void CReckonMKIII::Spawn()
{
	pev->classname = MAKE_STRING("weapon_reckonmkiii"); // hack to allow for old names
	Precache();
	SET_MODEL(ENT(pev), "models/w_9mmAR.mdl");
	m_iId = WEAPON_RECKONMKIII;

	m_iDefaultAmmo = gpGlobals->maxClients > 1 ? RECKONMK3_MAX_CLIP : RECKONMKIII_DEFAULT_GIVE;

	FallInit(); // get ready to fall down.
}


void CReckonMKIII::Precache()
{
	PRECACHE_MODEL("models/v_reckon_mkiii.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL("models/shell.mdl"); // brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl"); // grenade

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("weapons/Mk3/Magin.wav");
	PRECACHE_SOUND("weapons/Mk3/Magout.wav");
	PRECACHE_SOUND("weapons/Mk3/Slideclose.wav");
	PRECACHE_SOUND("weapons/Mk3/SlideOpen.wav");
	PRECACHE_SOUND("dart/hit_living.wav");
	PRECACHE_SOUND("weapons/hks1.wav"); // H to the K representing the a b q waddup biaatch leave it at the toone
	PRECACHE_SOUND("weapons/hks2.wav"); // H to the K
	PRECACHE_SOUND("weapons/hks3.wav"); // H to the K

	PRECACHE_SOUND("weapons/glauncher.wav");
	PRECACHE_SOUND("weapons/glauncher2.wav");

	PRECACHE_SOUND("weapons/357_cock1.wav");

	PRECACHE_MODEL("models/bludart.mdl");
	PRECACHE_SOUND("weapons/Revolver/primeforward.wav");

	m_usReckonMKIII = PRECACHE_EVENT(1, "events/mp5.sc");
}

bool CReckonMKIII::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = WEAPON_NOCLIP;
	p->iMaxClip = RECKONMK3_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_RECKONMKIII;
	p->iWeight = RECKONMKIII_WEIGHT;

	return true;
}

bool CReckonMKIII::Deploy()
{
	return DefaultDeploy("models/v_reckon_mkiii.mdl", "models/p_9mmAR.mdl", RECKONMK3_DRAW, "mp5");
}


void CReckonMKIII::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = gpGlobals->v_forward;

#ifndef CLIENT_DLL

		Vector anglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
		UTIL_MakeVectors(anglesAim);

		CDart* pDart = CDart::DartCreate(ALLOC_STRING("9mm"));
		pDart->pev->origin = vecSrc;
		anglesAim.x *= -1; //cuz for some reason pitch angle is inverted in this model
		pDart->pev->angles = anglesAim;
		pDart->pev->owner = m_pPlayer->edict();

		pDart->pev->velocity = vecDir * DART_AIR_VELOCITY;
		pDart->pev->speed = DART_AIR_VELOCITY;
		pDart->pev->avelocity.z = 10;
#endif

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usReckonMKIII, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

	if (0 == m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", false, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.53);

	if (m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.53;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
}



void CReckonMKIII::SecondaryAttack()
{
	//aim down sights maybe?
}

void CReckonMKIII::Reload()
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;

	DefaultReload(RECKONMK3_MAX_CLIP, RECKONMK3_RELOAD, 2.2);
}


void CReckonMKIII::WeaponIdle()
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(RECKONMK3_IDLE);

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}



class CMP5AmmoClip : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		bool bResult = (pOther->GiveAmmo(AMMO_MP5CLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5clip, CMP5AmmoClip);
LINK_ENTITY_TO_CLASS(ammo_9mmAR, CMP5AmmoClip);



class CMP5Chainammo : public CBasePlayerAmmo
{
	void Spawn() override
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache() override
	{
		PRECACHE_MODEL("models/w_chainammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	bool AddAmmo(CBaseEntity* pOther) override
	{
		bool bResult = (pOther->GiveAmmo(AMMO_CHAINBOX_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_9mmbox, CMP5Chainammo);