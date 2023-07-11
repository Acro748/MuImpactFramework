#pragma once

namespace Mus {
	using EventResult = RE::BSEventNotifyControl;
	constexpr RE::NiPoint3 emptyPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);

	class TaskImpactVFX : public SKSE::detail::TaskDelegate
	{
	public:
		TaskImpactVFX(RE::BGSImpactDataSet* impactData, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPoint, RE::NiAVObject* targetObj = nullptr)
			: mImpactData(impactData), mAggressor(aggressor), mTarget(target), mHitPoint(hitPoint), mTargetObj(targetObj) {};

		void Run();
		void Dispose();
	private:
		RE::BGSImpactDataSet* mImpactData = nullptr;
		RE::TESObjectREFR* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::NiPoint3 mHitPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		RE::NiAVObject* mTargetObj;
	};

	class TaskVFX : public SKSE::detail::TaskDelegate
	{
	public:
		TaskVFX(std::string VFXPath, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPoint, std::uint8_t vfxType, RE::NiAVObject* targetObj = nullptr)
			: mVFXPath(VFXPath), mAggressor(aggressor), mTarget(target), mHitPoint(hitPoint), mVFXType(VFXType(vfxType)), mTargetObj(targetObj) {};

		enum VFXType : std::uint8_t {
			Impact,
			Spell,

			Total
		};

		void Run();
		void Dispose();
	private:
		bool CreateImpactVFX();
		bool CreateArtVFX();

		std::string mVFXPath = "";
		RE::TESObjectREFR* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::NiPoint3 mHitPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		VFXType mVFXType;
		RE::NiAVObject* mTargetObj;
	};

	class TaskCastVFX : public SKSE::detail::TaskDelegate
	{
	public:
		TaskCastVFX(RE::SpellItem* spell, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target)
			: mSpell(spell), mAggressor(aggressor), mTarget(target) {};

		virtual void Run() override;
		virtual void Dispose() override;

	private:
		RE::SpellItem* mSpell = nullptr;
		RE::TESObjectREFR* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;

		static void Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target);
	};

	class ImpactManager_impl 
	{
		RE::FormID owner;
	public:
		ImpactManager_impl(RE::Actor* actor) : owner(actor->formID) {};
		~ImpactManager_impl() {};

		void Register(bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void Register(bool LeftHand, RE::SpellItem* spell);
		void UnRegister(bool LeftHand, RE::SpellItem* spell);
		void Register(bool LeftHand, std::string VFXPath, std::uint8_t VFXType);
		void UnRegister(bool LeftHand, std::string VFXPath);
		void UnRegister(bool LeftHand, uint32_t type);

		void ProcessHitEvent(const RE::TESHitEvent* evn);
		inline bool IsVaild() { return (ImpactDataSet[0].size() + ImpactDataSet[1].size()); };

		const std::vector<RE::BGSImpactDataSet*> GetImpactDataSet(bool LeftHand) {
			return ImpactDataSet[LeftHand];
		};
		const std::vector<RE::SpellItem*> GetSpell(bool LeftHand) {
			return Spell[LeftHand];
		};
		const std::unordered_map<std::string, std::uint8_t> GetVFX(bool LeftHand) {
			return VFX[LeftHand];
		};
	private:
		std::vector<RE::BGSImpactDataSet*> ImpactDataSet[2];
		std::vector<RE::SpellItem*> Spell[2];
		std::unordered_map<std::string, std::uint8_t> VFX[2];

		void LoadHitPlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint);
		void LoadHitPlayImpactData(RE::Actor* aggressor, RE::TESObjectREFR* target, bool LeftHand);

		void PlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());
		void CastSpell(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, bool instance = Config::GetSingleton().GetInstanceMode());
		void PlayVFX(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());

		const RE::BSFixedString HandL = "NPC L Hand [LHnd]";
		const RE::BSFixedString HandR = "NPC R Hand [RHnd]";
	};

	class ImpactManager : 
		public RE::BSTEventSink<RE::TESHitEvent>
	{
	public:
		[[nodiscard]] static ImpactManager& GetSingleton() {
			static ImpactManager instance;
			return instance;
		};

		const std::uint32_t ImpactManagerImpactData = _byteswap_ulong('IMID');
		const std::uint32_t ImpactManagerSpell = _byteswap_ulong('IMSP');
		const std::uint32_t ImpactManagerVFX = _byteswap_ulong('IMVF');
		static void Save(SKSE::SerializationInterface* serde);
		static void Load(SKSE::SerializationInterface* serde, std::uint32_t type);

		enum Type : std::uint32_t {
			None = 0,
			ImpactDataSet = 1,
			Spell = 1 << 1,
			VFX = 1 << 2,
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

		inline void ClearActorList() {
			actorImpactData.clear();
		}
	protected:
		EventResult ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>*);

	private:
		concurrency::concurrent_unordered_map<RE::FormID, ImpactManager_impl> actorImpactData;
		concurrency::concurrent_unordered_map<RE::FormID, std::shared_ptr<ImpactManager_impl>> conditionActorImpactData;

		void Register(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void Register(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void Register(RE::Actor* actor, bool LeftHand, std::string VFXPath, std::uint8_t VFXType);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void UnRegister(RE::Actor* actor, bool LeftHand, std::string VFXPath);
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
