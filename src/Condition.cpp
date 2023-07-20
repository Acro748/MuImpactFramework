#include "Condition.h"

namespace Mus {
	void ConditionManager::InitialConditionMap()
	{
		ConditionMap.clear();
		auto types = magic_enum::enum_entries<ConditionType>();
		for (auto& type : types)
		{
			ConditionMap.emplace(lowLetter(type.second.data()), type.first);
		}
	}

	bool ConditionManager::ConditionCheck(const HitEvent& e, Condition condition)
	{
		for (std::uint8_t option = 0; option < ConditionOption::OptionTotal; option++)
		{
			RE::TESObjectREFR* obj = nullptr;
			RE::Actor* actor = nullptr;
			if (option == ConditionOption::Aggressor)
			{
				obj = e.aggressor;
				actor = e.aggressor;
			}
			else if (option == ConditionOption::Target)
			{
				obj = e.target;
				actor = skyrim_cast<RE::Actor*>(e.target);
			}
			if (!obj)
				return false;
			logger::debug("{} {:x} : Checking Full Conditions {} on {}...", obj->GetName(), obj->formID, condition.originalCondition[option], magic_enum::enum_name(ConditionOption(option)).data());

			if (!std::ranges::all_of(condition.AND[option], [&](auto& AND)
				{
					return std::ranges::any_of(AND, [&](auto& OR)
						{
							bool isTrue = OR.NOT ? !OR.conditionFunction->Condition(obj, actor, e) : OR.conditionFunction->Condition(obj, actor, e);
							if (Config::GetSingleton().GetLogLevel() < 2)
								Logging(obj, option, OR, isTrue);
							return isTrue;
						}
					);
				}
			))
				return false;
		}
		return true;
	}

	const concurrency::concurrent_vector<ConditionManager::Condition> ConditionManager::GetCondition(const HitEvent& e)
	{
		logger::trace("Checking Conditions : Aggressor {} {:x} / Target {} {:x}", 
			e.aggressor ? e.aggressor->GetName() : "", e.aggressor ? e.aggressor->formID : 0, 
			e.target ? e.target->GetName() : "", e.target ? e.target->formID : 0);
		concurrency::concurrent_vector<Condition> found_condition;
		concurrency::parallel_for_each(ConditionList.begin(), ConditionList.end(), [&](auto& condition)
		{
			if (ConditionCheck(e, condition))
			{
				logger::debug("Found Condition on Aggressor {} {:x} / Target {} {:x} ", 
					e.aggressor ? e.aggressor->GetName() : "", e.aggressor ? e.aggressor->formID : 0,
					e.target ? e.target->GetName() : "", e.target ? e.target->formID : 0);
				found_condition.push_back(condition);
			}
		});
		return found_condition;
	}

	bool ConditionManager::RegisterCondition(Condition condition, std::string configPath)
	{
		ConditionList.push_back(ParseConditions(condition));
		return true;
	}

	const ConditionManager::Condition ConditionManager::ParseConditions(Condition condition)
	{
		for (std::uint8_t option = 0; option < ConditionOption::OptionTotal; option++)
		{
			logger::debug("Parse Full Condition \"{}\" on {}", condition.originalCondition[option], magic_enum::enum_name(ConditionOption(option)).data());
			std::vector<std::string> splittedANDs = Config::split(condition.originalCondition[option], "AND");

			bool firstAND = true;
			for (auto& strAnd : splittedANDs)
			{
				if (!firstAND)
					logger::debug("AND ...");
				firstAND = false;
				std::vector<std::string> splittedORs = Config::split(strAnd, "OR");
				ConditionItemOr conditionOr;

				bool firstOR = true;
				for (auto& strOr : splittedORs)
				{
					ConditionItem Item;
					if (MultipleConfig::stringStartsWith(strOr, "NOT"))
					{
						Item.NOT = true;

						strOr.erase(0, 3);

						Config::trim(strOr);
					}

					bool invalid = false;
					if (GetConditionType(strOr, Item) == ConditionType::Error)
						invalid = true;
					else
					{
						if (GetConditionFunction(Item) && Item.conditionFunction->IsValid(option))
						{
							conditionOr.emplace_back(Item);
							logger::debug("{}{}{} ...", firstOR ? "" : "OR ", Item.NOT ? "NOT " : "", magic_enum::enum_name(Item.type).data());
						}
						else
							invalid = false;
					}

					if (invalid)
					{
						logger::error("Found invalid condition! : {}{}{}", firstOR ? "" : "OR ", Item.NOT ? "NOT " : "", strOr);
						logger::error("Cause file : {}", condition.fileName);
					}
					firstOR = false;
				}
				condition.AND[option].emplace_back(conditionOr);
			}
		}
		return condition;
	}

