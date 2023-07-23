#pragma once

namespace Mus {
	class ImpactManager_impl
	{
	public:
		ImpactManager_impl() {};
		~ImpactManager_impl() {};

		void Register(RE::BGSImpactDataSet* dataSet);
		void UnRegister(RE::BGSImpactDataSet* dataSet);

		void Register(RE::SpellItem* spell);
		void UnRegister(RE::SpellItem* spell);

		void Register(std::string VFXPath, std::uint8_t VFXType);
		void UnRegister(std::string VFXPath);

		void Register(RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
		void UnRegister(RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);

		void Register(RE::TESEffectShader* effectShader);
		void UnRegister(RE::TESEffectShader* effectShader);

		void Register(RE::BGSArtObject* artObject);
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

		const std::unordered_map<RE::FormID, RE::BGSImpactDataSet*> GetImpactDataSet() const {
			return ImpactDataSet;
		};
		const std::unordered_map<RE::FormID, RE::SpellItem*> GetSpell() const {
			return Spell;
		};
		const std::unordered_map<std::string, std::uint8_t> GetVFX() const {
			return VFX;
		};
		const std::vector<RE::BGSSoundDescriptorForm*> GetSound(bool SecondSound = false) const {
			return Sound[SecondSound];
		};
		const std::unordered_map<RE::FormID, RE::TESEffectShader*> GetEffectShader() const {
			return EffectShader;
		};
		const std::unordered_map<RE::FormID, RE::BGSArtObject*> GetArtObject() const {
			return ArtObject;
		};

		std::size_t GetRegisteredCount() const {
			return ImpactDataSet.size() + Spell.size() + VFX.size() + Sound[0].size() + Sound[1].size() + EffectShader.size() + ArtObject.size();
		};

		void LoadImpactEffects(const HitEvent& e);
	private:
		std::unordered_map<RE::FormID, RE::BGSImpactDataSet*> ImpactDataSet;
		std::unordered_map<RE::FormID, RE::SpellItem*> Spell;
		std::unordered_map<std::string, std::uint8_t> VFX;
		std::vector<RE::BGSSoundDescriptorForm*> Sound[2];
		std::unordered_map<RE::FormID, RE::TESEffectShader*> EffectShader;
		std::unordered_map<RE::FormID, RE::BGSArtObject*> ArtObject;

		void LoadImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::BGSMaterialType* material, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadSpell(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadVFX(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadSound(RE::NiPoint3 hitPoint, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadEffectShader(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool instance = Config::GetSingleton().GetInstanceMode());
		void LoadArtObject(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool instance = Config::GetSingleton().GetInstanceMode());
	};
}