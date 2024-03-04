
#pragma once

#include "uevr/API.hpp"

using namespace uevr;

enum CameraType : uint8_t
{
	FIRST_PERSON = 0,
	COCKPIT,
	THIRD_PERSON,
	REPLAY,
	MINIGAME_CAMERA,
	IMPACT_CAMERA,
	VR_CAMERA,
	NO_CAMERA,
	ECameraType_MAX,
};

class CameraViewComponent : private API::UObject
{
  public:
	using API::UObject::get_full_name;

	static API::UClass *static_class()
	{
		static auto result =
			API::get()->find_uobject<API::UClass>(L"Class /Script/Nimbus.CameraViewComponent");
		return result;
	}

	CameraType get_current_camera_type()
	{
		static const auto func = static_class()->find_function(L"GetCurrentCameraViewType");
		if(!func) {
			API::get()->log_error("CameraViewComponent::GetCurrentCameraViewType not found");
			return CameraType::NO_CAMERA;
		}

		struct
		{
			uint8_t result;
		} params{.result = 0};

		process_event(func, &params);

		return (CameraType)params.result;
	}

	void switch_to_cockpit_view()
	{
		static const auto func = static_class()->find_function(L"SwitchToCockpitView");
		if(!func) {
			API::get()->log_error("CameraViewComponent::SwitchToCockpitView not found");
			return;
		}

		struct
		{
			intptr_t dummy;
		} params{0};

		process_event(func, &params);
	}

	void switch_to_first_person_view()
	{
		static const auto func = static_class()->find_function(L"SwitchToFirstPersonView");
		if(!func) {
			API::get()->log_error("CameraViewComponent::SwitchToFirstPersonView not found");
			return;
		}

		struct
		{
			intptr_t dummy;
		} params{0};

		process_event(func, &params);
	}

	void switch_to_third_person_view()
	{
		static const auto func = static_class()->find_function(L"SwitchToThirdPersonView");
		if(!func) {
			API::get()->log_error("CameraViewComponent::SwitchToThirdPersonView not found");
			return;
		}

		struct
		{
			intptr_t dummy;
		} params{0};

		process_event(func, &params);
	}
};
