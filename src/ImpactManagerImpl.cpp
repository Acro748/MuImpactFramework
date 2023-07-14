#include "ImpactManagerimpl.h"

namespace Mus {
	void ImpactManager_impl::Register(bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		ImpactDataSet[LeftHand][dataSet->formID] = dataSet;
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, RE::BGSImpactDataSet* dataSet)
	{
		ImpactDataSet[LeftHand].erase(dataSet->formID);
	}

	void ImpactManager_impl::Register(bool LeftHand, RE::SpellItem* spell)
	{
		Spell[LeftHand][spell->formID] = spell;
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, RE::SpellItem* spell)
	{
		Spell[LeftHand].erase(spell->formID);
	}

	void ImpactManager_impl::Register(bool LeftHand, std::string VFXPath, std::uint8_t VFXType)
	{
		VFXPath = lowLetter(VFXPath);
		VFX[LeftHand][VFXPath] = VFXType;
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, std::string VFXPath)
	{
		VFXPath = lowLetter(VFXPath);
		VFX[LeftHand].erase(VFXPath);
	}

	void ImpactManager_impl::Register(bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = std::find(Sound[LeftHand][SecondSound].begin(), Sound[LeftHand][SecondSound].end(), sound); found != Sound[LeftHand][SecondSound].end())
			return;
		Sound[LeftHand][SecondSound].push_back(sound);
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, RE::BGSSoundDescriptorForm* sound, bool SecondSound)
	{
		if (auto found = std::find(Sound[LeftHand][SecondSound].begin(), Sound[LeftHand][SecondSound].end(), sound); found != Sound[LeftHand][SecondSound].end())
			Sound[LeftHand][SecondSound].erase(found);
	}

	void ImpactManager_impl::Register(bool LeftHand, RE::TESEffectShader* effectShader)
	{
		EffectShader[LeftHand][effectShader->formID] = effectShader;
	}
	void ImpactManager_impl::UnRegister(bool LeftHand, RE::TESEffectShader* effectShader)
	{
		EffectShader[LeftHand].erase(effectShader->formID);
	}

	void ImpactManager_impl::UnRegister(bool LeftHand, uint32_t type)
	{
		if (type & ImpactManager::Type::ImpactDataSet)
			ImpactDataSet[LeftHand].clear();
		if (type & ImpactManager::Type::Spell)
			Spell[LeftHand].clear();
		if (type & ImpactManager::Type::VFX)
			VFX[LeftHand].clear();
		if (type & ImpactManager::Type::Sound)
		{
			Sound[LeftHand][0].clear();
			Sound[LeftHand][1].clear();
		}
		if (type & ImpactManager::Type::EffectShader)
		{
			EffectShader[LeftHand].clear();
		}
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
				LoadHitPlayImpactData(evn->cause.get(), evn->target.get(), LeftHand, hitData->hitPosition, hitData->hitDirection);
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
		PlayEffectShader(evn->cause.get(), evn->target.get(), LeftHand);
	}

	void ImpactManager_impl::LoadHitPlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection)
	{
		PlayImpactData(aggressor, target, LeftHand, hitPoint, hitDirection);
		PlayVFX(aggressor, target, LeftHand, hitPoint, hitDirection);
		PlaySound(LeftHand, hitPoint);
	}

	void ImpactManager_impl::LoadHitPlayImpactData(RE::Actor* aggressor, RE::TESObjectREFR* target, bool LeftHand)
	{
		if (!aggressor || !aggressor->loadedData || !aggressor->loadedData->data3D)
			return;
		if (!target || !target->loadedData || !target->loadedData->data3D)
			return;
		auto handNode = aggressor->loadedData->data3D.get()->GetObjectByName(LeftHand ? HandL : HandR);
		if (!handNode)
			return;
		RE::NiAVObject* root = handNode;
		RE::NiAVObject* obj = root;
		RE::NiPoint3 closePoint = target->loadedData->data3D.get()->world.translate;
		float d1 = 10000;
		RE::BSVisit::TraverseScenegraphObjects(root, [&](RE::NiAVObject* a_object) -> RE::BSVisit::BSVisitControl {
			float d2 = closePoint.GetSquaredDistance(a_object->world.translate);
			if (d2 < d1)
			{
				obj = a_object;
				d1 = d2;
			}
			return RE::BSVisit::BSVisitControl::kContinue;
			}
		);
		if (obj)
			LoadHitPlayImpactData(aggressor, target, LeftHand, obj->world.translate, emptyPoint);
	}

	void ImpactManager_impl::PlayImpactData(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& impactData : ImpactDataSet[LeftHand])
		{
			TaskImpactVFX* newTask = new TaskImpactVFX(impactData.second, aggressor, target, hitPoint, hitDirection, targetObj);
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
		for (const auto& spell : Spell[LeftHand])
		{
			TaskCastVFX* newTask = new TaskCastVFX(spell.second, aggressor, target);
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

	void ImpactManager_impl::PlayVFX(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, RE::NiPoint3 hitPoint, RE::NiPoint3 hitDirection, RE::NiAVObject* targetObj, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& VFX : VFX[LeftHand])
		{
			TaskVFX* newTask = new TaskVFX(VFX.first, aggressor, target, hitPoint, hitDirection, VFX.second, targetObj);
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

	void ImpactManager_impl::PlaySound(bool LeftHand, RE::NiPoint3 hitPoint, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (std::uint8_t i = 0; i < (Sound[LeftHand][0].size() >= Sound[LeftHand][1].size() ? Sound[LeftHand][0].size() : Sound[LeftHand][1].size()); i++)
		{
			RE::BGSSoundDescriptorForm* sound1 = nullptr;
			RE::BGSSoundDescriptorForm* sound2 = nullptr;
			if (Sound[LeftHand][0].size() > i)
				sound1 = Sound[LeftHand][0].at(i);
			if (Sound[LeftHand][1].size() > i)
				sound2 = Sound[LeftHand][1].at(i);
			if (!sound1 && !sound2)
				continue;
			TaskSound* newTask = new TaskSound(hitPoint, sound1, sound2);
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

	void ImpactManager_impl::PlayEffectShader(RE::TESObjectREFR* aggressor, RE::TESObjectREFR* target, bool LeftHand, bool instance)
	{
		const SKSE::detail::SKSETaskInterface* g_task = reinterpret_cast<const SKSE::detail::SKSETaskInterface*>(SKSE::GetTaskInterface());
		for (const auto& effectShader : EffectShader[LeftHand])
		{
			TaskEffectShader* newTask = new TaskEffectShader(aggressor, target, effectShader.second);
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