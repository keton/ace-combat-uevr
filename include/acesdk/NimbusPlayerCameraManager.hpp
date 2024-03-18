#pragma once

#include "uevr/API.hpp"

using namespace uevr;

class NimbusPlayerCameraManager : public API::UObject
{
  public:
	using API::UObject::get_full_name;

	static API::UClass *static_class()
	{
		static API::UClass *result = nullptr;
		if(!result) {
			result = API::get()->find_uobject<API::UClass>(
				L"BlueprintGeneratedClass "
				L"/Game/Blueprint/GameModes/"
				L"NimbusPlayerCameraManagerBP.NimbusPlayerCameraManagerBP_C");
		}
		return result;
	}

	static NimbusPlayerCameraManager *get_instance()
	{
		auto klass = NimbusPlayerCameraManager::static_class();
		if(klass) {
			return klass->get_first_object_matching<NimbusPlayerCameraManager>();
		}
		return nullptr;
	}

	void test_loop_camera_shake_play()
	{
		static const auto func =
			NimbusPlayerCameraManager::static_class()->find_function(L"TestLoopCameraShakePlay");
		if(!func) {
			API::get()->log_error("NimbusPlayerCameraManager::TestLoopCameraShakePlay not found");
			return;
		}

		struct
		{
			int dummy;
		} params{0};

		process_event(func, &params);
	}

	void test_loop_camera_shake_add_scale(const float scale)
	{
		static const auto func = NimbusPlayerCameraManager::static_class()->find_function(
			L"TestLoopCameraShakePlayAddScale");
		if(!func) {
			API::get()->log_error("NimbusPlayerCameraManager::TestLoopCameraShakePlay not found");
			return;
		}

		struct
		{
			float inScale;
		} params{.inScale = scale};

		process_event(func, &params);
	}

	void stop_all_camera_shakes(const bool immediately)
	{
		static const auto func =
			NimbusPlayerCameraManager::static_class()->find_function(L"StopAllCameraShakes");
		if(!func) {
			API::get()->log_error("NimbusPlayerCameraManager::StopAllCameraShakes not found");
			return;
		}

		struct
		{
			bool immediately;
		} params{.immediately = immediately};

		process_event(func, &params);
	}
};
