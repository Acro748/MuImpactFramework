#pragma once

namespace Mus {
	using EventResult = RE::BSEventNotifyControl;
	constexpr RE::NiPoint3 emptyPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);

	class TaskPlayImpactVFX : public SKSE::detail::TaskDelegate
	{
	public:
		virtual void Run() override;
		virtual void Dispose() override;

		TaskPlayImpactVFX(RE::BGSImpactDataSet* impactData, RE::TESObjectREFR* obj, const RE::BSFixedString& nodeName, RE::NiPoint3 direct = RE::NiPoint3(0.0f, 0.0f, 0.0f));

	private:
		RE::BGSImpactDataSet* mImpactData = nullptr;
		RE::TESObjectREFR* mObj = nullptr;
		RE::BSFixedString mNodeName;
		RE::NiPoint3 mDirect = RE::NiPoint3(0.0f, 0.0f, 0.0f);

		static bool PlayImpactEffect(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId
			, RE::TESObjectREFR* obj, RE::BGSImpactDataSet* impactData, const RE::BSFixedString& nodeName
			, float afPickDirX = 0.0f, float afPickDirY = 0.0f, float afPickDirZ = 0.0f, float afPickLength = 1.0f
			, bool abApplyNodeRotation = false, bool abUseNodeLocalRotation = false);
	};

	class TaskPlaySound : public SKSE::detail::TaskDelegate
	{
	public:
		virtual void Run() override;
		virtual void Dispose() override;

		TaskPlaySound(RE::TESSound* sound, RE::Actor* actor);

	private:
		RE::TESSound* mSound = nullptr;
		RE::Actor* mActor = nullptr;

		static void Play(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::TESSound* sound, RE::Actor* actor);
	};

	class ImpactDataManager_ 
	{
		RE::FormID owner;
	public:
		ImpactDataManager_(RE::Actor* actor) : owner(actor->formID) {};
		~ImpactDataManager_() {};

		void Register(bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(bool LeftHand);

		void ProcessHitEvent(const RE::TESHitEvent* evn);
		inline bool IsVaild() { return (ImpactDataSet[0].size() + ImpactDataSet[1].size()); };
	private:
		std::vector<RE::BGSImpactDataSet*> ImpactDataSet[2];

		void LoadHitPlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint);
		void LoadHitPlayImpactData(RE::Actor* cause, bool LeftHand);
		void PlayImpactData(RE::TESObjectREFR* target, bool LeftHand, const RE::BSFixedString& nodeName, bool instance = false);

		const RE::BSFixedString HandL = "NPC L Hand [LHnd]";
		const RE::BSFixedString HandR = "NPC R Hand [RHnd]";
	};

	class ImpactDataManager : 
		public RE::BSTEventSink<RE::TESLoadGameEvent>,
		public RE::BSTEventSink<RE::TESHitEvent>
	{
	public:
		[[nodiscard]] static ImpactDataManager& GetSingleton() {
			static ImpactDataManager instance;
			return instance;
		};

		void RegisterEvent();

		void AddImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData);
		void RemoveImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData);
		void RemoveImpactDataSet(RE::Actor* actor, bool LeftHand);

	protected:
		EventResult ProcessEvent(const RE::TESLoadGameEvent* evn, RE::BSTEventSource<RE::TESLoadGameEvent>*);
		EventResult ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>*);

	private:
		concurrency::concurrent_unordered_map<RE::FormID, ImpactDataManager_> actorImpactData;

		void Register(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(RE::Actor* actor, bool LeftHand);
	};
}
