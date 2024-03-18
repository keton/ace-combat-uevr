#pragma once

#include "uevr/API.hpp"

using namespace uevr;

class Mission : public API::UObject
{
  public:
	using API::UObject::get_full_name;

	static API::UClass *static_class()
	{
		static API::UClass *result = nullptr;
		if(!result) {
			result = API::get()->find_uobject<API::UClass>(L"Class /Script/Nimbus.Mission");
		}
		return result;
	}

	static Mission *get_instance()
	{
		auto klass = Mission::static_class();
		if(klass) {
			return klass->get_first_object_matching<Mission>();
		}
		return nullptr;
	}

	bool is_in_igc()
	{
		static const auto func = Mission::static_class()->find_function(L"IsInIGC");
		if(!func) {
			API::get()->log_error("Mission::IsInIGC not found");
			return false;
		}

		struct
		{
			bool res;
		} params{0};

		process_event(func, &params);

		return params.res;
	}

	void complete()
	{
		static const auto func = Mission::static_class()->find_function(L"Complete");
		if(!func) {
			API::get()->log_error("Mission::Complete not found");
			return;
		}

		struct
		{
			int dummy;
		} params{0};

		process_event(func, &params);
	}

	void complete_cooldown_override(float new_cooldown_fading_duration, float new_cooldown_duration)
	{
		static const auto func =
			Mission::static_class()->find_function(L"CompleteCoolDownOverride");
		if(!func) {
			API::get()->log_error("Mission::CompleteCoolDownOverride not found");
			return;
		}

		struct
		{
			float new_cooldown_fading_duration;
			float new_cooldown_duration;
		} params{
			.new_cooldown_fading_duration = new_cooldown_fading_duration,
			.new_cooldown_duration = new_cooldown_duration,
		};

		process_event(func, &params);
	}

	void force_pause_mission_timer(bool paused)
	{
		static const auto func =
			Mission::static_class()->find_function(L"ForcePauseMissionTimer_S");
		if(!func) {
			API::get()->log_error("Mission::ForcePauseMissionTimer_S not found");
			return;
		}

		struct
		{
			bool paused;
		} params{
			.paused = paused,
		};

		process_event(func, &params);
	}
};
