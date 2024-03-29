﻿#include "editor.h"
#include "window.h"
#include "input.h"
#include "system.h"
#include "logger.h"
#include "reflection.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "ecs.h"
#include "shader.h"
#include "model.h"
#include "texture.h"
#include "game_time.h"
#include "renderPass.h"
#include "ibl.h"
#include "lights.h"
#include "transform.h"
#include "camera.h"
#include "flyover.h"
#include "rhi.h"
#include "custom_inspect.h"
#include "picking.h"

//theme by Derydoca 
void set_darcula_theme() {
	ImGuiStyle* style = &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = ImVec4(0.9f, 0.9f, 0.9f, 1.000f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.500f, 0.500f, 0.500f, 1.000f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.16f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.280f, 0.280f, 0.280f, 0.000f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.313f, 0.313f, 0.313f, 1.000f);
	colors[ImGuiCol_Border] = ImVec4(0.28f, 0.28f, 0.28f, 1.000f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.28f, 0.28f, 0.28f, 1.000f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.200f, 0.200f, 0.200f, 1.000f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.280f, 0.280f, 0.280f, 1.000f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.000f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.000f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.148f, 0.148f, 0.148f, 1.000f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.000f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.277f, 0.277f, 0.277f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.300f, 0.300f, 0.300f, 1.000f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.000f, 1.000f, 1.000f, 1.000f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.391f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	colors[ImGuiCol_Header] = colors[ImGuiCol_TitleBg]; //ImVec4(0.25f, 0.25f, 0.25f, 1.000f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.000f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.000f);
	colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.391f, 0.391f, 0.391f, 1.000f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.000f, 1.000f, 1.000f, 0.670f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.16f, 0.16f, 1.000f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.352f, 0.352f, 0.352f, 1.000f);
	colors[ImGuiCol_TabActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.000f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.16f, 0.16f, 1.000f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.000f);
	colors[ImGuiCol_DockingPreview] = ImVec4(1.000f, 0.391f, 0.000f, 0.91f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.160f, 0.160f, 0.160f, 1.000f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.469f, 0.469f, 0.469f, 1.000f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.586f, 0.586f, 0.586f, 1.000f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.156f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0, 0, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.000f, 0.391f, 0.000f, 1.000f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.586f);

	style->ChildRounding = 4.0f;
	style->FrameBorderSize = 1.0f;
	style->FrameRounding = 2.0f;
	style->GrabMinSize = 7.0f;
	style->PopupRounding = 2.0f;
	style->ScrollbarRounding = 12.0f;
	style->ScrollbarSize = 13.0f;
	style->TabBorderSize = 0.0f;
	style->TabRounding = 10.0f;
	style->WindowRounding = 0.0f;

	style->FrameBorderSize = 0;
}

void override_key_callback(GLFWwindow* window_ptr, int key, int scancode, int action, int mods) {
	auto window = (Window*)glfwGetWindowUserPointer(window_ptr);
	
	ImGui_ImplGlfw_KeyCallback(window_ptr, key, scancode, action, mods);

	if (!ImGui::GetIO().WantCaptureKeyboard) {
		window->on_key.broadcast({ key, scancode, action, mods });
	}
}

void override_mouse_button_callback(GLFWwindow* window_ptr, int button, int action, int mods) {
	auto window = (Window*)glfwGetWindowUserPointer(window_ptr);
	
	ImGui_ImplGlfw_MouseButtonCallback(window_ptr, button, action, mods);

	if (!ImGui::GetIO().WantCaptureMouse) {
		window->on_mouse_button.broadcast({ button, action, mods });
	}
}

void Editor::select(ID id) {
	this->selected_id = id;
}

Editor::Editor(void (*register_systems_and_components)(World&), Window& window)
: window(window), picking_pass(window) {
	register_systems_and_components(world);
	world.add(new Store<EntityEditor>(100));
	
	register_on_inspect_callbacks();
	this->picking_pass = PickingPass(window);
}

void Editor::init_imgui() {
	// Setup ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigDockingWithShift = false;

	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

	ImFontConfig icons_config; icons_config.MergeMode = false; icons_config.PixelSnapH = true; icons_config.FontDataOwnedByAtlas = true; icons_config.FontDataSize = 72;

	auto font_path = Level::asset_path("fonts/segoeui.ttf");
	ImFont* font = io.Fonts->AddFontFromFileTTF(font_path.c_str(), 32.0f, &icons_config, io.Fonts->GetGlyphRangesDefault());

	set_darcula_theme();

	ImGui_ImplGlfw_InitForOpenGL(window.window_ptr, false);
	ImGui_ImplOpenGL3_Init();

	glfwSetKeyCallback(window.window_ptr, override_key_callback);
	glfwSetCharCallback(window.window_ptr, ImGui_ImplGlfw_CharCallback);
	glfwSetMouseButtonCallback(window.window_ptr, override_mouse_button_callback);
}

ImTextureID Editor::get_icon(const std::string& name) {
	for (auto icon : icons) {
		if (icon.name == name) {
			return (ImTextureID) RHI::texture_manager.get(icon.texture_id)->texture_id;
		}
	}
	
	throw "Could not find icon";
}

