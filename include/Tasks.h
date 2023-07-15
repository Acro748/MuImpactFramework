#pragma once

namespace Mus {
	class TaskImpactVFX : public SKSE::detail::TaskDelegate
	{
	public:
		TaskImpactVFX(RE::BGSImpactDataSet* impactData, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj = nullptr)
			: mImpactData(impactData), mAggressor(aggressor), mTarget(target), mHitPoint(hitPoint), mHitDirection(hitDirection), mTargetObj(targetObj) {};

		void Run();
		void Dispose();
	private:
		RE::BGSImpactDataSet* mImpactData = nullptr;
		RE::TESObjectREFR* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::NiPoint3 mHitPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		RE::NiPoint3 mHitDirection = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		RE::NiAVObject* mTargetObj;

		RE::NiAVObject* GetObjectByDistance();
	};

	class TaskTempFormManager
	{
	public:
		[[nodiscard]] static TaskTempFormManager& GetSingleton() {
			static TaskTempFormManager instance;
			return instance;
		};

		RE::BGSImpactData* GetImpactDataTempForm();
		RE::BGSArtObject* GetArtObjectTempForm();
	private:
		std::uint32_t ImpactDataNum = 0;
		std::uint32_t ArtObjectNum = 0;

		template <typename T>
		T* GetNewForm();

		std::mutex m_lock;
	};

	class TaskVFX : public SKSE::detail::TaskDelegate
	{
	public:
		TaskVFX(std::string VFXPath, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, std::uint8_t vfxType, RE::NiAVObject* targetObj = nullptr)
			: mVFXPath(VFXPath), mAggressor(aggressor), mTarget(target), mHitPoint(hitPoint), mHitDirection(hitDirection), mVFXType(VFXType(vfxType)), mTargetObj(targetObj) {};

		enum VFXType : std::uint8_t {
			Impact,
			HitEffect,
			Total,

			None
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
		RE::NiPoint3 mHitDirection = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		VFXType mVFXType;
		RE::NiAVObject* mTargetObj;
	};

	class TaskEffectShader: public SKSE::detail::TaskDelegate
	{
	public:
		TaskEffectShader(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::TESEffectShader* effectShader)
			: mAggressor(aggressor), mTarget(target), mEffectShader(effectShader) {};

		void Run();
		void Dispose();
	private:
		RE::TESObjectREFR* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::TESEffectShader* mEffectShader = nullptr;
	};

	class TaskSound : public SKSE::detail::TaskDelegate
	{
	public:
		TaskSound(RE::NiPoint3 hitPoint, RE::BGSSoundDescriptorForm* sound1, RE::BGSSoundDescriptorForm* sound2)
			: mHitPoint(hitPoint), mSound1(sound1), mSound2(sound2) {};

		void Run();
		void Dispose();
	private:
		RE::BGSSoundDescriptorForm* mSound1 = nullptr;
		RE::BGSSoundDescriptorForm* mSound2 = nullptr;
		RE::NiPoint3 mHitPoint = RE::NiPoint3(0.0f, 0.0f, 0.0f);
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
}