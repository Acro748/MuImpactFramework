#include "Tasks.h"

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
			
			auto closeObj = GetObjectByDistance();
			if (closeObj)
				materialType = RE::BGSMaterialType::GetMaterialType(RE::TES::GetSingleton()->GetLandMaterialType(closeObj->world.translate));
			found = mImpactData->impactMap.find(materialType);
			if (found == mImpactData->impactMap.end())
			{
				materialType = RE::BGSMaterialType::GetMaterialType(RE::MATERIAL_ID::kSkin);
				found = mImpactData->impactMap.find(materialType);
				if (found == mImpactData->impactMap.end())
				{
					found = mImpactData->impactMap.begin();
				}
			}
		}

		auto particle = RE::BSTempEffectParticle::Spawn(mAggressor->parentCell, 0.0f, found->second->GetModel(), mHitDirection, mHitPoint, 1.0f, 7, mTargetObj);
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
	RE::NiAVObject* TaskImpactVFX::GetObjectByDistance()
	{
		if (!mTarget || !mTarget->loadedData || !mTarget->loadedData->data3D)
			return nullptr;
		RE::NiAVObject* root = mTarget->loadedData->data3D.get();
		RE::NiAVObject* obj = root;
		float d1 = 10000;
		RE::BSVisit::TraverseScenegraphObjects(root, [&](RE::NiAVObject* a_object) -> RE::BSVisit::BSVisitControl {
			float d2 = mHitPoint.GetSquaredDistance(a_object->world.translate);
			if (d2 < d1)
			{
				obj = a_object;
				d1 = d2;
			}
			return RE::BSVisit::BSVisitControl::kContinue;
			}
		);
		return obj;
	}

	RE::BGSImpactData* TaskTempFormManager::GetImpactDataTempForm()
	{
		std::lock_guard<std::mutex> locker(m_lock);
		static std::vector<RE::BGSImpactData*> impactDataList;
		if (impactDataList.size() == 0)
		{
			auto handler = RE::TESDataHandler::GetSingleton();
			for (std::uint32_t i = 0; i < Config::GetSingleton().GetSoundLimit(); i++)
			{
				auto impactData = GetNewForm<RE::BGSImpactData>();
				std::string newEditorID = "MIFTempImpactData";
				newEditorID += std::to_string(i);
				impactData->SetFormEditorID(newEditorID.c_str());
				impactDataList.emplace_back(impactData);
				handler->GetFormArray<RE::BGSImpactData>().emplace_back(impactData);
			}
		}
		if (ImpactDataNum >= impactDataList.size())
			ImpactDataNum = 0;
		auto result = impactDataList.at(ImpactDataNum);
		ImpactDataNum++;
		return result;
	}
	RE::BGSArtObject* TaskTempFormManager::GetArtObjectTempForm()
	{
		std::lock_guard<std::mutex> locker(m_lock);
		static std::vector<RE::BGSArtObject*> artObjectList;
		if (artObjectList.size() == 0)
		{
			auto handler = RE::TESDataHandler::GetSingleton();
			for (std::uint32_t i = 0; i < Config::GetSingleton().GetArtObjectVFXLimit(); i++)
			{
				auto artObject = GetNewForm<RE::BGSArtObject>();
				std::string newEditorID = "MIFTempArtObject";
				newEditorID += std::to_string(i);
				artObject->SetFormEditorID(newEditorID.c_str());
				artObjectList.emplace_back(artObject);
				handler->GetFormArray<RE::BGSArtObject>().emplace_back(artObject);
			}
		}
		if (ArtObjectNum >= artObjectList.size())
			ArtObjectNum = 0;
		auto result = artObjectList.at(ArtObjectNum);
		ArtObjectNum++;
		return result;
	}
	template <typename T>
	T* TaskTempFormManager::GetNewForm()
	{
		auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<T>();
		if (!factory)
			return nullptr;
		T* newForm = factory->Create();
		return newForm;
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
		case VFXType::HitEffect:
			isCreated = CreateArtVFX();
			break;
		}

		logger::debug("{}create VFX({}) {} on ({}, {}, {}) for {:x} {}", isCreated ? "couldn't " : "", magic_enum::enum_name(mVFXType).data(), mVFXPath, mHitPoint.x, mHitPoint.y, mHitPoint.z, mTarget->formID, mTarget->GetName());
	}
	void TaskVFX::Dispose()
	{
		delete this;
	}
	bool TaskVFX::CreateImpactVFX()
	{
		if (!mAggressor)
			return false;
		auto particle = RE::BSTempEffectParticle::Spawn(mAggressor->parentCell, 0.0f, mVFXPath.c_str(), mHitDirection, mHitPoint, 1.0f, 7, mTargetObj);
		if (!particle)
			return false;
		auto processLists = RE::ProcessLists::GetSingleton();
		if (!processLists)
			return false;
		RE::BSSpinLockGuard locker(processLists->globalEffectsLock);
		processLists->globalTempEffects.emplace_back(particle);
		return true;
	}
	bool TaskVFX::CreateArtVFX()
	{
		if (!mTarget)
			return false;
		auto art = TaskTempFormManager::GetSingleton().GetArtObjectTempForm();
		if (!art)
			return false;
		art->SetModel(mVFXPath.c_str());
		auto hitEffect = mTarget->InstantiateHitArt(art, 0.0f, nullptr, false, false);
		if (!hitEffect)
			return false;
		return true;
	}

	void TaskEffectShader::Run()
	{
		if (!mTarget || !mEffectShader)
			return;
		auto hitShader = mTarget->InstantiateHitShader(mEffectShader, 0.0f, nullptr, false, false);
		logger::debug("{}create EffectShader for {:x} {}", hitShader ? "" : "couldn't ", mTarget->formID, mTarget->GetName());
		return;
	}
	void TaskEffectShader::Dispose()
	{
		delete this;
	}

	void TaskSound::Run()
	{
		RE::BSSoundHandle handle1, handle2;
		if (mSound1)
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(handle1, mSound1);
		if (mSound2)
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(handle2, mSound2);

		RE::BGSImpactData* impactData = TaskTempFormManager::GetSingleton().GetImpactDataTempForm();
		impactData->sound1 = mSound1;
		impactData->sound2 = mSound2;
		RE::BGSImpactManager::ImpactSoundData sound{
			impactData,
			&mHitPoint,
			nullptr,
			mSound1 ? &handle1 : nullptr,
			mSound2 ? &handle2 : nullptr,
			mSound1 ? true : false,
			mSound2 ? true : false
		};
		RE::BGSImpactManager::GetSingleton()->PlayImpactDataSounds(sound);
	}
	void TaskSound::Dispose()
	{
		delete this;
	}

	void TaskCastVFX::Run()
	{
		if (!mSpell || !mAggressor || !mTarget)
			return;

		Cast(nullptr, 0, mSpell, mAggressor, mTarget);
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
}