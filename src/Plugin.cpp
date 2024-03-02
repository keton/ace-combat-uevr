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

	void on_xinput_get_state(uint32_t *retval, uint32_t user_index, XINPUT_STATE *state) override
	{
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
		const auto vr = API::get()->param()->vr;

		if(ImGui::Begin("Ace Combat Plugin")) {
		}
		ImGui::End();

		if(ImGui::Begin("Ace Combat Debug Panel")) {
		}
		ImGui::End();
	}

	void plugin_on_pre_engine_tick(API::UGameEngine *engine, float delta)
	{
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
