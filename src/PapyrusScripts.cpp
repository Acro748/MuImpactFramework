#include "PapyrusScripts.h"

namespace Mus {
    namespace Papyrus {
        void RegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet) {
            if (!actor || !dataSet)
                return;
            ImpactDataManager::GetSingleton().AddImpactDataSet(actor, LeftHand, dataSet);
        }
        void UnRegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet) {
            if (!actor || !dataSet)
                return;
            ImpactDataManager::GetSingleton().RemoveImpactDataSet(actor, LeftHand, dataSet);
        }
        void UnRegisterActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand) {
            if (!actor)
                return;
            ImpactDataManager::GetSingleton().RemoveImpactDataSet(actor, LeftHand);
        }
        bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
            vm->RegisterFunction("RegisterImpactDataSetOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), RegisterImpactDataSetOnActor);
            vm->RegisterFunction("UnRegisterImpactDataSetOnActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterImpactDataSetOnActor);
            vm->RegisterFunction("UnRegisterActor", SKSE::PluginDeclaration::GetSingleton()->GetName().data(), UnRegisterActor);
            return true;
        }
    }
}  // namespace Mus