#pragma once

namespace Mus {
	class ImpactManager : 
		public ImpactManagerImpl,
		public IEventListener<HitEvent>
	{
	public:
		[[nodiscard]] static ImpactManager& GetSingleton() {
			static ImpactManager instance;
			return instance;
		};

		ImpactManager() {};
		~ImpactManager() {};
		
		const std::uint32_t ImpactManagerRecord = _byteswap_ulong('IMR');
		static void Save(SKSE::SerializationInterface* serde);
		static void Load(SKSE::SerializationInterface* serde, std::uint32_t type, std::uint32_t version);
	protected:
		void onEvent(const HitEvent& e) override;

	private:
	};
}
