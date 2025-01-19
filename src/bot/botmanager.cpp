#include "bot/manager.hpp"
namespace pokebot::bot {
	Manager::Manager() :
		troops{
			Troops(
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::T; },
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::T; },
				common::Team::T),
			Troops(
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; },
				[](const std::pair<std::string, Bot>& target) -> bool { return target.second.JoinedTeam() == common::Team::CT; },
				common::Team::CT)
	} {

	}

	void Manager::OnNewRoundPreparation() POKEBOT_NOEXCEPT {
		for (auto& bot : bots) {
			bot.second.OnNewRound();
		}
		c4_origin = std::nullopt;
		initialization_stage = InitializationStage::Player_Action_Ready;
		round_started_timer.SetTime(1.0f);
	}

	void Manager::OnNewRoundReady() POKEBOT_NOEXCEPT {
		if (!round_started_timer.IsRunning() && initialization_stage != InitializationStage::Player_Action_Ready) {
			return;
		}

		for (auto& troop : troops) {
			troop.DecideStrategy(&bots);
			troop.Command(&bots);

			for (auto& platoon : troop) {
				platoon.DecideStrategy(&bots);
				platoon.Command(&bots);
			}
		}
		initialization_stage = InitializationStage::Completed;
	}

	bool Manager::IsExist(const std::string& Bot_Name) const POKEBOT_NOEXCEPT {
		auto it = bots.find(Bot_Name);
		return (it != bots.end());
	}

	void Manager::Assign(const std::string_view Bot_Name, Message message) POKEBOT_NOEXCEPT {
		if (auto target = Get(Bot_Name.data()); target != nullptr) {
			target->start_action = message;
		}
	}

	void Manager::OnDied(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
		auto bot = Get(Bot_Name);
		if (bot != nullptr) {
			bot->current_weapon = game::Weapon::None;
			bot->goal_queue.Clear();
		}
	}

	void Manager::OnDamageTaken(const std::string_view Bot_Name, const edict_t* Inflictor, const int Damage, const int Armor, const int Bit) POKEBOT_NOEXCEPT {
		if (decltype(auto) target = Get(Bot_Name.data()); target->Health() <= 0) {
			OnDied(Bot_Name.data());
		} else {
			// TODO: Send the event message for a bot.
		}
	}

	void Manager::OnJoinedTeam(const std::string&) POKEBOT_NOEXCEPT {

	}

	void Manager::OnChatRecieved(const std::string&) POKEBOT_NOEXCEPT {

	}

	void Manager::OnTeamChatRecieved(const std::string&) POKEBOT_NOEXCEPT {

	}

	void Manager::OnRadioRecieved(const std::string& Sender_Name, const std::string& Radio_Sentence) POKEBOT_NOEXCEPT {
		// TODO: Get Sender's team
		radio_message.team = game::game.GetClientStatus(Sender_Name).GetTeam();
		radio_message.sender = Sender_Name;
		radio_message.message = Radio_Sentence;
	}

	void Manager::Insert(std::string bot_name, const common::Team team, const common::Model model, const bot::Difficult Assigned_Diffcult) POKEBOT_NOEXCEPT {
		if (auto spawn_result = game::game.Spawn(bot_name); std::get<bool>(spawn_result)) {
			bot_name = std::get<std::string>(spawn_result);
			auto insert_result = bots.insert({ bot_name, Bot(bot_name, team, model) });
			assert(insert_result.second);

			auto result = balancer.insert({ bot_name, BotBalancer{.gap = {} } });
			assert(result.second);

			switch (Assigned_Diffcult) {
				case bot::Difficult::Easy:
					result.first->second.gap.z = -10.0f;
					break;
				case bot::Difficult::Normal:
					result.first->second.gap.z = -5.0f;
					break;
				case bot::Difficult::Hard:
					break;
				default:
					assert(false);
			}
		}
	}

	void Manager::Update() POKEBOT_NOEXCEPT {
		if (bots.empty())
			return;

		OnNewRoundReady();

		if (!c4_origin.has_value()) {
			edict_t* c4{};
			while ((c4 = common::FindEntityByClassname(c4, "grenade")) != nullptr) {
				if (std::string(STRING(c4->v.model)) == "models/w_c4.mdl") {
					c4_origin = c4->v.origin;
					break;
				}
			}
			if (c4_origin.has_value()) {
				OnBombPlanted();
			}
		}

		for (auto& bot : bots) {
			bot.second.Run();
			if (!radio_message.sender.empty() && radio_message.team == bot.second.JoinedTeam()) {
				bot.second.OnRadioRecieved(radio_message.sender, radio_message.message);
				radio_message.sender.clear();
			}
		}
	}

	Bot* const Manager::Get(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
		auto bot_iterator = bots.find(Bot_Name);
		return (bot_iterator != bots.end() ? &bot_iterator->second : nullptr);
	}

	void Manager::Kick(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
		(*g_engfuncs.pfnServerCommand)(std::format("kick \"{}\"", Bot_Name).c_str());
	}

	void Manager::Remove(const std::string& Bot_Name) POKEBOT_NOEXCEPT {
		if (auto bot_iterator = bots.find(Bot_Name); bot_iterator != bots.end()) {
			bots.erase(Bot_Name);
			balancer.erase(Bot_Name);
		}
	}

	void Manager::OnBombPlanted() POKEBOT_NOEXCEPT {
		for (auto& bot : bots) {
			bot.second.OnBombPlanted();
		}
	}

	void Manager::OnBombPickedUp(const std::string& Client_Name) POKEBOT_NOEXCEPT {
		bomber_name = Client_Name;
	}

	void Manager::OnBombDropped(const std::string& Client_Name) POKEBOT_NOEXCEPT {
		bomber_name = "";
	}

	void Manager::OnBotJoinedCompletely(Bot* const completed_guy) POKEBOT_NOEXCEPT {
		assert(completed_guy->JoinedTeam() == common::Team::T || completed_guy->JoinedTeam() == common::Team::CT);
		const int Team_Index = static_cast<int>(completed_guy->JoinedTeam()) - 1;

		if (auto& troop = troops[Team_Index]; troop.NeedToDevise()) {
			troop.DecideStrategy(&bots);
			troop.Command(&bots);
		}
	}

	node::NodeID Manager::GetGoalNode(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
		auto& troop = troops[static_cast<int>(Target_Team) - 1];
		if (!Index.has_value()) {
			return troop.GetGoalNode();
		} else {
			return troop.at(*Index).GetGoalNode();
		}
	}

	std::optional<Vector> Manager::GetLeaderOrigin(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
		auto& troop = troops[static_cast<int>(Target_Team) - 1];
		if (!Index.has_value()) {
			return troop.CurrentLeaderOrigin();
		} else {
			return troop.at(*Index).CurrentLeaderOrigin();
		}
	}

	bool Manager::IsFollowerPlatoon(const common::Team Target_Team, const PlatoonID Index) const POKEBOT_NOEXCEPT {
		if (!Index.has_value()) {
			return false;
		} else {
			auto& troop = troops[static_cast<int>(Target_Team) - 1];
			return troop.at(*Index).IsStrategyToFollow();
		}
	}
}