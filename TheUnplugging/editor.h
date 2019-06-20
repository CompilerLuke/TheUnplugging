#pragma once

#include <string>
#include "id.h"
#include "eventDispatcher.h"
#include "modelAssetTab.h"
#include "reflection.h"
#include "lister.h"
#include "displayComponents.h"
#include "ecs.h"
#include "gizmo.h"

struct DroppableField {
	void* ptr;
	std::string typ;
	ID id;
};

struct SwitchCase {
	void* ptr;
	int tag;
	reflect::TypeDescriptor_Union* type;
	int case_of_union;
};

struct AddComponent {
	ID id;
	struct ComponentStore* store;
};

struct DeleteComponent {
	ID id;
	struct ComponentStore* store;
};

struct Icon {
	std::string name;
	ID texture_id;
};

struct Editor {
	World world;
	Window& window;

	int selected_id = -1;
	float editor_tab_width = 0.25;
	float model_tab_width = 0.2;
	bool playing_game = false;
	bool exit = false;

	vector<Icon> icons;

	unsigned int get_icon(const std::string& name);
	
	ModelAssetTab asset_tab;
	Lister lister;
	DisplayComponents display_components;
	Gizmo gizmo;

	bool droppable_field_active;
	DroppableField droppable_field;

	EventDispatcher<ID> selected;

	Editor(std::string& level, void (*)(World&), struct Window&);
	~Editor();

	void update(struct UpdateParams&);
	void render(struct RenderParams&);
	void init_imgui();

	void run();

	void select(ID);
};
