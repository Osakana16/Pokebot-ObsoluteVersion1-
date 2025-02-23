#include "behavior.hpp"
#include "bot/manager.hpp"

#include <thread>

namespace pokebot::bot {
	void Bot::Run() POKEBOT_NOEXCEPT {
		(this->*(updateFuncs[static_cast<int>(start_action)]))();
		frame_interval = gpGlobals->time - last_command_time;

		const std::uint8_t Msec_Value = ComputeMsec();
		last_command_time = gpGlobals->time;

		game::Client* client = const_cast<game::Client*>(game::game.clients.Get(Name().data()));
		client->flags |= common::Third_Party_Bot_Flag;
		g_engfuncs.pfnRunPlayerMove(client->Edict(),
				movement_angle,
				move_speed,
				strafe_speed,
				0.0f,
				client->button,
				client->impulse,
				Msec_Value);

		move_speed = strafe_speed = 0.0f;
		UnlockByBomb();
	}

	void Bot::TurnViewAngle() {
		auto client = game::game.clients.Get(Name().data());
		assert(client != nullptr);
		Vector destination{};
		common::OriginToAngle(&destination, *look_direction.view, Origin());
		if (destination.x > 180.0f) {
			destination.x -= 360.0f;
		}

		auto v_angle = client->v_angle;
		v_angle.x = destination.x;

		if (std::abs(destination.y - v_angle.y) <= 1.0f) {
			v_angle.y = destination.y;
		} else {
			auto AngleClamp = [](const float angle, const float destination) {
				return (angle > destination ? std::clamp(angle, destination, angle) : std::clamp(angle, angle, destination));
			};

			auto CalculateNextAngle = [](const float dest, const float angle) POKEBOT_NOEXCEPT{
				return std::clamp(dest - angle, -180.0f, 180.0f);
			};

			constexpr float Base_Frame = 30.0f;
			constexpr float Sensitivity = 1.0f;
			const Vector Next_Angle = {
				CalculateNextAngle(destination.x, v_angle.x) / (Base_Frame - Sensitivity),
				CalculateNextAngle(destination.y, v_angle.y) / (Base_Frame - Sensitivity),
				0.0
			};

			v_angle.y += Next_Angle.y;
			v_angle.y = AngleClamp(v_angle.y, destination.y);
		}
		v_angle.z = 0.0f;
		client->v_angle = v_angle;
		client->angles.x = client->v_angle.x / 3;
		client->angles.y = client->v_angle.y;
		client->v_angle.x = -client->v_angle.x;
		client->angles.z = 0;
			
		client->ideal_yaw = client->v_angle.y;
		if (client->ideal_yaw > 180.0f) {
			client->ideal_yaw -= 360.0f;
		} else if (client->ideal_yaw < -180.0f) {
			client->ideal_yaw += 360.0f;
		}
			
		client->idealpitch = client->v_angle.x;
		if (client->idealpitch > 180.0f) {
			client->idealpitch-= 360.0f;
		} else if (client->idealpitch < -180.0f) {
			client->idealpitch += 360.0f;
		}
	}

	void Bot::TurnMovementAngle() {
		common::OriginToAngle(&movement_angle, *look_direction.movement, Origin());
	}

	void Bot::OnNewRound() POKEBOT_NOEXCEPT {
		goal_queue.Clear();
		routes.Clear();

		goal_node = node::Invalid_NodeID;
		next_dest_node = node::Invalid_NodeID;

		LeavePlatoon();
		look_direction.Clear();
		ideal_direction.Clear();

		danger_time.SetTime(0);
		freeze_time.SetTime(g_engfuncs.pfnCVarGetFloat("mp_freezetime") + 1.0f);
		spawn_cooldown_time.SetTime(1.0f);

		start_action = Message::Buy;
		buy_wait_timer.SetTime(1.0f);

		// mapdata::BSP().LoadWorldMap();
	}

