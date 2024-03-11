
#pragma once

#include "uevr/API.hpp"

#include "uesdk/USceneComponent.hpp"

using namespace uevr;

class CameraComponent : public USceneComponent
{
  public:
	static API::UClass *static_class()
	{
		static auto result =
			API::get()->find_uobject<API::UClass>(L"Class /Script/Engine.CameraComponent");
		return result;
	}

	bool get_prop_lock_to_hmd()
	{
		return get_bool_property(L"bLockToHmd");
	}

	void set_prop_lock_to_hmd(const bool val)
	{
		set_bool_property(L"bLockToHmd", val);
	}

	bool get_prop_do_not_update_transform_by_hmd()
	{
		return get_bool_property(L"bDoNotUpdateTransformByHmd");
	}

	void set_prop_do_not_update_transform_by_hmd(const bool val)
	{
		set_bool_property(L"bDoNotUpdateTransformByHmd", val);
	}

	bool get_prop_use_pawn_control_rotation()
	{
		return get_bool_property(L"bUsePawnControlRotation");
	}

	void set_prop_use_pawn_control_rotation(const bool val)
	{
		set_bool_property(L"bUsePawnControlRotation", val);
	}

	bool get_prop_use_controller_view_rotation()
	{
		return get_bool_property(L"bUseControllerViewRotation");
	}

	void set_prop_use_controller_view_rotation(const bool val)
	{
		set_bool_property(L"bUseControllerViewRotation", val);
	}
};
