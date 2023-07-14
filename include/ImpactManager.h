#pragma once

namespace Mus {
	constexpr RE::NiPoint3 emptyPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);
	using EventResult = RE::BSEventNotifyControl;

	class ImpactManager : 
		public RE::BSTEventSink<RE::TESHitEvent>
	{
	public:
		[[nodiscard]] static ImpactManager& GetSingleton() {
			static ImpactManager instance;
			return instance;
		};

		ImpactManager() {};
		~ImpactManager() {};
		
		const std::uint32_t ImpactManagerImpactData = _byteswap_ulong('IMID');
		const std::uint32_t ImpactManagerSpell = _byteswap_ulong('IMSP');
		const std::uint32_t ImpactManagerVFX = _byteswap_ulong('IMVF');
		const std::uint32_t ImpactManagerSound = _byteswap_ulong('IMSN');
		const std::uint32_t ImpactManagerEffectShader = _byteswap_ulong('IMES');

		static void Save(SKSE::SerializationInterface* serde);
		static void Load(SKSE::SerializationInterface* serde, std::uint32_t type);

		enum Type : std::uint32_t {
			None = 0,
			ImpactDataSet = 1,
			Spell = 1 << 1,
			VFX = 1 << 2,
			Sound = 1 << 3,
			EffectShader = 1 << 4,
			Total
		};

		void AddImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData);
		void RemoveImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData);
		void RemoveImpactDataSet(RE::Actor* actor, bool LeftHand);

		void AddSpell(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void RemoveSpell(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void RemoveSpell(RE::Actor* actor, bool LeftHand);

		void AddVFX(RE::Actor* actor, bool LeftHand, std::string VFXPath, std::uint8_t VFXType);
		void RemoveVFX(RE::Actor* actor, bool LeftHand, std::string VFXPath);
		void RemoveVFX(RE::Actor* actor, bool LeftHand);

		void AddSound(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
		void RemoveSound(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
		void RemoveSound(RE::Actor* actor, bool LeftHand, bool SecondSound = false);

		void AddEffectShader(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader);
		void RemoveEffectShader(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader);
		void RemoveEffectShader(RE::Actor* actor, bool LeftHand);

		inline void ClearActorList() {
			actorImpactData.clear();
		}
	protected:
		EventResult ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>*);

	private:
		concurrency::concurrent_unordered_map<RE::FormID, ImpactManager_impl> actorImpactData;
		ImpactManager_impl conditionActorImpactData = ImpactManager_impl();

		void Register(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void Register(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void Register(RE::Actor* actor, bool LeftHand, std::string VFXPath, std::uint8_t VFXType);
		void Register(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
		void Register(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void UnRegister(RE::Actor* actor, bool LeftHand, std::string VFXPath);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader);
		void UnRegister(RE::Actor* actor, bool LeftHand, Type type);

		inline bool IsValidHitEvent(const RE::TESHitEvent* evn) {
			if (!evn)
				return false;
			RE::Actor* cause = skyrim_cast<RE::Actor*>(evn->cause.get());
			RE::Actor* target = skyrim_cast<RE::Actor*>(evn->target.get());
			auto cause_aiprocess = cause ? cause->GetActorRuntimeData().currentProcess : nullptr;
			auto target_aiprocess = target ? target->GetActorRuntimeData().currentProcess : nullptr;
			if (target_aiprocess && target_aiprocess->middleHigh && target_aiprocess->middleHigh->lastHitData)
			{
				auto hitData = target_aiprocess->middleHigh->lastHitData;
				if (hitData && !IsEqual(hitData->hitPosition, emptyPoint))
					return true;
			}
			else if (!target && Config::GetSingleton().GetEnableInanimateObject())
				return true;
			else if (evn->projectile != 0 && Config::GetSingleton().GetEnableMagic())
				return true;
			return false;
		}
	};
}