	void Bot::NormalUpdate() POKEBOT_NOEXCEPT {
		auto client = game::game.clients.GetAsMutable(Name().data());
		assert(client != nullptr);
		assert(JoinedTeam() != common::Team::Spector && JoinedTeam() != common::Team::Random);
		if (client->IsDead()) {
			return;
		}

		if (!freeze_time.IsRunning() && !spawn_cooldown_time.IsRunning()) {
			// Do not do anything when freezing.
			// Due to game engine specifications or bugs, 
			// if we execute a heavy process immediately after the start of a round, 
			// the game will freeze.
			
			if (state != State::Crisis && Manager::Instance().IsFollowerPlatoon(JoinedTeam(), JoinedPlatoon())) {
				state = State::Follow;
			}

#if 0
			// NOTE: This code makes the game laggy so it is temporary disabled.
			// 
			// If the bot is not in the route, reset it.
			if (auto current_id = node::czworld.GetNearest(Origin()); current_id != nullptr) {
				if (!routes.Contains(current_id->m_id)) {
					routes.Clear();
				}
			}
#endif
			(this->*(doObjective[static_cast<int>(state)]))();

			// - Bot Navigation -

			CheckAround();		// Update the entity's viewment.
			CheckBlocking();	// Check the something is blocking myself.

			if (IsJumping()) {
				PressKey(ActionKey::Duck);
			}

			// Move forward if the bot has the route.
			if (!routes.Empty() && next_dest_node != node::Invalid_NodeID) {
#if 0
				if (!stuck_check_interval_timer.IsRunning()) {
					if (common::Distance(Origin(), stuck_check_origin) <= 5.0f) {
						// Stuck detected.
						routes.Clear();
					}
					stuck_check_origin = Origin();
					stuck_check_interval_timer.SetTime(1.0f);
				}
#endif
				if (look_direction.view.has_value() && look_direction.movement.has_value()) {
					PressKey(ActionKey::Run);
				}
			}
			look_direction.Clear();

			// - Update Key -

			if (bool(press_key & ActionKey::Run)) {
				if (bool(press_key & ActionKey::Shift)) {
					press_key &= ~ActionKey::Shift;
					move_speed = game::Default_Max_Move_Speed / 2.0f;
				} else {
					move_speed = game::Default_Max_Move_Speed;
				}
			}

			if (bool(press_key & ActionKey::Move_Right)) {
				strafe_speed = game::Default_Max_Move_Speed;
			}

			if (bool(press_key & ActionKey::Move_Left)) {
				strafe_speed = -game::Default_Max_Move_Speed;
			}

			if (bool(press_key & ActionKey::Back)) {

			}

			if (bool(press_key & ActionKey::Attack)) {

			}

			if (bool(press_key & ActionKey::Attack2)) {

			}

			if (bool(press_key & ActionKey::Use)) {

			}

			if (bool(press_key & ActionKey::Jump)) {
				if (!client->IsOnFloor()) {
					return;
				}
			}

			if (bool(press_key & ActionKey::Duck)) {

			}

			client->PressKey(static_cast<int>(press_key));
			press_key = ActionKey::None;
		}
	}

	void Bot::OnSelectionCompleted() POKEBOT_NOEXCEPT {
		Manager::Instance().OnBotJoinedCompletely(this);
		start_action = Message::Buy;
	}

	void Bot::AccomplishMission() POKEBOT_NOEXCEPT {
		auto current_mode = game::game.GetMapFlag();
		(this->*(accomplishState[static_cast<int>(JoinedTeam()) - 1][static_cast<int>(std::log2(static_cast<float>(current_mode)))]))();
	}

	void Bot::OnTerroristDemolition() noexcept {
		if (Manager::Instance().C4Origin().has_value()) {
			behavior::demolition::t_planted_wary->Evaluate(this);
		} else if (Manager::Instance().BackpackOrigin().has_value()) {
			behavior::demolition::t_pick_up_bomb->Evaluate(this);
		} else {
			if (HasWeapon(game::Weapon::C4)) {
				behavior::demolition::t_plant->Evaluate(this);
			}
		}
	}

	void Bot::OnTerroristHostage() noexcept {
		
	}

	void Bot::OnTerroristAssasination() noexcept {
		auto client = game::game.clients.Get(Name().data());
		if (client->IsVIP()) {
			behavior::assassination::ct_vip_escape->Evaluate(this);
		} else {

		}
	}

	void Bot::OnTerroristEscape() noexcept {

	}

