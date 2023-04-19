#pragma once

namespace Mus {
	class ConditionManager {
	public :
		ConditionManager() { InitialConditionMap(); };
		~ConditionManager() {};

		[[nodiscard]] static ConditionManager& GetSingleton() {
			static ConditionManager instance;
			return instance;
		}

		void InitialConditionMap();

		enum ConditionType : std::uint8_t {
			IsEquippedLeft,
			IsEquippedLeftType,
			IsEquippedLeftHasKeyword,
			IsEquippedLeftHasKeywordEditorID,
			IsEquippedRight,
			IsEquippedRightType,
			IsEquippedRightHasKeyword,
			IsEquippedRightHasKeywordEditorID,
			IsEquippedPowerOrShout,
			IsWorn,
			IsWornHasKeyword,
			IsWornHasKeywordEditorID,
			IsInFaction,
			HasKeyword,
			HasKeywordEditorID,
			HasMagicEffect,
			HasPerk,
			HasSpell,
			IsActorBase,
			IsActor,
			IsRace,
			IsClass,
			IsCombatStyle,

			IsFemale,
			IsChild,
			IsPlayerTeammate,
			IsUnique,
			IsSneaking,
			IsSprinting,
			IsSwimming,
			IsInAir,
			IsDead,

			IsLeftAttacking,
			IsRightAttacking,

			None
		};

		enum ConditionOption : std::uint8_t {
			Aggressor,
			Target,

			OptionTotal
		};

		struct ConditionItem {
			bool NOT = false;
			ConditionType type;
			std::string pluginName = "";
			RE::FormID id = 0;
			std::string arg = "";
		};
		typedef std::vector<ConditionItem> ConditionItemOr;

		struct PluginsInfo {
			std::string pluginName;
			RE::FormID id;
		};
		struct Condition {
			std::vector<PluginsInfo> ImpactDataSets;
			std::vector<PluginsInfo> SpellItems;
			std::string originalCondition[ConditionOption::OptionTotal];
			std::vector<ConditionItemOr> AND[ConditionOption::OptionTotal];
		};

		bool RegisterCondition(Condition condition, std::string presetPath);

		const concurrency::concurrent_vector<Condition> GetCondition(RE::Actor* Aggressor, RE::Actor* Target);

	private:
		concurrency::concurrent_vector<Condition> ConditionList;
		std::unordered_map<std::string, ConditionType> ConditionMap;

		const Condition ParseConditions(Condition condition);
		const ConditionType GetConditionType(std::string line, std::string& pluginName, RE::FormID& id, std::string& arg);

		inline bool isValidCondition(bool ConditionResult, bool NOT) {
			return (ConditionResult ^ NOT);
		}

		inline void Logging(RE::Actor* actor, std::uint8_t option, ConditionItem& OR, bool isTrue) {
			std::string typestr = magic_enum::enum_name(ConditionType(OR.type)).data();
			if (IsContainString(typestr, "EditorID"))
			{
				logger::debug("{} {:x} : {} Condition {}{}({}) is {}", actor->GetName(), actor->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, OR.arg,
					isValidCondition(isTrue, OR.NOT) ? "True" : "False");
			}
			else if (OR.type >= ConditionType::IsFemale)
			{
				std::string typestr = magic_enum::enum_name(ConditionType(OR.type)).data();
				logger::debug("{} {:x} : {} Condition {}{}() is {}", actor->GetName(), actor->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, isValidCondition(isTrue, OR.NOT) ? "True" : "False");
			}
			else
			{
				logger::debug("{} {:x} : {} Condition {}{}({}{}{:x}) is {}", actor->GetName(), actor->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, OR.pluginName, OR.pluginName == "" ? "" : "|", OR.id,
					isValidCondition(isTrue, OR.NOT) ? "True" : "False");
			}
		}

