#include "ImpactDataManager.h"

namespace Mus {
	TaskPlayImpactVFX::TaskPlayImpactVFX(RE::BGSImpactDataSet* impactData, RE::TESObjectREFR* obj, const RE::BSFixedString& nodeName, RE::NiPoint3 direct)
		: mImpactData(impactData), mObj(obj), mNodeName(nodeName), mDirect(direct)
	{
	
	}
	void TaskPlayImpactVFX::Run()
	{
		if (!mImpactData || !mObj)
			return;

		if (Config::GetSingleton().GetImpactMode() == 0)
		{
			auto vm = RE::SkyrimVM::GetSingleton();
			if (!vm || !vm->impl)
				return;
			if (!PlayImpactEffect(vm->impl.get(), 0, mObj, mImpactData, mNodeName,
				mDirect.x, mDirect.y, mDirect.z, 512.0f, false, false))
			{
				logger::debug("Failed to create ImpactVFX {:x} on {} of {:x}", mImpactData->formID, mNodeName.c_str(), mObj->formID);
			}
			else
				logger::debug("create ImpactVFX {:x} on {} of {:x}", mImpactData->formID, mNodeName.c_str(), mObj->formID);
		}
		else if (Config::GetSingleton().GetImpactMode() == 1)
		{
			auto impactManager = RE::BGSImpactManager::GetSingleton();
			if (!impactManager)
				return;
			if (!impactManager->PlayImpactEffect(mObj, mImpactData, mNodeName.c_str(), mDirect, 512.0f, false, false))
			{
				logger::debug("Failed to create ImpactVFX {:x} on {} of {:x}", mImpactData->formID, mNodeName.c_str(), mObj->formID);
			}
			else
				logger::debug("create ImpactVFX {:x} on {} of {:x}", mImpactData->formID, mNodeName.c_str(), mObj->formID);
		}
	}
	void TaskPlayImpactVFX::Dispose()
	{
		delete this;
	}
	bool TaskPlayImpactVFX::PlayImpactEffect(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::TESObjectREFR* obj, RE::BGSImpactDataSet* impactData, const RE::BSFixedString& nodeName, float afPickDirX, float afPickDirY, float afPickDirZ, float afPickLength, bool abApplyNodeRotation, bool abUseNodeLocalRotation)
	{
		using func_t = decltype(&TaskPlayImpactVFX::PlayImpactEffect);
		REL::VariantID offset(55677, 56208, 0x009D06C0);
		REL::Relocation<func_t> func{ offset };
		return func(VMinternal, stackId, obj, impactData, nodeName, afPickDirX, afPickDirY, afPickDirZ, afPickLength, abApplyNodeRotation, abUseNodeLocalRotation);
	}

	TaskPlaySound::TaskPlaySound(RE::TESSound* sound, RE::Actor* actor)
		: mSound(sound), mActor(actor)
	{
	}
	void TaskPlaySound::Run()
	{
		if (!mSound || !mActor)
			return;
		if (auto vm = RE::SkyrimVM::GetSingleton(); vm && vm->impl)
		{
			Play(vm->impl.get(), 0, mSound, mActor);
		}
	}
	void TaskPlaySound::Dispose()
	{
		delete this;
	}
	void TaskPlaySound::Play(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::TESSound* sound, RE::Actor* actor)
	{
		using func_t = decltype(&TaskPlaySound::Play);
		REL::Relocation<func_t> func{ RE::Offset::BSSoundHandle::Play };
		return func(VMinternal, stackId, sound, actor);
	}

