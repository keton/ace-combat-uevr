#pragma once

#include "uevr/API.hpp"

#include "uesdk/CheatManager.hpp"

using namespace uevr;

class NimbusCheatManager : public CheatManager
{
  public:
	static API::UClass *static_class()
	{
		static auto result =
			API::get()->find_uobject<API::UClass>(L"Class /Script/Nimbus.NimbusCheatManager");
		return result;
	}

	static NimbusCheatManager *get_instance()
	{
		auto instance = static_class()->get_first_object_matching<NimbusCheatManager>();
		return instance;
	}
};
