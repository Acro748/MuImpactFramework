#pragma once

namespace Mus {
	using EventResult = RE::BSEventNotifyControl;
	constexpr RE::NiPoint3 emptyPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);

	class TaskPlayImpactVFX :
		public RE::BSTEventSink<RE::BSAnimationGraphEvent>
	{
	public:
		TaskPlayImpactVFX(RE::BGSImpactDataSet* impactData, RE::TESObjectREFR* obj, RE::NiAVObject* node, RE::NiPoint3 direct = RE::NiPoint3(0.0f, 0.0f, 0.0f))
			: mImpactData(impactData), mObj(obj), mNode(node), mDirect(direct) {};

		void Run();
		void Dispose();
	protected:
		EventResult ProcessEvent(const RE::BSAnimationGraphEvent* evn, RE::BSTEventSource<RE::BSAnimationGraphEvent>*) override {
			Run();
			return EventResult::kContinue;
		};
	private:
		bool AddedEvent = false;
		RE::BGSImpactDataSet* mImpactData = nullptr;
		RE::TESObjectREFR* mObj = nullptr;
		RE::NiAVObject* mNode;
		RE::NiPoint3 mDirect = RE::NiPoint3(0.0f, 0.0f, 0.0f);

		enum PersistMode {
			Once,
			Event,
			Visit
		};
		void AddGraphEvent();
		void RemoveGraphEvent();

		void VisitNodes();

		static bool PlayImpactEffect(RE::BGSImpactManager* a_impactManager, RE::TESObjectREFR* a_ref, RE::BGSImpactDataSet* a_impactEffect, const char* a_nodeName,
			RE::NiPoint3& a_pickDirection, float a_pickLength, bool a_applyNodeRotation, bool a_useNodeLocalRotation)
		{
			using func_t = decltype(&TaskPlayImpactVFX::PlayImpactEffect);
			REL::Relocation<func_t> func{ RELOCATION_ID(35320, 36215) };
			return func(a_impactManager, a_ref, a_impactEffect, a_nodeName, a_pickDirection, a_pickLength, a_applyNodeRotation, a_useNodeLocalRotation);
		}
	};

	class TaskCast : public SKSE::detail::TaskDelegate
	{
	public:
		TaskCast(RE::SpellItem* spell, RE::TESObjectREFR* source, RE::TESObjectREFR* target) 
			: mSpell(spell), mSource(source), mTarget(target) {};

		virtual void Run() override;
		virtual void Dispose() override;

	private:
		RE::SpellItem* mSpell = nullptr;
		RE::TESObjectREFR* mSource = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;

		static void Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* source, RE::TESObjectREFR* target);
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
		void UnRegister(bool LeftHand, uint32_t type);

		void ProcessHitEvent(const RE::TESHitEvent* evn);
		inline bool IsVaild() { return (ImpactDataSet[0].size() + ImpactDataSet[1].size()); };

		const std::vector<RE::BGSImpactDataSet*> GetImpactDataSet(bool LeftHand) {
			return ImpactDataSet[LeftHand];
		};
		const std::vector<RE::SpellItem*> GetSpell(bool LeftHand) {
			return Spell[LeftHand];
		};
	private:
		std::vector<RE::BGSImpactDataSet*> ImpactDataSet[2];
		std::vector<RE::SpellItem*> Spell[2];

		void LoadHitPlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDir = emptyPoint);
		void LoadHitPlayImpactData(RE::Actor* cause, RE::TESObjectREFR* target, bool LeftHand);
		void PlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiAVObject* node, RE::NiPoint3 hitDir = emptyPoint, bool instance = false);

		void CastSpell(RE::TESObjectREFR* source, RE::TESObjectREFR* target, bool LeftHand, bool instance = false);

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
		static void Save(SKSE::SerializationInterface* serde);
		static void Load(SKSE::SerializationInterface* serde, std::uint32_t type);

		enum Type : std::uint32_t {
			None = 0,
			ImpactDataSet = 1,
			Spell = 1 << 1,
			Total
		};

		void AddImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData);
		void RemoveImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData);
		void RemoveImpactDataSet(RE::Actor* actor, bool LeftHand);

		void AddSpell(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void RemoveSpell(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void RemoveSpell(RE::Actor* actor, bool LeftHand);

		inline void ClearActorList() {
			actorImpactData.clear();
		}
	protected:
		EventResult ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>*);

	private:
		concurrency::concurrent_unordered_map<RE::FormID, ImpactManager_impl> actorImpactData;

		void Register(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void Register(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
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
