#include "ImpactManager.h"

namespace Mus {
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
		else if (serde->OpenRecord(GetSingleton().ImpactManagerSound, 0)) {
			logger::info("Saving on cosave for ImpactManagerSound...");
			auto dataSize = GetSingleton().actorImpactData.size();
			serde->WriteRecordData(&dataSize, sizeof(dataSize));
			for (auto& actorMap : GetSingleton().actorImpactData) {
				auto actorid = actorMap.first;
				serde->WriteRecordData(&actorid, sizeof(actorid));
				auto Sound1L = actorMap.second.GetSound(true, false);
				auto Sound1SizeL = Sound1L.size();
				serde->WriteRecordData(&Sound1SizeL, sizeof(Sound1SizeL));
				for (auto& sound : Sound1L) {
					auto soundid = sound->formID;
					serde->WriteRecordData(&soundid, sizeof(soundid));
				}
				auto Sound2L = actorMap.second.GetSound(true, true);
				auto Sound2SizeL = Sound2L.size();
				serde->WriteRecordData(&Sound2SizeL, sizeof(Sound2SizeL));
				for (auto& sound : Sound2L) {
					auto soundid = sound->formID;
					serde->WriteRecordData(&soundid, sizeof(soundid));
				}
				auto Sound1R = actorMap.second.GetSound(false, false);
				auto Sound1SizeR = Sound1R.size();
				serde->WriteRecordData(&Sound1SizeR, sizeof(Sound1SizeR));
				for (auto& sound : Sound1R) {
					auto soundid = sound->formID;
					serde->WriteRecordData(&soundid, sizeof(soundid));
				}
				auto Sound2R = actorMap.second.GetSound(false, true);
				auto Sound2SizeR = Sound2R.size();
				serde->WriteRecordData(&Sound2SizeR, sizeof(Sound2SizeR));
				for (auto& sound : Sound2R) {
					auto soundid = sound->formID;
					serde->WriteRecordData(&soundid, sizeof(soundid));
				}
			}
		}
		else if (serde->OpenRecord(GetSingleton().ImpactManagerEffectShader, 0)) {
			logger::info("Saving on cosave for ImpactManagerEffectShader...");
			auto dataSize = GetSingleton().actorImpactData.size();
			serde->WriteRecordData(&dataSize, sizeof(dataSize));
			for (auto& actorMap : GetSingleton().actorImpactData) {
				auto actorid = actorMap.first;
				serde->WriteRecordData(&actorid, sizeof(actorid));
				auto effectShaderL = actorMap.second.GetEffectShader(true);
				auto effectShaderSizeL = effectShaderL.size();
				serde->WriteRecordData(&effectShaderSizeL, sizeof(effectShaderSizeL));
				for (auto& effectShader : effectShaderL) {
					auto effectShaderid = effectShader->formID;
					serde->WriteRecordData(&effectShaderid, sizeof(effectShaderid));
				}
				auto effectShaderR = actorMap.second.GetEffectShader(false);
				auto effectShaderSizeR = effectShaderR.size();
				serde->WriteRecordData(&effectShaderSizeR, sizeof(effectShaderSizeR));
				for (auto& effectShader : effectShaderR) {
					auto effectShaderid = effectShader->formID;
					serde->WriteRecordData(&effectShaderid, sizeof(effectShaderid));
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
		else if (type == GetSingleton().ImpactManagerSound) {
			logger::info("Loding on cosave ImpactManagerSound...");
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
				std::size_t SoundLSize;
				serde->ReadRecordData(&SoundLSize, sizeof(SoundLSize));
				for (; SoundLSize > 0; --SoundLSize) {
					std::size_t Sound1Size;
					serde->ReadRecordData(&Sound1Size, sizeof(Sound1Size));
					for (; Sound1Size > 0; --Sound1Size) {
						RE::FormID SoundID;
						serde->ReadRecordData(&SoundID, sizeof(SoundID));
						RE::FormID newSoundID;
						RE::BGSSoundDescriptorForm* sound = nullptr;
						if (!serde->ResolveFormID(SoundID, newSoundID)) {
							logger::warn("Sound ID {:X} could not be found after loading the save.", SoundID);
						}
						else
							sound = skyrim_cast<RE::BGSSoundDescriptorForm*>(RE::TESForm::LookupByID(SoundID));
						if (actor && sound)
							GetSingleton().AddSound(actor, true, sound, false);
					}
					std::size_t Sound2Size;
					serde->ReadRecordData(&Sound2Size, sizeof(Sound2Size));
					for (; Sound2Size > 0; --Sound2Size) {
						RE::FormID SoundID;
						serde->ReadRecordData(&SoundID, sizeof(SoundID));
						RE::FormID newSoundID;
						RE::BGSSoundDescriptorForm* sound = nullptr;
						if (!serde->ResolveFormID(SoundID, newSoundID)) {
							logger::warn("Sound ID {:X} could not be found after loading the save.", SoundID);
						}
						else
							sound = skyrim_cast<RE::BGSSoundDescriptorForm*>(RE::TESForm::LookupByID(SoundID));
						if (actor && sound)
							GetSingleton().AddSound(actor, true, sound, true);
					}
				}
				std::size_t SoundRSize;
				serde->ReadRecordData(&SoundRSize, sizeof(SoundRSize));
				for (; SoundRSize > 0; --SoundRSize) {
					std::size_t Sound1Size;
					serde->ReadRecordData(&Sound1Size, sizeof(Sound1Size));
					for (; Sound1Size > 0; --Sound1Size) {
						RE::FormID SoundID;
						serde->ReadRecordData(&SoundID, sizeof(SoundID));
						RE::FormID newSoundID;
						RE::BGSSoundDescriptorForm* sound = nullptr;
						if (!serde->ResolveFormID(SoundID, newSoundID)) {
							logger::warn("Sound ID {:X} could not be found after loading the save.", SoundID);
						}
						else
							sound = skyrim_cast<RE::BGSSoundDescriptorForm*>(RE::TESForm::LookupByID(SoundID));
						if (actor && sound)
							GetSingleton().AddSound(actor, false, sound, false);
					}
					std::size_t Sound2Size;
					serde->ReadRecordData(&Sound2Size, sizeof(Sound2Size));
					for (; Sound2Size > 0; --Sound2Size) {
						RE::FormID SoundID;
						serde->ReadRecordData(&SoundID, sizeof(SoundID));
						RE::FormID newSoundID;
						RE::BGSSoundDescriptorForm* sound = nullptr;
						if (!serde->ResolveFormID(SoundID, newSoundID)) {
							logger::warn("Sound ID {:X} could not be found after loading the save.", SoundID);
						}
						else
							sound = skyrim_cast<RE::BGSSoundDescriptorForm*>(RE::TESForm::LookupByID(SoundID));
						if (actor && sound)
							GetSingleton().AddSound(actor, false, sound, true);
					}
				}
			}
		}
		else if (type == GetSingleton().ImpactManagerEffectShader) {
			logger::info("Loding on cosave ImpactManagerEffectShader...");
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
				std::size_t effectShadertLSize;
				serde->ReadRecordData(&effectShadertLSize, sizeof(effectShadertLSize));
				for (; effectShadertLSize > 0; --effectShadertLSize) {
					RE::FormID EffectShaderID;
					serde->ReadRecordData(&EffectShaderID, sizeof(EffectShaderID));
					RE::FormID newEffectShaderID;
					RE::TESEffectShader* EffectShader = nullptr;
					if (!serde->ResolveFormID(EffectShaderID, newEffectShaderID)) {
						logger::warn("EffectShader ID {:X} could not be found after loading the save.", EffectShaderID);
					}
					else
						EffectShader = skyrim_cast<RE::TESEffectShader*>(RE::TESForm::LookupByID(newEffectShaderID));
					if (actor && EffectShader)
						GetSingleton().AddEffectShader(actor, true, EffectShader);
				}
				std::size_t effectShadertRSize;
				serde->ReadRecordData(&effectShadertRSize, sizeof(effectShadertRSize));
				for (; effectShadertRSize > 0; --effectShadertRSize) {
					RE::FormID EffectShaderID;
					serde->ReadRecordData(&EffectShaderID, sizeof(EffectShaderID));
					RE::FormID newEffectShaderID;
					RE::TESEffectShader* EffectShader = nullptr;
					if (!serde->ResolveFormID(EffectShaderID, newEffectShaderID)) {
						logger::warn("EffectShader ID {:X} could not be found after loading the save.", EffectShaderID);
					}
					else
						EffectShader = skyrim_cast<RE::TESEffectShader*>(RE::TESForm::LookupByID(newEffectShaderID));
					if (actor && EffectShader)
						GetSingleton().AddEffectShader(actor, false, EffectShader);
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

	void ImpactManager::AddSound(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (actor && sound)
			Register(actor, LeftHand, sound, SecondSound);
	}
	void ImpactManager::RemoveSound(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (actor && sound)
			UnRegister(actor, LeftHand, sound, SecondSound);
	}
	void ImpactManager::RemoveSound(RE::Actor* actor, bool LeftHand, bool SecondSound)
	{
		if (actor)
			UnRegister(actor, LeftHand, Type::VFX);
	}

	void ImpactManager::AddEffectShader(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader)
	{
		if (actor && effectShader)
			Register(actor, LeftHand, effectShader);
	}
	void ImpactManager::RemoveEffectShader(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader)
	{
		if (actor && effectShader)
			UnRegister(actor, LeftHand, effectShader);
	}
	void ImpactManager::RemoveEffectShader(RE::Actor* actor, bool LeftHand)
	{
		if (actor)
			UnRegister(actor, LeftHand, Type::EffectShader);
	}

	EventResult ImpactManager::ProcessEvent(const RE::TESHitEvent* evn, RE::BSTEventSource<RE::TESHitEvent>*)
	{
		//TimeLogger(false);

		if (!evn || !evn->cause || !evn->target)
			return EventResult::kContinue;

		if (!IsValidHitEvent(evn))
			return EventResult::kContinue;;

		RE::Actor* cause = skyrim_cast<RE::Actor*>(evn->cause.get());
		if (!cause)
			return EventResult::kContinue;

		if (auto found = actorImpactData.find(cause->formID); found != actorImpactData.end())
		{
			found->second.ProcessHitEvent(evn);
			logger::debug("target : {:x} {}, aggressor : {:x} {}", evn->target.get()->formID, evn->target.get()->GetName(), cause->formID, evn->cause.get()->GetName());
		}

		for (const auto& condition : ConditionManager::GetSingleton().GetCondition(evn))
		{
			for (const auto& item : condition.ImpactDataSets)
			{
				auto impact = skyrim_cast<RE::BGSImpactDataSet*>(GetFormByID(item.id, item.pluginName));
				if (impact)
				{
					conditionActorImpactData.Register(false, impact);
					conditionActorImpactData.Register(true, impact);
				}
			}
			for (const auto& item : condition.SpellItems)
			{
				auto spell = skyrim_cast<RE::SpellItem*>(GetFormByID(item.id, item.pluginName));
				if (spell)
				{
					conditionActorImpactData.Register(false, spell);
					conditionActorImpactData.Register(true, spell);
				}
			}
			for (const auto& item : condition.VFXItems)
			{
				if (!item.vfxPath.empty())
				{
					conditionActorImpactData.Register(false, item.vfxPath, item.vfxType);
					conditionActorImpactData.Register(true, item.vfxPath, item.vfxType);
				}
			}
			for (const auto& item : condition.SoundDescriptor1Items)
			{
				auto sound = skyrim_cast<RE::BGSSoundDescriptorForm*>(GetFormByID(item.id, item.pluginName));
				if (sound)
				{
					conditionActorImpactData.Register(false, sound, false);
					conditionActorImpactData.Register(true, sound, false);
				}
			}
			for (const auto& item : condition.SoundDescriptor2Items)
			{
				auto sound = skyrim_cast<RE::BGSSoundDescriptorForm*>(GetFormByID(item.id, item.pluginName));
				if (sound)
				{
					conditionActorImpactData.Register(false, sound, true);
					conditionActorImpactData.Register(true, sound, true);
				}
			}
			for (const auto& item : condition.EffectShaderItems)
			{
				auto effectShader = skyrim_cast<RE::TESEffectShader*>(GetFormByID(item.id, item.pluginName));
				if (effectShader)
				{
					conditionActorImpactData.Register(false, effectShader);
					conditionActorImpactData.Register(true, effectShader);
				}
			}
		}
		conditionActorImpactData.ProcessHitEvent(evn);

		for (std::uint32_t i = 0; i < ImpactManager::Type::Total; i++)
		{
			conditionActorImpactData.UnRegister(true, i);
			conditionActorImpactData.UnRegister(false, i);
		}
	
		//TimeLogger(true);

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

	void ImpactManager::Register(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.Register(LeftHand, sound, SecondSound);
		}
		else
		{
			ImpactManager_impl idm_ = ImpactManager_impl(actor);
			idm_.Register(LeftHand, sound, SecondSound);
			actorImpactData.insert(std::make_pair(actor->formID, idm_));
		}
		logger::info("Registered : Sound{} {:x} on Actor {:x} {}", SecondSound ? 2 : 1, sound->formID, actor->formID, actor->GetName());
	}

	void ImpactManager::Register(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.Register(LeftHand, effectShader);
		}
		else
		{
			ImpactManager_impl idm_ = ImpactManager_impl(actor);
			idm_.Register(LeftHand, effectShader);
			actorImpactData.insert(std::make_pair(actor->formID, idm_));
		}
		logger::info("Registered : EffectShader {:x} on Actor {:x} {}", effectShader->formID, actor->formID, actor->GetName());
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

	void ImpactManager::UnRegister(RE::Actor* actor, bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand, sound, SecondSound);
		}
		logger::info("UnRegistered : Sound{} {:x} on Actor {:x} {}", SecondSound ? 2 : 1, sound->formID, actor->formID, actor->GetName());
	}

	void ImpactManager::UnRegister(RE::Actor* actor, bool LeftHand, RE::TESEffectShader* effectShader)
	{
		if (auto found = actorImpactData.find(actor->formID); found != actorImpactData.end())
		{
			found->second.UnRegister(LeftHand, effectShader);
		}
		logger::info("UnRegistered : EffectShader {:x} on Actor {:x} {}", effectShader->formID, actor->formID, actor->GetName());
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
