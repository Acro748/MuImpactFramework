#include "hook.h"
#include <xbyak/xbyak.h>

namespace Mus {
	constexpr auto ProjectileHitFunction = REL::VariantID(43022, 44213, 0x0077E4E0);

	EventDispatcherImpl<HitEvent> g_HitEventDispatcher;
	constexpr RE::FormID EquipType_BothHands = 0x13F45;

	float GetMagicEffectInfo(RE::MagicItem* magicItem, RE::ActorValueOwner* targetValue, HitEvent& e) {
		if (!magicItem)
			return 0.0f;

		float damage = 0.0f;
		for (auto effect : magicItem->effects)
		{
			if (!effect || !effect->baseEffect)
				continue;
			if (!effect->baseEffect->data.flags.any(RE::EffectSetting::EffectSettingData::Flag::kDetrimental)) //is damage?
				continue;
			if (effect->baseEffect->data.archetype == RE::EffectArchetypes::ArchetypeID::kValueModifier
				|| effect->baseEffect->data.archetype == RE::EffectArchetypes::ArchetypeID::kDualValueModifier)
			{
				if (effect->baseEffect->data.primaryAV == RE::ActorValue::kHealth)
				{
					e.damageType |= HitEvent::DamageType::Health;
					damage += (effect->effectItem.magnitude);
				}
				else if (effect->baseEffect->data.primaryAV == RE::ActorValue::kMagicka)
					e.damageType |= HitEvent::DamageType::Magicka;
				else if (effect->baseEffect->data.primaryAV == RE::ActorValue::kStamina)
					e.damageType |= HitEvent::DamageType::Stamina;

				if (effect->baseEffect->data.secondaryAV == RE::ActorValue::kHealth)
				{
					e.damageType |= HitEvent::DamageType::Health;
					damage += (effect->effectItem.magnitude);
				}
				else if (effect->baseEffect->data.secondaryAV == RE::ActorValue::kMagicka)
					e.damageType |= HitEvent::DamageType::Magicka;
				else if (effect->baseEffect->data.secondaryAV == RE::ActorValue::kStamina)
					e.damageType |= HitEvent::DamageType::Stamina;
			}

			if (targetValue)
			{
				effect->baseEffect->ForEachKeyword([&](const RE::BGSKeyword& a_keyword) {
					if (IsContainString(a_keyword.formEditorID.c_str(), "fire"))
					{
						damage *= 1.0f - (targetValue->GetActorValue(RE::ActorValue::kResistFire) * 0.01f);
						e.elementalType |= HitEvent::ElementalType::Fire;
						return RE::BSContainer::ForEachResult::kStop;
					}
					else if (IsContainString(a_keyword.formEditorID.c_str(), "frost"))
					{
						damage *= 1.0f - (targetValue->GetActorValue(RE::ActorValue::kResistFrost) * 0.01f);
						e.elementalType |= HitEvent::ElementalType::Frost;
						return RE::BSContainer::ForEachResult::kStop;
					}
					else if (IsContainString(a_keyword.formEditorID.c_str(), "shock"))
					{
						damage *= 1.0f - (targetValue->GetActorValue(RE::ActorValue::kResistShock) * 0.01f);
						e.elementalType |= HitEvent::ElementalType::Shock;
						return RE::BSContainer::ForEachResult::kStop;
					}
					return RE::BSContainer::ForEachResult::kContinue;
				});
			}
		}

		if (auto aggressorActorValues = e.aggressor->AsActorValueOwner(); aggressorActorValues)
		{
			float spellDamageMult = 1.0f;
			if (e.attackType != HitEvent::AttackType::Shout)
				spellDamageMult *= aggressorActorValues->GetActorValue(RE::ActorValue::kDestructionPowerModifier) * 0.01f + 1;
			spellDamageMult *= targetValue ? 1.0f - (targetValue->GetActorValue(RE::ActorValue::kResistMagic) * 0.01f) : 1.0f;
			damage *= spellDamageMult;
		}
		return damage;
	}

	class ProjectileHit : public RE::FlameProjectile
	{
		MEMBER_FN_PREFIX(ProjectileHit);

	public:
		DEFINE_MEMBER_FN_HOOK(onProjectileHit, RE::Actor*, ProjectileHitFunction.address(), RE::TESObjectREFR* target, RE::NiPoint3& position, RE::NiPoint3& direction, std::uint32_t materialID, std::uint8_t flags);

