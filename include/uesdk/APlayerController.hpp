#pragma once

#include "uevr/API.hpp"

#include "uesdk/AActor.hpp"
#include "uesdk/APawn.hpp"

class APlayerController : public AActor
{
  public:
	static API::UClass *static_class()
	{
		static auto result =
			API::get()->find_uobject<API::UClass>(L"Class /Script/Engine.PlayerController");
		return result;
	}

	void set_control_rotation(UEVR_Rotatorf *const new_rotation)
	{
		static const auto func =
			APlayerController::static_class()->find_function(L"SetControlRotation");
		if(!func) {
			API::get()->log_error("APlayerController::SetControlRotation not found");
			return;
		}

		struct
		{
			UEVR_Rotatorf rotation;
		} params{.rotation = *new_rotation};

		process_event(func, &params);
	}

	UEVR_Rotatorf get_control_rotation()
	{
		static const auto func =
			APlayerController::static_class()->find_function(L"GetControlRotation");
		if(!func) {
			API::get()->log_error("APlayerController::GetControlRotation not found");
			return {0};
		}

		struct
		{
			UEVR_Rotatorf rotation;
		} params{0};

		process_event(func, &params);
		return params.rotation;
	}

	APawn *get_acknowledged_pawn()
	{
		const auto p_prop = get_property_data<APawn *>(L"AcknowledgedPawn");
		if(p_prop && *p_prop) {
			return *p_prop;
		}

		return nullptr;
	}
};
