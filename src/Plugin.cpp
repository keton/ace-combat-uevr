#include <codecvt>
#include <locale>
#include <memory>
#include <mutex>
#include <sstream>

#include <Windows.h>

// only really necessary if you want to render to the screen
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui_internal.h"

#include "rendering/d3d11.hpp"
#include "rendering/d3d12.hpp"

#include "uevr/Plugin.hpp"

using namespace uevr;

class AceCombatPlugin : public uevr::Plugin
{
  public:
	AceCombatPlugin() = default;

	void on_initialize() override
	{
		ImGui::CreateContext();
	}

	void on_present() override
	{
		std::scoped_lock _{m_imgui_mutex};

		if(!m_initialized) {
			if(!initialize_imgui()) {
				API::get()->log_info("Failed to initialize imgui");
				return;
			} else {
				API::get()->log_info("Initialized imgui");
			}
		}

		const auto renderer_data = API::get()->param()->renderer;

		if(!API::get()->param()->vr->is_hmd_active()) {
			if(!m_was_rendering_desktop) {
				m_was_rendering_desktop = true;
				on_device_reset();
				return;
			}

			m_was_rendering_desktop = true;

			if(renderer_data->renderer_type == UEVR_RENDERER_D3D11) {
				ImGui_ImplDX11_NewFrame();
				g_d3d11.render_imgui();
			} else if(renderer_data->renderer_type == UEVR_RENDERER_D3D12) {
				auto command_queue = (ID3D12CommandQueue *)renderer_data->command_queue;

				if(command_queue == nullptr) {
					return;
				}

				ImGui_ImplDX12_NewFrame();
				g_d3d12.render_imgui();
			}
		}
	}

	void on_device_reset() override
	{
		std::scoped_lock _{m_imgui_mutex};

		const auto renderer_data = API::get()->param()->renderer;

		if(renderer_data->renderer_type == UEVR_RENDERER_D3D11) {
			ImGui_ImplDX11_Shutdown();
			g_d3d11 = {};
		}

		if(renderer_data->renderer_type == UEVR_RENDERER_D3D12) {
			g_d3d12.reset();
			ImGui_ImplDX12_Shutdown();
			g_d3d12 = {};
		}

		m_initialized = false;
	}

	void on_post_render_vr_framework_dx11(ID3D11DeviceContext *context, ID3D11Texture2D *texture,
										  ID3D11RenderTargetView *rtv) override
	{
		const auto vr_active = API::get()->param()->vr->is_hmd_active();

		if(!m_initialized || !vr_active) {
			return;
		}

		if(m_was_rendering_desktop) {
			m_was_rendering_desktop = false;
			on_device_reset();
			return;
		}

		std::scoped_lock _{m_imgui_mutex};

		ImGui_ImplDX11_NewFrame();
		g_d3d11.render_imgui_vr(context, rtv);
	}

	void on_post_render_vr_framework_dx12(ID3D12GraphicsCommandList *command_list,
										  ID3D12Resource *rt,
										  D3D12_CPU_DESCRIPTOR_HANDLE *rtv) override
	{
		const auto vr_active = API::get()->param()->vr->is_hmd_active();

		if(!m_initialized || !vr_active) {
			return;
		}

		if(m_was_rendering_desktop) {
			m_was_rendering_desktop = false;
			on_device_reset();
			return;
		}

		std::scoped_lock _{m_imgui_mutex};

		ImGui_ImplDX12_NewFrame();
		g_d3d12.render_imgui_vr(command_list, rtv);
	}