	void Bot::OnCTDemolition() noexcept {
		if (Manager::Instance().C4Origin().has_value()) {
			if (common::Distance(Origin(), *Manager::Instance().C4Origin()) <= 50.0f) {
				behavior::demolition::ct_defusing->Evaluate(this);
			} else {
				behavior::demolition::ct_planted->Evaluate(this);
			}
		} else {
			behavior::demolition::ct_defend->Evaluate(this);
		}
	}

	void Bot::OnCTHostage() noexcept {
		auto client = game::game.clients.Get(Name().data());
		if (!client->HasHostages()) {
			behavior::rescue::ct_try->Evaluate(this);
		} else {
			behavior::rescue::ct_leave->Evaluate(this);
		}
	}

	void Bot::OnCTAssasination() noexcept {
		auto client = game::game.clients.Get(Name().data());
		if (client->IsVIP()) {
			behavior::assassination::ct_vip_escape->Evaluate(this);
		} else {

		}
	}

	void Bot::OnCTEscape() noexcept {

	}

	void Bot::Combat() POKEBOT_NOEXCEPT {
		auto client = game::game.clients.Get(Name().data());
		assert(!target_enemies.empty() && "The bot has no enemies despite being in combat mode.");
		/*
			Choose to fight or to flee.
			The bot has 'fighting spirit' that affects willingness to fight.
		*/

		constexpr int Max_Health = 100;
		constexpr int Max_Armor = 100;
		constexpr int Base_Fearless = Max_Health / 2 + Max_Armor / 5;

		const game::Client *enemy_client{};
		for (auto& target_name : target_enemies) {
			auto target_client = game::game.clients.Get(target_name.c_str());
			if (!target_client->IsDead()) {
				enemy_client = target_client;
				break;
			}
		}
		if (enemy_client == nullptr) {
			danger_time.SetTime(0.0f);
			return;
		}

		const bool Enemy_Has_Primary = enemy_client->HasPrimaryWeapon();
		const bool Enemy_Has_Secondary = enemy_client->HasSecondaryWeapon();

		int fighting_spirit = Max_Health + Max_Armor + (HasPrimaryWeapon() ? 50 : 0) + (HasSecondaryWeapon() ? 50 : 0);
		if (!HasPrimaryWeapon() && !HasSecondaryWeapon() ||
			(enemy_client->IsOutOfClip() && enemy_client->IsOutOfCurrentWeaponAmmo())) {
			// If I have no guns.
			if (Enemy_Has_Primary || Enemy_Has_Secondary) {
				// The enemy has weapon so I should flee.
				fighting_spirit = std::numeric_limits<decltype(fighting_spirit)>::min();
			} else {
				// This is the good chance to beat enemies.
			}
		} else {
			if (IsReloading() || enemy_client->IsOutOfClip()) {
				// I'm reloading so I have to flee.
				fighting_spirit = std::numeric_limits<decltype(fighting_spirit)>::min();
			} else {
				// If I have guns.
				fighting_spirit -= (Max_Health - Health());
				fighting_spirit -= (Max_Armor - enemy_client->armor);
				// fighting_spirit -= (Enemy_Has_Primary ? 25 : 0);
				// fighting_spirit -= (Enemy_Has_Secondary ? 25 : 0);
			}
		}

		if (fighting_spirit <= Base_Fearless) {
			behavior::fight::retreat->Evaluate(this);
		}

		if (CanSeeEntity(enemy_client->Edict())) {
		behavior::fight::beat_enemies->Evaluate(this);
	}
	}

