#pragma once

#include "uevr/API.hpp"

#include "acesdk/AcePlayerPawn.hpp"
#include "uesdk/APlayerController.hpp"

using namespace uevr;

class NimbusPlayerController : public APlayerController
{
  public:
	using API::UObject::get_full_name;

	static API::UClass *static_class()
	{
		static auto result = API::get()->find_uobject<API::UClass>(
			L"BlueprintGeneratedClass "
			L"/Game/Blueprint/GameModes/NimbusPlayerControllerBP.NimbusPlayerControllerBP_C");
		return result;
	}

	static NimbusPlayerController *get_instance()
	{
		auto instance = static_class()->get_first_object_matching<NimbusPlayerController>();
		return instance;
	}

	AcePlayerPawn *get_acknowledged_pawn()
	{
		return (AcePlayerPawn *)APlayerController::get_acknowledged_pawn();
	}
};