	const ConditionManager::ConditionType ConditionManager::GetConditionType(std::string line, ConditionItem& item)
	{
		std::vector<std::string> splittedMain = Config::splitMulti(line, "()");
		if (splittedMain.size() == 0)
		{
			item.type = ConditionType::None;
			return ConditionType::None;
		}
		std::string low = lowLetter(splittedMain.at(0));
		if (auto found = ConditionMap.find(low); found != ConditionMap.end())
			item.type = found->second;
		else
		{
			item.type = ConditionType::Error;
			return ConditionType::Error;
		}

		if (splittedMain.size() > 1)
		{
			Config::ltrim(line, '(');
			Config::rtrim(line, ')');
			std::vector<std::string> splitted = Config::split(line, "|");
			if (splitted.size() == 1)
			{
				if (MultipleConfig::stringStartsWith(splitted.at(0), "0x"))
					item.id = Config::getHex(splitted.at(0));
				else
				{
					item.pluginName = splitted.at(0);
					item.arg = splitted.at(0);
				}
			}
			else if (splitted.size() == 2)
			{
				item.pluginName = splitted.at(0);
				item.id = Config::getHex(splitted.at(1));
			}
			else if (splitted.size() == 3)
			{
				item.pluginName = splitted.at(0);
				item.id = Config::getHex(splitted.at(1));
				item.arg2 = splitted.at(2);
			}
		}
		return item.type;
	}

