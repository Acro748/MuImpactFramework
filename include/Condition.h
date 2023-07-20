#pragma once

namespace Mus {
	namespace ConditionFragment
	{
		class ConditionBase;
	}
	class ConditionManager {
	public :
		ConditionManager() {};
		~ConditionManager() {};

		[[nodiscard]] static ConditionManager& GetSingleton() {
			static ConditionManager instance;
			return instance;
		}

		void InitialConditionMap();

		enum ConditionType : std::uint8_t {
			IsEquippedLeft,
			IsEquippedLeftType,
			IsEquippedLeftTypeAlt,
			IsEquippedLeftHasKeyword,
			IsEquippedLeftHasKeywordEditorID,
			IsEquippedRight,
			IsEquippedRightType,
			IsEquippedRightTypeAlt,
			IsEquippedRightHasKeyword,
			IsEquippedRightHasKeywordEditorID,
			IsEquippedPowerOrShout,
			IsWorn,
			IsWornSlot,
			IsWornHasKeyword,
			IsWornHasKeywordEditorID,
			IsInFaction,
			HasKeyword,
			HasKeywordEditorID,
			HasMagicEffect,
			HasPerk,
			HasSpell,
			IsActorBase,
			IsActor,
			IsObjectReference,
			IsRace,
			IsClass,
			IsCombatStyle,

			IsFemale,
			IsChild,
			IsPlayerTeammate,
			IsUnique,
			IsSneaking,
			IsSprinting,
			IsSwimming,
			IsInAir,
			IsDead,

			IsLeftAttacking, //AggressorOnly
			IsRightAttacking, //AggressorOnly
			IsAttackingType, //AggressorOnly
			IsAttackingHasKeyword, //AggressorOnly
			IsAttackingHasKeywordEditorID, //AggressorOnly

			IsBlocked, //TargetOnly
			IsCritical, //TargetOnly
			IsSneakCritical, //TargetOnly
			IsBash, //AggressorOnly
			IsPowerAttack, //AggressorOnly
			IsInanimateObject, //TargetOnly

			None,
			Error
		};

		enum ConditionOption : std::uint8_t {
			Aggressor,
			Target,

			OptionTotal
		};

		struct ConditionItem {
			std::shared_ptr<ConditionFragment::ConditionBase> conditionFunction;

			bool NOT = false;
			ConditionType type;
			std::string pluginName = "";
			RE::FormID id = 0;
			std::string arg = "";
			std::string arg2 = "";
		};
		typedef std::vector<ConditionItem> ConditionItemOr;

		struct VFXInfo {
			std::string vfxPath;
			std::uint8_t vfxType;
		};
		static std::uint8_t GetVFXType(std::string vfxPath);
		struct Condition {
			std::vector<RE::BGSImpactDataSet*> ImpactDataSets;
			std::vector<RE::SpellItem*> SpellItems;
			std::vector<VFXInfo> VFXItems;
			std::vector<RE::BGSSoundDescriptorForm*> SoundDescriptor1Items;
			std::vector<RE::BGSSoundDescriptorForm*> SoundDescriptor2Items;
			std::vector<RE::TESEffectShader*> EffectShaderItems;

			std::string fileName;
			std::string originalCondition[ConditionOption::OptionTotal];
			std::vector<ConditionItemOr> AND[ConditionOption::OptionTotal];
		};

		bool RegisterCondition(Condition condition, std::string configPath);

		const concurrency::concurrent_vector<Condition> GetCondition(const HitEvent& e);

		std::size_t ConditionCount() const { return ConditionList.size(); }
	private:
		concurrency::concurrent_vector<Condition> ConditionList;
		std::unordered_map<std::string, ConditionType> ConditionMap;

		const Condition ParseConditions(Condition condition);
		const ConditionType GetConditionType(std::string line, ConditionItem& item);

		bool GetConditionFunction(ConditionItem& item);
		bool ConditionCheck(const HitEvent& e, Condition condition);

		inline void Logging(RE::TESObjectREFR* obj, std::uint8_t option, const ConditionItem& OR, bool isTrue) {
			std::string typestr = magic_enum::enum_name(ConditionType(OR.type)).data();
			if (IsContainString(typestr, "EditorID") || IsContainString(typestr, "Type"))
			{
				logger::debug("{} {:x} : {} Condition {}{}({}) is {}", obj->GetName(), obj->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, OR.arg,
					isTrue ? "True" : "False");
			}
			else if (OR.type >= ConditionType::IsFemale)
			{
				std::string typestr = magic_enum::enum_name(ConditionType(OR.type)).data();
				logger::debug("{} {:x} : {} Condition {}{}() is {}", obj->GetName(), obj->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, 
					isTrue ? "True" : "False");
			}
			else
			{
				logger::debug("{} {:x} : {} Condition {}{}({}{}{:x}) is {}", obj->GetName(), obj->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, OR.pluginName, OR.pluginName == "" ? "" : "|", OR.id,
					isTrue ? "True" : "False");
			}
		}
	};

	namespace ConditionFragment
	{
		class ConditionBase {
		public:
			virtual ~ConditionBase() = default;

