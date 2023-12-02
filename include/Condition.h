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

		enum ConditionType : std::uint32_t {
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
			HasMagicEffectWithKeyword,
			HasMagicEffectWithKeywordEditorID,
			HasPerk,
			HasSpell,
			IsActorBase,
			IsActor,
			IsObjectReference,
			IsRace,
			IsClass,
			IsCombatStyle,

			IsStatusLessorThan,
			IsStatusGreaterThan,
			IsStatusEqual,
			IsLevelLessorThan,
			IsLevelGreaterThan,
			IsLevelEqual,

			IsDamageLesserThan, //TargetOnly
			IsDamageGreaterThan, //TargetOnly
			IsDamageEqual, //TargetOnly

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
			IsAttackWith, //AggressorOnly
			IsAttackWithType, //AggressorOnly
			IsAttackHasKeyword, //AggressorOnly
			IsAttackHasKeywordEditorID, //AggressorOnly
			IsAttackHasMagicEffect, //AggressorOnly

			IsFireAttack, //AggressorOnly
			IsFrostAttack, //AggressorOnly
			IsShockAttack, //AggressorOnly

			IsBlocked, //TargetOnly
			IsCritical, //TargetOnly
			IsSneakCritical, //TargetOnly
			IsBash, //AggressorOnly
			IsPowerAttack, //AggressorOnly
			IsInKillMove,

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

		enum VFXType : std::uint8_t {
			Impact,
			HitEffect,
			Total,

			Invalid
		};
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
			std::vector<RE::BGSArtObject*> ArtObjectItems;
			struct Option {
				float Scale = 1.0f;
				float Duration = 0.0f;
				bool RandomDirection = false;
			};
			Option option;

			std::string fileName;
			std::string originalCondition[ConditionOption::OptionTotal];
			std::vector<ConditionItemOr> AND[ConditionOption::OptionTotal];
		};
		static RE::ActorValue GetActorValueByString(std::string str);

		bool RegisterCondition(Condition condition);

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
			if (IsContainString(typestr, "EditorID") 
				|| IsContainString(typestr, "Type"))
			{
				logger::debug("{} {:x} : {} Condition {}{}({}{}{}) is {}", obj->GetName(), obj->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, OR.arg, 
					OR.arg2.empty() ? "" : "|", OR.arg2.empty() ? "" : OR.arg2,
					isTrue ? "True" : "False");
			}
			else if (OR.type >= ConditionType::IsFemale 
				&& OR.type != ConditionType::IsAttackWith 
				&& OR.type != ConditionType::IsAttackHasKeyword
				&& OR.type != ConditionType::IsAttackHasMagicEffect)
			{
				std::string typestr = magic_enum::enum_name(ConditionType(OR.type)).data();
				logger::debug("{} {:x} : {} Condition {}{}() is {}", obj->GetName(), obj->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, 
					isTrue ? "True" : "False");
			}
			else
			{
				logger::debug("{} {:x} : {} Condition {}{}({}{}{:x}{}{}) is {}", obj->GetName(), obj->formID,
					magic_enum::enum_name(ConditionOption(option)).data(), OR.NOT ? "NOT " : "", typestr, OR.pluginName, OR.pluginName.empty() ? "" : "|", OR.id,
					OR.arg2.empty() ? "" : "|", OR.arg2.empty() ? "" : OR.arg2,
					isTrue ? "True" : "False");
			}
		}

		inline bool stringStartsWith(std::string str, std::string prefix)
		{
			str = lowLetter(str);
			prefix = lowLetter(prefix);
			return str.starts_with(prefix);
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
			RE::BGSKeyword* keyword = nullptr;
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

		class HasMagicEffectWithKeyword : public ConditionBase {
		public:
			HasMagicEffectWithKeyword() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::BGSKeyword* keyword = nullptr;
		};

		class HasMagicEffectWithKeywordEditorID : public ConditionBase {
		public:
			HasMagicEffectWithKeywordEditorID() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::string keywordEditorID = "";
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
			RE::SpellItem* spell = nullptr;
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

		class IsStatusLessorThan : public ConditionBase {
		public:
			IsStatusLessorThan() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::ActorValue type = RE::ActorValue::kNone;
			float status = 0;
		};

		class IsStatusGreaterThan : public ConditionBase {
		public:
			IsStatusGreaterThan() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::ActorValue type = RE::ActorValue::kNone;
			float status = 0;
		};

		class IsStatusEqual : public ConditionBase {
		public:
			IsStatusEqual() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::ActorValue type = RE::ActorValue::kNone;
			float status = 0;
		};

		class IsLevelLessorThan : public ConditionBase {
		public:
			IsLevelLessorThan() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::uint16_t level = 0;
		};

		class IsLevelGreaterThan : public ConditionBase {
		public:
			IsLevelGreaterThan() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::uint16_t level = 0;
		};

		class IsLevelEqual : public ConditionBase {
		public:
			IsLevelEqual() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::uint16_t level = 0;
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

		class IsAttackWith : public ConditionBase {
		public:
			IsAttackWith() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::TESForm* form = nullptr;
		};

		class IsAttackWithType : public ConditionBase {
		public:
			IsAttackWithType() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			enum AttackingWeaponType {
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
			AttackingWeaponType type = AttackingWeaponType::kHandToHandMelee;
		};

		class IsAttackHasKeyword : public ConditionBase {
		public:
			IsAttackHasKeyword() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::BGSKeyword* keyword = nullptr;
		};

		class IsAttackHasKeywordEditorID : public ConditionBase {
		public:
			IsAttackHasKeywordEditorID() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::string keywordEditorID = "";
		};

		class IsAttackHasMagicEffect : public ConditionBase {
		public:
			IsAttackHasMagicEffect() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			RE::EffectSetting* effect = nullptr;
		};

		class IsFireAttack : public ConditionBase {
		public:
			IsFireAttack() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsFrostAttack : public ConditionBase {
		public:
			IsFrostAttack() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsShockAttack : public ConditionBase {
		public:
			IsShockAttack() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
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

		class IsInKillMove : public ConditionBase {
		public:
			IsInKillMove() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsInanimateObject : public ConditionBase {
		public:
			IsInanimateObject() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};

		class IsDamageLesserThan : public ConditionBase {
		public:
			IsDamageLesserThan() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			float damage = 0.0f;
		};

		class IsDamageGreaterThan : public ConditionBase {
		public:
			IsDamageGreaterThan() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			float damage = 0.0f;
		};

		class IsDamageEqual : public ConditionBase {
		public:
			IsDamageEqual() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool IsValid(std::uint8_t option) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		private:
			std::int32_t damage = 0.0f;
		};

		class NoneCondition : public ConditionBase {
		public:
			NoneCondition() = default;
			void Initial(ConditionManager::ConditionItem& item, bool IsLeft = true) override;
			bool Condition(RE::TESObjectREFR* ref, RE::Actor* actor, const HitEvent& e) override;
		};
	}
}
