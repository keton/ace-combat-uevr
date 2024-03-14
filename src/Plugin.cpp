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

#include "acesdk/AcePlayerPawn.hpp"
#include "acesdk/Mission.hpp"
#include "acesdk/NimbusPlayerCameraManager.hpp"
#include "acesdk/NimbusPlayerController.hpp"

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

	void on_xinput_get_state(uint32_t *retval, uint32_t user_index,
							 XINPUT_STATE *target_state) override
	{
		if((API::get()->param()->vr->get_lowest_xinput_index() != user_index)) {
			return;
		}

		// add 'unstuck camera' function
		if(target_state->Gamepad.wButtons & XINPUT_GAMEPAD_BACK) {
			stop_camera_setup();
			start_camera_setup(false);
		}

		// allow control remap to be bypassed
		if(m_control_scheme == ControlScheme::DefaultControls) {
			return;
		}

		XINPUT_STATE starting_state{*target_state};
		const auto controls = remap_controls(&starting_state, m_control_scheme);

		target_state->Gamepad.bLeftTrigger = 0;
		target_state->Gamepad.bRightTrigger = 0;

		target_state->Gamepad.sThumbLX = 0;
		target_state->Gamepad.sThumbLY = 0;

		target_state->Gamepad.sThumbRX = 0;
		target_state->Gamepad.sThumbRY = 0;

		// pitch
		if((controls.pitch < -controls.pitch_deadzone) ||
		   (controls.pitch > controls.pitch_deadzone)) {
			target_state->Gamepad.sThumbLY = controls.pitch;
		}

		// yaw, negative values == left
		if(controls.yaw < -controls.yaw_deadzone) {
			target_state->Gamepad.wButtons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
		} else if(controls.yaw > controls.yaw_deadzone) {
			target_state->Gamepad.wButtons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
		}

		// roll
		if((controls.roll < -controls.roll_deadzone) || (controls.roll > controls.roll_deadzone)) {
			target_state->Gamepad.sThumbLX = controls.roll;
		}

		// throttle
		if(starting_state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD ||
		   starting_state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {

			// allow to passthrough triggers
			target_state->Gamepad.bLeftTrigger = starting_state.Gamepad.bLeftTrigger;
			target_state->Gamepad.bRightTrigger = starting_state.Gamepad.bRightTrigger;
		} else {
			// user is not holding triggers

			// negative values == down
			if(controls.throttle < -controls.throttle_deadzone) {
				target_state->Gamepad.bLeftTrigger = std::abs(
					linear_scale(controls.throttle, -SHRT_MAX, -254, -controls.throttle_deadzone,
								 -XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
			} else if(controls.throttle > controls.throttle_deadzone) {
				target_state->Gamepad.bRightTrigger = std::abs(
					linear_scale(controls.throttle, SHRT_MAX, 255, controls.throttle_deadzone,
								 XINPUT_GAMEPAD_TRIGGER_THRESHOLD));
			}
		}
	}

  private:
	enum CameraSetupState
	{
		None = 0,
		WaitingForPlaneMovement,
		WaitingToStopShake,
		WaitingToStartShake
	};

	// Refer to standard RC control modes for example: https://i.stack.imgur.com/3O98c.png
	enum ControlScheme
	{
		DefaultControls = 0,
		Mode1,
		Mode2,
		Mode3,
		Mode4,
	};

	struct ControlInputs
	{
		SHORT pitch; // elevator
		SHORT pitch_deadzone;
		SHORT yaw; // rudder
		SHORT yaw_deadzone;
		SHORT roll; // aileron
		SHORT roll_deadzone;
		SHORT throttle;
		SHORT throttle_deadzone;
	};

	static void *ini_handler_read_open(ImGuiContext *, ImGuiSettingsHandler *, const char *name)
	{
		ImGuiID id = ImHashStr(name);
		ImGuiWindowSettings *settings = new ImGuiWindowSettings();
		settings->ID = id;
		settings->WantApply = true;
		return (void *)settings;
	}

	static void ini_handler_read_line(ImGuiContext *ctx, ImGuiSettingsHandler *handler, void *entry,
									  const char *line)
	{
		int alternate_control_scheme = 0;

		if(sscanf(line, "ControlScheme=%d", &alternate_control_scheme) == 1 &&
		   alternate_control_scheme >= 0 && alternate_control_scheme <= ControlScheme::Mode4) {
			m_control_scheme = (ControlScheme)alternate_control_scheme;
		}
	}

	static void ini_handler_write_all(ImGuiContext *ctx, ImGuiSettingsHandler *handler,
									  ImGuiTextBuffer *buf)
	{
		buf->reserve(buf->size() + 200); // ballpark reserve
		buf->append("[UserData][Ace Combat Plugin]\n");
		buf->appendf("ControlScheme=%d\n", m_control_scheme);
		buf->append("\n");
	}

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

		ImGuiSettingsHandler ini_handler;
		ini_handler.TypeName = "UserData";
		ini_handler.TypeHash = ImHashStr("UserData");
		ini_handler.ReadOpenFn = &ini_handler_read_open;
		ini_handler.ReadLineFn = &ini_handler_read_line;
		ini_handler.WriteAllFn = &ini_handler_write_all;

		ImGuiContext &g = *GImGui;
		IM_ASSERT(g.Initialized);

		g.SettingsHandlers.push_back(ini_handler);

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
			const auto labels = {
				"Default", "Mode 1", "Mode 2", "Mode 3", "Mode 4",
			};

			int selection = m_control_scheme;

			if(ImGui::Combo("Control Scheme", &selection, labels.begin(), labels.size())) {
				if(selection >= 0 && selection <= ControlScheme::Mode4) {
					m_control_scheme = (ControlScheme)selection;
				}
			}
		}
		ImGui::End();

		if(ImGui::Begin("Ace Combat Debug Data")) {
			ImGui::Text("m_last_root_rotation: Pitch: %f, Yaw: %f, Roll: %f",
						m_last_root_rotation.pitch, m_last_root_rotation.yaw,
						m_last_root_rotation.roll);
			ImGui::Text("m_last_root_location: X: %f, Y: %f, Z: %f", m_last_root_location.x,
						m_last_root_location.y, m_last_root_location.z);
		}
		ImGui::End();
	}

	void process_level_load(NimbusPlayerController *const player_controller,
							AcePlayerPawn *const player_pawn)
	{
		if(player_pawn != m_last_player_pawn) {
			API::get()->log_info("player_pawn change: old %p, current %p", m_last_player_pawn,
								 player_pawn);

			// update class name in cache
			const auto pawn_class = AcePlayerPawn::static_class(true);
			if(pawn_class) {
				API::get()->log_info("New pawn class: %ls", pawn_class->get_full_name().c_str());

				m_last_player_pawn = player_pawn;
				m_last_root_location = {0};
				m_last_root_rotation = {0};

				m_last_is_in_igc = false;
				m_last_camera_type = CameraType::NO_CAMERA;

				stop_camera_setup();

			} else {
				API::get()->log_info("pawn class NULL");

				m_last_player_pawn = nullptr;
			}

			const auto camera_manager_class = NimbusPlayerCameraManager::static_class(true);
			if(camera_manager_class) {
				API::get()->log_info("New camera_manager_class: %ls",
									 camera_manager_class->get_full_name().c_str());
			} else {
				API::get()->log_info("camera_manager_class NULL");
			}

			const auto mission_class = NimbusPlayerCameraManager::get_instance(true);
			if(mission_class) {
				API::get()->log_info("New mission_class: %ls",
									 mission_class->get_full_name().c_str());
			} else {
				API::get()->log_info("mission_class NULL");
			}
		}
	}

	void stop_camera_setup()
	{
		m_last_camera_setup_state = CameraSetupState::None;
		m_last_camera_setup_step = {};
		m_camera_setup_start_location = {0};
	}

	void start_camera_setup(bool wait_for_plane_movement)
	{
		if(wait_for_plane_movement) {
			m_last_camera_setup_state = CameraSetupState::WaitingForPlaneMovement;
		} else {
			m_last_camera_setup_state = CameraSetupState::WaitingToStopShake;
		}

		m_last_camera_setup_step = std::chrono::high_resolution_clock::now();
		m_camera_setup_start_location = m_last_root_location;
	}

	void do_camera_setup_step(NimbusPlayerCameraManager *const camera_manager)
	{
		const auto now = std::chrono::high_resolution_clock::now();
		const auto last_step_duration = now - m_last_camera_setup_step;

		UEVR_Vector3f movement_delta = {
			.x = abs(m_last_root_location.x - m_camera_setup_start_location.x),
			.y = abs(m_last_root_location.y - m_camera_setup_start_location.y),
			.z = abs(m_last_root_location.z - m_camera_setup_start_location.z),
		};

		const bool has_plane_moved = (movement_delta.x > m_plane_moved_threshold) ||
									 (movement_delta.y > m_plane_moved_threshold) ||
									 (movement_delta.z > m_plane_moved_threshold);

		switch(m_last_camera_setup_state) {
		case CameraSetupState::None:
			break;

		case CameraSetupState::WaitingForPlaneMovement:
			if(has_plane_moved || (last_step_duration > m_wait_for_plane_movement_timeout)) {
				if(has_plane_moved) {
					API::get()->log_info(
						"CameraSetupState::WaitingForPlaneMovement: Airspeed Live!");
				} else {
					API::get()->log_warn("CameraSetupState::WaitingForPlaneMovement: TIMEOUT");
				}

				m_last_camera_setup_step = now;
				m_last_camera_setup_state = CameraSetupState::WaitingToStopShake;
			}
			break;

		case CameraSetupState::WaitingToStopShake:
			if(last_step_duration > m_delay_before_stop_shake) {
				API::get()->log_info("CameraSetupState::WaitingToStopShake done");
				camera_manager->stop_all_camera_shakes(true);
				m_last_camera_setup_step = now;
				m_last_camera_setup_state = CameraSetupState::WaitingToStartShake;
			}
			break;

		case CameraSetupState::WaitingToStartShake:
			if(last_step_duration > m_delay_before_start_shake) {
				API::get()->log_info("CameraSetupState::WaitingToStartShake done");
				camera_manager->test_loop_camera_shake_play();
				camera_manager->test_loop_camera_shake_add_scale(-5000.0);
				m_last_camera_setup_step = {};
				m_last_camera_setup_state = CameraSetupState::None;
			}
			break;

		default:
			break;
		}
	}

	void process_camera_switch(NimbusPlayerController *const player_controller,
							   AcePlayerPawn *const player_pawn)
	{
		const auto camera_manager = NimbusPlayerCameraManager::get_instance();
		if(!camera_manager) {
			return;
		}

		const auto mission_inst = Mission::get_instance();
		if(!mission_inst) {
			return;
		}

		const auto camera_view = player_pawn->prop_camera_view();
		if(!camera_view) {
			return;
		}

		const auto is_in_igc = mission_inst->is_in_igc();

		if(m_last_is_in_igc != is_in_igc) {
			m_last_is_in_igc = is_in_igc;
			if(is_in_igc) {
				// IGC enter
				API::get()->log_info("IGC enter");
				camera_manager->stop_all_camera_shakes(true);
				stop_camera_setup();
			} else {
				// IGC exit
				API::get()->log_info("IGC exit");
			}
		}

		const auto camera_type = camera_view->get_current_camera_type();

		if(m_last_camera_type != camera_type) {
			API::get()->log_info("New camera: %d", camera_type);

			m_last_camera_type = camera_type;

			if(camera_type == CameraType::COCKPIT) {
				// cockpit enter
				API::get()->log_info("cockpit enter");
				start_camera_setup(true);
			} else {
				// cockpit exit
				API::get()->log_info("cockpit exit");
				camera_manager->stop_all_camera_shakes(true);
				stop_camera_setup();
			}
		}

		do_camera_setup_step(camera_manager);
	}

	void process_root_component_data(NimbusPlayerController *const player_controller,
									 AcePlayerPawn *const player_pawn)
	{
		const auto pawn_root_component = player_pawn->get_root_component();
		if(!pawn_root_component) {
			API::get()->log_info("pawn_root_component NULL");
			return;
		}

		const auto root_rotation = pawn_root_component->prop_relative_rotation();
		if(!root_rotation) {
			API::get()->log_info("root_rotation NULL");
			return;
		}
		m_last_root_rotation = *root_rotation;

		const auto root_location = pawn_root_component->prop_relative_location();
		if(!root_location) {
			API::get()->log_info("root_location NULL");
			return;
		}
		m_last_root_location = *root_location;
	}

	void process_ui_enter_exit()
	{
		const bool is_drawing_ui = API::get()->param()->functions->is_drawing_ui();

		if(is_drawing_ui == last_is_is_drawing_ui) {
			return;
		}

		if(is_drawing_ui) {
			// menu enter event
		} else {
			// menu exit event
			ImGui::MarkIniSettingsDirty();
		}

		last_is_is_drawing_ui = is_drawing_ui;
	}

	void plugin_on_pre_engine_tick(API::UGameEngine *engine, float delta)
	{
		// trigger regardless if player_pawn (the plane) exists
		process_ui_enter_exit();

		const auto player_controller = NimbusPlayerController::get_instance();
		if(!player_controller) {
			return;
		}

		const auto player_pawn = player_controller->get_acknowledged_pawn();
		if(!player_pawn) {
			return;
		}

		process_level_load(player_controller, player_pawn);
		process_root_component_data(player_controller, player_pawn);
		process_camera_switch(player_controller, player_pawn);
	}

	inline int32_t linear_scale(const int32_t val, const int32_t val_max, const int32_t target_max,
								const int32_t val_min = 0, const int32_t target_min = 0)
	{
		double ratio = (double)(target_max - target_min) / (double)(val_max - val_min);
		return std::floor((double)(val - val_min) * ratio + target_min);
	}

	inline ControlInputs remap_controls(const XINPUT_STATE *const starting_state,
								 const ControlScheme control_scheme)
	{
		switch(control_scheme) {
		case ControlScheme::Mode1:
			return {
				.pitch = starting_state->Gamepad.sThumbLY,
				.pitch_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
				.yaw = starting_state->Gamepad.sThumbLX,
				.yaw_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
				.roll = starting_state->Gamepad.sThumbRX,
				.roll_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
				.throttle = starting_state->Gamepad.sThumbRY,
				.throttle_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
			};
		case ControlScheme::Mode2:
			return {
				.pitch = starting_state->Gamepad.sThumbRY,
				.pitch_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
				.yaw = starting_state->Gamepad.sThumbLX,
				.yaw_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
				.roll = starting_state->Gamepad.sThumbRX,
				.roll_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
				.throttle = starting_state->Gamepad.sThumbLY,
				.throttle_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
			};

		case ControlScheme::Mode4:
			return {
				.pitch = starting_state->Gamepad.sThumbRY,
				.pitch_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
				.yaw = starting_state->Gamepad.sThumbRX,
				.yaw_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
				.roll = starting_state->Gamepad.sThumbLX,
				.roll_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
				.throttle = starting_state->Gamepad.sThumbLY,
				.throttle_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
			};

		case ControlScheme::Mode3:
			[[fallthrough]];
		default:
			return {
				.pitch = starting_state->Gamepad.sThumbLY,
				.pitch_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
				.yaw = starting_state->Gamepad.sThumbRX,
				.yaw_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
				.roll = starting_state->Gamepad.sThumbLX,
				.roll_deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,
				.throttle = starting_state->Gamepad.sThumbRY,
				.throttle_deadzone = XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,
			};
		}
	}

  private:
	HWND m_wnd{};
	bool m_initialized{false};
	bool m_was_rendering_desktop{false};

	std::recursive_mutex m_imgui_mutex{};

	AcePlayerPawn *m_last_player_pawn = nullptr;

	CameraType m_last_camera_type = CameraType::NO_CAMERA;
	bool m_last_is_in_igc = false;

	CameraSetupState m_last_camera_setup_state = CameraSetupState::None;
	std::chrono::steady_clock::time_point m_last_camera_setup_step{};
	UEVR_Vector3f m_camera_setup_start_location{0};

	const std::chrono::milliseconds m_delay_before_stop_shake = std::chrono::milliseconds(50);
	const std::chrono::milliseconds m_delay_before_start_shake = std::chrono::milliseconds(50);
	const std::chrono::seconds m_wait_for_plane_movement_timeout = std::chrono::seconds(5);
	const float m_plane_moved_threshold = 0.5f;

	UEVR_Vector3f m_last_root_location{0};
	UEVR_Rotatorf m_last_root_rotation{0};

	static inline ControlScheme m_control_scheme = ControlScheme::DefaultControls;
	bool last_is_is_drawing_ui = false;
};

// Actually creates the plugin. Very important that this global is created.
// The fact that it's using std::unique_ptr is not important, as long as the constructor is called
// in some way.
std::unique_ptr<AceCombatPlugin> g_plugin{new AceCombatPlugin()};
