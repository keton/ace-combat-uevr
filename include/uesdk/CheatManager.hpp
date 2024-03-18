#pragma once

#include "uevr/API.hpp"

using namespace uevr;

class CheatManager : public API::UObject
{
  public:
	static API::UClass *static_class()
	{
		static auto result =
			API::get()->find_uobject<API::UClass>(L"Class /Script/Engine.CheatManager");
		return result;
	}

	void god()
	{
		static const auto func = CheatManager::static_class()->find_function(L"God");
		if(!func) {
			API::get()->log_error("CheatManager::God not found");
			return;
		}

		struct
		{
			int dummy;
		} params{0};

		process_event(func, &params);
	}
};
