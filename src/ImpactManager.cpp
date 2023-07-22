#include "ImpactManager.h"

namespace Mus {
	void ImpactManager::Save(SKSE::SerializationInterface* serde)
	{
		if (serde->OpenRecord(GetSingleton().ImpactManagerRecord, 0)) {
			logger::info("Saving on cosave for ImpactManager...");
			
		}
	}
	void ImpactManager::Load(SKSE::SerializationInterface* serde, std::uint32_t type, std::uint32_t version)
	{
		if (type == GetSingleton().ImpactManagerRecord) {
			logger::info("Loding on cosave ImpactManager...");
			
		}
	}

	void ImpactManager::onEvent(const HitEvent& e)
	{
		for (const auto& condition : ConditionManager::GetSingleton().GetCondition(e))
		{
			for (const auto& item : condition.ImpactDataSets)
			{
				if (item)
				{
					Register(item);
				}
			}
			for (const auto& item : condition.SpellItems)
			{
				if (item)
				{
					Register(item);
				}
			}
			for (const auto& item : condition.VFXItems)
			{
				Register(item.vfxPath, item.vfxType);
			}
			for (const auto& item : condition.SoundDescriptor1Items)
			{
				if (item)
				{
					Register(item, false);
				}
			}
			for (const auto& item : condition.SoundDescriptor2Items)
			{
				if (item)
				{
					Register(item, true);
				}
			}
			for (const auto& item : condition.EffectShaderItems)
			{
				if (item)
				{
					Register(item);
				}
			}
		}

		LoadImpactEffects(e);

		TimeLogger(true, Config::GetSingleton().GetEnableTimeCounter());
	}
}