	void ImpactDataManager_::Register(bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = std::find(ImpactDataSet[LeftHand].begin(), ImpactDataSet[LeftHand].end(), dataSet); found != ImpactDataSet[LeftHand].end())
			return;
		ImpactDataSet[LeftHand].emplace_back(dataSet);
	}
	void ImpactDataManager_::UnRegister(bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = std::find(ImpactDataSet[LeftHand].begin(), ImpactDataSet[LeftHand].end(), dataSet); found != ImpactDataSet[LeftHand].end())
			ImpactDataSet[LeftHand].erase(found);
	}
	void ImpactDataManager_::UnRegister(bool LeftHand)
	{
		ImpactDataSet[LeftHand].clear();
	}

	void ImpactDataManager_::ProcessHitEvent(const RE::TESHitEvent* evn)
	{
		if (!evn->cause || !evn->target)
			return;
		RE::Actor* cause = skyrim_cast<RE::Actor*>(evn->cause.get());
		if (!cause)
			return;

		auto* cause_aiprocess = cause->GetActorRuntimeData().currentProcess;
		if (!cause_aiprocess || !cause_aiprocess->high || !cause_aiprocess->high->attackData) {
			return;
		}
		bool LeftHand = cause_aiprocess->high->attackData->IsLeftAttack();

		RE::Actor* target = skyrim_cast<RE::Actor*>(evn->target.get());
		auto target_aiprocess = target ? target->GetActorRuntimeData().currentProcess : nullptr;
		if (!target_aiprocess || !target_aiprocess->middleHigh)
			return;

		auto hitData = target_aiprocess->middleHigh->lastHitData;
		if (hitData)
		{
			if (!IsEqual(hitData->hitPosition, emptyPoint))
			{
				logger::debug("Find node by HitPosition...");
				LoadHitPlayImpactData(evn->target.get(), LeftHand, hitData->hitPosition);
			}
		}
		else
		{
			logger::debug("Find node by HandNodePosition on causeActor...");
			LoadHitPlayImpactData(cause, LeftHand);
		}
	}
	void ImpactDataManager_::LoadHitPlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint)
	{
		if (!target || !target->loadedData || !target->loadedData->data3D)
			return;
		auto loot = target->loadedData->data3D.get();
		if (!loot)
			return;

		RE::Actor* actor = skyrim_cast<RE::Actor*>(target);
		RE::TESRace* race = actor ? actor->GetRace() : nullptr;
		RE::TESNPC* npc = actor ? actor->GetActorBase() : nullptr;
		if (!actor || !race || !npc)
			return;

		std::string pathString = std::string(race->behaviorGraphs[npc->GetSex()].GetModel());
		std::vector<Pair> nodeNames = locationalNodeMap.count(pathString) >= 1 ? locationalNodeMap.at(pathString) : defaultNodeNames;
		if (nodeNames.empty())
			return;
		RE::BSFixedString hitNodeName;
		const float scale = actor->GetScale();
		RE::NiAVObject* obj = nullptr;
		for (auto& nodeName : nodeNames)
		{
			obj = loot->GetObjectByName(nodeName.first); 
			if (obj)
			{
				RE::NiPoint3 node_pos = obj->world.translate;
				const float d2 = hitPoint.Dot(node_pos);

				if (d2 < nodeName.second * scale * nodeName.second * scale)
				{
					hitNodeName = nodeName.first;
					break;
				}
			}
		}
		if (obj)
			PlayImpactData(target, LeftHand, obj->name);

		/*RE::NiAVObject* obj = nullptr;
		float d1 = -10000;
		RE::BSVisit::TraverseScenegraphObjects(loot, [&](RE::NiAVObject* a_object) -> RE::BSVisit::BSVisitControl {
			float d2 = hitPoint.Dot(a_object->world.translate);
			if (d2 < d1 || d1 < 0.0f)
			{
				obj = a_object;
				d1 = d2;
				if (d1 <= 20.0f * target->GetScale())
					return RE::BSVisit::BSVisitControl::kStop;
			}
			return RE::BSVisit::BSVisitControl::kContinue;
			}
		);
		if (obj)
			PlayImpactData(target, LeftHand, obj->name);*/
	}
	void ImpactDataManager_::LoadHitPlayImpactData(RE::Actor* cause, bool LeftHand)
	{
		if (!cause || !cause->loadedData || !cause->loadedData->data3D)
			return;
		auto handNode = cause->loadedData->data3D.get()->GetObjectByName(LeftHand ? HandL : HandR);
		if (!handNode)
			return;
		RE::NiAVObject* obj = nullptr;
		RE::NiPoint3 closePoint = handNode->world.translate;
		float d1 = -10000;
		RE::BSVisit::TraverseScenegraphObjects(handNode, [&](RE::NiAVObject* a_object) -> RE::BSVisit::BSVisitControl {
			float d2 = closePoint.Dot(a_object->world.translate);
			if (d2 > d1)
			{
				obj = a_object;
				d1 = d2;
				if (d1 <= 20.0f * cause->GetScale())
					return RE::BSVisit::BSVisitControl::kStop;
			}
			return RE::BSVisit::BSVisitControl::kContinue;
			}
		);
		if (obj)
			PlayImpactData(cause, LeftHand, obj->name);
	}
	void ImpactDataManager_::PlayImpactData(RE::TESObjectREFR* target, bool LeftHand, const RE::BSFixedString& nodeName, bool instance)
	{
		if (!target->loadedData || !target->loadedData->data3D)
			return;
		auto loot = target->loadedData->data3D.get();

		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		if (!g_task)
			logger::error("Couldn't get SKSETaskInterface, so switch to instance mode");
		for (auto impactData : ImpactDataSet[LeftHand])
		{
			if (Config::GetSingleton().GetInstanceMode())
				instance = true;
			TaskPlayImpactVFX* newTask = new TaskPlayImpactVFX(impactData, target, nodeName);
			if (!g_task || instance)
			{
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}

	void ImpactDataManager::Save(SKSE::SerializationInterface* serde)
	{
		if (!serde->OpenRecord(GetSingleton().ImpactDataManagerRecord, 0)) {
			logger::error("Unable to open record to write cosave data");
			return;
		}
		logger::info("Saving on cosave...");
		auto dataSize = GetSingleton().actorImpactData.size();
		serde->WriteRecordData(&dataSize, sizeof(dataSize));
		for (auto& actorMap : GetSingleton().actorImpactData) {
			auto actorid = actorMap.first;
			serde->WriteRecordData(&actorid, sizeof(actorid));
			auto datasetL = actorMap.second.GetImpactDataSet(true);
			auto datasetSizeL = datasetL.size();
			serde->WriteRecordData(&datasetSizeL, sizeof(datasetSizeL));
			for (auto& data : datasetL) {
				auto dataid = data->formID;
				serde->WriteRecordData(&dataid, sizeof(dataid));
			}
			auto datasetR = actorMap.second.GetImpactDataSet(false);
			auto datasetSizeR = datasetR.size();
			serde->WriteRecordData(&datasetSizeR, sizeof(datasetSizeR));
			for (auto& data : datasetR) {
				auto dataid = data->formID;
				serde->WriteRecordData(&dataid, sizeof(dataid));
			}
		}
	}
	void ImpactDataManager::Load(SKSE::SerializationInterface* serde, std::uint32_t type)
	{
		GetSingleton().ClearActorList();
		if (type == GetSingleton().ImpactDataManagerRecord) {
			logger::info("Loding on cosave ImpactDataManagerRecord...");
			std::size_t mapSize;
			serde->ReadRecordData(&mapSize, sizeof(mapSize));
			for (; mapSize > 0; --mapSize) {
				RE::FormID ActorID;
				serde->ReadRecordData(&ActorID, sizeof(ActorID));
				RE::FormID newActorID;
				RE::Actor* actor = nullptr;
				if (!serde->ResolveFormID(ActorID, newActorID)) {
					logger::warn("Actor ID {:X} could not be found after loading the save.", ActorID);
				}
				else
					actor = skyrim_cast<RE::Actor*>(RE::TESForm::LookupByID(newActorID));
				std::size_t datasetLSize;
				serde->ReadRecordData(&datasetLSize, sizeof(datasetLSize));
				for (; datasetLSize > 0; --datasetLSize) {
					RE::FormID ImpactDataSetID;
					serde->ReadRecordData(&ImpactDataSetID, sizeof(ImpactDataSetID));
					RE::FormID newImpactDataSetID;
					RE::BGSImpactDataSet* ImpactDataSet = nullptr;
					if (!serde->ResolveFormID(ImpactDataSetID, newImpactDataSetID)) {
						logger::warn("ImpactDataSet ID {:X} could not be found after loading the save.", ImpactDataSetID);
					}
					else
						ImpactDataSet = skyrim_cast<RE::BGSImpactDataSet*>(RE::TESForm::LookupByID(newImpactDataSetID));
					if (actor && ImpactDataSet)
						GetSingleton().AddImpactDataSet(actor, true, ImpactDataSet);
				}
				std::size_t datasetRSize;
				serde->ReadRecordData(&datasetRSize, sizeof(datasetRSize));
				for (; datasetRSize > 0; --datasetRSize) {
					RE::FormID ImpactDataSetID;
					serde->ReadRecordData(&ImpactDataSetID, sizeof(ImpactDataSetID));
					RE::FormID newImpactDataSetID;
					RE::BGSImpactDataSet* ImpactDataSet = nullptr;
					if (!serde->ResolveFormID(ImpactDataSetID, newImpactDataSetID)) {
						logger::warn("ImpactDataSet ID {:X} could not be found after loading the save.", ImpactDataSetID);
					}
					else
						ImpactDataSet = skyrim_cast<RE::BGSImpactDataSet*>(RE::TESForm::LookupByID(newImpactDataSetID));
					if (actor && ImpactDataSet)
						GetSingleton().AddImpactDataSet(actor, false, ImpactDataSet);
				}
			}
		}
	}

	void ImpactDataManager::AddImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData)
	{
		if (actor && impactData)
			Register(actor, LeftHand, impactData);
	}
	void ImpactDataManager::RemoveImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData)
	{
		if (actor && impactData)
			UnRegister(actor, LeftHand, impactData);
	}
	void ImpactDataManager::RemoveImpactDataSet(RE::Actor* actor, bool LeftHand)
	{
		if (actor)
			UnRegister(actor, LeftHand);
	}

	EventResult ImpactDataManager::ProcessEvent(const RE::TESLoadGameEvent* evn, RE::BSTEventSource<RE::TESLoadGameEvent>*)
	{
		auto p = RE::PlayerCharacter::GetSingleton();
		if (!p)
			return EventResult::kContinue;

		auto d = skyrim_cast<RE::BGSImpactDataSet*>(RE::TESForm::LookupByID(0x0201A3FB));
		if (!d)
			return EventResult::kContinue;

		Register(p, false, d);
		Register(p, true, d);
		return EventResult::kContinue;
	}
	EventResult ImpactDataManager::ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>* hh)
	{
		if (!evn || !evn->cause || !evn->target)
			return EventResult::kContinue;

		RE::Actor* cause = skyrim_cast<RE::Actor*>(evn->cause.get());
		if (!cause)
			return EventResult::kContinue;

		if (auto found = actorImpactData.find(cause->formID); found != actorImpactData.end())
		{
			if (found->second.IsVaild())
			{
				found->second.ProcessHitEvent(evn);
				logger::debug("target : {:x} {}, cause : {:x} {}", evn->target->formID, evn->target->GetName(), cause->formID, evn->cause->GetName());
			}
		}

		ImpactDataManager_ idm_ = ImpactDataManager_(cause);
		if (RE::Actor* target = skyrim_cast<RE::Actor*>(evn->target.get()); target)
		{
			for (const auto& condition : ConditionManager::GetSingleton().GetCondition(cause, target))
			{
				for (const auto& ids : condition.ImpactDataSets)
				{
					auto impact = skyrim_cast<RE::BGSImpactDataSet*>(GetFormByID(ids.id, ids.pluginName));
					if (impact)
					{
						idm_.Register(false, impact);
						idm_.Register(true, impact);
					}
				}
			}
		}
		idm_.ProcessHitEvent(evn);

		return EventResult::kContinue;
	}

	void ImpactDataManager::Register(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.Register(LeftHand, dataSet);
		}
		else
		{
			ImpactDataManager_ idm_ = ImpactDataManager_(actor);
			idm_.Register(LeftHand, dataSet);
			actorImpactData.insert(std::make_pair(actor->formID, idm_));
		}
		logger::info("Registered : ImpactdataSet {:x} {} on Actor {:x} {}", dataSet->formID, dataSet->GetFormEditorID(), actor->formID, actor->GetName());
	}
	void ImpactDataManager::UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand, dataSet);
		}
		logger::info("UnRegistered : ImpactdataSet {:x} {} on Actor {:x} {}", dataSet->formID, dataSet->GetFormEditorID(), actor->formID, actor->GetName());
	}
	void ImpactDataManager::UnRegister(RE::Actor* actor, bool LeftHand)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand);
		}
		logger::info("UnRegistered : Actor {:x} {}", actor->formID, actor->GetName());
	}
}
