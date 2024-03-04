#pragma once

#include "uevr/API.hpp"

using namespace uevr;

class Mission : public API::UObject
{
  public:
	using API::UObject::get_full_name;

	static API::UClass *static_class(bool force_search = false)
	{
		static API::UClass *result = nullptr;
		if(!result || force_search) {
			result = API::get()->find_uobject<API::UClass>(L"Class /Script/Nimbus.Mission");
		}
		return result;
	}

	static Mission *get_instance(bool force_search = false)
	{
		auto klass = static_class(force_search);
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
};
