#pragma once

namespace Mus {
    namespace Papyrus {
        void RegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
        void UnRegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
        void UnRegisterAllImpactDataSetActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand);
        void RegisterSpellOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
        void UnRegisterSpellOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::SpellItem* spell);
        void UnRegisterAllSpellActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand);
        void RegisterVFXOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BSFixedString VFXPath);
        void UnRegisterVFXOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BSFixedString VFXPath);
        void UnRegisterAllVFXActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand);
        void RegisterSoundOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
        void UnRegisterSoundOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound = false);
        void UnRegisterAllSoundActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand);
        void RegisterEffectShaderOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader);
        void UnRegisterEffectShaderOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader);
        void UnRegisterAllEffectShaderActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand);
        bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
    }
}