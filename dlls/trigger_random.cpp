#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CTargetPrint : public CBaseEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pOther, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(trigger_random, CTargetPrint);

void CTargetPrint::Use(CBaseEntity* pActivator, CBaseEntity* pOther, USE_TYPE useType, float value)
{

}