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
        void RegisterSpell(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::SpellItem* spell)
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
        bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
            vm->RegisterFunction("RegisterImpactDataSetOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), RegisterImpactDataSetOnActor);
            vm->RegisterFunction("UnRegisterImpactDataSetOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterImpactDataSetOnActor);
            vm->RegisterFunction("UnRegisterAllImpactDataSetActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterAllImpactDataSetActor);
            vm->RegisterFunction("RegisterSpell", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), RegisterSpell);
            vm->RegisterFunction("UnRegisterSpellOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterSpellOnActor);
            vm->RegisterFunction("UnRegisterAllSpellActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterAllSpellActor);
            return true;
        }
    }
}  // namespace Mus