		RE::Actor* onProjectileHit(RE::TESObjectREFR* target, RE::NiPoint3& position, RE::NiPoint3& direction, std::uint32_t materialID, std::uint8_t flags)
		{
			//RE::Actor* actorTarget = CALL_MEMBER_FN(this, onProjectileHit)(target, position, direction, materialID, flags);
			TimeLogger(false, Config::GetSingleton().GetEnableTimeCounter());
			RE::Actor* actorTarget = skyrim_cast<RE::Actor*>(target);
			HitEvent e;
			e.eventType = HitEvent::EventType::Projectile;
			e.aggressor = skyrim_cast<RE::Actor*>(this->GetProjectileRuntimeData().shooter.get().get());
			if (!e.aggressor)
				return actorTarget;
			e.target = target;
			e.hitPosition = position;
			e.hitDirection = direction;
			e.materialID = RE::MATERIAL_ID(materialID);

			RevertFunction(e);
			if (!actorTarget && !Config::GetSingleton().GetEnableInanimateObject())
				return actorTarget;

			e.projectile = this;
			if (auto aggressorProcess = e.aggressor->GetActorRuntimeData().currentProcess;
				aggressorProcess && aggressorProcess->high && aggressorProcess->high->currentShout)
			{
				e.shout = aggressorProcess->high->currentShout;
				e.shoutNum = aggressorProcess->high->currentShoutVariation;
				e.attackType = HitEvent::AttackType::Shout;
			}
			else if (this->GetProjectileRuntimeData().spell)
			{
				e.spell = skyrim_cast<RE::SpellItem*>(this->GetProjectileRuntimeData().spell);
				RE::TESObjectWEAP* weapon = nullptr;
				if (e.spell && e.spell->equipSlot && e.spell->equipSlot->formID == EquipType_BothHands)
				{
					e.attackHand = HitEvent::AttackHand::Both;
				}
				else if (this->GetProjectileRuntimeData().castingSource == RE::MagicSystem::CastingSource::kLeftHand)
				{
					e.attackHand = HitEvent::AttackHand::Left;
					weapon = skyrim_cast<RE::TESObjectWEAP*>(e.aggressor->GetEquippedObject(true));
				}
				else if (this->GetProjectileRuntimeData().castingSource == RE::MagicSystem::CastingSource::kRightHand)
				{
					e.attackHand = HitEvent::AttackHand::Right;
					weapon = skyrim_cast<RE::TESObjectWEAP*>(e.aggressor->GetEquippedObject(false));
				}
			
				if (weapon) //staff
				{
					e.weapon = weapon;
					e.weaponType = weapon->GetWeaponType();
					e.attackType = HitEvent::AttackType::Weapon;
				}
				else
					e.attackType = HitEvent::AttackType::Spell;
			}
			else if (this->GetProjectileRuntimeData().weaponSource)
			{
				if (actorTarget)
				{
					return actorTarget; //use TESHitEvent instead of this

					if (auto targetProcess = actorTarget->GetActorRuntimeData().currentProcess;
						targetProcess && targetProcess->middleHigh && targetProcess->middleHigh->lastHitData)
					{
						e.damage = targetProcess->middleHigh->lastHitData->totalDamage;
						e.flags = targetProcess->middleHigh->lastHitData->flags;
					}
				}
				e.weapon = this->GetProjectileRuntimeData().weaponSource;
				e.weaponType = e.weapon->GetWeaponType();
				if (e.weapon->equipSlot && e.weapon->equipSlot->formID == EquipType_BothHands)
					e.attackHand = HitEvent::AttackHand::Both; //Is it only bow and crossbow that passes through this?
				else
					e.attackHand = HitEvent::AttackHand::Right; //maybe nothing will get this
				e.attackType = HitEvent::AttackType::Weapon;

				g_HitEventDispatcher.dispatch(e);
				return actorTarget; //target is not actor, so skip the damage calculate
			}

			//damage calculate
			if (actorTarget)//no inanimated object
			{
				if (auto magicEffect = this->GetProjectileRuntimeData().spell; magicEffect)
				{
					auto targetActorValues = actorTarget->AsActorValueOwner();
					e.damage += GetMagicEffectInfo(magicEffect, targetActorValues, e);
				}
			}
			g_HitEventDispatcher.dispatch(e);
			return actorTarget;
		}

	private:
		void RevertFunction(const HitEvent& e)
		{
			if (auto weapon = this->GetProjectileRuntimeData().weaponSource; weapon && weapon->impactDataSet)
			{
				ImpactManager::GetSingleton().Register(weapon->impactDataSet);
			}
			if (auto magicEffect = this->GetProjectileRuntimeData().spell; magicEffect)
			{
				for (const auto& effect : magicEffect->effects)
				{
					if (!effect || !effect->baseEffect)
						continue;
					if (effect->baseEffect->data.effectShader)
						ImpactManager::GetSingleton().Register(effect->baseEffect->data.effectShader);
					if (effect->baseEffect->data.hitEffectArt)
						ImpactManager::GetSingleton().Register(effect->baseEffect->data.hitEffectArt);
					if (effect->baseEffect->data.impactDataSet)
						ImpactManager::GetSingleton().Register(effect->baseEffect->data.impactDataSet);
				}
			}
			ImpactManager::GetSingleton().LoadImpactEffects(e);
		}
	};

