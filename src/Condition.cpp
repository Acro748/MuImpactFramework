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

	const std::vector<ConditionManager::Condition> ConditionManager::GetCondition(RE::Actor* Aggressor, RE::Actor* Target)
	{
		logger::trace("Checking Conditions : Aggressor {} {:x} / Target {} {:x}", Aggressor->GetName(), Aggressor->formID, Target->GetName(), Target->formID);
		std::vector<Condition> found_condition;
		for (auto& Condition : ConditionList)
		{
			std::uint32_t trueCount = 0;
			for (std::uint8_t option = 0; option < ConditionOption::OptionTotal; option++)
			{
				RE::Actor* actor = nullptr;
				if (option == ConditionOption::Aggressor)
					actor = Aggressor;
				else if (option == ConditionOption::Target)
					actor = Target;
				if (!actor)
					continue;
				logger::debug("{} {:x} : Checking Full Conditions {} on {}...", actor->GetName(), actor->formID, Condition.originalCondition[option], magic_enum::enum_name(ConditionOption(option)).data());
				for (auto& AND : Condition.AND[option])
				{
					for (auto& OR : AND)
					{
						bool isTrue = false;
						switch (OR.type) {
						case ConditionType::IsEquippedLeft:
							isTrue = isEquipped(actor, true, OR.pluginName, OR.id);
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
							isTrue = hasKeyword(actor, OR.pluginName, OR.id);
							break;
						case ConditionType::HasKeywordEditorID:
							isTrue = hasKeywordEditorID(actor, OR.arg);
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
						case ConditionType::None:
							isTrue = true;
							break;
						}
						Logging(actor, option, OR, isTrue);

						if (isValidCondition(isTrue, OR.NOT))
						{
							trueCount++;
							break;
						}
					}
				}
			}
			if (trueCount == (Condition.AND[ConditionOption::Aggressor].size() + Condition.AND[ConditionOption::Target].size()))
			{
				logger::debug("Found Condition on Aggressor {} {:x} / Target {} {:x} ", Aggressor->GetName(), Aggressor->formID, Target->GetName(), Target->formID);
				found_condition.emplace_back(Condition);
			}
		}

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
		std::vector<std::string> splittedMain = Config::splitMulti(line, "()[]{}");
		if (splittedMain.size() == 0)
			return ConditionType::None;
		std::string low = lowLetter(splittedMain.at(0));
		ConditionType type = ConditionMap[low];

		if (splittedMain.size() > 1)
		{
			std::vector<std::string> splitted = Config::split(splittedMain.at(1), "|");
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
}