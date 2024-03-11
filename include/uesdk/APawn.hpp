#pragma once

#include "uevr/API.hpp"

#include "uesdk/AActor.hpp"

class APawn : public AActor
{
  public:
	static API::UClass *static_class()
	{
		static auto result = API::get()->find_uobject<API::UClass>(L"Class /Script/Engine.Pawn");
		return result;
	}

	UEVR_Rotatorf get_control_rotation()
	{
		static const auto func = APawn::static_class()->find_function(L"GetControlRotation");
		if(!func) {
			API::get()->log_error("APawn::GetControlRotation not found");
			return {0};
		}

		struct
		{
			UEVR_Rotatorf rotation;
		} params{0};

		process_event(func, &params);
		return params.rotation;
	}

	UEVR_Rotatorf get_base_aim_rotation()
	{
		static const auto func = APawn::static_class()->find_function(L"GetBaseAimRotation");
		if(!func) {
			API::get()->log_error("APawn::GetBaseAimRotation not found");
			return {0};
		}

		struct
		{
			UEVR_Rotatorf rotation;
		} params{0};

		process_event(func, &params);
		return params.rotation;
	}
};