	void Bot::Follow() POKEBOT_NOEXCEPT {
		const auto Leader_Client = Manager::Instance().GetLeader(JoinedTeam(), JoinedPlatoon());
		assert(Leader_Client != nullptr);
		const auto Leader_Origin = Leader_Client->origin;
		if (const auto Leader_Area = node::czworld.GetNearest(Leader_Origin); Leader_Area != nullptr) {
			if (goal_node == node::Invalid_NodeID) {
				goal_node = Leader_Area->m_id;
			}

			if (!routes.Empty() && !routes.IsEnd()) {
				if (const auto Current_Area = node::czworld.GetNearest(Origin()); Current_Area != nullptr && Current_Area != Leader_Area) {
					// - Check -
					const auto Current_Node_ID = Current_Area->m_id;
					if (goal_node == Current_Node_ID) {
						// The bot is already reached at the destination.
						goal_queue.Clear();
						routes.Clear();
						goal_node = node::Invalid_NodeID;
					} else {
						if (next_dest_node == node::Invalid_NodeID) {
							next_dest_node = routes.Current();
						} else if (node::czworld.IsOnNode(Origin(), next_dest_node)) {
							if (!routes.IsEnd()) {
								if (routes.Next()) {
									next_dest_node = routes.Current();
								}
							}
						}
					}
				}
			} else {
				node::czworld.FindPath(&routes, Origin(), node::czworld.GetOrigin(goal_node), JoinedTeam());
				if (routes.Empty()) {
					goal_node = node::Invalid_NodeID;
				}
			}

			const auto Ditance_To_Leader = common::Distance(Origin(), Leader_Origin);
			if (Ditance_To_Leader <= 275.0f) {
				if (Ditance_To_Leader >= 75.0f) {
					if (Leader_Client->IsDucking()) {
						PressKey(ActionKey::Duck);
					} else if (Leader_Client->IsWalking()) {
						PressKey(ActionKey::Run | ActionKey::Shift);
					} else {
						PressKey(ActionKey::Run);
					}
				}
			} else {
				PressKey(ActionKey::Run);
			}
		}
	}

	template<typename Array>
	std::map<float, int> SortedDistances(const Vector& Base, const Array& list) {
		std::map<float, int> result{};
		for (int i = 0; i < list.size(); i++) {
			result[common::Distance(Base, game::game.clients.Get(list[i].c_str())->origin)] = i;
		}
		return result;
	}

	void Bot::CheckAround() {
		if (!look_direction.view.has_value()) {
#if !USE_NAVMESH
			look_direction.view = node::world.GetOrigin(next_dest_node);
#else
			look_direction.view = node::czworld.GetOrigin(next_dest_node);
			look_direction.view->z = Origin().z;
#endif
		}

		auto next_origin = node::czworld.GetOrigin(next_dest_node);
		if (!look_direction.movement.has_value()) {
#if !USE_NAVMESH
			look_direction.movement = node::world.GetOrigin(next_dest_node);
#else
			look_direction.movement = next_origin;
			look_direction.movement->z = Origin().z;
#endif
		}


		TurnViewAngle();
		TurnMovementAngle();
#if 1
		if (game::poke_fight) {
			auto client = game::game.clients.Get(Name().data());
			common::PlayerName enemies_in_view[32]{};
			int i{};

			client->GetEnemyNamesWithinView(enemies_in_view);
			if (!enemies_in_view[0].empty()) {
				for (auto& enemy : enemies_in_view) {
					if (enemy.empty()) {
						break;
					}
					if (std::find(target_enemies.begin(), target_enemies.end(), enemy) == target_enemies.end()) {
						target_enemies.push_back(enemy);
					}
				}
				danger_time.SetTime(5.0);
			}
		}
#endif
		if (!danger_time.IsRunning()) {
			target_enemies.clear();
			state = State::Accomplishment;
		} else {
			state = State::Crisis;
		}
	}

