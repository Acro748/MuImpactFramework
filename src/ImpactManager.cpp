#include "ImpactManager.h"

namespace Mus {
	void TaskImpactVFX::Run()
	{
		if (!mImpactData || !mTarget)
			return;
		
		auto materialType = RE::BGSMaterialType::GetMaterialType(RE::TES::GetSingleton()->GetLandMaterialType(mHitPoint));
		auto found = mImpactData->impactMap.find(materialType);
		if (found == mImpactData->impactMap.end())
		{
			logger::error("couldn't get material info {:x} on ({}, {}, {}) for {:x} {}", mImpactData->formID, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
			materialType = RE::BGSMaterialType::GetMaterialType(RE::MATERIAL_ID::kSkin);
			found = mImpactData->impactMap.find(materialType);
			if (found == mImpactData->impactMap.end())
			{
				found = mImpactData->impactMap.begin();
			}
		}
		
		auto particle = RE::BSTempEffectParticle::Spawn(mAggressor->parentCell, 0.0f, found->second->GetModel(), RE::NiMatrix3(), mHitPoint, 1.0f, 7, mTargetObj);
		if (particle)
		{
			auto processLists = RE::ProcessLists::GetSingleton();
			processLists->globalEffectsLock.Lock();
			processLists->globalTempEffects.emplace_back(particle);
			processLists->globalEffectsLock.Unlock();
			logger::debug("create ImpactVFX {:x} on ({}, {}, {}) for {:x} {}", mImpactData->formID, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
		}
		else
		{
			logger::debug("couldn't create ImpactVFX {:x} on ({}, {}, {}) for {:x} {}", mImpactData->formID, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
			return;
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
			found->second->sound1 ? &handle1 : nullptr,
			found->second->sound2 ? &handle2 : nullptr,
			found->second->sound1 ? true : false,
			found->second->sound2 ? true : false
		};
		RE::BGSImpactManager::GetSingleton()->PlayImpactDataSounds(sound);
	}
	void TaskImpactVFX::Dispose()
	{
		delete this;
	}
	
	void TaskVFX::Run()
	{
		if (mVFXPath.empty())
			return;

		bool isCreated = false;
		switch (mVFXType) {
		case VFXType::Impact:
			isCreated = CreateImpactVFX();
			break;
		case VFXType::Spell:
			isCreated = CreateArtVFX();
			break;
		}

		if (isCreated)
			logger::debug("create VFX {} on ({}, {}, {}) for {:x} {}", mVFXPath, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
		else
			logger::debug("couldn't VFX {} on ({}, {}, {}) for {:x} {}", mVFXPath, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
	}
	void TaskVFX::Dispose()
	{
		delete this;
	}
	bool TaskVFX::CreateImpactVFX()
	{
		auto particle = RE::BSTempEffectParticle::Spawn(mAggressor->parentCell, 0.0f, mVFXPath.c_str(), RE::NiMatrix3(), mHitPoint, 1.0f, 7, mTargetObj);
		if (particle)
		{
			auto processLists = RE::ProcessLists::GetSingleton();
			if (processLists)
			{
				RE::BSSpinLockGuard locker(processLists->globalEffectsLock);
				processLists->globalTempEffects.emplace_back(particle);
				return true;
			}
		}
		return false;
	}
	bool TaskVFX::CreateArtVFX()
	{
		return false;
	}

	void TaskCastVFX::Run()
	{
		if (!mSpell || !mAggressor || !mTarget)
			return;

		Cast(nullptr, 0, mSpell, mAggressor, mTarget);

		//RE::BGSArtObject* spell_art = mSpell->effects[0]->baseEffect->data.hitEffectArt;
		//if (!spell_art)
		//	return;
		//RE::ModelReferenceEffect* hitEffect = nullptr;
		//if (const auto processLists = RE::ProcessLists::GetSingleton(); processLists) {
		//	const auto handle = mTarget->CreateRefHandle();
		//	processLists->ForEachModelEffect([&](RE::ModelReferenceEffect& a_modelEffect) {
		//		if (a_modelEffect.target == handle && a_modelEffect.artObject == spell_art) {
		//			hitEffect = &a_modelEffect;
		//			return RE::BSContainer::ForEachResult::kStop;
		//		}
		//		return RE::BSContainer::ForEachResult::kContinue;
		//		});
		//}

		//logger::info("{} hitEffect {:x} on {:x} {}", hitEffect ? "Found" : "Not found", mSpell->formID, mTarget->formID, mTarget->GetName());
	}
	void TaskCastVFX::Dispose()
	{
		delete this;
	}
	void TaskCastVFX::Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target)
	{
		using func_t = decltype(&TaskCastVFX::Cast);
		REL::VariantID offset(55149, 55747, 0x930EC0);
		REL::Relocation<func_t> func{ offset };
		return func(VMinternal, stackId, spell, aggressor, target);
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

	void ImpactManager_impl::Register(bool LeftHand, std::string VFXPath, std::uint8_t VFXType)
	{
		VFXPath = lowLetter(VFXPath);
		if (auto found = VFX[LeftHand].find(VFXPath); found != VFX[LeftHand].end())
			return;
		VFX[LeftHand].emplace(VFXPath, VFXType);
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, std::string VFXPath)
	{
		VFXPath = lowLetter(VFXPath);
		if (auto found = VFX[LeftHand].find(VFXPath); found != VFX[LeftHand].end())
			VFX[LeftHand].erase(found);
	}

	void ImpactManager_impl::UnRegister(bool LeftHand, uint32_t type)
	{
		if (type & ImpactManager::Type::ImpactDataSet)
			ImpactDataSet[LeftHand].clear();
		if (type & ImpactManager::Type::Spell)
			Spell[LeftHand].clear();
		if (type & ImpactManager::Type::VFX)
			VFX[LeftHand].clear();
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
				LoadHitPlayImpactData(evn->cause.get(), evn->target.get(), LeftHand, hitData->hitPosition);
			}
		}
		else if (!target && Config::GetSingleton().GetEnableInanimateObject()) //for inanimate object
		{
			logger::debug("target : {:x}, aggressor : {:x} {} => Find HitPosition on attack to Inanimate object...", evn->target->formID, evn->cause->formID, evn->cause->GetName());
			LoadHitPlayImpactData(cause, evn->target.get(), LeftHand);
		}
		else if (evn->projectile != 0 && Config::GetSingleton().GetEnableMagic() && evn->cause.get()->loadedData && evn->cause.get()->loadedData->data3D) //for magic
		{
			logger::debug("target : {:x} {}, aggressor : {:x} {} => Find HitPosition on spell attack.....", evn->target->formID, evn->target->GetName(), evn->cause->formID, evn->cause->GetName());
			LoadHitPlayImpactData(cause, evn->target.get(), LeftHand);
		}
		CastSpell(evn->target.get(), evn->target.get(), LeftHand);
	}
	void ImpactManager_impl::LoadHitPlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint)
	{
		//RE::NiAVObject* targetObj = aggressor->loadedData->data3D->GetObjectByName(LeftHand ? "NPC L Hand [LHnd]" : "NPC R Hand [RHnd]");
		PlayImpactData(aggressor, target, LeftHand, hitPoint);
		PlayVFX(aggressor, target, LeftHand, hitPoint);
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
			PlayImpactData(cause, target, LeftHand, obj->world.translate);
	}
	void ImpactManager_impl::PlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (auto impactData : ImpactDataSet[LeftHand])
		{
			TaskImpactVFX* newTask = new TaskImpactVFX(impactData, aggressor, target, hitPoint, targetObj);
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
	void ImpactManager_impl::CastSpell(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (auto spell : Spell[LeftHand])
		{
			TaskCastVFX* newTask = new TaskCastVFX(spell, aggressor, target);
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
	void ImpactManager_impl::PlayVFX(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (auto VFX : VFX[LeftHand])
		{
			TaskVFX* newTask = new TaskVFX(VFX.first, aggressor, target, hitPoint, VFX.second, targetObj);
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
		else if (serde->OpenRecord(GetSingleton().ImpactManagerVFX, 0)) {
			logger::info("Saving on cosave for ImpactManagerVFX...");
			auto dataSize = GetSingleton().actorImpactData.size();
			serde->WriteRecordData(&dataSize, sizeof(dataSize));
			for (auto& actorMap : GetSingleton().actorImpactData) {
				auto actorid = actorMap.first;
				serde->WriteRecordData(&actorid, sizeof(actorid));
				auto VFXL = actorMap.second.GetVFX(true);
				auto VFXSizeL = VFXL.size();
				serde->WriteRecordData(&VFXSizeL, sizeof(VFXSizeL));
				for (auto& VFX : VFXL) {
					auto VFXSize = VFX.first.size();
					serde->WriteRecordData(&VFXSize, sizeof(VFXSize));
					for (auto& c : VFX.first)
					{
						serde->WriteRecordData(&c, sizeof(c));
					}
					auto VFXType = VFX.second;
					serde->WriteRecordData(&VFXType, sizeof(VFXType));
				}
				auto VFXR = actorMap.second.GetVFX(false);
				auto VFXSizeR = VFXR.size();
				serde->WriteRecordData(&VFXSizeR, sizeof(VFXSizeR));
				for (auto& VFX : VFXR) {
					auto VFXSize = VFX.first.size();
					serde->WriteRecordData(&VFXSize, sizeof(VFXSize));
					for (auto& c : VFX.first)
					{
						serde->WriteRecordData(&c, sizeof(c));
					}
					auto VFXType = VFX.second;
					serde->WriteRecordData(&VFXType, sizeof(VFXType));
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
		else if (type == GetSingleton().ImpactManagerVFX) {
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
				std::size_t VFXLSize;
				serde->ReadRecordData(&VFXLSize, sizeof(VFXLSize));
				for (; VFXLSize > 0; --VFXLSize) {
					std::size_t VFXSize;
					serde->ReadRecordData(&VFXSize, sizeof(VFXSize));
					std::string VFX;
					for (; VFXSize > 0; --VFXSize)
					{
						char c;
						serde->ReadRecordData(&c, sizeof(c));
						VFX += c;
					}
					std::uint8_t VFXType;
					serde->ReadRecordData(&VFXType, sizeof(VFXType));
					if (actor)
						GetSingleton().AddVFX(actor, true, VFX, VFXType);
				}
				std::size_t VFXRSize;
				serde->ReadRecordData(&VFXRSize, sizeof(VFXRSize));
				for (; VFXRSize > 0; --VFXRSize) {
					std::size_t VFXSize;
					serde->ReadRecordData(&VFXSize, sizeof(VFXSize));
					std::string VFX;
					for (; VFXSize > 0; --VFXSize)
					{
						char c;
						serde->ReadRecordData(&c, sizeof(c));
						VFX += c;
					}
					std::uint8_t VFXType;
					serde->ReadRecordData(&VFXType, sizeof(VFXType));
					if (actor)
						GetSingleton().AddVFX(actor, true, VFX, VFXType);
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

	void ImpactManager::AddVFX(RE::Actor* actor, bool LeftHand, std::string VFXPath, std::uint8_t VFXType)
	{
		if (actor && !VFXPath.empty())
			Register(actor, LeftHand, VFXPath, VFXType);
	}
	void ImpactManager::RemoveVFX(RE::Actor* actor, bool LeftHand, std::string VFXPath)
	{
		if (actor && !VFXPath.empty())
			UnRegister(actor, LeftHand, VFXPath);
	}
	void ImpactManager::RemoveVFX(RE::Actor* actor, bool LeftHand)
	{
		if (actor)
			UnRegister(actor, LeftHand, Type::VFX);
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
				logger::debug("target : {:x} {}, aggressor : {:x} {}", evn->target.get()->formID, evn->target.get()->GetName(), cause->formID, evn->cause.get()->GetName());
			}
		}

		std::shared_ptr<ImpactManager_impl> idm_;
		if (auto found = conditionActorImpactData.find(cause->formID); found == conditionActorImpactData.end())
		{
			idm_ = found->second;
		}
		else
		{
			idm_ = std::make_shared<ImpactManager_impl>(cause);
			conditionActorImpactData.insert(idm_);
		}
		for (const auto& condition : ConditionManager::GetSingleton().GetCondition(evn))
		{
			for (const auto& ids : condition.ImpactDataSets)
			{
				auto impact = skyrim_cast<RE::BGSImpactDataSet*>(GetFormByID(ids.id, ids.pluginName));
				if (impact)
				{
					idm_->Register(false, impact);
					idm_->Register(true, impact);
				}
			}
			for (const auto& ids : condition.SpellItems)
			{
				auto spell = skyrim_cast<RE::SpellItem*>(GetFormByID(ids.id, ids.pluginName));
				if (spell)
				{
					idm_->Register(false, spell);
					idm_->Register(true, spell);
				}
			}
			for (const auto& ids : condition.VFXItems)
			{
				if (!ids.vfxPath.empty())
				{
					idm_->Register(false, ids.vfxPath, ids.vfxType);
					idm_->Register(true, ids.vfxPath, ids.vfxType);
				}
			}
		}
		idm_->ProcessHitEvent(evn);

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
	void ImpactManager::Register(RE::Actor* actor, bool LeftHand, std::string VFXPath, std::uint8_t VFXType)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.Register(LeftHand, VFXPath, VFXType);
		}
		else
		{
			ImpactManager_impl idm_ = ImpactManager_impl(actor);
			idm_.Register(LeftHand, VFXPath, VFXType);
			actorImpactData.insert(std::make_pair(actor->formID, idm_));
		}
		logger::info("Registered : VFX {} on Actor {:x} {}", VFXPath, actor->formID, actor->GetName());
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
	void ImpactManager::UnRegister(RE::Actor* actor, bool LeftHand, std::string VFXPath)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand, VFXPath);
		}
		logger::info("UnRegistered : VFX {} on Actor {:x} {}", VFXPath, actor->formID, actor->GetName());
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
