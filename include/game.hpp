﻿#pragma once
#include "database.hpp"
namespace pokebot {
	namespace game {
		using ClientName = std::string;

		inline bool is_enabled_auto_waypoint = true;

		POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
			StatusIcon,
			Not_Displayed,
			Buy_Zone = 1 << 0,
			Defuser = 1 << 1,
			C4 = 1 << 2,
			Rescue_Zone = 1 << 3,
			Vip_Safety = 1 << 4,
			Escape_Zone = 1 << 5
		);

		POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
			Item,
			None,
			Nightvision=1<<0,
			Defuse_Kit=1<<1
		);

		enum class Weapon {
			None = -1,
			P228 = 1,
			Shield = 2,
			Scout = 3,
			HEGrenade = 4,
			XM1014 = 5,
			C4 = 6,
			MAC10 = 7,
			AUG = 8,
			Smoke = 9,
			Elite = 10,
			FiveSeven = 11,
			UMP45 = 12,
			SG550 = 13,
			Galil = 14,
			Famas = 15,
			USP = 16,
			Glock18 = 17,
			AWP = 18,
			MP5 = 19,
			M249 = 20,
			M3 = 21,
			M4A1 = 22,
			TMP = 23,
			G3SG1 = 24,
			Flashbang = 25,
			Deagle = 26,
			SG552 = 27,
			AK47 = 28,
			Knife = 29,
			P90 = 30,
			Armor = 31,
			ArmorHelm = 32,
			Defuser = 33
		};

		enum class AmmoID {
			None,
			Magnum338,
			Nato776,
			NatoBox556,
			Nato556,
			Buckshot,
			ACP45,
			MM57,
			AE50,
			SIG357,
			MM9
		};

		constexpr float Default_Max_Move_Speed = 255.0f;

		constexpr int Primary_Weapon_Bit = (common::ToBit<int>(Weapon::M3) | common::ToBit<int>(Weapon::XM1014) | common::ToBit<int>(Weapon::MAC10) | common::ToBit<int>(Weapon::TMP) | common::ToBit<int>(Weapon::MP5) | common::ToBit<int>(Weapon::UMP45) | common::ToBit<int>(Weapon::P90) | common::ToBit<int>(Weapon::Famas) | common::ToBit<int>(Weapon::Galil) | common::ToBit<int>(Weapon::AK47) | common::ToBit<int>(Weapon::M4A1) | common::ToBit<int>(Weapon::AUG) | common::ToBit<int>(Weapon::SG552) | common::ToBit<int>(Weapon::SG550) | common::ToBit<int>(Weapon::G3SG1) | common::ToBit<int>(Weapon::Scout) | common::ToBit<int>(Weapon::AWP) | common::ToBit<int>(Weapon::M249));
		constexpr int Secondary_Weapon_Bit = (common::ToBit<int>(Weapon::P228) | common::ToBit<int>(Weapon::USP) | common::ToBit<int>(Weapon::Deagle) | common::ToBit<int>(Weapon::FiveSeven) | common::ToBit<int>(Weapon::Glock18) | common::ToBit<int>(Weapon::Elite));
		constexpr int Melee_Bit = (common::ToBit<int>(Weapon::Knife));
		constexpr int Grenade_Bit = (common::ToBit<int>(Weapon::HEGrenade) | common::ToBit<int>(Weapon::Flashbang) | common::ToBit<int>(Weapon::Smoke));
		constexpr int C4_Bit = (common::ToBit<int>(Weapon::C4));

		enum class WeaponType {
			Secondary,
			Primary,
			Melee,
			Grenade,
			Special
		};

		constexpr WeaponType Weapon_Type[31]{
			WeaponType::Special,
			WeaponType::Secondary,//1
			WeaponType::Secondary,//2
			WeaponType::Primary,//3
			WeaponType::Grenade,//4
			WeaponType::Primary,//5
			WeaponType::Special,//6
			WeaponType::Primary,//7
			WeaponType::Primary,//8
			WeaponType::Grenade,//9
			WeaponType::Secondary,//10
			WeaponType::Secondary,//11
			WeaponType::Primary,//12
			WeaponType::Primary,//13
			WeaponType::Primary,//14
			WeaponType::Primary,//15
			WeaponType::Secondary,//16
			WeaponType::Secondary,//17
			WeaponType::Primary,//18
			WeaponType::Primary,//19
			WeaponType::Primary,//20
			WeaponType::Primary,//21
			WeaponType::Primary,//22
			WeaponType::Primary,//23
			WeaponType::Primary,//24
			WeaponType::Grenade,//25
			WeaponType::Secondary,//26
			WeaponType::Primary,//27
			WeaponType::Primary,//28
			WeaponType::Melee,//29
			WeaponType::Primary//30
		};

		using WeaponName = const char* const;
		constexpr std::tuple<WeaponName, AmmoID> Weapon_CVT[30]{
			std::make_tuple<WeaponName, AmmoID>("weapon_p228", AmmoID::SIG357),
			std::make_tuple<WeaponName, AmmoID>("weapon_shield", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_scout", AmmoID::Nato776),
			std::make_tuple<WeaponName, AmmoID>("weapon_hegrenade", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_xm1014", AmmoID::Buckshot),
			std::make_tuple<WeaponName, AmmoID>("weapon_c4", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_aug", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_mac10", AmmoID::ACP45),
			std::make_tuple<WeaponName, AmmoID>("weapon_smoke", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_elite", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_fiveseven", AmmoID::MM57),
			std::make_tuple<WeaponName, AmmoID>("weapon_ump45", AmmoID::ACP45),
			std::make_tuple<WeaponName, AmmoID>("weapon_sg550", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_galil", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_famas", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_usp", AmmoID::ACP45),
			std::make_tuple<WeaponName, AmmoID>("weapon_glock18", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_awp", AmmoID::Magnum338),
			std::make_tuple<WeaponName, AmmoID>("weapon_mp5navy", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_m249", AmmoID::NatoBox556),
			std::make_tuple<WeaponName, AmmoID>("weapon_m3", AmmoID::Buckshot),
			std::make_tuple<WeaponName, AmmoID>("weapon_m4a1", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_tmp", AmmoID::MM9),
			std::make_tuple<WeaponName, AmmoID>("weapon_g3sg1", AmmoID::Nato776),
			std::make_tuple<WeaponName, AmmoID>("weapon_flashbang", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_deagle", AmmoID::AE50),
			std::make_tuple<WeaponName, AmmoID>("weapon_sg552", AmmoID::Nato556),
			std::make_tuple<WeaponName, AmmoID>("weapon_ak47", AmmoID::Nato776),
			std::make_tuple<WeaponName, AmmoID>("weapon_knife", AmmoID::None),
			std::make_tuple<WeaponName, AmmoID>("weapon_p90", AmmoID::MM57)
		};

		// variable type
		enum class Var {
			Normal = 0,
			ReadOnly,
			Password,
			NoServer,
			GameRef
		};
		
		// ConVar class from YapBot © Copyright YaPB Project Developers
		// 
		// simplify access for console variables
		class ConVar final {
		public:
			cvar_t* ptr;

			ConVar() = delete;
			~ConVar() = default;

			ConVar(const char* name, const char* initval, Var type = Var::NoServer, bool regMissing = false, const char* regVal = nullptr);
			ConVar(const char* name, const char* initval, const char* info, bool bounded = true, float min = 0.0f, float max = 1.0f, Var type = Var::NoServer, bool regMissing = false, const char* regVal = nullptr);
			explicit operator bool() const noexcept { return ptr->value > 0.0f; }
			explicit operator int() const noexcept { return static_cast<int>(ptr->value); }
			explicit operator float() const noexcept { return ptr->value; }
			explicit operator const char* () const noexcept { return ptr->string;  }
		
			void operator=(const float val) noexcept { g_engfuncs.pfnCVarSetFloat(ptr->name, val); }
			void operator=(const int val) noexcept { operator=(static_cast<float>(val)); }
			void operator=(const char* val) noexcept { g_engfuncs.pfnCvar_DirectSet(ptr, const_cast<char*>(val)); }

		};

		extern ConVar poke_freeze;
		extern ConVar poke_fight;
		extern ConVar poke_buy;

		struct ConVarReg {
			cvar_t reg;
			std::string info;
			std::string init;
			const char* regval;
			class ConVar* self;
			float initial, min, max;
			bool missing;
			bool bounded;
			Var type;
		};

		class Client {
			friend class ClientManager;

			edict_t* const client{};
			int& button() { return client->v.button; }

			common::Team team{};
			int money{};
			StatusIcon status_icon{};
			Item item{};
			bool is_nvg_on{};
			bool is_vip{};

			int weapon_ammo[15]{};
			int weapon_clip{};
			Weapon current_weapon{};
		public:
			static bool IsDead(const edict_t* const Target) noexcept { return Target->v.deadflag == DEAD_DEAD || Target->v.health <= 0 || Target->v.movetype == MOVETYPE_NOCLIP; }
			static bool IsValid(const edict_t* const Target) noexcept { return (Target != nullptr && !Target->free); }
			static int Index(const edict_t* const Target) noexcept { return ENTINDEX(const_cast<edict_t*>(Target)); }

			Client() = delete;
			Client(const Client&) = delete;
			Client& operator=(const Client&) = delete;
		public:
			Client(edict_t* e) noexcept : client(e) {}

			edict_t* Edict() noexcept { return client; }
			const edict_t* Edict() const noexcept { return client; }
			operator edict_t* () noexcept { return Edict(); }
			operator const edict_t* () const noexcept { return client; }

			int Index() const noexcept { return Index(client); }

			auto Button() const noexcept { return client->v.button; }
			auto Impulse() const noexcept { return client->v.impulse; }
			const char* ClassName() const noexcept { return STRING(client->v.classname); }
			std::string_view Name() const noexcept { return STRING(client->v.netname); }
			void PressKey(const int Key) noexcept { client->v.button |= Key; }
			common::Team GetTeam() const noexcept { return common::GetTeamFromModel(client); }

			bool IsValid() const noexcept { return IsValid(client); }
			bool IsDead() const noexcept { return IsDead(client); }


			bool IsVIP() const noexcept { return is_vip; }

			int WeaponAmmo(const AmmoID Ammo_ID) const noexcept { return weapon_ammo[static_cast<int>(Ammo_ID)]; }

			bool HasHostages() const noexcept;

			const float& Health() const { return client->v.health; }
			const float& MaxHealth() const { return client->v.max_health; }
			const float& Speed() const { return client->v.speed; }
			const int& Money() const { return money; }

			Vector& view_ofs() { return client->v.view_ofs; }
			Vector& origin() { return client->v.origin; }
			Vector& angles() { return client->v.angles; }
			Vector& avelocity() { return client->v.avelocity; }
			Vector& punchangle() { return client->v.punchangle; }
			Vector& v_angle() { return client->v.v_angle; }
			float& ideal_yaw() { return client->v.ideal_yaw; }
			float& idealpitch() { return client->v.idealpitch; }
			int& flags() { return client->v.flags; }

			const Vector& view_ofs() const { return client->v.view_ofs; }
			const Vector& origin() const { return client->v.origin; }
			const Vector& angles() const { return client->v.angles; }
			const Vector& avelocity() const { return client->v.avelocity; }
			const Vector& punchangle() const { return client->v.punchangle; }
			const Vector& v_angle() const { return client->v.v_angle; }
			float ideal_yaw() const { return client->v.ideal_yaw; }
			float idealpitch() const { return client->v.idealpitch; }
			int flags() const { return client->v.flags; }
			int movetype() const { return client->v.movetype; }
			int weapons() const { return client->v.weapons; }
			int sequence() const { return client->v.sequence; }
			StatusIcon DisplayingStatusIcon() const noexcept { return status_icon; }
			Weapon CurrentWeapon() const noexcept { return current_weapon; }
			int CurrentWeaponClip() const noexcept { return weapon_clip; }
		};

		// The status in the game
		class ClientStatus {
			const Client* client;
		public:
			ClientStatus(const ClientName&);

			common::Team GetTeam() const noexcept;
			bool CanSeeFriend() const noexcept;
			ClientName GetEnemyNameWithinView() const noexcept;
			std::vector<ClientName> GetEntityNamesInView() const noexcept;
			common::Team GetTeamFromModel() const noexcept;
						
			/**
			* @brief Check the p_model animation is reloading.
			* @return True if player is reloading, false if the player is not reloading or is swimming.
			*/
			bool IsPlayerModelReloading() const noexcept;

			/**
			* @brief Check the v_model animation is reloading.
			* @return True if player is reloading, false if the player is not reloading.
			* @remarks This function does not work correctly if the player keeps holding a
			*/
			bool IsViewModelReloading() const noexcept;

			/**
			* @brief Check the client is a fakeclient.
			*/
			bool IsFakeClient() const noexcept;
			const float& Impulse() const { return client->Impulse(); }


			/**
			* @brief Check the clip of current weapon remains or not
			* @return 
			*/
			bool IsOutOfClip() const noexcept { return (Weapon_Type[static_cast<int>(client->CurrentWeapon())] == WeaponType::Primary || Weapon_Type[static_cast<int>(client->CurrentWeapon())] == WeaponType::Secondary) && client->CurrentWeaponClip() <= 0; }
			bool IsOutOfAmmo(const AmmoID Ammo_ID) const noexcept { return client->WeaponAmmo(Ammo_ID) <= 0; }
			bool IsOutOfCurrentWeaponAmmo() const noexcept { 
				return 
					(Weapon_Type[static_cast<int>(client->CurrentWeapon())] == WeaponType::Primary || 
					 Weapon_Type[static_cast<int>(client->CurrentWeapon())] == WeaponType::Secondary) && 
					IsOutOfAmmo(std::get<AmmoID>(Weapon_CVT[static_cast<int>(client->CurrentWeapon()) - 1])); 
			}

			bool IsInBuyzone() const noexcept { return bool(client->DisplayingStatusIcon() & StatusIcon::Buy_Zone); }
			bool IsInEscapezone() const noexcept { return bool(client->DisplayingStatusIcon() & StatusIcon::Escape_Zone); }
			bool IsInRescuezone() const noexcept { return bool(client->DisplayingStatusIcon() & StatusIcon::Rescue_Zone); }
			bool IsInVipSafety() const noexcept { return bool(client->DisplayingStatusIcon() & StatusIcon::Vip_Safety); }
			bool HasDefuser() const noexcept { return bool(client->DisplayingStatusIcon() & StatusIcon::Defuser); }

			bool IsDucking() const noexcept { return (client->flags() & FL_DUCKING); }
			bool IsInWater() const noexcept { return (client->flags() & FL_INWATER); }
			bool IsOnFloor() const noexcept { return (client->flags() & (FL_ONGROUND | FL_PARTIALGROUND)) != 0; }
			bool IsOnTrain() const noexcept { return (client->flags() & FL_ONTRAIN); }
			bool IsFiring() const noexcept { return (client->Button() & IN_ATTACK); }
			bool IsReadyToThrowGrenade() const noexcept { return IsFiring() && bool(client->weapons() & Grenade_Bit); }
			bool IsPlantingBomb() const noexcept { return IsFiring() && bool(client->weapons() & C4_Bit) && (client->sequence() == 63 || client->sequence() == 61); }
			bool IsClimblingLadder() const noexcept { return (client->movetype() & MOVETYPE_FLY); }

			const float& Health() const { return client->Health(); }
			const float& MaxHealth() const { return client->MaxHealth(); }
			const float& Speed() const { return client->Speed(); }
			const int& Money() const { return client->Money(); }

			const Vector& view_ofs() const { return client->view_ofs(); }
			const Vector& origin() const { return client->origin(); }
			const Vector& angles() const { return client->angles(); }
			const Vector& avelocity() const { return client->avelocity(); }
			const Vector& punchangle() const { return client->punchangle(); }
			const Vector& v_angle() const { return client->v_angle(); }
			float ideal_yaw() const { return client->ideal_yaw(); }
			float idealpitch() const { return client->idealpitch(); }
			int flags() const { return client->flags(); }
			int movetype() const { return client->movetype(); }
			int weapons() const { return client->weapons(); }
			int sequence() const { return client->sequence(); }
			StatusIcon DisplayingStatusIcon() const noexcept { return client->DisplayingStatusIcon(); }
			Weapon CurrentWeapon() const noexcept { return client->CurrentWeapon(); }
			int CurrentWeaponClip() const noexcept { return client->CurrentWeaponClip(); }
		};
		
		/**
		* 
		*/
		class ClientCommiter {
			friend class ClientManager;
			struct {
				std::vector<std::string> commands{};
			} commit_status;
		public:

		};


		class ClientManager {
			std::unordered_map<std::string, Client> clients{};
		public:
			void OnNewRound();
			ClientStatus GetClientStatus(std::string_view client_name);
			bool Create(std::string client_name);
			bool Register(edict_t*);
			auto& GetAll() const noexcept {
				return clients;
			}

			const Client* Get(const std::string& Name) const noexcept {
				if (auto it = clients.find(Name); it != clients.end()) {
					return &it->second;
				}
				return nullptr;
			}
						
			Client* GetAsMutable(const std::string& Name) noexcept {
				if (auto it = clients.find(Name); it != clients.end()) {
					return &it->second;
				}
				return nullptr;
			}

			void OnVIPChanged(const std::string_view Client_Name) noexcept;
			void OnDefuseKitEquiped(const std::string_view Client_Name) noexcept;
			void OnDeath(const std::string_view Client_Name) noexcept;
			void OnDamageTaken(const std::string_view Client_Name, const edict_t* Inflictor, const int Health, const int Armor, const int Bit) noexcept;
			void OnMoneyChanged(const std::string_view Client_Name, const int) noexcept;
			void OnScreenFaded(const std::string_view Client_Name) noexcept;
			void OnNVGToggled(const std::string_view Client_Name, const bool) noexcept;
			void OnWeaponChanged(const std::string_view Client_Name, const game::Weapon) noexcept;
			void OnClipChanged(const std::string_view Client_Name, const game::Weapon, const int) noexcept;
			void OnAmmoPickedup(const std::string_view Client_Name, const game::AmmoID, const int) noexcept;
			void OnTeamAssigned(const std::string_view Client_Name, common::Team) noexcept;
			void OnItemChanged(const std::string_view Client_Name, game::Item) noexcept;
			void OnStatusIconShown(const std::string_view Client_Name, const StatusIcon) noexcept;
		};

		struct Sound final {
			Vector origin{};
			int volume{};
		};

		POKEBOT_DEFINE_ENUM_WITH_BIT_OPERATOR(
			MapFlags,
			Assassination = 1 << 0,
			HostageRescue = 1 << 1,
			Demolition = 1 << 2,
			Escape = 1 << 3
		);

		class Hostage final {
			Hostage() = default;
			Hostage(const Hostage&);
			Hostage& operator=(const Hostage&) = delete;

			common::Time time{};

			const edict_t* entity;
			ClientName owner_name{};
		public:
			operator const edict_t* const () const noexcept {
				return entity;
			}
			void Update() noexcept;
			bool RecoginzeOwner(const ClientName&) noexcept;

			bool IsUsed() const noexcept;
			bool IsOwnedBy(const std::string_view& Name) const noexcept;
			bool IsReleased() const noexcept;
			static Hostage AttachHostage(const edict_t*) noexcept;
			const Vector& Origin() const noexcept;

			Hostage(Hostage&& h) noexcept {
				owner_name = std::move(h.owner_name);
				assert(h.owner_name.empty());
				entity = h.entity;
				h.entity = nullptr;
			}
		};

		class Host {
			edict_t* host{};
		public:
			const edict_t* AsEdict() const noexcept { return host; }
			bool IsHostValid() const noexcept;
			void SetHost(edict_t* const target) noexcept;
			const char* const HostName() const noexcept;
			const Vector& Origin() const noexcept;
			void Update();
		};

		inline class Game {
			database::Database database{};
			std::vector<Hostage> hostages{};

			std::vector<std::string> bot_args{};
			MapFlags map_flags{};
			uint32_t round{};
			bool is_newround{};

			std::vector<ConVarReg> convars{};
		public:
			Host host{};
			ClientManager clients{};

			size_t GetHostageNumber() const noexcept;
			bool IsHostageUsed(const int Index) const noexcept;
			bool IsHostageOwnedBy(const int Index, const std::string_view& Owner_Name);
			const edict_t* const GetClosedHostage(const Vector& Origin, const float Base_Distance);

			const std::string& GetBotArg(const size_t Index) const noexcept;
			size_t GetBotArgCount() const noexcept;
			bool IsBotCmd() const noexcept;

			size_t GetLives(const common::Team) const noexcept;	// Get the number of lives of the team.
			uint32_t CurrentRonud() const noexcept;
			bool IsCurrentMode(const MapFlags) const noexcept;
			MapFlags GetMapFlag() const noexcept;
			void IssueCommand(const ClientName&, const std::string& Sentence) noexcept;

			void Init(edict_t* entities, int max);
			void PreUpdate();
			void PostUpdate();

			void AddCvar(const char *name, const char *value, const char *info, bool bounded, float min, float max, Var varType, bool missingAction, const char *regval, ConVar *self);
			void RegisterCvars();

			// - Event funcs -

			void OnNewRound() noexcept;
		} game{};
	}

	namespace entity {
		bool CanSeeEntity(const edict_t* const self, const const edict_t* Target) noexcept;
		bool InViewCone(const edict_t* const self, const Vector& Origin) noexcept;
		bool IsVisible(const edict_t* const Self, const Vector& Origin) noexcept;
	}

	namespace engine {
		class ClientController {
		public:
			bool Connect();

		};

		class ClientKey final {
			const int Client_Index{};
			char* const infobuffer{};
		public:
			ClientKey(edict_t* target) noexcept;
			ClientKey& SetValue(const char* Key, const char* Value) noexcept;
		};
	}
}