	bool ConditionManager::GetConditionFunction(ConditionItem& item)
	{
		bool isLeft = true;
		switch (item.type) {
		case ConditionType::IsEquippedLeft:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquipped>();
			isLeft = true;
			break;
		case ConditionType::IsEquippedLeftType:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedType>();
			isLeft = true;
			break;
		case ConditionType::IsEquippedLeftTypeAlt:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedTypeAlt>();
			isLeft = true;
			break;
		case ConditionType::IsEquippedLeftHasKeyword:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedHasKeyword>();
			isLeft = true;
			break;
		case ConditionType::IsEquippedLeftHasKeywordEditorID:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedHasKeywordEditorID>();
			isLeft = true;
			break;
		case ConditionType::IsEquippedRight:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquipped>();
			isLeft = false;
			break;
		case ConditionType::IsEquippedRightType:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedType>();
			isLeft = false;
			break;
		case ConditionType::IsEquippedRightTypeAlt:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedTypeAlt>();
			isLeft = false;
			break;
		case ConditionType::IsEquippedRightHasKeyword:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedHasKeyword>();
			isLeft = false;
			break;
		case ConditionType::IsEquippedRightHasKeywordEditorID:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedHasKeywordEditorID>();
			isLeft = false;
			break;
		case ConditionType::IsEquippedPowerOrShout:
			item.conditionFunction = std::make_shared<ConditionFragment::IsEquippedPowerOrShout>();
			break;
		case ConditionType::IsWorn:
			item.conditionFunction = std::make_shared<ConditionFragment::IsWorn>();
			break;
		case ConditionType::IsWornSlot:
			item.conditionFunction = std::make_shared<ConditionFragment::IsWornSlot>();
			break;
		case ConditionType::IsWornHasKeyword:
			item.conditionFunction = std::make_shared<ConditionFragment::IsWornHasKeyword>();
			break;
		case ConditionType::IsWornHasKeywordEditorID:
			item.conditionFunction = std::make_shared<ConditionFragment::IsWornHasKeywordEditorID>();
			break;
		case ConditionType::IsInFaction:
			item.conditionFunction = std::make_shared<ConditionFragment::IsInFaction>();
			break;
		case ConditionType::HasKeyword:
			item.conditionFunction = std::make_shared<ConditionFragment::HasKeyword>();
			break;
		case ConditionType::HasKeywordEditorID:
			item.conditionFunction = std::make_shared<ConditionFragment::HasKeywordEditorID>();
			break;
		case ConditionType::HasMagicEffect:
			item.conditionFunction = std::make_shared<ConditionFragment::HasMagicEffect>();
			break;
		case ConditionType::HasPerk:
			item.conditionFunction = std::make_shared<ConditionFragment::HasPerk>();
			break;
		case ConditionType::HasSpell:
			item.conditionFunction = std::make_shared<ConditionFragment::HasSpell>();
			break;
		case ConditionType::IsActorBase:
			item.conditionFunction = std::make_shared<ConditionFragment::IsActorBase>();
			break;
		case ConditionType::IsActor:
			item.conditionFunction = std::make_shared<ConditionFragment::IsActor>();
			break;
		case ConditionType::IsObjectReference:
			item.conditionFunction = std::make_shared<ConditionFragment::IsObjectReference>();
			break;
		case ConditionType::IsRace:
			item.conditionFunction = std::make_shared<ConditionFragment::IsRace>();
			break;
		case ConditionType::IsClass:
			item.conditionFunction = std::make_shared<ConditionFragment::IsClass>();
			break;
		case ConditionType::IsCombatStyle:
			item.conditionFunction = std::make_shared<ConditionFragment::IsCombatStyle>();
			break;
		case ConditionType::IsFemale:
			item.conditionFunction = std::make_shared<ConditionFragment::IsFemale>();
			break;
		case ConditionType::IsChild:
			item.conditionFunction = std::make_shared<ConditionFragment::IsChild>();
			break;
		case ConditionType::IsUnique:
			item.conditionFunction = std::make_shared<ConditionFragment::IsUnique>();
			break;
		case ConditionType::IsSneaking:
			item.conditionFunction = std::make_shared<ConditionFragment::IsSneaking>();
			break;
		case ConditionType::IsSprinting:
			item.conditionFunction = std::make_shared<ConditionFragment::IsSprinting>();
			break;
		case ConditionType::IsInAir:
			item.conditionFunction = std::make_shared<ConditionFragment::IsInAir>();
			break;
		case ConditionType::IsDead:
			item.conditionFunction = std::make_shared<ConditionFragment::IsDead>();
			break;
		case ConditionType::IsLeftAttacking:
			item.conditionFunction = std::make_shared<ConditionFragment::IsAttacking>();
			isLeft = true;
			break;
		case ConditionType::IsRightAttacking:
			item.conditionFunction = std::make_shared<ConditionFragment::IsAttacking>();
			isLeft = false;
			break;
		case ConditionType::IsAttackingType:
			item.conditionFunction = std::make_shared<ConditionFragment::IsAttackingType>();
			break;
		case ConditionType::IsAttackingHasKeyword:
			item.conditionFunction = std::make_shared<ConditionFragment::IsAttackingHasKeyword>();
			break;
		case ConditionType::IsAttackingHasKeywordEditorID:
			item.conditionFunction = std::make_shared<ConditionFragment::IsAttackingHasKeywordEditorID>();
			break;
		case ConditionType::IsBlocked:
			item.conditionFunction = std::make_shared<ConditionFragment::IsBlocked>();
			break;
		case ConditionType::IsCritical:
			item.conditionFunction = std::make_shared<ConditionFragment::IsCritical>();
			break;
		case ConditionType::IsSneakCritical:
			item.conditionFunction = std::make_shared<ConditionFragment::IsSneakCritical>();
			break;
		case ConditionType::IsBash:
			item.conditionFunction = std::make_shared<ConditionFragment::IsBash>();
			break;
		case ConditionType::IsPowerAttack:
			item.conditionFunction = std::make_shared<ConditionFragment::IsPowerAttack>();
			break;
		case ConditionType::IsInanimateObject:
			item.conditionFunction = std::make_shared<ConditionFragment::IsInanimateObject>();
			break;
		case ConditionType::None:
			item.conditionFunction = std::make_shared<ConditionFragment::NoneCondition>();
			break;
		}
		if (!item.conditionFunction)
			return false;
		item.conditionFunction->Initial(item, isLeft);
		return true;
	}

