#include "ImpactManager.h"

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
			if (!PlayImpactEffect(vm->impl.get(), 0, mObj, mImpactData, mNodeName.c_str(),
				mDirect.x, mDirect.y, mDirect.z, 0.0f, false, false))
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
			if (!impactManager->PlayImpactEffect(mObj, mImpactData, mNodeName.c_str(),
				mDirect, 0.0f, false, false))
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
	bool TaskPlayImpactVFX::PlayImpactEffect(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, 
		RE::TESObjectREFR* a_ref, RE::BGSImpactDataSet* a_impactEffect, const RE::BSFixedString& a_nodeName, 
		float a_pickDirection_x, float a_pickDirection_y, float a_pickDirection_z,
		float a_pickLength, bool a_applyNodeRotation, bool a_useNodeLocalRotation)
	{
		using func_t = decltype(&TaskPlayImpactVFX::PlayImpactEffect);
		REL::VariantID offset(55677, 56208, 0x009D06C0);
		REL::Relocation<func_t> func{ offset };
		return func(VMinternal, stackId, a_ref, a_impactEffect, a_nodeName, a_pickDirection_x, a_pickDirection_y, a_pickDirection_z,
			a_pickLength, a_applyNodeRotation, a_useNodeLocalRotation);
	}

	void TaskSpwanVFX::Run()
	{
		RE::Actor* actor = skyrim_cast<RE::Actor*>(mObj);
		if (!actor || !actor->loadedData || !actor->loadedData->data3D || !mObj->parentCell)
			return;
		RE::TESRace* race = actor->GetRace();
		if (!race || !race->bloodImpactMaterial)
			return;
		auto found = mImpactData->impactMap.find(race->bloodImpactMaterial);
		if (found == mImpactData->impactMap.end())
			return;
		auto data = found->second;
		//for (uint32_t i = 0; i < 100; i++)
		//{
			auto tep = RE::BSTempEffectParticle::Spawn(mObj->parentCell, 10000, "meshes\\effects\\impacteffects\\AP05\\SLASHEFFECT.nif", emptyPoint, mHitPoint, 10.0f, 0, actor->loadedData->data3D.get()->GetObjectByName(mNodeName));
			tep->impactData = data;
			logger::info("Spawn {}", tep ? "done" : "fail");
			if (tep)
				logger::info("particle {} / spawn {}", tep->particleObject ? tep->particleObject.get()->name.c_str() : "null"
					, tep->spawnNode ? tep->spawnNode.get()->name.c_str() : "null");
			tep = RE::BSTempEffectParticle::Spawn(mObj->parentCell, 10000, "Data\\meshes\\effects\\impacteffects\\AP05\\SLASHEFFECT.nif", emptyPoint, mHitPoint, 10.0f, 0, actor->loadedData->data3D.get()->GetObjectByName(mNodeName));
			tep->impactData = data;
			logger::info("Spawn {}", tep ? "done" : "fail");
			if (tep)
				logger::info("particle {} / spawn {}", tep->particleObject ? tep->particleObject.get()->name.c_str() : "null"
					, tep->spawnNode ? tep->spawnNode.get()->name.c_str() : "null");
			tep = RE::BSTempEffectParticle::Spawn(mObj->parentCell, 10000, "effects\\impacteffects\\AP05\\SLASHEFFECT", emptyPoint, mHitPoint, 10.0f, 0, actor->loadedData->data3D.get()->GetObjectByName(mNodeName));
			tep->impactData = data;
			logger::info("Spawn {}", tep ? "done" : "fail");
			if (tep)
				logger::info("particle {} / spawn {}", tep->particleObject ? tep->particleObject.get()->name.c_str() : "null"
					, tep->spawnNode ? tep->spawnNode.get()->name.c_str() : "null");
			tep = RE::BSTempEffectParticle::Spawn(mObj->parentCell, 10000, "meshes\\effects\\impacteffects\\AP05\\SLASHEFFECT", emptyPoint, mHitPoint, 10.0f, 0, actor->loadedData->data3D.get()->GetObjectByName(mNodeName));
			tep->impactData = data;
			logger::info("Spawn {}", tep ? "done" : "fail");
			if (tep)
				logger::info("particle {} / spawn {}", tep->particleObject ? tep->particleObject.get()->name.c_str() : "null"
					, tep->spawnNode ? tep->spawnNode.get()->name.c_str() : "null");
			tep = RE::BSTempEffectParticle::Spawn(mObj->parentCell, 10000, "impacteffects\\AP05\\SLASHEFFECT", emptyPoint, mHitPoint, 10.0f, 0, actor->loadedData->data3D.get()->GetObjectByName(mNodeName));
			tep->impactData = data;
			logger::info("Spawn {}", tep ? "done" : "fail");
			if (tep)
				logger::info("particle {} / spawn {}", tep->particleObject ? tep->particleObject.get()->name.c_str() : "null"
					, tep->spawnNode ? tep->spawnNode.get()->name.c_str() : "null");
		//}
	}
	void TaskSpwanVFX::Dispose()
	{
		delete this;
	}

	void TaskCast::Run()
	{
		auto vm = RE::SkyrimVM::GetSingleton();
		if (!vm || !vm->impl)
			return;
		Cast(vm->impl.get(), 0, mSpell, mSource, mTarget);
	}
	void TaskCast::Dispose()
	{
		delete this;
	}
	void TaskCast::Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* source, RE::TESObjectREFR* target)
	{
		if (REL::Module::IsVR())
			return;
		using func_t = decltype(&TaskCast::Cast);
		REL::VariantID offset(55149, 55747, 0x0);
		REL::Relocation<func_t> func{ offset };
		return func(VMinternal, stackId, spell, source, target);
	}

	void ImpactManager_impl::Register(bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = std::find(ImpactDataSet[LeftHand].begin(), ImpactDataSet[LeftHand].end(), dataSet); found != ImpactDataSet[LeftHand].end())
			return;
		ImpactDataSet[LeftHand].emplace_back(dataSet);
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = std::find(ImpactDataSet[LeftHand].begin(), ImpactDataSet[LeftHand].end(), dataSet); found != ImpactDataSet[LeftHand].end())
			ImpactDataSet[LeftHand].erase(found);
	}
	void ImpactManager_impl::Register(bool LeftHand, RE::SpellItem* spell)
	{
		if (auto found = std::find(Spell[LeftHand].begin(), Spell[LeftHand].end(), spell); found != Spell[LeftHand].end())
			return;
		Spell[LeftHand].emplace_back(spell);
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, RE::SpellItem* spell)
	{
		if (auto found = std::find(Spell[LeftHand].begin(), Spell[LeftHand].end(), spell); found != Spell[LeftHand].end())
			Spell[LeftHand].erase(found);
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, uint32_t type)
	{
		if (type & ImpactManager::Type::ImpactDataSet)
			ImpactDataSet[LeftHand].clear();
		if (type & ImpactManager::Type::Spell)
			Spell[LeftHand].clear();
	}

	void ImpactManager_impl::ProcessHitEvent(const RE::TESHitEvent* evn)
	{
		if (!evn->cause || !evn->target)
			return;
		RE::Actor* cause = skyrim_cast<RE::Actor*>(evn->cause.get());
		if (!cause)
			return;
		bool LeftHand = false;
		auto* cause_aiprocess = cause->GetActorRuntimeData().currentProcess;
		if (cause_aiprocess && cause_aiprocess->high && cause_aiprocess->high->attackData) {
			LeftHand = cause_aiprocess->high->attackData->IsLeftAttack();
		}
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
				LoadHitPlayImpactData(evn->target.get(), LeftHand, hitData->hitPosition, RE::NiPoint3(0.0f, 0.0f, 50.0f));
			}
		}
		else
		{
			logger::debug("Find node by HandNodePosition on causeActor...");
			LoadHitPlayImpactData(cause, LeftHand);
		}
		CastSpell(evn->target.get(), evn->target.get(), LeftHand);
	}
	void ImpactManager_impl::LoadHitPlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDir)
	{
		if (!target || !target->loadedData || !target->loadedData->data3D)
			return;
		auto root = target->loadedData->data3D.get();
		if (!root)
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
		float d1 = -10000;

		if (Config::GetSingleton().GetNodeDiscoveryMode() == 0)
		{
			for (auto& nodeName : nodeNames)
			{
				obj = root->GetObjectByName(nodeName.first);
				if (obj)
				{
					RE::NiPoint3 node_pos = obj->world.translate;
					const float d2 = hitPoint.Dot(node_pos);
					if (d1 < 0 || d2 < d1)
					{
						hitNodeName = nodeName.first;
						d1 = d2;
					}
					/*if (d2 < nodeName.second * scale * nodeName.second * scale)
					{
						hitNodeName = nodeName.first;
						break;
					}*/
				}
			}
		}
		else if (Config::GetSingleton().GetNodeDiscoveryMode() == 1)
		{
			RE::BSVisit::TraverseScenegraphObjects(root, [&](RE::NiAVObject* a_object) -> RE::BSVisit::BSVisitControl {
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
		}
		if (obj)
			PlayImpactData(target, LeftHand, obj->name, hitDir);
	}
	void ImpactManager_impl::LoadHitPlayImpactData(RE::Actor* cause, bool LeftHand)
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
				/*if (d1 <= 20.0f * cause->GetScale())
					return RE::BSVisit::BSVisitControl::kStop;*/
			}
			return RE::BSVisit::BSVisitControl::kContinue;
			}
		);
		if (obj)
			PlayImpactData(cause, LeftHand, obj->name);
	}
	void ImpactManager_impl::PlayImpactData(RE::TESObjectREFR* target, bool LeftHand, const RE::BSFixedString& nodeName, RE::NiPoint3 hitDir, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		if (!g_task)
			logger::error("Couldn't get SKSETaskInterface, so switch to instance mode");
		for (auto impactData : ImpactDataSet[LeftHand])
		{
			TaskPlayImpactVFX* newTask = new TaskPlayImpactVFX(impactData, target, nodeName.c_str(), hitDir);
			if (!g_task || instance || Config::GetSingleton().GetInstanceMode())
			{
				//logger::info("instance");
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}
	void ImpactManager_impl::CastSpell(RE::TESObjectREFR* source, RE::TESObjectREFR* target, bool LeftHand, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		if (!g_task)
			logger::error("Couldn't get SKSETaskInterface, so switch to instance mode");
		for (auto spell : Spell[LeftHand])
		{
			TaskCast* newTask = new TaskCast(spell, source, target);
			if (!g_task || instance || Config::GetSingleton().GetInstanceMode())
			{
				//logger::info("instance");
				newTask->Run();
				newTask->Dispose();
			}
			else
			{
				g_task->AddTask(newTask);
			}
		}
	}

	void ImpactManager::Save(SKSE::SerializationInterface* serde)
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
	void ImpactManager::Load(SKSE::SerializationInterface* serde, std::uint32_t type)
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

	void ImpactManager::AddImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData)
	{
		if (actor && impactData)
			Register(actor, LeftHand, impactData);
	}
	void ImpactManager::RemoveImpactDataSet(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* impactData)
	{
		if (actor && impactData)
			UnRegister(actor, LeftHand, impactData);
	}
	void ImpactManager::RemoveImpactDataSet(RE::Actor* actor, bool LeftHand)
	{
		if (actor)
			UnRegister(actor, LeftHand, Type::ImpactDataSet);
	}
	void ImpactManager::AddSpell(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell)
	{
		if (actor && spell)
			Register(actor, LeftHand, spell);
	}
	void ImpactManager::RemoveSpell(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell)
	{
		if (actor && spell)
			UnRegister(actor, LeftHand, spell);
	}
	void ImpactManager::RemoveSpell(RE::Actor* actor, bool LeftHand)
	{
		if (actor)
			UnRegister(actor, LeftHand, Type::Spell);
	}

	EventResult ImpactManager::ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>* hh)
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

		ImpactManager_impl idm_ = ImpactManager_impl(cause);
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
				for (const auto& ids : condition.SpellItems)
				{
					auto spell = skyrim_cast<RE::SpellItem*>(GetFormByID(ids.id, ids.pluginName));
					if (spell)
					{
						idm_.Register(false, spell);
						idm_.Register(true, spell);
					}
				}
			}
		}
		idm_.ProcessHitEvent(evn);

		return EventResult::kContinue;
	}

	void ImpactManager::Register(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.Register(LeftHand, dataSet);
		}
		else
		{
			ImpactManager_impl idm_ = ImpactManager_impl(actor);
			idm_.Register(LeftHand, dataSet);
			actorImpactData.insert(std::make_pair(actor->formID, idm_));
		}
		logger::info("Registered : ImpactdataSet {:x} {} on Actor {:x} {}", dataSet->formID, dataSet->GetFormEditorID(), actor->formID, actor->GetName());
	}
	void ImpactManager::Register(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.Register(LeftHand, spell);
		}
		else
		{
			ImpactManager_impl idm_ = ImpactManager_impl(actor);
			idm_.Register(LeftHand, spell);
			actorImpactData.insert(std::make_pair(actor->formID, idm_));
		}
		logger::info("Registered : Spell {:x} {} on Actor {:x} {}", spell->formID, spell->GetFormEditorID(), actor->formID, actor->GetName());
	}
	void ImpactManager::UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand, dataSet);
		}
		logger::info("UnRegistered : ImpactdataSet {:x} {} on Actor {:x} {}", dataSet->formID, dataSet->GetFormEditorID(), actor->formID, actor->GetName());
	}
	void ImpactManager::UnRegister(RE::Actor* actor, bool LeftHand, RE::SpellItem* spell)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand, spell);
		}
		logger::info("UnRegistered : Spell {:x} {} on Actor {:x} {}", spell->formID, spell->GetFormEditorID(), actor->formID, actor->GetName());
	}
	void ImpactManager::UnRegister(RE::Actor* actor, bool LeftHand, Type type)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand, type);
		}
		logger::info("UnRegistered : Actor {:x} {}", actor->formID, actor->GetName());
	}
}
