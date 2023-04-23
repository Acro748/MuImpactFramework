#pragma once

namespace Mus {
	using EventResult = RE::BSEventNotifyControl;
	constexpr RE::NiPoint3 emptyPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);

	class TaskPlayImpactVFX : public SKSE::detail::TaskDelegate
	{
	public:
		TaskPlayImpactVFX(RE::BGSImpactDataSet* impactData, RE::TESObjectREFR* obj, const RE::BSFixedString& nodeName, RE::NiPoint3 direct = RE::NiPoint3(0.0f, 0.0f, 0.0f));

		virtual void Run() override;
		virtual void Dispose() override;

	private:
		RE::BGSImpactDataSet* mImpactData = nullptr;
		RE::TESObjectREFR* mObj = nullptr;
		RE::BSFixedString mNodeName;
		RE::NiPoint3 mDirect = RE::NiPoint3(0.0f, 0.0f, 0.0f);

		static bool PlayImpactEffect(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, 
			RE::TESObjectREFR* a_ref, RE::BGSImpactDataSet* a_impactEffect, const RE::BSFixedString& a_nodeName, 
			float a_pickDirection_x, float a_pickDirection_y, float a_pickDirection_z,
			float a_pickLength = 0.0f, bool a_applyNodeRotation = false, bool a_useNodeLocalRotation = false);
	};

	constexpr double unit = 0.0142875;
	class TaskSpwanVFX : public SKSE::detail::TaskDelegate
	{
	public:
		TaskSpwanVFX(RE::BGSImpactDataSet* impactData, RE::TESObjectREFR* obj, RE::BSFixedString nodeName)
			: mImpactData(impactData), mObj(obj), mNodeName(nodeName) {};

		virtual void Run() override;
		virtual void Dispose() override;

	private:
		RE::BGSImpactDataSet* mImpactData = nullptr;
		RE::TESObjectREFR* mObj = nullptr;
		RE::BSFixedString mNodeName;

		static RE::BSTempEffectParticle* Spawn(RE::TESObjectCELL * a_cell, float a_lifetime, const char* a_modelName, const RE::NiMatrix3& a_normal, const RE::NiPoint3& a_position, float a_scale, std::uint32_t a_flags, RE::NiAVObject* a_target)
		{
			using func_t = decltype(&TaskSpwanVFX::Spawn);
			REL::Relocation<func_t> func{ RELOCATION_ID(29219, 30072) };
			return func(a_cell, a_lifetime, a_modelName, a_normal, a_position, a_scale, a_flags, a_target);
		}
		static void* sub_1401B8660(RE::BGSDecalManager* decalManager, float* value, RE::NiAVObject* a4)
		{
			using func_t = decltype(&TaskSpwanVFX::sub_1401B8660);
			REL::Relocation<func_t> func{ RELOCATION_ID(15029, 15203) };
			return func(decalManager, value, a4);
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
	private:
		std::vector<RE::BGSImpactDataSet*> ImpactDataSet[2];
		std::vector<RE::SpellItem*> Spell[2];

		void LoadHitPlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDir = emptyPoint);
		void LoadHitPlayImpactData(RE::Actor* cause, bool LeftHand);
		void PlayImpactData(RE::TESObjectREFR* target, bool LeftHand, const RE::BSFixedString& nodeName, RE::NiPoint3 hitDir = emptyPoint, bool instance = false);

		void CastSpell(RE::TESObjectREFR* source, RE::TESObjectREFR* target, bool LeftHand, bool instance = false);

		const RE::BSFixedString HandL = "NPC L Hand [LHnd]";
		const RE::BSFixedString HandR = "NPC R Hand [RHnd]";
	};



	//20408, 20860, void*(ImpactDataSet*, uint64, uint64)
	//, 15197, void(decalmanager*, uint8_t, uint64_t, void*
	class ImpactManager : 
		public RE::BSTEventSink<RE::TESHitEvent>
	{
	public:
		[[nodiscard]] static ImpactManager& GetSingleton() {
			static ImpactManager instance;
			return instance;
		};

		const std::uint32_t ImpactDataManagerRecord = _byteswap_ulong('IDMR');
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
			RE::Actor* target = skyrim_cast<RE::Actor*>(evn->target.get());
			auto target_aiprocess = target ? target->GetActorRuntimeData().currentProcess : nullptr;
			if (!target_aiprocess || !target_aiprocess->middleHigh)
				return false;
			auto hitData = target_aiprocess->middleHigh->lastHitData;
			if (hitData)
			{
				if (!IsEqual(hitData->hitPosition, emptyPoint))
				{
					return true;
				}
				else
					return false;
			}
			return true;
		}
	};
}