	struct TESHitEvent : public RE::BSTEventSink<RE::TESHitEvent>
	{
	public:
		using EventResult = RE::BSEventNotifyControl;
		EventResult ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>*) override {
			if (!evn || !evn->cause || !evn->target)
				return EventResult::kContinue;

			TimeLogger(false, Config::GetSingleton().GetEnableTimeCounter());

			RE::Actor* aggressor = skyrim_cast<RE::Actor*>(evn->cause.get());
			RE::Actor* target = skyrim_cast<RE::Actor*>(evn->target.get());
			if (!aggressor || !target)
				return EventResult::kContinue;
			auto target_aiprocess = target ? target->GetActorRuntimeData().currentProcess : nullptr;
			if (!target_aiprocess || !target_aiprocess->middleHigh || !target_aiprocess->middleHigh->lastHitData)
				return EventResult::kContinue;
			RE::HitData* hitData = target_aiprocess->middleHigh->lastHitData;
			if (IsEqual(hitData->hitPosition, emptyPoint))
				return EventResult::kContinue;

			HitEvent e;
			e.eventType = HitEvent::EventType::HitData;

			e.aggressor = aggressor;
			e.target = target;
			e.hitPosition = hitData->hitPosition;
			e.hitDirection = hitData->hitDirection;

			e.weapon = hitData->weapon;
			e.flags = hitData->flags;
			e.damage = hitData->totalDamage;
			if (e.weapon)
			{
				e.weaponType = e.weapon->GetWeaponType();
				if (auto enchantment = e.weapon->formEnchanting; enchantment)
				{
					e.magicItem = enchantment;
					auto targetActorValues = target->AsActorValueOwner();
					e.damage += GetMagicEffectInfo(enchantment, targetActorValues, e);
				}
			}

			if (e.weapon && e.weaponType < RE::WEAPON_TYPE::kBow)
			{
				if (e.weapon->equipSlot && e.weapon->equipSlot->formID == EquipType_BothHands)
					e.attackHand = HitEvent::AttackHand::Both;
				else
				{
					if (auto equipped = aggressor->GetEquippedObject(true); equipped && equipped->formID == e.weapon->formID)
						e.attackHand |= HitEvent::AttackHand::Left;
					if (auto equipped = aggressor->GetEquippedObject(false); equipped && equipped->formID == e.weapon->formID)
						e.attackHand |= HitEvent::AttackHand::Right;

					if (e.attackHand == HitEvent::AttackHand::Both)
						e.attackHand =
						hitData->attackData ? (hitData->attackData->IsLeftAttack() ? HitEvent::AttackHand::Left : HitEvent::AttackHand::Right) : HitEvent::AttackHand::Left;
				}
			}
			else
			{
				e.attackHand =
					hitData->attackData ? (hitData->attackData->IsLeftAttack() ? HitEvent::AttackHand::Left : HitEvent::AttackHand::Right) : HitEvent::AttackHand::Left;
			}
			e.attackType = HitEvent::AttackType::Weapon;
			g_HitEventDispatcher.dispatch(e);
			return EventResult::kContinue;
		};
	private:
	} tesHitEvent;

	void hook()
	{
		logger::info("Skyrim Hooking...");
		DetourRestoreAfterWith();
		DetourTransactionBegin();
		if (Config::GetSingleton().GetProxyProjectileEnable())
			DetourAttach((void**)ProjectileHit::_onProjectileHit_GetPtrAddr(), (void*)GetFnAddr(&ProjectileHit::onProjectileHit));
		DetourTransactionCommit();

		if (const auto EventHolder = RE::ScriptEventSourceHolder::GetSingleton(); EventHolder) {
			EventHolder->AddEventSink<RE::TESHitEvent>(&tesHitEvent);
		}
	}

	void unhook()
	{
		DetourRestoreAfterWith();
		DetourTransactionBegin();
		if (Config::GetSingleton().GetProxyProjectileEnable())
			DetourDetach((void**)ProjectileHit::_onProjectileHit_GetPtrAddr(), (void*)GetFnAddr(&ProjectileHit::onProjectileHit));
		DetourTransactionCommit();

		if (const auto EventHolder = RE::ScriptEventSourceHolder::GetSingleton(); EventHolder) {
			EventHolder->RemoveEventSink<RE::TESHitEvent>(&tesHitEvent);
		}
	}
}
