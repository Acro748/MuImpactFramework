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

	const concurrency::concurrent_vector<ConditionManager::Condition> ConditionManager::GetCondition(const RE::TESHitEvent* evn)
	{
		RE::TESObjectREFR* Aggressor = evn->cause.get();
		RE::TESObjectREFR* Target = evn->target.get();
		logger::trace("Checking Conditions : Aggressor {} {:x} / Target {} {:x}", Aggressor->GetName(), Aggressor->formID, Target->GetName(), Target->formID);
		concurrency::concurrent_vector<Condition> found_condition;
		concurrency::parallel_for_each(ConditionList.begin(), ConditionList.end(), [&](auto& Condition)
		{
			std::uint32_t optionTrueCount = 0;
			for (std::uint8_t option = 0; option < ConditionOption::OptionTotal; option++)
			{
				if (option > optionTrueCount)
					break;

				RE::TESObjectREFR* obj = nullptr;
				RE::Actor* actor = nullptr;
				if (option == ConditionOption::Aggressor)
				{
					obj = Aggressor;
					actor = skyrim_cast<RE::Actor*>(Aggressor);
				}
				else if (option == ConditionOption::Target)
				{
					obj = Target;
					actor = skyrim_cast<RE::Actor*>(Target);
				}
				logger::debug("{} {:x} : Checking Full Conditions {} on {}...", obj->GetName(), obj->formID, Condition.originalCondition[option], magic_enum::enum_name(ConditionOption(option)).data());
				std::uint32_t trueCount = 0;
				for (std::size_t i = 0; i < Condition.AND[option].size(); i++)
				{
					if (i > trueCount)
						break;

					auto& AND = Condition.AND[option].at(i);
					for (auto& OR : AND)
					{
						if (OR.type != ConditionType::IsInanimateObject && isInanimateObject(actor))
							continue;

						bool isTrue = false;
						switch (OR.type) {
						case ConditionType::IsEquippedLeft:
							isTrue = isEquipped(actor, true, OR.pluginName, OR.id);
							break;
						case ConditionType::IsEquippedLeftType:
							isTrue = IsEquippedType(actor, true, OR.arg);
							break;
						case ConditionType::IsEquippedLeftHasKeyword:
							isTrue = isEquippedHasKeyword(actor, true, OR.pluginName, OR.id);
							break;
						case ConditionType::IsEquippedLeftHasKeywordEditorID:
							isTrue = isEquippedHasKeywordEditorID(actor, true, OR.arg);
							break;
						case ConditionType::IsEquippedRight:
							isTrue = isEquipped(actor, false, OR.pluginName, OR.id);
							break;
						case ConditionType::IsEquippedRightType:
							isTrue = IsEquippedType(actor, false, OR.arg);
							break;
						case ConditionType::IsEquippedRightHasKeyword:
							isTrue = isEquippedHasKeyword(actor, false, OR.pluginName, OR.id);
							break;
						case ConditionType::IsEquippedRightHasKeywordEditorID:
							isTrue = isEquippedHasKeywordEditorID(actor, false, OR.arg);
							break;
						case ConditionType::IsEquippedPowerOrShout:
							isTrue = isEquippedPowerOrShout(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsWorn:
							isTrue = isWorn(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsWornHasKeyword:
							isTrue = isWornHasKeyword(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsWornHasKeywordEditorID:
							isTrue = isWornHasKeywordEditorID(actor, OR.arg);
							break;
						case ConditionType::IsInFaction:
							isTrue = isInFaction(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::HasKeyword:
							isTrue = hasKeyword(obj, OR.pluginName, OR.id);
							break;
						case ConditionType::HasKeywordEditorID:
							isTrue = hasKeywordEditorID(obj, OR.arg);
							break;
						case ConditionType::HasMagicEffect:
							isTrue = hasMagicEffect(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::HasPerk:
							isTrue = hasPerk(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::HasSpell:
							isTrue = hasSpell(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsActorBase:
							isTrue = isActorBase(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsActor:
							isTrue = isActor(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsRace:
							isTrue = isRace(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsClass:
							isTrue = isClass(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsCombatStyle:
							isTrue = isCombatStyle(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::IsFemale:
							isTrue = isFemale(actor);
							break;
						case ConditionType::IsChild:
							isTrue = isChild(actor);
							break;
						case ConditionType::IsUnique:
							isTrue = isUnique(actor);
							break;
						case ConditionType::IsSneaking:
							isTrue = isSneaking(actor);
							break;
						case ConditionType::IsSprinting:
							isTrue = isSprinting(actor);
							break;
						case ConditionType::IsInAir:
							isTrue = isInAir(actor);
							break;
						case ConditionType::IsDead:
							isTrue = isDead(actor);
							break;
						case ConditionType::IsLeftAttacking:
							if (option == ConditionOption::Aggressor)
								isTrue = isAttacking(actor, true);
							break;
						case ConditionType::IsRightAttacking:
							if (option == ConditionOption::Aggressor)
								isTrue = isAttacking(actor, false);
							break;
						case ConditionType::IsBlocked:
							if (option == ConditionOption::Target)
								isTrue = isBlocked(evn->flags);
							break;
						case ConditionType::IsCritical:
							if (option == ConditionOption::Target)
								isTrue = isCritical(actor, false);
							break;
						case ConditionType::IsSneakCritical:
							if (option == ConditionOption::Target)
								isTrue = isSneakCritical(actor, false);
							break;
						case ConditionType::IsBash:
							if (option == ConditionOption::Aggressor)
								isTrue = isBash(evn->flags);
							break;
						case ConditionType::IsPowerAttack:
							if (option == ConditionOption::Aggressor)
								isTrue = isPowerAttack(evn->flags);
							break;
						case ConditionType::IsInanimateObject:
							if (option == ConditionOption::Target)
								isTrue = isInanimateObject(actor);
							break;
						case ConditionType::None:
							isTrue = true;
							break;
						}
						Logging(obj, option, OR, isTrue);

						if (isValidCondition(isTrue, OR.NOT))
						{
							trueCount++;
							break;
						}
					}
				}
				if (trueCount == Condition.AND[option].size())
					optionTrueCount++;
			}
			if (optionTrueCount == ConditionOption::OptionTotal)
			{
				logger::debug("Found Condition on Aggressor {} {:x} / Target {} {:x} ", Aggressor->GetName(), Aggressor->formID, Target->GetName(), Target->formID);
				found_condition.push_back(Condition);
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
				ConditionItemOr conditionItem;

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
					Item.type = GetConditionType(strOr, Item.pluginName, Item.id, Item.arg);
					conditionItem.emplace_back(Item);
					logger::debug("{}{}{} ...", firstOR ? "" : "OR ", Item.NOT ? "NOT " : "", magic_enum::enum_name(Item.type).data());
					firstOR = false;
				}
				condition.AND[option].emplace_back(conditionItem);
			}
		}
		return condition;
	}

	const ConditionManager::ConditionType ConditionManager::GetConditionType(std::string line, std::string& pluginName, RE::FormID& id, std::string& arg)
	{
		std::vector<std::string> splittedMain = Config::splitMulti(line, "()");
		if (splittedMain.size() == 0)
			return ConditionType::None;
		std::string low = lowLetter(splittedMain.at(0));
		ConditionType type = ConditionMap[low];

		if (splittedMain.size() > 1)
		{
			Config::ltrim(line, '(');
			Config::rtrim(line, ')');
			std::vector<std::string> splitted = Config::split(line, "|");
			if (splitted.size() == 1)
			{
				if (MultipleConfig::stringStartsWith(splitted.at(0), "0x"))
					id = Config::getHex(splitted.at(0));
				else
				{
					pluginName = splitted.at(0);
					arg = splitted.at(0);
				}
			}
			else if (splitted.size() == 2)
			{
				pluginName = splitted.at(0);
				id = Config::getHex(splitted.at(1));
			}
		}
		return type;
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
			return 200;

		std::string newPath = "Meshes\\" + vfxPath;
		RE::BSResourceNiBinaryStream binaryStream(newPath.c_str());
		if (!binaryStream.good()) {
			logger::error("Failed load to nif file - {}", newPath.c_str());
			return 200;
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
					vfxType = TaskVFX::VFXType::Spell;
					break;
				}
			}
		}

		NiStream_dtor(niStream);

		logger::info("{} => {}", vfxPath, magic_enum::enum_name(TaskVFX::VFXType(vfxType)).data());
		return vfxType;
	}
}