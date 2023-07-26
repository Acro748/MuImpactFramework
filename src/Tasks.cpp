#include "Tasks.h"

namespace Mus {
	RE::BGSImpactData* TaskTempFormManager::GetImpactDataTempForm()
	{
		static std::mutex m_lock;
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
		static std::mutex m_lock;
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

	RE::NiPoint3 TaskOptionManager::GetRandomDirection() {
		std::mt19937 generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		std::uniform_real_distribution<float> distribution(-MATH_PI, MATH_PI);
		return RE::NiPoint3(distribution(generator), distribution(generator), distribution(generator));
	}

	void VFXTaskImpactDataSet::Run()
	{
		if (!mImpactData || !mAggressor)
			return;

		if (!mMaterial)
		{
			if (RE::Actor* target = skyrim_cast<RE::Actor*>(mTarget))
			{
				if (auto race = target->GetRace(); race && race->bloodImpactMaterial)
					mMaterial = race->bloodImpactMaterial;
				else
					mMaterial = RE::BGSMaterialType::GetMaterialType(RE::MATERIAL_ID::kSkin);
			}
			else
				mMaterial = RE::BGSMaterialType::GetMaterialType(RE::MATERIAL_ID::kStone);
		}
		
		auto found = mMaterial ? mImpactData->impactMap.find(mMaterial) : mImpactData->impactMap.end();
		if (found == mImpactData->impactMap.end() || !found->second)
			return;

		logger::debug("material type {} for {:x} {}", static_cast<std::uint32_t>(found->first->materialID), mTarget ? mTarget->formID : 0, mTarget ? mTarget->GetName() : "Inanimate Object");
		auto particle = RE::BSTempEffectParticle::Spawn(mAggressor->parentCell, mOption.Duration, found->second->GetModel(), mOption.RandomDirection ? TaskOptionManager::GetSingleton().GetRandomDirection() : mHitDirection, mhitPosition, mOption.Scale, 7, nullptr);
		if (particle)
		{
			auto processLists = RE::ProcessLists::GetSingleton();
			processLists->globalEffectsLock.Lock();
			processLists->globalTempEffects.emplace_back(particle);
			processLists->globalEffectsLock.Unlock();
			logger::trace("create ImpactVFX {:x} for {:x} {}", mImpactData->formID, mTarget ? mTarget->formID : 0, mTarget ? mTarget->GetName() : "Inanimate Object");
		}
		else
		{
			logger::error("couldn't create ImpactVFX {:x} for {:x} {}", mImpactData->formID, mTarget ? mTarget->formID : 0, mTarget ? mTarget->GetName() : "Inanimate Object");
			return;
		}

		if (!mTarget) //if target is invalid then skip sound
			return;

		RE::BSSoundHandle handle1, handle2;
		if (found->second->sound1)
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(handle1, found->second->sound1);
		if (found->second->sound2)
			RE::BSAudioManager::GetSingleton()->BuildSoundDataFromDescriptor(handle2, found->second->sound2);
		RE::BGSImpactManager::ImpactSoundData sound{
			found->second,
			&mhitPosition,
			nullptr,
			found->second->sound1 ? &handle1 : nullptr,
			found->second->sound2 ? &handle2 : nullptr,
			found->second->sound1 ? true : false,
			found->second->sound2 ? true : false
		};
		RE::BGSImpactManager::GetSingleton()->PlayImpactDataSounds(sound);
	}
	void VFXTaskImpactDataSet::Dispose()
	{
		delete this;
	}

	void VFXTaskSpell::Run()
	{
		if (!mSpell || !mAggressor || !mTarget)
			return;

		Cast(nullptr, 0, mSpell, mAggressor, mTarget);
	}
	void VFXTaskSpell::Dispose()
	{
		delete this;
	}
	void VFXTaskSpell::Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target)
	{
		using func_t = decltype(&VFXTaskSpell::Cast);
		REL::VariantID offset(55149, 55747, 0x930EC0);
		REL::Relocation<func_t> func{ offset };
		return func(VMinternal, stackId, spell, aggressor, target);
	}

	void VFXTask::Run()
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

		logger::debug("{}create VFX({}) {} on ({}, {}, {}) for {:x} {}", isCreated ? "couldn't " : "", magic_enum::enum_name(mVFXType).data(), mVFXPath, mhitPosition.x, mhitPosition.y, mhitPosition.z, mTarget->formID, mTarget->GetName());
	}
	void VFXTask::Dispose()
	{
		delete this;
	}
	bool VFXTask::CreateImpactVFX()
	{
		if (!mAggressor)
			return false;
		auto particle = RE::BSTempEffectParticle::Spawn(mAggressor->parentCell, mOption.Duration, mVFXPath.c_str(), mOption.RandomDirection ? TaskOptionManager::GetSingleton().GetRandomDirection() : mHitDirection, mhitPosition, mOption.Scale, 7, nullptr);
		if (!particle)
			return false;
		auto processLists = RE::ProcessLists::GetSingleton();
		if (!processLists)
			return false;
		RE::BSSpinLockGuard locker(processLists->globalEffectsLock);
		processLists->globalTempEffects.emplace_back(particle);
		return true;
	}
	bool VFXTask::CreateArtVFX()
	{
		if (!mTarget)
			return false;
		auto art = TaskTempFormManager::GetSingleton().GetArtObjectTempForm();
		if (!art)
			return false;
		art->SetModel(mVFXPath.c_str());
		auto hitEffect = mTarget->InstantiateHitArt(art, mOption.Duration, nullptr, false, false);
		if (!hitEffect)
			return false;
		return true;
	}

	void SFXTaskSound::Run()
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
			&mhitPosition,
			nullptr,
			mSound1 ? &handle1 : nullptr,
			mSound2 ? &handle2 : nullptr,
			mSound1 ? true : false,
			mSound2 ? true : false
		};
		RE::BGSImpactManager::GetSingleton()->PlayImpactDataSounds(sound);
	}
	void SFXTaskSound::Dispose()
	{
		delete this;
	}

	void VFXTaskEffectShader::Run()
	{
		if (!mTarget || !mEffectShader)
			return;
		auto hitShader = mTarget->InstantiateHitShader(mEffectShader, mOption.Duration, nullptr, false, false);
		logger::debug("{}create EffectShader for {:x} {}", hitShader ? "" : "couldn't ", mTarget->formID, mTarget->GetName());
		return;
	}
	void VFXTaskEffectShader::Dispose()
	{
		delete this;
	}

	void VFXTaskArtObject::Run()
	{
		if (!mTarget || !mArtObject)
			return;
		auto artObject = mTarget->InstantiateHitArt(mArtObject, mOption.Duration, nullptr, false, false);
		if (!artObject)
			return;
		logger::debug("{}create EffectShader for {:x} {}", artObject ? "" : "couldn't ", mTarget->formID, mTarget->GetName());
		return;
	}
	void VFXTaskArtObject::Dispose()
	{
		delete this;
	}
}