	static RE::NiStream* NiStream_ctor(RE::NiStream* stream) {
		using func_t = decltype(&NiStream_ctor);
		REL::VariantID offset(68971, 70324, 0x00C9EC40);
		REL::Relocation<func_t> func{ offset };
		return func(stream);
	}
	static void NiStream_dtor(RE::NiStream* stream) {
		using func_t = decltype(&NiStream_dtor);
		REL::VariantID offset(68972, 70325, 0x00C9EEA0);
		REL::Relocation<func_t> func{ offset };
		return func(stream);
	}
	std::uint8_t ConditionManager::GetVFXType(std::string vfxPath)
	{
		std::uint8_t vfxType = TaskVFX::VFXType::Impact;
		if (vfxPath.empty())
			return TaskVFX::VFXType::None;

		std::string newPath = "Meshes\\" + vfxPath;
		RE::BSResourceNiBinaryStream binaryStream(newPath.c_str());
		if (!binaryStream.good()) {
			logger::error("Failed load to nif file - {}", newPath.c_str());
			return TaskVFX::VFXType::None;
		}

		std::uint8_t niStreamMemory[sizeof(RE::NiStream)];
		memset(niStreamMemory, 0, sizeof(RE::NiStream));
		RE::NiStream* niStream = (RE::NiStream*)niStreamMemory;
		NiStream_ctor(niStream);
		niStream->Load1(&binaryStream);

		for (auto& obj : niStream->topObjects)
		{
			RE::NiAVObject* node = netimmerse_cast<RE::NiAVObject*>(obj.get());
			if (!node)
				continue;
			auto controller = node->GetControllers();
			if (controller)
			{
				auto manager = controller->AsNiControllerManager();
				if (manager)
				{
					vfxType = TaskVFX::VFXType::HitEffect;
					break;
				}
			}
		}

		NiStream_dtor(niStream);

		logger::info("{} => {}", vfxPath, magic_enum::enum_name(TaskVFX::VFXType(vfxType)).data());
		return vfxType;
	}