	bool on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) override
	{
		ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

		return !ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard;
	}

	void on_pre_engine_tick(API::UGameEngine *engine, float delta) override
	{
		plugin_on_pre_engine_tick(engine, delta);

		if(m_initialized) {
			std::scoped_lock _{m_imgui_mutex};

			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			internal_frame();

			ImGui::EndFrame();
			ImGui::Render();
		}
	}

	/*
		Mappings:

		LS:
			* up/down - pitch - LS Y axis
			* left/right - yaw - LB/RB
		RS:
			* up/down - throttle - LT for up, RT for down
			* left/right - roll - LS X axis

		buttons - no change (yet?)
	*/
	void on_xinput_get_state(uint32_t *retval, uint32_t user_index,
							 XINPUT_STATE *target_state) override
	{
		if(API::get()->param()->vr->get_lowest_xinput_index() != user_index) {
			return;
		}

		XINPUT_STATE starting_state{*target_state};

		target_state->Gamepad.bLeftTrigger = 0;
		target_state->Gamepad.bRightTrigger = 0;

		target_state->Gamepad.sThumbLX = 0;
		target_state->Gamepad.sThumbLY = 0;

		target_state->Gamepad.sThumbRX = 0;
		target_state->Gamepad.sThumbRY = 0;

		// pitch - do nothing
		target_state->Gamepad.sThumbLY = starting_state.Gamepad.sThumbLY;

		// yaw, negative values == left
		if(starting_state.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			target_state->Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
		} else if(starting_state.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
			target_state->Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
		}

		// roll
		target_state->Gamepad.sThumbLX = starting_state.Gamepad.sThumbRX;

		// throttle
		if(starting_state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		   starting_state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {

			// allow to passthrough triggers
			target_state->Gamepad.bLeftTrigger = starting_state.Gamepad.bLeftTrigger;
			target_state->Gamepad.bRightTrigger = starting_state.Gamepad.bRightTrigger;
		} else {
			// user is not holding triggers

			// negative values == down
			if(starting_state.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
				target_state->Gamepad.bLeftTrigger = std::abs(linear_scale(
					starting_state.Gamepad.sThumbRY, -SHRT_MAX, -254,
					-XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, -XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
			} else if(starting_state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
				target_state->Gamepad.bRightTrigger = std::abs(linear_scale(
					starting_state.Gamepad.sThumbRY, SHRT_MAX, 255,
					XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
			}
		}
	}

  private:
	bool initialize_imgui()
	{
		if(m_initialized) {
			return true;
		}

		std::scoped_lock _{m_imgui_mutex};

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		static const auto imgui_ini =
			API::get()->get_persistent_dir(L"ace_combat_plugin.ini").string();
		ImGui::GetIO().IniFilename = imgui_ini.c_str();

		const auto renderer_data = API::get()->param()->renderer;

		DXGI_SWAP_CHAIN_DESC swap_desc{};
		auto swapchain = (IDXGISwapChain *)renderer_data->swapchain;
		swapchain->GetDesc(&swap_desc);

		m_wnd = swap_desc.OutputWindow;

		if(!ImGui_ImplWin32_Init(m_wnd)) {
			return false;
		}

		if(renderer_data->renderer_type == UEVR_RENDERER_D3D11) {
			if(!g_d3d11.initialize()) {
				return false;
			}
		} else if(renderer_data->renderer_type == UEVR_RENDERER_D3D12) {
			if(!g_d3d12.initialize()) {
				return false;
			}
		}

		m_initialized = true;
		return true;
	}

	void internal_frame()
	{
		if(ImGui::Begin("Ace Combat Plugin")) {
		}
		ImGui::End();
	}

	void plugin_on_pre_engine_tick(API::UGameEngine *engine, float delta)
	{
	}

	inline int32_t linear_scale(const int32_t val, const int32_t val_max, const int32_t target_max,
								const int32_t val_min = 0, const int32_t target_min = 0)
	{
		double ratio = (double)(target_max - target_min) / (double)(val_max - val_min);
		return std::floor((double)(val - val_min) * ratio + target_min);
	}

  private:
	HWND m_wnd{};
	bool m_initialized{false};
	bool m_was_rendering_desktop{false};

	std::recursive_mutex m_imgui_mutex{};
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called
// in some way.
std::unique_ptr<AceCombatPlugin> g_plugin{new AceCombatPlugin()};