	void Bot::CheckBlocking() noexcept {
		if (is_locked_by_bomb)
			return;

		// Check if the player is blocking and avoid it.
		for (edict_t* entity{}; (entity = common::FindEntityInSphere(entity, Origin(), 90.0f)) != nullptr;) {
			if (std::string_view(STRING(entity->v.classname)) == "player") {
				if (CanSeeEntity(entity)) {
					PressKey(ActionKey::Move_Left);
					return;
				}
			}
		}

		// Check if the worldspawn is blocking:
		const auto Foot = Origin();
		const auto Head = Foot + game::game.clients.Get(Name().data())->view_ofs;

		common::Tracer tracer{};
		const bool Is_Head_Forward_Center_Hit = tracer.MoveStart(Head).MoveDest(Head + gpGlobals->v_forward * 90.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr).IsHit();
		const bool Is_Head_Forward_Left_Hit = tracer.MoveDest(Head + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * -45.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr).IsHit();
		const bool Is_Head_Forward_Right_Hit = tracer.MoveDest(Head + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * 45.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr).IsHit();
		const bool Is_Head_Forward_Hit = Is_Head_Forward_Center_Hit || Is_Head_Forward_Left_Hit || Is_Head_Forward_Right_Hit;

		if (Is_Head_Forward_Hit) {
			// Check left
			tracer.MoveDest(Head + gpGlobals->v_right * -90.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr);
			if (tracer.IsHit()) {
				PressKey(ActionKey::Move_Right);
			} else {
				// Check right
				tracer.MoveDest(Head + gpGlobals->v_right * 90.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr);
				if (tracer.IsHit()) {
					PressKey(ActionKey::Move_Left);
				}
			}
		} else {
			const bool Is_Foot_Forward_Center_Hit = tracer.MoveStart(Foot).MoveDest(Foot + gpGlobals->v_forward * 90.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr).IsHit();
			const bool Is_Foot_Forward_Left_Hit = tracer.MoveDest(Foot + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * -45.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr).IsHit();
			const bool Is_Foot_Forward_Right_Hit = tracer.MoveDest(Foot + gpGlobals->v_forward * 90.0f + gpGlobals->v_right * 45.0f).TraceLine(common::Tracer::Monsters::Ignore, common::Tracer::Glass::Dont_Ignore, nullptr).IsHit();
			const bool Is_Foot_Forward_Hit = Is_Foot_Forward_Center_Hit || Is_Foot_Forward_Left_Hit || Is_Foot_Forward_Right_Hit;
			if (Is_Foot_Forward_Hit) {
				PressKey(ActionKey::Jump);
			} else {
				// Check the forward navarea
				for (auto& vector : { Origin(), Origin() + gpGlobals->v_forward * 90.0f }) {
					// 
					if (auto area = node::czworld.GetNearest(vector); area != nullptr) {
						// Jump if it is specified.
						if (!node::czworld.HasFlag(area->m_id, node::NavmeshFlag::No_Jump) && node::czworld.HasFlag(area->m_id, node::NavmeshFlag::Jump)) {
							PressKey(ActionKey::Jump);
						}
						// Duck if it is specified.
						if (node::czworld.HasFlag(area->m_id, node::NavmeshFlag::Crouch)) {
							PressKey(ActionKey::Duck);
						}
					}
				}
			}
		}
	}

	
	bool Bot::CanSeeEntity(const edict_t* entity) const POKEBOT_NOEXCEPT {
		auto client = game::game.clients.Get(Name().data()); client->CanSeeEntity(entity);
		return client->CanSeeEntity(entity);
	}
	
	void Bot::PressKey(ActionKey pressable_key) {
		press_key |= pressable_key;
	}

	bool Bot::IsPressingKey(const ActionKey Key) const POKEBOT_NOEXCEPT {
		auto client = game::game.clients.Get(Name().data());
		return (client->button & static_cast<int>(Key));
	}

	void Bot::SelectionUpdate() POKEBOT_NOEXCEPT {
		int value{};
		switch (start_action) {
			case Message::Team_Select:
			{
				// assert(JoinedTeam() != common::Team::T && JoinedTeam()  != common::Team::CT);
				value = static_cast<int>(JoinedTeam());
				break;
			}
			case Message::Model_Select:
			{
				assert(JoinedTeam() != common::Team::Spector && JoinedTeam() != common::Team::Random);
				start_action = Message::Selection_Completed;
				value = static_cast<int>(model);
				break;
			}
			default:
			{
				assert(false);
				return;
			}
		}
		game::game.IssueCommand(Name().data(), std::format("menuselect {}", value).c_str());
	}

	void Bot::SelectWeapon(const game::Weapon Target_Weapon) {
		if (HasWeapon(Target_Weapon)) {
			current_weapon = Target_Weapon;
			game::game.IssueCommand(Name().data(), std::format("{}", std::get<game::WeaponName>(game::Weapon_CVT[static_cast<int>(Target_Weapon) - 1])).c_str());
		}
	}

	void Bot::SelectPrimaryWeapon() {
		SelectWeapon(static_cast<game::Weapon>(std::log2(game::game.clients.Get(Name().data())->weapons & game::Primary_Weapon_Bit)));
	}

