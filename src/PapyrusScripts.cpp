#include "PapyrusScripts.h"

namespace Mus {
    namespace Papyrus {
        void RegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet) {
            if (!actor || !dataSet)
                return;
            ImpactManager::GetSingleton().AddImpactDataSet(actor, LeftHand, dataSet);
        }
        void UnRegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet) {
            if (!actor || !dataSet)
                return;
            ImpactManager::GetSingleton().RemoveImpactDataSet(actor, LeftHand, dataSet);
        }
        void UnRegisterAllImpactDataSetActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand) {
            if (!actor)
                return;
            ImpactManager::GetSingleton().RemoveImpactDataSet(actor, LeftHand);
        }
        void RegisterSpellOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::SpellItem* spell)
        {
            if (!actor || !spell)
                return;
            ImpactManager::GetSingleton().AddSpell(actor, LeftHand, spell);
        }
        void UnRegisterSpellOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::SpellItem* spell)
        {
            if (!actor || !spell)
                return;
            ImpactManager::GetSingleton().RemoveSpell(actor, LeftHand, spell);
        }
        void UnRegisterAllSpellActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand)
        {
            if (!actor)
                return;
            ImpactManager::GetSingleton().RemoveSpell(actor, LeftHand);
        }
        void RegisterVFXOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BSFixedString VFXPath)
        {
            if (!actor || VFXPath.empty())
                return;
            ImpactManager::GetSingleton().AddVFX(actor, LeftHand, VFXPath.c_str(), ConditionManager::GetVFXType(VFXPath.c_str()));
        }
        void UnRegisterVFXOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BSFixedString VFXPath)
        {
            if (!actor || VFXPath.empty())
                return;
            ImpactManager::GetSingleton().RemoveVFX(actor, LeftHand, VFXPath.c_str());
        }
        void UnRegisterAllVFXActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand)
        {
            if (!actor)
                return;
            ImpactManager::GetSingleton().RemoveVFX(actor, LeftHand);

        }
        bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
            vm->RegisterFunction("RegisterImpactDataSetOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), RegisterImpactDataSetOnActor);
            vm->RegisterFunction("UnRegisterImpactDataSetOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterImpactDataSetOnActor);
            vm->RegisterFunction("UnRegisterAllImpactDataSetActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterAllImpactDataSetActor);
            vm->RegisterFunction("RegisterSpellOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), RegisterSpellOnActor);
            vm->RegisterFunction("UnRegisterSpellOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterSpellOnActor);
            vm->RegisterFunction("UnRegisterAllSpellActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterAllSpellActor);
            vm->RegisterFunction("RegisterVFXOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), RegisterVFXOnActor);
            vm->RegisterFunction("UnRegisterVFXOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterVFXOnActor);
            vm->RegisterFunction("UnRegisterAllVFXActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterAllVFXActor);
            return true;
        }
    }
}  // namespace Mus