	namespace ConditionFragment
	{
		void IsEquipped::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
			isLeft = IsLeft;
		}
		bool IsEquipped::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			RE::TESForm* equipped = actor->GetEquippedObject(isLeft);
			return equipped && equipped->formID == form->formID;
		}

		void IsEquippedType::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			type = RE::WEAPON_TYPE(std::stoul(item.arg));
			isLeft = IsLeft;
		}
		bool IsEquippedType::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			RE::TESForm* equipped = actor->GetEquippedObject(isLeft);
			RE::TESObjectWEAP* weapon = equipped ? skyrim_cast<RE::TESObjectWEAP*>(equipped) : nullptr;
			if (weapon && weapon->weaponData.animationType.any(type))
				return true;
			else if (!weapon && type == RE::WEAPON_TYPE::kHandToHandMelee)
				return true;
			return false;
		}

		void IsEquippedTypeAlt::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			type = EquippedTypeAlt(std::stoul(item.arg));
			isLeft = IsLeft;
		}
		bool IsEquippedTypeAlt::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;

			RE::TESForm* equipped = actor->GetEquippedObject(isLeft);
			RE::TESObjectWEAP* weapon = equipped ? skyrim_cast<RE::TESObjectWEAP*>(equipped) : nullptr;
			RE::WEAPON_TYPE weaponType = weapon ? weapon->weaponData.animationType.get() : RE::WEAPON_TYPE::kHandToHandMelee;

			switch (type) {
			case EquippedTypeAlt::kHandToHandMelee:
			case EquippedTypeAlt::kOneHandSword:
			case EquippedTypeAlt::kOneHandDagger:
			case EquippedTypeAlt::kOneHandAxe:
			case EquippedTypeAlt::kOneHandMace:
			case EquippedTypeAlt::kTwoHandSword:
				return std::to_underlying(weaponType) == type;
				break;

			case EquippedTypeAlt::kTwoHandAxe:
				return weaponType == RE::WEAPON_TYPE::kTwoHandAxe && weapon ? weapon->HasKeywordString("WeapTypeBattleaxe") : false;
				break;
			case EquippedTypeAlt::kTwoHandWarHammer:
				return weaponType == RE::WEAPON_TYPE::kTwoHandAxe && weapon ? weapon->HasKeywordString("WeapTypeWarhammer") : false;
				break;

			case EquippedTypeAlt::kBow:
			case EquippedTypeAlt::kStaff:
			case EquippedTypeAlt::kCrossbow:
				return std::to_underlying(e.weaponType) == (type - 1);
				break;

			case EquippedTypeAlt::kSpell:
				RE::SpellItem* spell = equipped ? skyrim_cast<RE::SpellItem*>(equipped) : nullptr;
				return spell ? true : false;
				break;
			}
			return false;
		}

		void IsEquippedHasKeyword::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keyword = GetFormByID<RE::BGSKeyword*>(item.id, item.pluginName);
			isLeft = IsLeft;
		}
		bool IsEquippedHasKeyword::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !keyword)
				return false;
			RE::TESForm* equipped = actor->GetEquippedObject(isLeft);
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

		void IsEquippedHasKeywordEditorID::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keywordEditorID = item.arg;
			isLeft = IsLeft;
		}
		bool IsEquippedHasKeywordEditorID::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			RE::TESForm* equipped = actor->GetEquippedObject(isLeft);
			if (!equipped)
				return false;
			if (equipped->Is(RE::FormType::Weapon))
			{
				RE::TESObjectWEAP* weapon = skyrim_cast<RE::TESObjectWEAP*>(equipped);
				return weapon && weapon->HasKeywordString(keywordEditorID);
			}
			else if (equipped->Is(RE::FormType::Spell))
			{
				RE::MagicItem* magic = skyrim_cast<RE::MagicItem*>(equipped);
				return magic && magic->HasKeywordString(keywordEditorID);
			}
			return false;
		}

		void IsEquippedPowerOrShout::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsEquippedPowerOrShout::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			RE::MagicItem* PowerOrShout = actor->GetActorRuntimeData().selectedSpells[RE::Actor::SlotTypes::kPowerOrShout];
			if (PowerOrShout && PowerOrShout->formID == form->formID)
				return true;
			RE::AIProcess* aiprocess = actor->GetActorRuntimeData().currentProcess;
			if (aiprocess && aiprocess->high && aiprocess->high->currentShout)
				return aiprocess->high->currentShout->formID == form->formID;
			return false;
		}

		void IsWorn::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsWorn::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			return form && actor->GetWornArmor(form->formID);
		}

		void IsWornSlot::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			slot = std::stoul(item.arg) - 30;
		}
		bool IsWornSlot::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			auto wornArmor = actor->GetWornArmor(RE::BIPED_MODEL::BipedObjectSlot(1 << slot));
			if (slot == 2)
			{
				return wornArmor ? wornArmor == actor->GetSkin() : false;
			}
			return wornArmor ? true : false;
		}

		void IsWornHasKeyword::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keyword = GetFormByID<RE::BGSKeyword*>(item.id, item.pluginName);
			if (!item.arg2.empty())
				slot = std::stoul(item.arg2) - 30;
		}
		bool IsWornHasKeyword::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !keyword)
				return false;
			RE::TESObjectARMO* armor = nullptr;
			if (slot == 255)
				armor = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
			else
				armor = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot(1 << slot));
			return armor && armor->HasKeyword(keyword);
		}

		void IsWornHasKeywordEditorID::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keywordEditorID = item.arg;
			if (!item.arg2.empty())
				slot = std::stoul(item.arg2) - 30;
		}
		bool IsWornHasKeywordEditorID::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			RE::TESObjectARMO* armor = nullptr;
			if (slot == 255)
				armor = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot::kBody);
			else
				armor = actor->GetWornArmor(RE::BGSBipedObjectForm::BipedObjectSlot(1 << slot));
			return armor && armor->HasKeywordString(keywordEditorID);
		}

		void IsInFaction::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			faction = GetFormByID<RE::TESFaction*>(item.id, item.pluginName);
		}
		bool IsInFaction::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !faction)
				return false;
			return actor->IsInFaction(faction);
		}

		void HasKeyword::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keyword = GetFormByID<RE::BGSKeyword*>(item.id, item.pluginName);
		}
		bool HasKeyword::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!keyword)
				return false;
			if (actor)
			{
				RE::TESRace* race = actor->GetRace();
				return (actor->HasKeyword(keyword) || (race ? race->HasKeyword(keyword) : false));
			}
			return ref ? ref->HasKeyword(keyword) : false;
		}

		void HasKeywordEditorID::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keywordEditorID = item.arg;
		}
		bool HasKeywordEditorID::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (actor)
				return actor->HasKeywordString(keywordEditorID.c_str());
			RE::BGSKeyword* keyword = skyrim_cast<RE::BGSKeyword*>(RE::TESForm::LookupByEditorID(keywordEditorID.c_str()));
			return (keyword && ref) ? ref->HasKeyword(keyword) : false;
		}

		void HasMagicEffect::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			effect = GetFormByID<RE::EffectSetting*>(item.id, item.pluginName);
		}
		bool HasMagicEffect::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !effect)
				return false;
			RE::MagicTarget* magictarget = actor->GetMagicTarget();
			return magictarget && magictarget->HasMagicEffect(effect);
		}

		void HasPerk::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			perk = GetFormByID<RE::BGSPerk*>(item.id, item.pluginName);
		}
		bool HasPerk::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !perk)
				return false;
			return actor->HasPerk(perk);
		}

		void HasSpell::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool HasSpell::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
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

		void IsActorBase::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsActorBase::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			RE::TESNPC* actorBase = actor->GetActorBase();
			return actorBase && actorBase->formID == form->formID;
		}

		void IsActor::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsActor::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			return actor->formID == form->formID;
		}

		void IsObjectReference::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsObjectReference::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (actor || !ref || !form)
				return false;
			return ref->formID == form->formID;
		}

		void IsRace::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsRace::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			RE::TESRace* race = actor->GetRace();
			return race && race->formID == form->formID;
		}

		void IsClass::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsClass::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			RE::TESNPC* npc = actor->GetActorBase();
			return npc && npc->npcClass && npc->npcClass->formID == form->formID;
		}

		void IsCombatStyle::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			form = GetFormByID(item.id, item.pluginName);
		}
		bool IsCombatStyle::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !form)
				return false;
			RE::CombatController* controller = actor->GetActorRuntimeData().combatController;
			return controller && controller->combatStyle && controller->combatStyle->formID == form->formID;
		}

		void IsFemale::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsFemale::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			RE::TESNPC* actorBase = actor->GetActorBase();
			return actorBase && actorBase->GetSex() == RE::SEX::kFemale;
		}

		void IsChild::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsChild::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			return actor->IsChild();
		}

		void IsPlayerTeammate::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsPlayerTeammate::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			return actor->IsPlayerTeammate();
		}

		void IsUnique::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsUnique::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			RE::TESNPC* actorbase = actor->GetActorBase();
			return actorbase && actorbase->IsUnique();
		}

		void IsSneaking::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsSneaking::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			return actor->IsSneaking();
		}

		void IsSprinting::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsSprinting::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			RE::ActorState* state = actor->AsActorState();
			return state && state->IsSprinting();
		}

		void IsSwimming::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsSwimming::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			RE::ActorState* state = actor->AsActorState();
			return state && state->IsSwimming();
		}

		void IsInAir::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsInAir::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			return actor->IsInMidair();
		}

		void IsDead::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsDead::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			return actor->IsDead();
		}

		void IsAttacking::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			isLeft = IsLeft;
		}
		bool IsAttacking::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Aggressor;
		}
		bool IsAttacking::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			return isLeft ? e.attackHand.any(HitEvent::AttackHand::Left) : e.attackHand.any(HitEvent::AttackHand::Right);
		}

		void IsAttackingType::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			type = AttackingType(std::stoul(item.arg));
		}
		bool IsAttackingType::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Aggressor;
		}
		bool IsAttackingType::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			switch (type) {
			case AttackingType::kHandToHandMelee:
			case AttackingType::kOneHandSword:
			case AttackingType::kOneHandDagger:
			case AttackingType::kOneHandAxe:
			case AttackingType::kOneHandMace:
			case AttackingType::kTwoHandSword:
				return std::to_underlying(e.weaponType) == type;
				break;

			case AttackingType::kTwoHandAxe:
				return e.weaponType == RE::WEAPON_TYPE::kTwoHandAxe && e.weapon->HasKeywordString("WeapTypeBattleaxe");
				break;
			case AttackingType::kTwoHandWarHammer:
				return e.weaponType == RE::WEAPON_TYPE::kTwoHandAxe && e.weapon->HasKeywordString("WeapTypeWarhammer");
				break;

			case AttackingType::kBow:
			case AttackingType::kStaff:
			case AttackingType::kCrossbow:
				return std::to_underlying(e.weaponType) == (type - 1);
				break;

			case AttackingType::kSpell:
				return e.attackType == HitEvent::AttackType::Spell;
				break;
			case AttackingType::kShout:
				return e.attackType == HitEvent::AttackType::Shout;
				break;
			}
			return false;
		}

		void IsAttackingHasKeyword::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keyword = GetFormByID<RE::BGSKeyword*>(item.id, item.pluginName);
		}
		bool IsAttackingHasKeyword::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Aggressor;
		}
		bool IsAttackingHasKeyword::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor || !keyword)
				return false;
			if (e.weapon)
				return e.weapon->HasKeyword(keyword);
			else if (e.spell)
				return e.spell->HasKeyword(keyword);
			else if (e.magicItem)
				return e.magicItem->HasKeyword(keyword);
			return false;
		}

		void IsAttackingHasKeywordEditorID::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
			keywordEditorID = item.arg;
		}
		bool IsAttackingHasKeywordEditorID::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Aggressor;
		}
		bool IsAttackingHasKeywordEditorID::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			if (!actor)
				return false;
			if (e.weapon)
				return e.weapon->HasKeywordString(keywordEditorID);
			else if (e.magicItem)
				return e.magicItem->HasKeywordString(keywordEditorID);
			return false;
		}

		void IsBlocked::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsBlocked::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Target;
		}
		bool IsBlocked::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			return e.flags.any(RE::HitData::Flag::kBlocked, RE::HitData::Flag::kBlockWithWeapon);
		}

		void IsCritical::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsCritical::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Target;
		}
		bool IsCritical::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			return e.flags.any(RE::HitData::Flag::kCritical);
		}

		void IsSneakCritical::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsSneakCritical::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Target;
		}
		bool IsSneakCritical::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			return e.flags.any(RE::HitData::Flag::kSneakAttack);
		}

		void IsBash::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsBash::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Aggressor;
		}
		bool IsBash::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			return e.flags.any(RE::HitData::Flag::kBash, RE::HitData::Flag::kTimedBash);
		}

		void IsPowerAttack::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsPowerAttack::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Aggressor;
		}
		bool IsPowerAttack::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			return e.flags.any(RE::HitData::Flag::kPowerAttack);
		}

		void IsInanimateObject::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool IsInanimateObject::IsValid(std::uint8_t option)
		{
			return option == ConditionManager::ConditionOption::Target;
		}
		bool IsInanimateObject::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			return actor ? false : true;
		}

		void NoneCondition::Initial(ConditionManager::ConditionItem& item, bool IsLeft)
		{
		}
		bool NoneCondition::Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e)
		{
			return true;
		}
	}
}
