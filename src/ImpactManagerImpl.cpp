#include "ImpactManagerimpl.h"

namespace Mus {
	void ImpactManagerImpl::Register(RE::BGSImpactDataSet* dataSet, Option option)
	{
		if (dataSet)
			ImpactDataSet[dataSet->formID] = { dataSet, option };
	}
	void ImpactManagerImpl::UnRegister(RE::BGSImpactDataSet* dataSet)
	{
		if (dataSet)
			ImpactDataSet.erase(dataSet->formID);
	}

	void ImpactManagerImpl::Register(RE::SpellItem* spell, Option option)
	{
		if (spell)
			Spell[spell->formID] = { spell, option };
	}
	void ImpactManagerImpl::UnRegister(RE::SpellItem* spell)
	{
		if (spell)
			Spell.erase(spell->formID);
	}

	void ImpactManagerImpl::Register(std::string VFXPath, std::uint8_t VFXType, Option option)
	{
		VFXPath = lowLetter(VFXPath);
		VFX[VFXPath] = { VFXPath, VFXType, option };
	}
	void ImpactManagerImpl::UnRegister(std::string VFXPath)
	{
		VFXPath = lowLetter(VFXPath);
		VFX.erase(VFXPath);
	}

	void ImpactManagerImpl::Register(RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = std::find(Sound[SecondSound].begin(), Sound[SecondSound].end(), sound); found != Sound[SecondSound].end())
			return;
		Sound[SecondSound].push_back(sound);
	}
	void ImpactManagerImpl::UnRegister(RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = std::find(Sound[SecondSound].begin(), Sound[SecondSound].end(), sound); found != Sound[SecondSound].end())
			Sound[SecondSound].erase(found);
	}

	void ImpactManagerImpl::Register(RE::TESEffectShader* effectShader, Option option)
	{
		if (effectShader)
			EffectShader[effectShader->formID] = { effectShader, option };
	}
	void ImpactManagerImpl::UnRegister(RE::TESEffectShader* effectShader)
	{
		if (effectShader)
			EffectShader.erase(effectShader->formID);
	}

	void ImpactManagerImpl::Register(RE::BGSArtObject* artObject, Option option)
	{
		if (artObject)
			ArtObject[artObject->formID] = { artObject, option };
	}
	void ImpactManagerImpl::UnRegister(RE::BGSArtObject* artObject)
	{
		if (artObject)
			ArtObject.erase(artObject->formID);
	}

	void ImpactManagerImpl::UnRegister(RegisterType type)
	{
		if (type & RegisterType::kImpactDataSet)
			ImpactDataSet.clear();
		if (type & RegisterType::kSpell)
			Spell.clear();
		if (type & RegisterType::kVFX)
			VFX.clear();
		if (type & RegisterType::kSound)
		{
			Sound[0].clear();
			Sound[1].clear();
		}
		if (type & RegisterType::kEffectShader)
			EffectShader.clear();
		if (type & RegisterType::kArtObject)
			ArtObject.clear();
	}

	void ImpactManagerImpl::LoadEffects(const HitEvent& e)
	{
		if (ImpactDataSet.size() > 0)
			LoadImpactData(e.aggressor, e.target, e.hitPosition, e.hitDirection, e.material);
		if (Spell.size() > 0)
			LoadSpell(e.aggressor, e.target);
		if (VFX.size() > 0)
			LoadVFX(e.aggressor, e.target, e.hitPosition, e.hitDirection);
		if (Sound[0].size() + Sound[1].size() > 0)
			LoadSound(e.hitPosition);
		if (EffectShader.size() > 0)
			LoadEffectShader(e.aggressor, e.target);
		if (ArtObject.size() > 0)
			LoadArtObject(e.aggressor, e.target);
		UnRegister(RegisterType::kAll);
	}

	void ImpactManagerImpl::LoadImpactData(RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::BGSMaterialType* material, RE::NiAVObject* targetObj)
	{
		if (!aggressor)
			return;

		if (!material)
		{
			if (RE::Actor* target = skyrim_cast<RE::Actor*>(target))
			{
				if (auto race = target->GetRace(); race && race->bloodImpactMaterial)
					material = race->bloodImpactMaterial;
				else
					material = RE::BGSMaterialType::GetMaterialType(RE::MATERIAL_ID::kSkin);
			}
			else
				material = RE::BGSMaterialType::GetMaterialType(RE::MATERIAL_ID::kStone);
		}
		if (!material)
			return;

		auto processLists = RE::ProcessLists::GetSingleton();
		if (!processLists)
			return;

		auto audioManager = RE::BSAudioManager::GetSingleton();
		auto impactManager = RE::BGSImpactManager::GetSingleton();

		logger::debug("material type {} for {:x} {}", static_cast<std::uint32_t>(material->materialID), target ? target->formID : 0, target ? target->GetName() : "Inanimate Object");

		for (const auto& impactData : ImpactDataSet)
		{
			if (!impactData.second.item)
				continue;

			auto found = material ? impactData.second.item->impactMap.find(material) : impactData.second.item->impactMap.end();
			if (found == impactData.second.item->impactMap.end() || !found->second)
				continue;

			RE::BSTempEffectParticle* particle = nullptr;
			if (processLists)
				particle = RE::BSTempEffectParticle::Spawn(aggressor->parentCell, impactData.second.option.Duration, found->second->GetModel(), impactData.second.option.RandomDirection ? GetRandomDirection() : hitDirection, hitPosition, impactData.second.option.Scale, 7, nullptr);
			if (particle)
			{
				processLists->globalEffectsLock.Lock();
				processLists->globalTempEffects.emplace_back(particle);
				processLists->globalEffectsLock.Unlock();
				logger::trace("create ImpactVFX {:x} for {:x} {}", impactData.second.item->formID, target ? target->formID : 0, target ? target->GetName() : "Inanimate Object");
			}
			else
			{
				logger::error("couldn't create ImpactVFX {:x} for {:x} {}", impactData.second.item->formID, target ? target->formID : 0, target ? target->GetName() : "Inanimate Object");
				continue;
			}

			if (!target) //if target is invalid then skip sound
				continue;

			if (!audioManager || !impactManager)
				continue;
			if (!found->second->sound1 && !found->second->sound2)
				continue;

			RE::BSSoundHandle handle1, handle2;
			if (found->second->sound1)
				audioManager->BuildSoundDataFromDescriptor(handle1, found->second->sound1);
			if (found->second->sound2)
				audioManager->BuildSoundDataFromDescriptor(handle2, found->second->sound2);
			RE::BGSImpactManager::ImpactSoundData sound{
				found->second,
				&hitPosition,
				nullptr,
				found->second->sound1 ? &handle1 : nullptr,
				found->second->sound2 ? &handle2 : nullptr,
				found->second->sound1 ? true : false,
				found->second->sound2 ? true : false
			};
			if (impactManager->PlayImpactDataSounds(sound))
				logger::trace("create ImpactSFX {:x} for {:x} {}", impactData.second.item->formID, target ? target->formID : 0, target ? target->GetName() : "Inanimate Object");
			else
				logger::error("couldn't create ImpactSFX {:x} for {:x} {}", impactData.second.item->formID, target ? target->formID : 0, target ? target->GetName() : "Inanimate Object");
		}
	}

	void ImpactManagerImpl::LoadSpell(RE::Actor* aggressor, RE::TESObjectREFR* target)
	{
		if (!target)
			return;

		for (const auto& spell : Spell)
		{
			if (spell.second.item)
			{
				Cast(nullptr, 0, spell.second.item, target, target);
				logger::trace("create Spell Effect {:x} for {:x} {}", spell.second.item->formID, target->formID, target->GetName());
			}
		}
	}

	void ImpactManagerImpl::LoadVFX(RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj)
	{
		auto processLists = RE::ProcessLists::GetSingleton();
		for (const auto& VFX : VFX)
		{
			if (VFX.second.vfxPath.empty())
				return;

			bool isCreated = false;

			switch (VFX.second.vfxType) {
			case ConditionManager::Impact: {
				if (!processLists)
					break;
				RE::BSTempEffectParticle* particle = nullptr;
				if (aggressor && aggressor->parentCell)
					particle = RE::BSTempEffectParticle::Spawn(aggressor->parentCell, VFX.second.option.Duration, VFX.second.vfxPath.c_str(), VFX.second.option.RandomDirection ? GetRandomDirection() : hitDirection, hitPosition, VFX.second.option.Scale, 7, nullptr);
				else if (target && target->parentCell)
					particle = RE::BSTempEffectParticle::Spawn(target->parentCell, VFX.second.option.Duration, VFX.second.vfxPath.c_str(), VFX.second.option.RandomDirection ? GetRandomDirection() : hitDirection, hitPosition, VFX.second.option.Scale, 7, nullptr);
				if (!particle)
					break;
				processLists->globalEffectsLock.Lock();
				processLists->globalTempEffects.emplace_back(particle);
				processLists->globalEffectsLock.Unlock();
				isCreated = true;
			}
				break;
			case ConditionManager::HitEffect: {
				if (!target)
					break;
				auto art = GetArtObjectTempForm();
				if (!art)
					break;
				art->SetModel(VFX.second.vfxPath.c_str());
				auto hitEffect = target->InstantiateHitArt(art, VFX.second.option.Duration, nullptr, false, false);
				if (!hitEffect)
					break;
				isCreated = true;
			}
				break;
			}

			if (isCreated)
				logger::trace("create VFX({}) {} on ({}, {}, {}) for {:x} {}", magic_enum::enum_name(ConditionManager::VFXType(VFX.second.vfxType)).data(), VFX.second.vfxPath, hitPosition.x, hitPosition.y, hitPosition.z, target ? target->formID : 0, target ? target->GetName() : "");
			else
				logger::error("couldn't create VFX({}) {} on ({}, {}, {}) for {:x} {}", magic_enum::enum_name(ConditionManager::VFXType(VFX.second.vfxType)).data(), VFX.second.vfxPath, hitPosition.x, hitPosition.y, hitPosition.z, target ? target->formID : 0, target ? target->GetName() : "");
		}
	}

	void ImpactManagerImpl::LoadSound(RE::NiPoint3 hitPosition)
	{
		auto audioManager = RE::BSAudioManager::GetSingleton();
		auto impactManager = RE::BGSImpactManager::GetSingleton();
		if (!audioManager || !impactManager)
			return;

		for (std::uint8_t i = 0; i < (Sound[0].size() >= Sound[1].size() ? Sound[0].size() : Sound[1].size()); i++)
		{
			RE::BGSSoundDescriptorForm* sound1 = nullptr;
			RE::BGSSoundDescriptorForm* sound2 = nullptr;
			if (Sound[0].size() > i)
				sound1 = Sound[0].at(i);
			if (Sound[1].size() > i)
				sound2 = Sound[1].at(i);
			if (!sound1 && !sound2)
				continue;
			
			RE::BSSoundHandle handle1, handle2;
			if (sound1)
				audioManager->BuildSoundDataFromDescriptor(handle1, sound1);
			if (sound2)
				audioManager->BuildSoundDataFromDescriptor(handle2, sound2);

			RE::BGSImpactData* impactData = GetImpactDataTempForm();
			impactData->sound1 = sound1;
			impactData->sound2 = sound2;
			RE::BGSImpactManager::ImpactSoundData sound{
				impactData,
				&hitPosition,
				nullptr,
				sound1 ? &handle1 : nullptr,
				sound2 ? &handle2 : nullptr,
				sound1 ? true : false,
				sound2 ? true : false
			};
			if (impactManager->PlayImpactDataSounds(sound))
				logger::trace("create Sound {:x}/{:x}", sound1 ? sound1->formID : 0, sound2 ? sound2->formID : 0);
			else
				logger::error("couldn't create Sound {:x}/{:x}", sound1 ? sound1->formID : 0, sound2 ? sound2->formID : 0);
		}
	}

	void ImpactManagerImpl::LoadEffectShader(RE::Actor* aggressor, RE::TESObjectREFR* target)
	{
		if (!target)
			return;

		for (const auto& effectShader : EffectShader)
		{
			if (!effectShader.second.item)
				continue;
			auto hitShader = target->InstantiateHitShader(effectShader.second.item, effectShader.second.option.Duration, nullptr, false, false);
			if (hitShader)
				logger::trace("create EffectShader for {:x} {}", target->formID, target->GetName());
			else
				logger::error("couldn't create EffectShader for {:x} {}", target->formID, target->GetName());
		}
	}

	void ImpactManagerImpl::LoadArtObject(RE::Actor* aggressor, RE::TESObjectREFR* target)
	{
		if (!target)
			return;

		for (const auto& artObject : ArtObject)
		{
			if (artObject.second.item)
				continue;;
			auto ArtObject = target->InstantiateHitArt(artObject.second.item, artObject.second.option.Duration, nullptr, false, false);
			if (ArtObject)
				logger::trace("create EffectShader for {:x} {}", target->formID, target->GetName());
			else
				logger::error("couldn't create EffectShader for {:x} {}", target->formID, target->GetName());
		}
	}

	void ImpactManagerImpl::Cast(RE::BSScript::IVirtualMachine* VMinternal, std::uint32_t stackId, RE::SpellItem* spell, RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target)
	{
		using func_t = decltype(&ImpactManagerImpl::Cast);
		REL::VariantID offset(55149, 55747, 0x930EC0);
		REL::Relocation<func_t> func{ offset };
		return func(VMinternal, stackId, spell, aggressor, target);
	}

	RE::BGSImpactData* ImpactManagerImpl::GetImpactDataTempForm()
	{
		//static std::mutex m_lock;
		//std::lock_guard<std::mutex> locker(m_lock);
		static std::vector<RE::BGSImpactData*> impactDataList;
		static std::uint32_t ImpactDataNum = 0;
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
	RE::BGSArtObject* ImpactManagerImpl::GetArtObjectTempForm()
	{
		//static std::mutex m_lock;
		//std::lock_guard<std::mutex> locker(m_lock);
		static std::vector<RE::BGSArtObject*> artObjectList;
		static std::uint32_t ArtObjectNum = 0;
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
	T* ImpactManagerImpl::GetNewForm()
	{
		auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<T>();
		if (!factory)
			return nullptr;
		T* newForm = factory->Create();
		return newForm;
	}

	RE::NiPoint3 ImpactManagerImpl::GetRandomDirection() {
		std::mt19937 generator(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		std::uniform_real_distribution<float> distribution(-MATH_PI, MATH_PI);
		return RE::NiPoint3(distribution(generator), distribution(generator), distribution(generator));
	}
}