	void Bot::SelectSecondaryWeapon() {
		SelectWeapon(static_cast<game::Weapon>(std::log2(game::game.clients.Get(Name().data())->weapons & game::Secondary_Weapon_Bit)));
	}

	void Bot::LookAtClosestEnemy() {
		if (target_enemies.empty()) {
			return;
		}
		const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
		const auto& Nearest_Enemy = game::game.clients.Get(target_enemies[Enemy_Distances.begin()->second].c_str());
		look_direction.view = Nearest_Enemy->origin - Vector(20.0f, 0, 0);
	}

	bool Bot::HasEnemy() const POKEBOT_NOEXCEPT {
		return !target_enemies.empty();
	}

	bool Bot::IsLookingAtEnemy() const POKEBOT_NOEXCEPT {
		if (target_enemies.empty()) {
			return false;
		}

		const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
		const auto& Nearest_Enemy = game::game.clients.Get(target_enemies[Enemy_Distances.begin()->second].c_str());
		return IsLookingAt(Nearest_Enemy->origin, 1.0f);
	}

	bool Bot::IsEnemyFar() const POKEBOT_NOEXCEPT {
		if (target_enemies.empty()) {
			return false;
		}
		const auto Enemy_Distances = std::move(SortedDistances(Origin(), target_enemies));
		const auto& Nearest_Enemy = game::game.clients.Get(target_enemies[Enemy_Distances.begin()->second].c_str());
		return common::Distance(Origin(), Nearest_Enemy->origin) > 1000.0f;
	}

	bool Bot::IsLookingAt(const Vector& Dest, const float Range) const POKEBOT_NOEXCEPT {
		float vecout[3]{};
		Vector angle = Dest - Origin();
		VEC_TO_ANGLES(angle, vecout);
		if (vecout[0] > 180.0f)
			vecout[0] -= 360.0f;
		vecout[0] = -vecout[0];
		auto result = common::Distance2D(Vector(vecout), game::game.clients.Get(Name().data())->v_angle);
		return (result <= Range);
	}

	bool Bot::HasGoalToHead() const POKEBOT_NOEXCEPT {
		return goal_node != node::Invalid_NodeID;
	}


