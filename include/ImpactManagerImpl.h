#pragma once

namespace Mus {
	class ImpactManager_impl
	{
		RE::FormID owner;
	public:
		ImpactManager_impl() : owner(0) {};
		ImpactManager_impl(RE::Actor* actor) : owner(actor->formID) {};
		~ImpactManager_impl() {};

		void Register(bool LeftHand, RE::BGSImpactDataSet* dataSet);
		void UnRegister(bool LeftHand, RE::BGSImpactDataSet* dataSet);

		void Register(bool LeftHand, RE::SpellItem* spell);
		void UnRegister(bool LeftHand, RE::SpellItem* spell);

		void Register(bool LeftHand, std::string VFXPath, std::uint8_t VFXType);
		void UnRegister(bool LeftHand, std::string VFXPath);

		void Register(bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
		void UnRegister(bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);

		void Register(bool LeftHand, RE::TESEffectShader* effectShader);
		void UnRegister(bool LeftHand, RE::TESEffectShader* effectShader);

		void UnRegister(bool LeftHand, uint32_t type);

		void ProcessHitEvent(const RE::TESHitEvent* evn);

		const std::unordered_map<RE::FormID, RE::BGSImpactDataSet*> GetImpactDataSet(bool LeftHand) {
			return ImpactDataSet[LeftHand];
		};
		const std::unordered_map<RE::FormID, RE::SpellItem*> GetSpell(bool LeftHand) {
			return Spell[LeftHand];
		};
		const std::unordered_map<std::string, std::uint8_t> GetVFX(bool LeftHand) {
			return VFX[LeftHand];
		};
		const std::vector<RE::BGSSoundDescriptorForm*> GetSound(bool LeftHand, bool SecondSound = false) {
			return Sound[LeftHand][SecondSound];
		};
		const std::unordered_map<RE::FormID, RE::TESEffectShader*> GetEffectShader(bool LeftHand) {
			return EffectShader[LeftHand];
		};
	private:
		std::unordered_map<RE::FormID, RE::BGSImpactDataSet*> ImpactDataSet[2];
		std::unordered_map<RE::FormID, RE::SpellItem*> Spell[2];
		std::unordered_map<std::string, std::uint8_t> VFX[2];
		std::vector<RE::BGSSoundDescriptorForm*> Sound[2][2];
		std::unordered_map<RE::FormID, RE::TESEffectShader*> EffectShader[2];

		void LoadHitPlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection);
		void LoadHitPlayImpactData(RE::Actor* aggressor, RE::TESObjectREFR* target, bool LeftHand);

		void PlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());
		void CastSpell(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, bool instance = Config::GetSingleton().GetInstanceMode());
		void PlayVFX(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj = nullptr, bool instance = Config::GetSingleton().GetInstanceMode());
		void PlaySound(bool LeftHand, RE::NiPoint3 hitPoint, bool instance = Config::GetSingleton().GetInstanceMode());
		void PlayEffectShader(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, bool instance = Config::GetSingleton().GetInstanceMode());

		const RE::BSFixedString HandL = "NPC L Hand [LHnd]";
		const RE::BSFixedString HandR = "NPC R Hand [RHnd]";
	};
}