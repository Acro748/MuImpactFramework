#pragma once

#include <detours/detours.h>

namespace Mus {
#define MEMBER_FN_PREFIX(className)	\
	typedef className _MEMBER_FN_BASE_TYPE

#define CALL_MEMBER_FN(obj, fn) ((*(obj)).*(*((obj)->_##fn##_GetPtr())))

#define DEFINE_MEMBER_FN_LONG_HOOK(className, functionName, retnType, address, ...) \
    typedef retnType (className::*_##functionName##_type)(__VA_ARGS__);             \
    static inline uintptr_t* _##functionName##_GetPtrAddr(void) {                   \
        static uintptr_t _address = address;	\
        return &_address;                                                           \
    }                                                                               \
                                                                                    \
    static inline _##functionName##_type* _##functionName##_GetPtr(void) {          \
        return (_##functionName##_type*)_##functionName##_GetPtrAddr();             \
    }

#define DEFINE_MEMBER_FN_HOOK(functionName, retnType, address, ...)	\
	DEFINE_MEMBER_FN_LONG_HOOK(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, __VA_ARGS__)

	struct HitEvent
	{
		//enums
		enum class EventType : std::uint8_t {
			None,
			HitData,
			Projectile,

			Total
		};
		enum class AttackType : std::uint8_t {
			None,
			Weapon, //sword, dagger, bow, staff
			Shout, //fus ro dah
			Spell, //spell

			Total
		};
		enum class AttackHand : std::uint8_t {
			Left = 1 << 0,
			Right = 1 << 1,
			Both = Left + Right
		};
		enum class DamageType : std::uint32_t {
			None = 0,
			Health = 1 << 0,
			Magicka = 1 << 1,
			Stamina = 1 << 2,

			Total
		};
		enum class ElementalType : std::uint32_t {
			None = 0,
			Fire = 1 << 0,
			Frost = 1 << 1,
			Shock = 1 << 2,

			Total
		};

		//default info
		RE::Actor* aggressor = nullptr;
		RE::TESObjectREFR* target = nullptr;
		RE::NiPoint3 hitPosition;
		RE::NiPoint3 hitDirection;
		RE::BGSMaterialType* material = nullptr;

		//non weapon attack
		RE::Projectile* projectile = nullptr;
		RE::TESShout* shout = nullptr;
		RE::TESShout::VariationID shoutNum = RE::TESShout::VariationID::kNone;
		RE::SpellItem* spell = nullptr;

		//weapon attack
		RE::TESObjectWEAP* weapon = nullptr;
		RE::WEAPON_TYPE weaponType = RE::WEAPON_TYPE::kHandToHandMelee;
		RE::stl::enumeration<RE::HitData::Flag, std::uint32_t> flags = RE::HitData::Flag(0);

		//misc
		RE::MagicItem* magicItem = nullptr;
		float damage = 0.0f;
		RE::stl::enumeration<AttackHand, std::uint8_t> attackHand = AttackHand::Left;
		EventType eventType = EventType::None;
		AttackType attackType = AttackType::None;
		RE::stl::enumeration<DamageType, std::uint32_t> damageType = DamageType::None;
		RE::stl::enumeration<ElementalType, std::uint32_t> elementalType = ElementalType::None;
	};

	template <class Event = void>
	class IEventListener
	{
	public:
		virtual void onEvent(const Event&) = 0;
	};

	template <class Event = void>
	class IEventDispatcher
	{
	public:

		virtual ~IEventDispatcher()
		{
		}

		virtual void addListener(IEventListener<Event>*) = 0;
		virtual void removeListener(IEventListener<Event>*) = 0;
		virtual void dispatch(const Event&) = 0;
	};

	template <class Event = void>
	class EventDispatcherImpl : public IEventDispatcher<Event>
	{
	public:
		EventDispatcherImpl() {}
		~EventDispatcherImpl() {}

		void addListener(IEventListener<Event>* listener) override {
			m_listeners.insert(listener);
			m_cacheDirt = true;
		};
		void removeListener(IEventListener<Event>* listener) override {
			m_listeners.erase(listener);
			m_cacheDirt = true;
		};
		void dispatch(const Event& event) override {
			if (m_cacheDirt)
			{
				m_caches.clear();
				for (auto& i : m_listeners)
					m_caches.emplace_back(i);
				m_cacheDirt = false;
			}

			for (auto i : m_caches)
				i->onEvent(event);
		};

	private:
		std::unordered_set<IEventListener<Event>*> m_listeners;
		std::vector<IEventListener<Event>*> m_caches;
		bool m_cacheDirt = false;
	};

	extern EventDispatcherImpl<HitEvent> g_HitEventDispatcher;

	void hook();
	void unhook();
}
