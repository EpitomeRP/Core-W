#include "Chat.h"
#include "World.h"
#include "AccountMgr.h"
#include "Language.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "SpellHistory.h"
#include "ScriptMgr.h"
#include <sstream>

class execute_commandscript : public CommandScript
{
public:
	execute_commandscript() : CommandScript("execute_commandscript") { }

	ChatCommand *GetCommands() const override
	{
		static ChatCommand commandTable[] =
		{
			{ "execute", rbac::RBAC_PERM_COMMAND_SAVE, false, &HandleCharacterExecuteCommand, "", NULL },
			{ NULL, 0, false, NULL, "", NULL }
		};
		return commandTable;
	}

	static bool HandleCharacterExecuteCommand(ChatHandler *handler, const char *args)
	{
        Player *target;
        const char *executerName;
        const char *victimName;
 
        executerName = 0;
        victimName = 0;
        if (*args && args)
            target = ObjectAccessor::FindPlayerByName(args);
        else
            target = handler->getSelectedPlayerOrSelf();

		if (target == handler->GetSession()->GetPlayer())
		{
			handler->SendSysMessage("You can't execute yourself!");
			handler->SetSentErrorMessage(true);
			return false;
		}
        else if (target == 0)
        {
            handler->SendSysMessage("Player not found.");
            handler->SetSentErrorMessage(true);
            return false;
        }
        
		if (target)
		{
            victimName = target->GetName().c_str();
            executerName = handler->GetSession()->GetPlayerName().c_str();
			if (target->HasAura(81000))
			{
				switch (sWorld->BanCharacter(victimName, "-1", "execute", executerName))
                {
				    case BAN_SUCCESS:
				    {
                        std::stringstream message;

					    target->KillPlayer();
					    message << "Player " << victimName << " has been executed by " << executerName << ".";
					    sWorld->SendGMText(LANG_GM_ANNOUNCE_COLOR, executerName, message.str().c_str());
					    break;
				    }
				    case BAN_NOTFOUND:
				    {
					    handler->PSendSysMessage(LANG_BAN_NOTFOUND, "character", victimName);
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
			Player *victim = ((Player *)GetTarget());
			Unit *attacker = dmgInfo.GetAttacker();
			SpellHistory *spellhistory = victim->GetSpellHistory();
			int32 remainingHealth = victim->GetHealth() - dmgInfo.GetDamage();
			uint32 allowedHealth = victim->CountPctFromMaxHealth(3);
			if (victim->duel)
			{
				if (victim->duel->opponent == attacker)
					return;
			}

			// If damage kills us
			if (remainingHealth <= 0 && !spellhistory->HasCooldown(SPELL_EXECUTE_CD))
			{
				victim->m_lastKillerGUID = attacker->GetGUID();
				if (((victim->m_lastKillerGUID >> 52) & 0x00000FFF) == 0x00000000)
				{
					// Cast healing spell, completely avoid damage
					absorbAmount = dmgInfo.GetDamage();
					int32 healAmount = int32(victim->CountPctFromMaxHealth(uint32(healPct)));
					victim->CastCustomSpell(victim, SPELL_EXECUTE_HEAL, &healAmount, NULL, NULL, true, NULL, aurEff);
					spellhistory->AddCooldown(SPELL_EXECUTE_HEAL, 0, std::chrono::seconds(120));
					victim->AddAura(SPELL_EXECUTE_STUN, victim);
					spellhistory->AddCooldown(SPELL_EXECUTE_CD, 0, std::chrono::seconds(600));
					spellhistory->AddCooldown(SPELL_EXECUTE_STUN, 0, std::chrono::seconds(600));
					victim->AddAura(SPELL_EXECUTE_CD, victim);
					std::stringstream message;
					message << "Player " << victim->GetName() << " has been stunned.";
					sWorld->SendGMText(LANG_GM_ANNOUNCE_COLOR, victim->GetName().c_str(), message.str().c_str());
				}
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
		if (player->IsInWorld() && !player->HasSpell(81003))
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
