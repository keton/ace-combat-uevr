#pragma once

#include "uevr/API.hpp"

#include "uesdk/AActor.hpp"
#include "uesdk/APawn.hpp"
#include "uesdk/CameraComponent.hpp"
#include "uesdk/USceneComponent.hpp"

#include "acesdk/CameraViewComponent.hpp"

using namespace uevr;

// AcePlayerPawn has dynamic class name
class AcePlayerPawn : public APawn
{
  public:
	static API::UClass *static_class(bool force_search = false)
	{
		static API::UClass *result = nullptr;

		if(result == nullptr || force_search) {
			result = find_pawn_class();
		}

		return result;
	}

	CameraViewComponent *prop_camera_view()
	{
		const auto p_prop = get_property_data<CameraViewComponent *>(L"CameraView");
		if(p_prop && *p_prop) {
			return *p_prop;
		}

		return nullptr;
	}

	CameraComponent *prop_cockpit_camera()
	{
		const auto p_prop = get_property_data<CameraComponent *>(L"CockpitCamera");
		if(p_prop && *p_prop) {
			return *p_prop;
		}

		return nullptr;
	}

	USceneComponent *prop_cockpit_mesh()
	{
		const auto p_prop = get_property_data<USceneComponent *>(L"CockpitMesh");
		if(p_prop && *p_prop) {
			return *p_prop;
		}

		return nullptr;
	}

	USceneComponent *prop_plane_body_mesh()
	{
		const auto p_prop = get_property_data<USceneComponent *>(L"PlaneBodyMesh");
		if(p_prop && *p_prop) {
			return *p_prop;
		}

		return nullptr;
	}

  private:
	static inline API::UClass *find_pawn_class()
	{
		const auto pawn = API::get()->get_local_pawn(0);
		if(!pawn) {
			return nullptr;
		}

		if(!pawn->get_full_name().starts_with(L"AcePlayerPawn_")) {
			return nullptr;
		}

		return pawn->get_class();
	}
};