		//Conditions
		inline bool isEquipped(RE::Actor* actor, bool LeftHand, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			RE::TESForm* equipped = actor->GetEquippedObject(LeftHand);
			return equipped && form && equipped->formID == form->formID;
		}
		inline bool isEquippedHasKeyword(RE::Actor* actor, bool LeftHand, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::BGSKeyword* keyword = skyrim_cast<RE::BGSKeyword*>(GetFormByID(id, pluginname));
			if (!keyword)
				return false;
			RE::TESForm* equipped = actor->GetEquippedObject(LeftHand);
			if (!equipped)
				return false;
			if (equipped->Is(RE::FormType::Weapon))
			{
				RE::TESObjectWEAP* weapon = skyrim_cast<RE::TESObjectWEAP*>(equipped);
				return weapon && weapon->HasKeyword(keyword);
			}
			else if (equipped->Is(RE::FormType::Spell))
			{
				RE::MagicItem* magic = skyrim_cast<RE::MagicItem*>(equipped);
				return magic && magic->HasKeyword(keyword);
			}
			return false;
		}
		inline bool isEquippedHasKeywordEditorID(RE::Actor* actor, bool LeftHand, std::string editorID) {
			if (!actor)
				return false;
			RE::TESForm* equipped = actor->GetEquippedObject(LeftHand);
			if (!equipped)
				return false;
			if (equipped->Is(RE::FormType::Weapon))
			{
				RE::TESObjectWEAP* weapon = skyrim_cast<RE::TESObjectWEAP*>(equipped);
				return weapon && weapon->HasKeywordString(editorID);
			}
			else if (equipped->Is(RE::FormType::Spell))
			{
				RE::MagicItem* magic = skyrim_cast<RE::MagicItem*>(equipped);
				return magic && magic->HasKeywordString(editorID);
			}
			return false;
		}

		inline bool isEquippedPowerOrShout(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			if (!form)
				return false;
			RE::MagicItem* PowerOrShout = actor->GetActorRuntimeData().selectedSpells[RE::Actor::SlotTypes::kPowerOrShout];
			if (PowerOrShout && PowerOrShout->formID == form->formID)
				return true;
			RE::AIProcess* aiprocess = actor->GetActorRuntimeData().currentProcess;
			if (!aiprocess || !aiprocess->high || !aiprocess->high->currentShout)
				return false;
			RE::TESShout* shout = aiprocess->high->currentShout;
			return shout->formID == form->formID;
		}
		
		inline bool isWorn(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			return form && actor->GetWornArmor(form->formID);
		}
		inline bool isWornHasKeyword(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::BGSKeyword* keyword = skyrim_cast<RE::BGSKeyword*>(GetFormByID(id, pluginname));
			if (!keyword)
				return false;
			RE::TESObjectARMO* armor = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
			return armor && armor->HasKeyword(keyword);
		}
		inline bool isWornHasKeywordEditorID(RE::Actor* actor, std::string editorID) {
			if (!actor)
				return false;
			RE::TESObjectARMO* armor = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
			return armor && armor->HasKeywordString(editorID);
		}

		inline bool isInFaction(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESFaction* faction = skyrim_cast<RE::TESFaction*>(GetFormByID(id, pluginname));
			return faction && actor->IsInFaction(faction);
		}

		inline bool hasKeyword(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::BGSKeyword* keyword = skyrim_cast<RE::BGSKeyword*>(GetFormByID(id, pluginname));
			if (!keyword)
				return false;
			RE::TESRace* race = actor->GetRace();
			return (actor->HasKeyword(keyword) || (race ? race->HasKeyword(keyword) : false));
		}
		inline bool hasKeywordEditorID(RE::Actor* actor, std::string editorID) {
			if (!actor)
				return false;
			return actor->HasKeywordString(editorID);
		}
		
		inline bool hasMagicEffect(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::EffectSetting* effect = skyrim_cast<RE::EffectSetting*>(GetFormByID(id, pluginname));
			if (!effect)
				return false;
			RE::MagicTarget* magictarget = actor->GetMagicTarget();
			return magictarget && magictarget->HasMagicEffect(effect);
		}

