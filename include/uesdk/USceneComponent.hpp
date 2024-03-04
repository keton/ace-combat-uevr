#pragma once

#include "uevr/API.hpp"
#include <vector>

using namespace uevr;

class USceneComponent : public API::UObject
{
  public:
	static API::UClass *static_class()
	{
		static auto result =
			API::get()->find_uobject<API::UClass>(L"Class /Script/Engine.SceneComponent");
		return result;
	}

	UEVR_Vector3f get_component_location()
	{
		static const auto func_candidate_1 =
			USceneComponent::static_class()->find_function(L"K2_GetComponentLocation");
		static const auto func_candidate_2 =
			USceneComponent::static_class()->find_function(L"GetComponentLocation");

		if(func_candidate_1 == nullptr && func_candidate_2 == nullptr) {
			API::get()->log_error("GetComponentLocation not found");
			return UEVR_Vector3f{0.0f, 0.0f, 0.0f};
		}

		auto func = func_candidate_1 != nullptr ? func_candidate_1 : func_candidate_2;

		struct
		{
			UEVR_Vector3f ret;
		} params{0};

		this->process_event(func, &params);

		return params.ret;
	}

	UEVR_Rotatorf get_component_rotation()
	{
		static const auto func_candidate_1 =
			USceneComponent::static_class()->find_function(L"K2_GetComponentRotation");
		static const auto func_candidate_2 =
			USceneComponent::static_class()->find_function(L"GetComponentRotation");

		if(func_candidate_1 == nullptr && func_candidate_2 == nullptr) {
			API::get()->log_error("GetComponentRotation not found");
			return UEVR_Rotatorf{0.0f, 0.0f, 0.0f};
		}

		auto func = func_candidate_1 != nullptr ? func_candidate_1 : func_candidate_2;

		struct
		{
			UEVR_Rotatorf ret;
		} params{0};

		this->process_event(func, &params);

		return params.ret;
	}

	UEVR_Rotatorf get_relative_rotation()
	{
		static const auto func_candidate_1 =
			USceneComponent::static_class()->find_function(L"K2_GetRelativeRotation");
		static const auto func_candidate_2 =
			USceneComponent::static_class()->find_function(L"GetRelativeRotation");

		if(func_candidate_1 == nullptr && func_candidate_2 == nullptr) {
			API::get()->log_error("GetRelativeRotation not found");
			return UEVR_Rotatorf{0.0f, 0.0f, 0.0f};
		}

		auto func = func_candidate_1 != nullptr ? func_candidate_1 : func_candidate_2;

		struct
		{
			UEVR_Rotatorf ret;
		} params{0};

		this->process_event(func, &params);
		return params.ret;
	}

	void set_relative_rotation(const UEVR_Rotatorf *const new_rotation, const bool teleport = false,
							   const bool sweep = false)
	{
		static const auto fn =
			USceneComponent::static_class()->find_function(L"K2_SetRelativeRotation");

		static const auto fhitresult =
			API::get()->find_uobject<API::UScriptStruct>(L"ScriptStruct /Script/Engine.HitResult");

		if(fn == nullptr) {
			API::get()->log_error("K2_SetRelativeRotation not found");
			return;
		}

		// Need to dynamically allocate the params because of unknown FHitResult size
		std::vector<uint8_t> params{};
		params.insert(params.end(), (uint8_t *)&new_rotation,
					  (uint8_t *)&new_rotation + sizeof(UEVR_Rotatorf));

		// add a bool
		params.insert(params.end(), (uint8_t *)&teleport, (uint8_t *)&sweep + sizeof(bool));

		// align up to 8 based on size
		if(params.size() % sizeof(void *) != 0) {
			params.insert(params.end(), sizeof(void *) - (params.size() % sizeof(void *)), 0);
		}

		// add a FHitResult
		params.insert(params.end(), fhitresult->get_struct_size(), 0);
		// add a bool
		params.insert(params.end(), (uint8_t *)&teleport, (uint8_t *)&teleport + sizeof(bool));

		this->process_event(fn, params.data());
	}

	void set_world_rotation(const UEVR_Rotatorf *const new_rotation, const bool teleport = false,
							const bool sweep = false)
	{
		static auto fn = USceneComponent::static_class()->find_function(L"K2_SetWorldRotation");

		static const auto fhitresult =
			API::get()->find_uobject<API::UScriptStruct>(L"ScriptStruct /Script/Engine.HitResult");

		if(fn == nullptr) {
			API::get()->log_error("K2_SetWorldRotation not found");
			return;
		}

		// Need to dynamically allocate the params because of unknown FHitResult size
		std::vector<uint8_t> params{};
		params.insert(params.end(), (uint8_t *)&new_rotation,
					  (uint8_t *)&new_rotation + sizeof(UEVR_Rotatorf));

		// add a bool
		params.insert(params.end(), (uint8_t *)&teleport, (uint8_t *)&sweep + sizeof(bool));

		// align up to 8 based on size
		if(params.size() % sizeof(void *) != 0) {
			params.insert(params.end(), sizeof(void *) - (params.size() % sizeof(void *)), 0);
		}

		// add a FHitResult
		params.insert(params.end(), fhitresult->get_struct_size(), 0);
		// add a bool
		params.insert(params.end(), (uint8_t *)&teleport, (uint8_t *)&teleport + sizeof(bool));

		this->process_event(fn, params.data());
	}

	UEVR_Rotatorf *prop_relative_rotation()
	{
		return get_property_data<UEVR_Rotatorf>(L"RelativeRotation");
	}

	UEVR_Vector3f *prop_relative_location()
	{
		return get_property_data<UEVR_Vector3f>(L"RelativeLocation");
	}

	bool *prop_b_absolute_rotation()
	{
		return get_property_data<bool>(L"bAbsoluteRotation");
	}

	bool *prop_b_absolute_location()
	{
		return get_property_data<bool>(L"bAbsoluteLocation");
	}
};
