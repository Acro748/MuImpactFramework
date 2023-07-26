#pragma once

namespace Mus {
	class ImpactManagerImpl
	{
	public:
		ImpactManagerImpl() {};
		~ImpactManagerImpl() {};

		void Register(RE::BGSImpactDataSet* dataSet, Option option);
		void UnRegister(RE::BGSImpactDataSet* dataSet);

		void Register(RE::SpellItem* spell, Option option);
		void UnRegister(RE::SpellItem* spell);

		void Register(std::string VFXPath, std::uint8_t VFXType, Option option);
		void UnRegister(std::string VFXPath);

		void Register(RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
		void UnRegister(RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);

		void Register(RE::TESEffectShader* effectShader, Option option);
		void UnRegister(RE::TESEffectShader* effectShader);

		void Register(RE::BGSArtObject* artObject, Option option);
		void UnRegister(RE::BGSArtObject* artObject);

		enum RegisterType : std::uint32_t {
			kNone = 0,
			kImpactDataSet = 1 << 0,
			kSpell = 1 << 1,
			kVFX = 1 << 2,
			kSound = 1 << 3,
			kEffectShader = 1 << 4,
			kArtObject = 1 << 5,

			kAll = kImpactDataSet + kSpell + kVFX + kSound + kEffectShader + kArtObject
		};

		void UnRegister(RegisterType type = RegisterType::kAll);

		struct ImpactDataSet_ {
			RE::BGSImpactDataSet* item;
			Option option;
		};
		struct SpellItem_ {
			RE::SpellItem* item;
			Option option;
		};
		struct VFX_ {
			std::string vfxPath;
			std::uint8_t vfxType;
			Option option;
		};
		struct TESEffectShader_ {
			RE::TESEffectShader* item;
			Option option;
		};
		struct BGSArtObject_ {
			RE::BGSArtObject* item;
			Option option;
		};

		const std::unordered_map<RE::FormID, ImpactDataSet_> GetImpactDataSet() const {
			return ImpactDataSet;
		};
		const std::unordered_map<RE::FormID, SpellItem_> GetSpell() const {
			return Spell;
		};
		const std::unordered_map<std::string, VFX_> GetVFX() const {
			return VFX;
		};
		const std::vector<RE::BGSSoundDescriptorForm*> GetSound(bool SecondSound = false) const {
			return Sound[SecondSound];
		};
		const std::unordered_map<RE::FormID, TESEffectShader_> GetEffectShader() const {
			return EffectShader;
		};
		const std::unordered_map<RE::FormID, BGSArtObject_> GetArtObject() const {
			return ArtObject;
		};

		std::size_t GetRegisteredCount() const {
			return ImpactDataSet.size() + Spell.size() + VFX.size() + Sound[0].size() + Sound[1].size() + EffectShader.size() + ArtObject.size();
		};

		void LoadImpactEffects(const HitEvent& e);
	private:
		std::unordered_map<RE::FormID, ImpactDataSet_> ImpactDataSet;
		std::unordered_map<RE::FormID, SpellItem_> Spell;
		std::unordered_map<std::string, VFX_> VFX;
		std::vector<RE::BGSSoundDescriptorForm*> Sound[2];
		std::unordered_map<RE::FormID, TESEffectShader_> EffectShader;
		std::unordered_map<RE::FormID, BGSArtObject_> ArtObject;

		void LoadImpactData(RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::BGSMaterialType* material, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadSpell(RE::Actor* aggressor, RE::TESObjectREFR* target, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadVFX(RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadSound(RE::NiPoint3 hitPosition, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadEffectShader(RE::Actor* aggressor, RE::TESObjectREFR* target, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadArtObject(RE::Actor* aggressor, RE::TESObjectREFR* target, bool instance = Config::GetSingleton().GetInstanceMode());
	};
}