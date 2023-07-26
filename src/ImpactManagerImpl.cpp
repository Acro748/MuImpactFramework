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

	void ImpactManagerImpl::LoadImpactEffects(const HitEvent& e)
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

	void ImpactManagerImpl::LoadImpactData(RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::BGSMaterialType* material, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& impactData : ImpactDataSet)
		{
			VFXTaskImpactDataSet* newTask = new VFXTaskImpactDataSet(impactData.second.item, aggressor, target, hitPosition, hitDirection, material, impactData.second.option);
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

	void ImpactManagerImpl::LoadSpell(RE::Actor* aggressor, RE::TESObjectREFR* target, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& spell : Spell)
		{
			VFXTaskSpell* newTask = new VFXTaskSpell(spell.second.item, aggressor, target, spell.second.option);
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

	void ImpactManagerImpl::LoadVFX(RE::Actor* aggressor, RE::TESObjectREFR* target, RE::NiPoint3 hitPosition, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& VFX : VFX)
		{
			VFXTask* newTask = new VFXTask(VFX.second.vfxPath, aggressor, target, hitPosition, hitDirection, VFX.second.vfxType, VFX.second.option);
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

	void ImpactManagerImpl::LoadSound(RE::NiPoint3 hitPosition, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
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
			SFXTaskSound* newTask = new SFXTaskSound(sound1, sound2, hitPosition);
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

	void ImpactManagerImpl::LoadEffectShader(RE::Actor* aggressor, RE::TESObjectREFR* target, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& effectShader : EffectShader)
		{
			VFXTaskEffectShader* newTask = new VFXTaskEffectShader(effectShader.second.item, aggressor, target, effectShader.second.option);
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

	void ImpactManagerImpl::LoadArtObject(RE::Actor* aggressor, RE::TESObjectREFR* target, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& artObject : ArtObject)
		{
			VFXTaskArtObject* newTask = new VFXTaskArtObject(artObject.second.item, aggressor, target, artObject.second.option);
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
}