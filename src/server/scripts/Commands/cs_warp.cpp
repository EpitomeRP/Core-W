/* ScriptData
Name: warp_commandscript
%Complete: 100
Comment: Warp command implementation
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "ObjectMgr.h"
#include "MapManager.h"
#include "TicketMgr.h"
#include "Chat.h"
#include "Language.h"
#include "Player.h"
#include "Transport.h"

class warp_commandscript : public CommandScript
{
public:
	warp_commandscript() : CommandScript("warp_commandscript") { }

	ChatCommand *GetCommands() const override
	{
		static ChatCommand commandTable[] =
		{
			{ "warp", rbac::RBAC_PERM_COMMAND_WARP, 0, &HandleWarp, "Usage: warp [x y z] [facing]", 0 },
			{ 0, 0, 0, 0, "", 0 }
		};
		return commandTable;
	}

	// Ports the player at the given coordinates, on the same world
	static bool HandleWarp(ChatHandler *handler, const char *args)
	{
		Player *plyr;
		float x;
		float y;
		float z;
		float facing;
		char *token[4];
		uint32 worldID;

		if (!*args)
			return false;

		plyr = handler->GetSession()->GetPlayer();
		worldID = 0;
		token[worldID] = strtok((char*)args, " ");
		do 
		{
			worldID++;
			token[worldID] = strtok(0, " ");
		} while (worldID < 3);

		if (!token[0] || !token[1] || !token[2])
		{
			handler->PSendSysMessage("Usage: warp [x y z] [facing]");
			handler->SetSentErrorMessage(true);
			return false;
		}

		x = (float)atof(token[0]);
		y = (float)atof(token[1]);
		z = (float)atof(token[2]);

		if (!token[3])
			facing = plyr->GetOrientation();
		else
		{
			facing = (float)atof(token[3]);
			if (!facing)
				facing = plyr->GetOrientation();
		}

		worldID = plyr->GetMapId();
		x += plyr->GetPositionX();
		y += plyr->GetPositionY();
		z += plyr->GetPositionZ();

		if (!MapManager::IsValidMapCoord(worldID, x, y, z))
		{
			handler->PSendSysMessage(LANG_INVALID_TARGET_COORD, x, y, worldID);
			handler->SetSentErrorMessage(true);
			return false;
		}

		// Cancel spline movements if needed, or save the PC position for recall
		if (plyr->IsInFlight())
		{
			plyr->GetMotionMaster()->MovementExpired();
			plyr->CleanupAfterTaxiFlight();
		}
		else
			plyr->SaveRecallPosition();

		plyr->TeleportTo(worldID, x, y, z, facing);
		return true;
	}
};

void AddSC_warp_commandscript()
{
	new warp_commandscript();
}