	bool Bot::IsInBuyzone() const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->IsInBuyzone(); }

	const common::PlayerName& Bot::Name() const POKEBOT_NOEXCEPT { return name; }

	Vector Bot::Origin() const POKEBOT_NOEXCEPT {
		return game::game.clients.Get(Name().data())->origin;
	}

	float Bot::Health() const POKEBOT_NOEXCEPT {
		return game::game.clients.Get(Name().data())->health;
	}

	bool Bot::HasPrimaryWeapon() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->HasPrimaryWeapon()); }
	bool Bot::HasSecondaryWeapon() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->HasSecondaryWeapon()); }
	bool Bot::HasWeapon(const game::Weapon Weapon_ID) const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->HasWeapon(Weapon_ID); }
	bool Bot::IsDucking() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsDucking()); }
	bool Bot::IsDriving() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsOnTrain()); }
	bool Bot::IsInWater() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsInWater()); }
	bool Bot::IsSwimming() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsInWater()); }
	bool Bot::IsOnFloor() const POKEBOT_NOEXCEPT { return (game::game.clients.Get(Name().data())->IsOnFloor()); }
	bool Bot::IsClimbingLadder() const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->IsClimblingLadder(); }
	bool Bot::IsReloading() const POKEBOT_NOEXCEPT { return game::game.clients.Get(Name().data())->IsPlayerModelReloading(); }
	bool Bot::IsPlantingBomb() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsChangingWeapon() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsFalling() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::Jumped() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsJumping() const POKEBOT_NOEXCEPT { return !game::game.clients.Get(Name().data())->IsOnFloor(); }
	bool Bot::IsLeadingHostages() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsLookingThroughScope() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsLookingThroughCamera() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsChangingSilencer() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsEnabledFlashlight() const POKEBOT_NOEXCEPT { return false; }
	bool Bot::IsEnabledNightvision() const POKEBOT_NOEXCEPT { return false; }

	uint8_t Bot::ComputeMsec() POKEBOT_NOEXCEPT {
		return static_cast<std::uint8_t>(std::min({ static_cast<int>(std::roundf((gpGlobals->time - last_command_time) * 1000.0f)), 255 }));
	}

	void Bot::OnRadioRecieved(const std::string_view& Sender_Name, const std::string_view& Radio_Sentence) POKEBOT_NOEXCEPT {
		static bool is_sent{};
		const std::unordered_map<common::fixed_string<32u>, std::function<void()>, common::fixed_string<32u>::Hash> Radios{
			{
				"#Cover_me",
				[] {

				}
			},
			{
				"#You_take_the_point",
				[] {

				}
			},
			{
				"#Hold_this_position",
				[] {

				}
			},
			{
				"#Regroup_team",
				[&] {
#if !USE_NAVMESH
						goal_queue.AddGoalQueue(node::world.GetNearest(game::game.clients.Get(Sender_Name)->origin));
#else
						goal_queue.AddGoalQueue(node::czworld.GetNearest(Origin())->m_id);
#endif
					}
				},
				{
					"#Follow_me",
					[] {

					}
				},
				{
					"#Taking_fire",
					[] {

					}
				},
				{
					"#Go_go_go",
					[this] {
						game::game.IssueCommand(Name().data(), "radio3");
						game::game.IssueCommand(Name().data(), "menuselect 1");
					}
				},
				{
					"#Team_fall_back",
					[] {

					}
				},
				{
					"#Stick_together_team",
					[] {

					}
				},
				{
					"#Get_in_position_and_wait",
					[] {

					}
				},
				{
					"#Storm_the_front", [] {

					}
				},
				{
					"#Report_in_team", [] {

					}
				},
				{ "#Affirmative", [] {}},
				{ "#Roger_that", [] {} },
				{
					"#Enemy_spotted", [] {

					}
				},
				{
					"#Need_backup", [] {

					}
				},
				{
					"#Sector_clear", [] {

					}
				},
				{
					"#In_position", [] {

					}
				},
				{
					"#Reporting_in", [] {

					}
				},
				{
					"#Get_out_of_there", [] {

					}
				},
				{
					"#Negative", [] {

					}
				},
				{
					"#Enemy_down", [] {

					}
				},
				{
					"#Fire_in_the_hole", [] {}
				}
		};
		Radios.at(Radio_Sentence.data())();
	}


	void Bot::OnBombPlanted() POKEBOT_NOEXCEPT {
		switch (JoinedTeam()) {
			case common::Team::CT:
				goal_queue.Clear();
				break;
		}
	}

	Bot::Bot(const std::string_view& Bot_Name, const common::Team Join_Team, const common::Model Select_Model) POKEBOT_NOEXCEPT {
		team = Join_Team;
		model = Select_Model;

		name = Bot_Name.data();

		OnNewRound();
	}

	PlatoonID Bot::JoinedPlatoon() const POKEBOT_NOEXCEPT {
		return platoon;
	}

	common::Team Bot::JoinedTeam() const POKEBOT_NOEXCEPT {
		return team;
	}

	void Bot::JoinPlatoon(const PlatoonID Target_Platoon) noexcept {
		assert(Target_Platoon.has_value());
		platoon = *Target_Platoon;
	}

	void Bot::ReceiveCommand(const TroopsStrategy& Received_Strategy) {
		switch (Received_Strategy.strategy) {
			case TroopsStrategy::Strategy::Follow:
			{
				assert(!Received_Strategy.leader_name.empty());
				break;
			}
			case TroopsStrategy::Strategy::Rush_And_Rescue:
			{
				break;
			}
			case TroopsStrategy::Strategy::Defend_Bombsite_Divided:
			{
				if (Received_Strategy.objective_goal_node == node::Invalid_NodeID) {
					return;
				}
				goto set_goal;
			}
			default:
			set_goal:
			{
				assert(Received_Strategy.leader_name.empty());
				goal_queue.AddGoalQueue(Received_Strategy.objective_goal_node, 1);
				break;
			}
		}
		// SERVER_PRINT(std::format("[POKEBOT]New Goal ID:{}\n", goal_node).c_str());
	}
}