			virtual void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) = 0;
			virtual bool IsValid(std::uint8_t option) { return true; };
			virtual bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) = 0;
		protected:
			bool isLeft = true;
		};

		class IsEquipped : public ConditionBase {
		public:
			IsEquipped() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsEquippedType : public ConditionBase {
		public:
			IsEquippedType() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::WEAPON_TYPE type = RE::WEAPON_TYPE::kHandToHandMelee;
		};

		class IsEquippedTypeAlt : public ConditionBase {
		public:
			IsEquippedTypeAlt() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			enum EquippedTypeAlt : std::uint32_t {
				kHandToHandMelee = 0,
				kOneHandSword = 1,
				kOneHandDagger = 2,
				kOneHandAxe = 3,
				kOneHandMace = 4,
				kTwoHandSword = 5,
				kTwoHandAxe = 6,
				kTwoHandWarHammer = 7,
				kBow = 8,
				kStaff = 9,
				kCrossbow = 10,
				kSpell = 11
			};
			EquippedTypeAlt type = EquippedTypeAlt::kHandToHandMelee;
		};

		class IsEquippedHasKeyword : public ConditionBase {
		public:
			IsEquippedHasKeyword() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::BGSKeyword* keyword = nullptr;
		};

		class IsEquippedHasKeywordEditorID : public ConditionBase {
		public:	
			IsEquippedHasKeywordEditorID() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::string keywordEditorID = "";
		};

		class IsEquippedPowerOrShout : public ConditionBase {
		public:
			IsEquippedPowerOrShout() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsWorn : public ConditionBase {
		public:
			IsWorn() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsWornSlot : public ConditionBase {
		public:
			IsWornSlot() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::uint32_t slot = 2;
		};

		class IsWornHasKeyword : public ConditionBase {
		public:
			IsWornHasKeyword() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::BGSKeyword* keyword;
			std::uint32_t slot = 255;
		};

		class IsWornHasKeywordEditorID : public ConditionBase {
		public:
			IsWornHasKeywordEditorID() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::string keywordEditorID = "";
			std::uint32_t slot = 255;
		};

		class IsInFaction : public ConditionBase {
		public:
			IsInFaction() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESFaction* faction = nullptr;
		};

		class HasKeyword : public ConditionBase {
		public:
			HasKeyword() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::BGSKeyword* keyword = nullptr;
		};

		class HasKeywordEditorID : public ConditionBase {
		public:
			HasKeywordEditorID() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::string keywordEditorID = "";
		};

		class HasMagicEffect : public ConditionBase {
		public:
			HasMagicEffect() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::EffectSetting* effect = nullptr;
		};

		class HasPerk : public ConditionBase {
		public:
			HasPerk() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::BGSPerk* perk = nullptr;
		};

		class HasSpell : public ConditionBase {
		public:
			HasSpell() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsActorBase : public ConditionBase {
		public:
			IsActorBase() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsActor : public ConditionBase {
		public:
			IsActor() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsObjectReference : public ConditionBase {
		public:
			IsObjectReference() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsRace : public ConditionBase {
		public:
			IsRace() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsClass : public ConditionBase {
		public:
			IsClass() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsCombatStyle : public ConditionBase {
		public:
			IsCombatStyle() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsFemale : public ConditionBase {
		public:
			IsFemale() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsChild : public ConditionBase {
		public:
			IsChild() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsPlayerTeammate : public ConditionBase {
		public:
			IsPlayerTeammate() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsUnique : public ConditionBase {
		public:
			IsUnique() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsSneaking : public ConditionBase {
		public:
			IsSneaking() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsSprinting : public ConditionBase {
		public:
			IsSprinting() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsSwimming : public ConditionBase {
		public:
			IsSwimming() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsInAir : public ConditionBase {
		public:
			IsInAir() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsDead : public ConditionBase {
		public:
			IsDead() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsAttacking : public ConditionBase {
		public:
			IsAttacking() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsAttackingType : public ConditionBase {
		public:
			IsAttackingType() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			enum AttackingType {
				kHandToHandMelee = 0,
				kOneHandSword = 1,
				kOneHandDagger = 2,
				kOneHandAxe = 3,
				kOneHandMace = 4,
				kTwoHandSword = 5,
				kTwoHandAxe = 6,
				kTwoHandWarHammer = 7,
				kBow = 8,
				kStaff = 9,
				kCrossbow = 10,
				kSpell = 11,
				kShout = 12
			};
			AttackingType type = AttackingType::kHandToHandMelee;
		};

		class IsAttackingHasKeyword : public ConditionBase {
		public:
			IsAttackingHasKeyword() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::BGSKeyword* keyword = nullptr;
		};

		class IsAttackingHasKeywordEditorID : public ConditionBase {
		public:
			IsAttackingHasKeywordEditorID() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::string keywordEditorID = "";
		};

		class IsBlocked : public ConditionBase {
		public:
			IsBlocked() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};
		
		class IsCritical : public ConditionBase {
		public:
			IsCritical() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsSneakCritical : public ConditionBase {
		public:
			IsSneakCritical() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsBash : public ConditionBase {
		public:
			IsBash() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsPowerAttack : public ConditionBase {
		public:
			IsPowerAttack() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsInanimateObject : public ConditionBase {
		public:
			IsInanimateObject() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class NoneCondition : public ConditionBase {
		public:
			NoneCondition() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};
	}
}