void Editor::run() {
	this->asset_tab.register_callbacks(window, *this);

	init_imgui();

	Input input(window);
	Time time;

	icons = {
		{"play", load_Texture("editor/play_button3.png")}
	};

	auto shader = load_Shader("shaders/pbr.vert", "shaders/gizmo.frag");
	auto texture = load_Texture("normal.jpg");
	auto model = load_Model(world, "HOVERTANK.fbx");
	auto skybox = load_Skybox(world, "Tropical_Beach_3k.hdr");

	{
		auto id = world.make_ID();
		auto e = world.make<Entity>(id);
		auto trans = world.make<Transform>(id);
		auto dir_light = world.make<DirLight>(id);

		auto name = world.make<EntityEditor>(id);
		name->name = "Light";
	}

	CommandBuffer cmd_buffer;
	MainPass main_pass(world, window);
	RenderParams render_params(&cmd_buffer, &main_pass);

	main_pass.post_process.append(&picking_pass);

	PickingSystem* picking_system = new PickingSystem(*this);

	world.add(picking_system);
	
	UpdateParams update_params(input);

	auto model_renderer_id = world.make_ID();

	{
		auto id = model_renderer_id;
		auto e = world.make<Entity>(id);
		auto trans = world.make<Transform>(id);
		trans->position.z = -5;

		select(id);

		auto name = world.make<EntityEditor>(id);
		name->name = "Plane";
	}

	{
		auto id = world.make_ID();
		auto e = world.make<Entity>(id);
		auto trans = world.make<Transform>(id);
		auto camera = world.make <Camera>(id);
		auto flyover = world.make<Flyover>(id);
		auto name = world.make<EntityEditor>(id);
		name->name = "Main camera";
	}

	vector<Param> params = make_SubstanceMaterial(world, "wood_2", "Stylized_Wood");

	Material material = {
		std::string("DefaultMaterial"),
		load_Shader("shaders/pbr.vert", "shaders/pbr.frag"),
		params,
		&default_draw_state
	};
	vector<Material> materials = { material };

	auto model_render = world.make<ModelRenderer>(model_renderer_id);
	model_render->model_id = model;
	model_render->set_materials(world, materials);

	while (!window.should_close() && !exit) {
		temporary_allocator.clear();
		cmd_buffer.clear();

		time.update_time(update_params);
		input.clear();
		window.poll_inputs();

		update_params.layermask = game_layer;

		render_params.layermask = game_layer;
		render_params.width = window.width;
		render_params.height = window.height;
		render_params.dir_light = get_dir_light(world, render_params.layermask);
		render_params.skybox = skybox;

		world.update(update_params);
		update(update_params);

		render_params.pass->render(world, render_params);
		
		render(render_params);

		window.swap_buffers();
	}
}

void Editor::update(UpdateParams& params) {
	if (params.input.key_pressed(GLFW_KEY_X)) {
		if (this->selected_id >= 0) {
			world.free_by_id<Entity>(this->selected_id);
			this->selected_id = -1;
		}
	}

	if (params.input.key_pressed(GLFW_KEY_L)) {
		log("about to serialize");
	}

	if (params.input.mouse_button_pressed(MouseButton::Left)) {
		this->selected_id = picking_pass.pick(world, params);
	}

	display_components.update(world, params);
	gizmo.update(world, *this, params);
}

void Editor::render(RenderParams& params) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	unsigned char* pixels;
	int width, height;
	ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	ImFontAtlas* atlas = ImGui::GetIO().Fonts;

	ImGui::PushFont(atlas->Fonts[0]);

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	bool p_open;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &p_open, window_flags);
	ImGui::PopStyleVar(3);

	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File")) {
		ImGui::MenuItem("New Scene", "CTRL+N");
		ImGui::MenuItem("Open Scene");
		ImGui::MenuItem("Recents");
		ImGui::MenuItem("Save", "CTRL+S");
		ImGui::MenuItem("Exit", "ALT+F4");
		ImGui::EndMenu();
	}
	
	if (ImGui::BeginMenu("Edit")) {
		ImGui::MenuItem("Copy", "CTRL+C");
		ImGui::MenuItem("Paste", "CTRL+V");
		ImGui::MenuItem("Duplicate", "CTRL+D");
		ImGui::EndMenu();
	}

	if (ImGui::BeginMenu("Settings")) {
		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();


	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	ImGui::End();

	ImGui::Begin("Controls", &p_open, ImGuiWindowFlags_NoDecoration);
	
	ImGui::ImageButton(get_icon("play"), ImVec2(40, 40));
	ImGui::Button("Play");
	ImGui::SameLine();
	ImGui::Button("Pause");
	ImGui::End();

	//==========================

	display_components.render(world, params, *this);
	lister.render(world, *this, params);
	gizmo.render(world, *this, params);
	
	ImGui::PopFont();
	ImGui::EndFrame();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

Editor::~Editor() {
	log("destructor");
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}