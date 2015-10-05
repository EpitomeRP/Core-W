#include "ScriptPCH.h"
#include "Chat.h"
#include "World.h"
#include "AccountMgr.h"
#include "Language.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ScriptMgr.h"
#include <sstream>

class execute_commandscript : public CommandScript
{
public:
	execute_commandscript() : CommandScript("execute_commandscript") { }

	ChatCommand* GetCommands() const override
	{
		static ChatCommand commandTable[] =
		{
			{ "execute", rbac::RBAC_PERM_COMMAND_SAVE, false, &HandleCharacterExecuteCommand, "", NULL },
			{ NULL, 0, false, NULL, "", NULL }
		};
		return commandTable;
	}

	static bool HandleCharacterExecuteCommand(ChatHandler* handler, char const* args)
	{
		Player* target = handler->getSelectedPlayerOrSelf();

		if (target == handler->GetSession()->GetPlayer())
		{
			handler->SendSysMessage(LANG_PLAYER_NOT_FOUND);
			handler->SetSentErrorMessage(true);
			return false;
		}

		std::string author = handler->GetSession() ? handler->GetSession()->GetPlayerName() : "Server";
		std::string name1 = "";
		char* durationStr = "-1";
		char* reasonStr = "executed";

		std::string name = target->GetSession()->GetPlayerName();
		if (target)
		{
			if (target->HasAura(81000))
			{
				switch (sWorld->BanCharacter(name, durationStr, reasonStr, author))
				{
				case BAN_SUCCESS:
				{
					handler->PSendSysMessage(LANG_BAN_YOUPERMBANNED, name.c_str(), reasonStr);
					std::stringstream message;
					if (WorldSession* session = handler->GetSession())
						name1 = session->GetPlayer()->GetName();
					message << "Player " << name << " has been executed by " << name1;
					sWorld->SendGMText(LANG_GM_ANNOUNCE_COLOR, name1.c_str(), message.str().c_str());
					break;
				}
				case BAN_NOTFOUND:
				{
					handler->PSendSysMessage(LANG_BAN_NOTFOUND, "character", name.c_str());
					handler->SetSentErrorMessage(true);
					return false;
				}
				default:
					break;
				}
			}
		}
		return true;
	}
};

// Spell script for 81003
// INSERT INTO spell_script_names VALUES (81003, "spell_execute_stun");
class spell_execute_stun : public SpellScriptLoader
{
public:
	spell_execute_stun() : SpellScriptLoader("spell_execute_stun") { }

	class spell_execute_stun_AuraScript : public AuraScript
	{
		PrepareAuraScript(spell_execute_stun_AuraScript);

	public:
		spell_execute_stun_AuraScript()
		{
			absorbPct = 0;
			healPct = 0;
		}

	private:
		uint32 absorbPct, healPct;

		enum Spell
		{
			SPELL_EXECUTE_HEAL = 81002,
			SPELL_EXECUTE_CD = 81001,
			SPELL_EXECUTE_STUN = 81000
		};

		bool Load() override
		{
			healPct = GetSpellInfo()->Effects[EFFECT_1].CalcValue();
			absorbPct = GetSpellInfo()->Effects[EFFECT_0].CalcValue();
			return GetUnitOwner()->GetTypeId() == TYPEID_PLAYER;
		}

		void CalculateAmount(AuraEffect const* /*aurEff*/, int32 & amount, bool & /*canBeRecalculated*/)
		{
			// Set absorbtion amount to unlimited
			amount = -1;
		}

		void Absorb(AuraEffect* aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
		{
			Unit* victim = GetTarget();
			int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();
			uint32 allowedHealth = victim->CountPctFromMaxHealth(3);
			if (victim->ToPlayer()->duel != NULL)
				return;

			// If damage kills us
			if (remainingHealth <= 0 && !victim->ToPlayer()->HasSpellCooldown(SPELL_EXECUTE_CD))
			{
				// Cast healing spell, completely avoid damage
				absorbAmount = dmgInfo.GetDamage();
				int32 healAmount = int32(victim->CountPctFromMaxHealth(uint32(healPct)));
				victim->CastCustomSpell(victim, SPELL_EXECUTE_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
				victim->ToPlayer()->AddSpellCooldown(SPELL_EXECUTE_HEAL, 0, time(NULL) + 120);
				victim->AddAura(SPELL_EXECUTE_STUN, victim);
				victim->ToPlayer()->AddSpellCooldown(SPELL_EXECUTE_CD, 0, time(NULL) + 600);
				victim->ToPlayer()->AddSpellCooldown(SPELL_EXECUTE_STUN, 0, time(NULL) + 600);
				victim->AddAura(SPELL_EXECUTE_CD, victim);
				std::stringstream message;
				message << "Player " << victim->ToPlayer()->GetName() << " has been stunned.";
				sWorld->SendGMText(LANG_GM_ANNOUNCE_COLOR, victim->GetName().c_str(), message.str().c_str());
			}
		}

		void Register() override
		{
			DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_execute_stun_AuraScript::CalculateAmount, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB);
			OnEffectAbsorb += AuraEffectAbsorbFn(spell_execute_stun_AuraScript::Absorb, EFFECT_0);
		}
	};

	AuraScript* GetAuraScript() const override
	{
		return new spell_execute_stun_AuraScript();
	}
};

class player_on_login_execute : public PlayerScript
{
public:
	player_on_login_execute() : PlayerScript("player_on_login_execute") {}

	void OnLogin(Player* player, bool firstLogin) override
	{
		if (player->IsInWorld())
		{
			player->LearnSpell(81003, false); // teach him the proper aura
		}
	}
};

void AddSC_execute_custom_spell_scripts()
{
	new spell_execute_stun();
	new execute_commandscript();
	new player_on_login_execute();
}