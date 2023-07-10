#include "ImpactManager.h"

namespace Mus {
	void TaskPlayImpactVFX::Run()
	{
		if (!mImpactData || !mTarget)
		{
			return;
		}

		auto materialType = RE::BGSMaterialType::GetMaterialType(RE::TES::GetSingleton()->GetLandMaterialType(mHitPoint));
		auto found = mImpactData->impactMap.find(materialType);
		if (found == mImpactData->impactMap.end())
		{
			logger::error("Couldn't get material info {:x} on ({}, {}, {}) for {:x} {}", mImpactData->formID, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
			return;
		}
		auto particle = RE::BSTempEffectParticle::Spawn(mTarget->parentCell, 0.0f, found->second->GetModel(), RE::NiMatrix3(), mHitPoint, 1.0f, 7, nullptr);
		if (particle)
		{
			auto processLists = RE::ProcessLists::GetSingleton();
			processLists->globalEffectsLock.Lock();
			processLists->globalTempEffects.emplace_back(particle);
			processLists->globalEffectsLock.Unlock();
		}

		RE::BSSoundHandle handle1, handle2;
		if (found->second->sound1)
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(handle1, found->second->sound1);
		if (found->second->sound2)
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(handle2, found->second->sound2);
		RE::BGSImpactManager::ImpactSoundData sound{
			found->second,
			&mHitPoint,
			nullptr,
			&handle1,
			&handle2,
			found->second->sound1 ? true : false,
			found->second->sound2 ? true : false
		};
		RE::BGSImpactManager::GetSingleton()->PlayImpactDataSounds(sound);

		logger::debug("create ImpactVFX {:x} on ({}, {}, {}) for {:x} {}", mImpactData->formID, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
	}
	void TaskPlayImpactVFX::Dispose()
	{
		delete this;
	}
	
	void TaskCast::Run()
	{
		Cast(nullptr, 0, mSpell, mSource, mTarget);
	}
	void TaskCast::Dispose()
	{
		delete this;
	}
	void TaskCast::Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* source, RE::TESObjectREFR* target)
	{
		using func_t = decltype(&TaskCast::Cast);
		REL::VariantID offset(55149, 55747, 0x930EC0);
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
		if (target_aiprocess && target_aiprocess->middleHigh && target_aiprocess->middleHigh->lastHitData)
		{
			auto hitData = target_aiprocess->middleHigh->lastHitData;
			if (hitData)
			{
				logger::debug("Find node by HitPosition...");
				LoadHitPlayImpactData(evn->target.get(), LeftHand, hitData->hitPosition);
			}
		}
		else if (!target && Config::GetSingleton().GetEnableInanimateObject()) //for inanimate object
		{
			logger::debug("Find node by HandNodePosition on causeActor...");
			LoadHitPlayImpactData(cause, evn->target.get(), LeftHand);
		}
		else if (evn->projectile != 0 && Config::GetSingleton().GetEnableMagic() && evn->cause.get()->loadedData && evn->cause.get()->loadedData->data3D) //for magic
		{
			auto root = evn->cause.get()->loadedData->data3D.get();
			LoadHitPlayImpactData(cause, evn->target.get(), LeftHand);
		}
		CastSpell(evn->target.get(), evn->target.get(), LeftHand);
	}
	void ImpactManager_impl::LoadHitPlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint)
	{
		PlayImpactData(target, LeftHand, hitPoint);
	}
	void ImpactManager_impl::LoadHitPlayImpactData(RE::Actor* cause, RE::TESObjectREFR* target, bool LeftHand)
	{
		if (!cause || !cause->loadedData || !cause->loadedData->data3D)
			return;
		if (!target || !target->loadedData || !target->loadedData->data3D)
			return;
		auto handNode = cause->loadedData->data3D.get()->GetObjectByName(LeftHand ? HandL : HandR);
		if (!handNode)
			return;
		RE::NiAVObject* root = handNode;
		RE::NiAVObject* obj = root;
		RE::NiPoint3 closePoint = target->loadedData->data3D.get()->world.translate;
		float d1 = -10000;
		RE::BSVisit::TraverseScenegraphObjects(root, [&](RE::NiAVObject* a_object) -> RE::BSVisit::BSVisitControl {
			float d2 = closePoint.Dot(a_object->world.translate);
			if (d2 > d1)
			{
				obj = a_object;
				d1 = d2;
			}
			return RE::BSVisit::BSVisitControl::kContinue;
			}
		);
		if (obj)
			PlayImpactData(target, LeftHand, obj->world.translate);
	}
	void ImpactManager_impl::PlayImpactData(RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (auto impactData : ImpactDataSet[LeftHand])
		{
			TaskPlayImpactVFX* newTask = new TaskPlayImpactVFX(impactData, target, hitPoint);
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
	void ImpactManager_impl::CastSpell(RE::TESObjectREFR* source, RE::TESObjectREFR* target, bool LeftHand, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (auto spell : Spell[LeftHand])
		{
			TaskCast* newTask = new TaskCast(spell, source, target);
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

	void ImpactManager::Save(SKSE::SerializationInterface* serde)
	{
		if (serde->OpenRecord(GetSingleton().ImpactManagerImpactData, 0)) {
			logger::info("Saving on cosave for ImpactManagerImpactData...");
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
		else if (serde->OpenRecord(GetSingleton().ImpactManagerSpell, 0)) {
			logger::info("Saving on cosave for ImpactManagerSpell...");
			auto dataSize = GetSingleton().actorImpactData.size();
			serde->WriteRecordData(&dataSize, sizeof(dataSize));
			for (auto& actorMap : GetSingleton().actorImpactData) {
				auto actorid = actorMap.first;
				serde->WriteRecordData(&actorid, sizeof(actorid));
				auto spellL = actorMap.second.GetSpell(true);
				auto spellSizeL = spellL.size();
				serde->WriteRecordData(&spellSizeL, sizeof(spellSizeL));
				for (auto& spell : spellL) {
					auto spellid = spell->formID;
					serde->WriteRecordData(&spellid, sizeof(spellid));
				}
				auto spellR = actorMap.second.GetSpell(false);
				auto spellSizeR = spellR.size();
				serde->WriteRecordData(&spellSizeR, sizeof(spellSizeR));
				for (auto& spell : spellR) {
					auto spellid = spell->formID;
					serde->WriteRecordData(&spellid, sizeof(spellid));
				}
			}
		}
	}
	void ImpactManager::Load(SKSE::SerializationInterface* serde, std::uint32_t type)
	{
		GetSingleton().ClearActorList();
		if (type == GetSingleton().ImpactManagerImpactData) {
			logger::info("Loding on cosave ImpactManagerImpactData...");
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
		else if (type == GetSingleton().ImpactManagerSpell) {
			logger::info("Loding on cosave ImpactManagerSpell...");
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
				std::size_t spellLSize;
				serde->ReadRecordData(&spellLSize, sizeof(spellLSize));
				for (; spellLSize > 0; --spellLSize) {
					RE::FormID SpellID;
					serde->ReadRecordData(&SpellID, sizeof(SpellID));
					RE::FormID newSpellID;
					RE::SpellItem* Spell = nullptr;
					if (!serde->ResolveFormID(SpellID, newSpellID)) {
						logger::warn("Spell ID {:X} could not be found after loading the save.", SpellID);
					}
					else
						Spell = skyrim_cast<RE::SpellItem*>(RE::TESForm::LookupByID(newSpellID));
					if (actor)
						GetSingleton().AddSpell(actor, true, Spell);
				}
				std::size_t spellRSize;
				serde->ReadRecordData(&spellRSize, sizeof(spellRSize));
				for (; spellRSize > 0; --spellRSize) {
					RE::FormID SpellID;
					serde->ReadRecordData(&SpellID, sizeof(SpellID));
					RE::FormID newSpellID;
					RE::SpellItem* Spell = nullptr;
					if (!serde->ResolveFormID(SpellID, newSpellID)) {
						logger::warn("Spell ID {:X} could not be found after loading the save.", SpellID);
					}
					else
						Spell = skyrim_cast<RE::SpellItem*>(RE::TESForm::LookupByID(newSpellID));
					if (actor)
						GetSingleton().AddSpell(actor, false, Spell);
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

	EventResult ImpactManager::ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>*)
	{
		if (!evn || !evn->cause || !evn->target)
			return EventResult::kContinue;

		if (!IsValidHitEvent(evn))
			return EventResult::kContinue;;

		RE::Actor* cause = skyrim_cast<RE::Actor*>(evn->cause.get());
		if (!cause)
			return EventResult::kContinue;

		if (auto found = actorImpactData.find(cause->formID); found != actorImpactData.end())
		{
			if (found->second.IsVaild())
			{
				found->second.ProcessHitEvent(evn);
				logger::debug("target : {:x} {}, cause : {:x} {}", evn->target.get()->formID, evn->target.get()->GetName(), cause->formID, evn->cause.get()->GetName());
			}
		}

		ImpactManager_impl idm_ = ImpactManager_impl(cause);
		for (const auto& condition : ConditionManager::GetSingleton().GetCondition(evn))
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
