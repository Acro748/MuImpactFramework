#pragma once

namespace Mus {
	using Option = ConditionManager::Condition::Option;
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
	};

	class TaskOptionManager
	{
	public:
		[[nodiscard]] static TaskOptionManager& GetSingleton() {
			static TaskOptionManager instance;
			return instance;
		};

		RE::NiPoint3 GetRandomDirection();
	};

	class VFXTaskImpactDataSet : public SKSE::detail::TaskDelegate
	{
	public:
		VFXTaskImpactDataSet(RE::BGSImpactDataSet* impactData, RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::BGSMaterialType* material, Option option)
			: mImpactData(impactData), mAggressor(aggressor), mTarget(target), mhitPosition(hitPosition), mHitDirection(hitDirection), mMaterial(material), mOption(option) {};

		void Run();
		void Dispose();

	private:
		RE::BGSImpactDataSet* mImpactData = nullptr;
		RE::Actor* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::NiPoint3 mhitPosition = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		RE::NiPoint3 mHitDirection = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		RE::BGSMaterialType* mMaterial;
		Option mOption;
	};

	class VFXTaskSpell : public SKSE::detail::TaskDelegate
	{
	public:
		VFXTaskSpell(RE::SpellItem* spell, RE::Actor* aggressor, RE::TESObjectREFR* target, Option option)
			: mSpell(spell), mAggressor(aggressor), mTarget(target), mOption(option) {};

		virtual void Run() override;
		virtual void Dispose() override;

	private:
		RE::SpellItem* mSpell = nullptr;
		RE::Actor* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		Option mOption;

		static void Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target);
	};

	class VFXTask : public SKSE::detail::TaskDelegate
	{
	public:
		VFXTask(std::string VFXPath, RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, std::uint8_t vfxType, Option option)
			: mVFXPath(VFXPath), mAggressor(aggressor), mTarget(target), mhitPosition(hitPosition), mHitDirection(hitDirection), mVFXType(VFXType(vfxType)), mOption(option) {};

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
		RE::Actor* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::NiPoint3 mhitPosition = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		RE::NiPoint3 mHitDirection = RE::NiPoint3(0.0f, 0.0f, 0.0f);
		VFXType mVFXType;
		RE::NiAVObject* mTargetObj;
		Option mOption;
	};

	class SFXTaskSound : public SKSE::detail::TaskDelegate
	{
	public:
		SFXTaskSound(RE::BGSSoundDescriptorForm* sound1, RE::BGSSoundDescriptorForm* sound2, RE::NiPoint3 hitPosition)
			: mSound1(sound1), mSound2(sound2), mhitPosition(hitPosition) {};

		void Run();
		void Dispose();
	private:
		RE::BGSSoundDescriptorForm* mSound1 = nullptr;
		RE::BGSSoundDescriptorForm* mSound2 = nullptr;
		RE::NiPoint3 mhitPosition = RE::NiPoint3(0.0f, 0.0f, 0.0f);
	};

	class VFXTaskEffectShader : public SKSE::detail::TaskDelegate
	{
	public:
		VFXTaskEffectShader(RE::TESEffectShader* effectShader, RE::Actor* aggressor, RE::TESObjectREFR* target, Option option)
			: mEffectShader(effectShader), mAggressor(aggressor), mTarget(target), mOption(option) {};

		void Run();
		void Dispose();
	private:
		RE::Actor* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::TESEffectShader* mEffectShader = nullptr;
		Option mOption;
	};

	class VFXTaskArtObject : public SKSE::detail::TaskDelegate
	{
	public:
		VFXTaskArtObject(RE::BGSArtObject* artObject, RE::Actor* aggressor, RE::TESObjectREFR* target, Option option)
			: mArtObject(artObject), mAggressor(aggressor), mTarget(target), mOption(option) {};

		void Run();
		void Dispose();
	private:
		RE::Actor* mAggressor = nullptr;
		RE::TESObjectREFR* mTarget = nullptr;
		RE::BGSArtObject* mArtObject = nullptr;
		Option mOption;
	};
}