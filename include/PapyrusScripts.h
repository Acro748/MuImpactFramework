#pragma once

namespace Mus {
    namespace Papyrus {
        void RegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
        void UnRegisterImpactDataSetOnActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet);
        void UnRegisterActor(RE::StaticFunctionTag*, RE::Actor* actor, bool LeftHand);
        bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm);
    }
}