		inline bool hasPerk(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::BGSPerk* perk = skyrim_cast<RE::BGSPerk*>(GetFormByID(id, pluginname));
			if (!perk)
				return false;
			return actor->HasPerk(perk);
		}

		inline bool hasSpell(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			if (!form)
				return false;
			RE::TESNPC* npc = actor->GetActorBase();
			if (!npc || !npc->actorEffects)
				return false;
			for (std::uint32_t i = 0; i < npc->actorEffects->numSpells; i++) {
				if (npc->actorEffects->spells[i] && npc->actorEffects->spells[i]->formID == form->formID)
					return true;
			}
			for (std::uint32_t i = 0; i < npc->actorEffects->numShouts; i++) {
				if (npc->actorEffects->shouts[i] && npc->actorEffects->shouts[i]->formID == form->formID)
					return true;
			}
			return false;
		}

		inline bool isActorBase(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			if (!form)
				return false;
			RE::TESNPC* actorBase = actor->GetActorBase();
			return actorBase && actorBase->formID == form->formID;
		}

		inline bool isActor(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			return form && actor->formID == form->formID;
		}

		inline bool isRace(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			if (!form)
				return false;
			RE::TESRace* race = actor->GetRace();
			return race && race->formID == form->formID;
		}

		inline bool isClass(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			if (!form)
				return false;
			RE::TESNPC* npc = actor->GetActorBase();
			return npc && npc->npcClass && form && npc->npcClass->formID == form->formID;
		}
		inline bool isCombatStyle(RE::Actor* actor, std::string pluginname, RE::FormID id) {
			if (!actor)
				return false;
			RE::TESForm* form = GetFormByID(id, pluginname);
			if (!form)
				return false;
			RE::CombatController* controller = actor->GetActorRuntimeData().combatController;
			return controller && controller->combatStyle && controller->combatStyle->formID == form->formID;
		}

		inline bool isFemale(RE::Actor* actor) {
			if (!actor)
				return false;
			RE::TESNPC* actorBase = actor->GetActorBase();
			return actorBase && actorBase->GetSex() == RE::SEX::kFemale;
		}
		inline bool isChild(RE::Actor* actor) {
			if (!actor)
				return false;
			return actor->IsChild();
		}
		inline bool isPlayerTeammate(RE::Actor* actor) {
			if (!actor)
				return false;
			return actor->IsPlayerTeammate();
		}
		inline bool isUnique(RE::Actor* actor) {
			if (!actor)
				return false;
			RE::TESNPC* actorbase = actor->GetActorBase();
			return actorbase && actorbase->IsUnique();
		}
		inline bool isSneaking(RE::Actor* actor) {
			if (!actor)
				return false;
			return actor->IsSneaking();
		}
		inline bool isSprinting(RE::Actor* actor) {
			if (!actor)
				return false;
			RE::ActorState* state = actor->AsActorState();
			return state && state->IsSprinting();
		}
		inline bool isSwimming(RE::Actor* actor) {
			if (!actor)
				return false;
			RE::ActorState* state = actor->AsActorState();
			return state && state->IsSwimming();
		}
		inline bool isInAir(RE::Actor* actor) {
			if (!actor)
				return false;
			return actor->IsInMidair();
		}
		inline bool isDead(RE::Actor* actor) {
			if (!actor)
				return false;
			return actor->IsDead();
		}

		inline bool isAttacking(RE::Actor* actor, bool LeftHand) {
			if (!actor)
				return false;
			RE::AIProcess* aiprocess = actor->GetActorRuntimeData().currentProcess;
			return aiprocess && aiprocess->high && aiprocess->high->attackData && !(aiprocess->high->attackData->IsLeftAttack() ^ LeftHand);
		}

		inline bool isEquel(float base, float value) {
			return (base - value) >= -0.0001f && (base - value) <= 0.0001f;
		}

		inline bool isEquel(RE::NiPoint3 base, RE::NiPoint3 value) {
			return isEquel(base.x, value.x) && isEquel(base.y, value.y) && isEquel(base.z, value.z);
		}
	};
}