#pragma once

#include "uevr/API.hpp"

#include "uesdk/USceneComponent.hpp"

using namespace uevr;

class AActor : public API::UObject
{
  public:
	static API::UClass *static_class()
	{
		static auto result = API::get()->find_uobject<API::UClass>(L"Class /Script/Engine.Actor");
		return result;
	}

	void set_actor_scale3d(const UEVR_Vector3f *const new_scale)
	{
		static const auto func = AActor::static_class()->find_function(L"SetActorScale3D");
		if(!func) {
			API::get()->log_error("AActor::SetActorScale3D not found");
			return;
		}

		struct
		{
			UEVR_Vector3f new_scale;
		} params{.new_scale = {.x = new_scale->x, .y = new_scale->y, .z = new_scale->z}};

		process_event(func, &params);
	}

	USceneComponent *get_root_component()
	{
		static const auto func_candidate_1 =
			AActor::static_class()->find_function(L"K2_GetRootComponent");
		static const auto func_candidate_2 =
			AActor::static_class()->find_function(L"GetRootComponent");

		auto func = func_candidate_1 != nullptr ? func_candidate_1 : func_candidate_2;
		if(!func) {
			API::get()->log_error("AActor::GetRootComponent not found");
			return nullptr;
		}

		struct
		{
			USceneComponent *ret;
		} params{0};

		process_event(func, &params);

		return params.ret;
	}

	std::vector<API::UObject *> get_components_by_class(API::UClass *const uclass)
	{
		static const auto func_candidate_1 =
			AActor::static_class()->find_function(L"K2_GetComponentsByClass");
		static const auto func_candidate_2 =
			AActor::static_class()->find_function(L"GetComponentsByClass");

		if(func_candidate_1 == nullptr && func_candidate_2 == nullptr) {
			return {};
		}

		auto func = func_candidate_1 != nullptr ? func_candidate_1 : func_candidate_2;

		struct Params_K2_GetComponentsByClass
		{
			API::UClass *ComponentClass{};			   // 0x0
			API::TArray<API::UObject *> ReturnValue{}; // 0x8
		};											   // Size: 0x18

		Params_K2_GetComponentsByClass params{};
		params.ComponentClass = uclass;

		this->process_event(func, &params);

		std::vector<API::UObject *> ret{};

		if(params.ReturnValue.data != nullptr) {
			for(int i = 0; i < params.ReturnValue.count; ++i) {
				ret.push_back(params.ReturnValue.data[i]);
			}
		}

		return ret;
	}

	std::vector<API::UObject *> get_all_components()
	{
		static const auto actor_component_t =
			API::get()->find_uobject<API::UClass>(L"Class /Script/Engine.ActorComponent");

		if(actor_component_t == nullptr) {
			return {};
		}

		return get_components_by_class(actor_component_t);
	}
};
