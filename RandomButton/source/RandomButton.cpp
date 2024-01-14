#include <YYToolkit/Shared.hpp>
#include <CallbackManager/CallbackManagerInterface.h>
#include <InfiCore/InfiCoreInterface.h>
#include <nlohmann/json.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string_view>

using namespace Aurie;
using namespace YYTK;
using json = nlohmann::json;

static YYTKInterface* g_ModuleInterface = nullptr;
static CallbackManagerInterface* g_CmInterface = nullptr;
static InfiCoreInterface* g_IcInterface = nullptr;

std::string g_ModName = "Random Button";
bool g_HasConfig = true;

/*
		Config Variables
*/
enum SettingType {
	SETTING_BOOL = 0,
	SETTING_STRING_ARRAY = 1
};

struct Setting {
	std::string m_Name = "";
	int m_Icon = -1;
	SettingType m_Type = SETTING_BOOL;
	bool m_BoolValue = false;
	std::vector<std::string> m_StringArrayValue = {};

	Setting(std::string n, int i, bool bV) {
		m_Name = n;
		m_Icon = i;
		m_Type = SETTING_BOOL;
		m_BoolValue = bV;
	}

	Setting(std::string n, int i, std::vector<std::string> saV) {
		m_Name = n;
		m_Icon = i;
		m_Type = SETTING_STRING_ARRAY;
		m_StringArrayValue = saV;
	}
};

struct ModConfig {
	Setting m_DebugEnabled = Setting("debugEnabled", 9, false);
	Setting m_Blacklist = Setting("blacklist", 15, std::vector<std::string> {});
	Setting m_UseFavorites = Setting("useFavorites", 21, false);
	std::vector<Setting> m_Settings = { m_DebugEnabled, m_Blacklist, m_UseFavorites };
} g_Config;

/*
		Inline Functions
*/
template <typename... TArgs>
inline static void Print(CmColor Color, std::string_view Format, TArgs&&... Args) {
	g_ModuleInterface->Print(Color, Format, Args...);
}

template <typename... TArgs>
inline static void PrintError(std::string_view Filepath, int Line, std::string_view Format, TArgs&&... Args) {
	g_ModuleInterface->PrintError(Filepath, Line, Format, Args...);
}

inline static RValue CallBuiltin(const char* FunctionName, std::vector<RValue> Arguments) {
	return g_ModuleInterface->CallBuiltin(FunctionName, Arguments);
}

inline static double GetAssetIndexFromName(const char* name) {
	double index = CallBuiltin("asset_get_index", { name }).AsReal();
	return index;
}

inline static std::string U8ToStr(std::u8string u8str) {
	std::string str(u8str.cbegin(), u8str.cend());
	return str;
}

/*
		Helper Functions
*/
std::string FormatString(const std::string& Input) {
	std::string formatted_string = Input;

	formatted_string.erase(std::remove(formatted_string.begin(), formatted_string.end(), ' '), formatted_string.end());

	return formatted_string;
}

static void GenerateConfig(std::string FileName) {
	json data;

	for (const Setting& setting : g_Config.m_Settings) {
		json setting_data;
		setting_data["icon"] = setting.m_Icon;
		if (setting.m_Type == SETTING_BOOL) setting_data["value"] = setting.m_BoolValue;
		else if (setting.m_Type == SETTING_STRING_ARRAY) setting_data["value"] = setting.m_StringArrayValue;
		data[setting.m_Name.c_str()] = setting_data;
	}

	std::ofstream config_file("modconfigs/" + FileName);
	if (config_file.is_open()) {
		Print(CM_WHITE, "[%s] - Config file \"%s\" created!", g_ModName, FileName);
		config_file << std::setw(4) << data << std::endl;
		config_file.close();
	} else {
		PrintError(__FILE__, __LINE__, "[%s] - Error opening config file \"%s\"", g_ModName, FileName);
	}
}

void to_json(json& j, const ModConfig& c) {
	j = json {
		{ "debugEnabled", {
			{ "icon", c.m_DebugEnabled.m_Icon },
			{ "value", c.m_DebugEnabled.m_BoolValue }
		}},
		{ "blacklist", {
			{ "icon", c.m_Blacklist.m_Icon },
			{ "value", c.m_Blacklist.m_StringArrayValue }
		}},
		{ "useFavorites", {
			{ "icon", c.m_UseFavorites.m_Icon },
			{ "value", c.m_UseFavorites.m_BoolValue }
		}}
	};
}

void from_json(const json& j, ModConfig& c) {
	try {
		j.at("debugEnabled").at("icon").get_to(c.m_DebugEnabled.m_Icon);
		j.at("debugEnabled").at("value").get_to(c.m_DebugEnabled.m_BoolValue);
		j.at("blacklist").at("icon").get_to(c.m_Blacklist.m_Icon);
		j.at("blacklist").at("value").get_to(c.m_Blacklist.m_StringArrayValue);
		j.at("useFavorites").at("icon").get_to(c.m_UseFavorites.m_Icon);
		j.at("useFavorites").at("value").get_to(c.m_UseFavorites.m_BoolValue);
	} catch (const json::out_of_range& e) {
		PrintError(__FILE__, __LINE__, "%s", e.what());
		std::string file_name = FormatString(g_ModName) + ".config.json";
		GenerateConfig(file_name);
	}
}

/*
		GML Inline Functions
*/
inline static RValue variable_instance_get(CInstance* instance_id, const char* name) {
	RValue id, result;
	AurieStatus status = g_ModuleInterface->GetBuiltin("id", instance_id, NULL_INDEX, id);
	if (AurieSuccess(status)) {
		result = CallBuiltin("variable_instance_get", { id, name });
	}
	return result;
}

inline static void variable_instance_set(CInstance* instance_id, const char* name, double val) {
	RValue id;
	AurieStatus status = g_ModuleInterface->GetBuiltin("id", instance_id, NULL_INDEX, id);
	if (AurieSuccess(status)) {
		CallBuiltin("variable_instance_set", { id, name, val });
	}
}

inline static void variable_instance_set(CInstance* instance_id, const char* name, int val) {
	RValue id;
	AurieStatus status = g_ModuleInterface->GetBuiltin("id", instance_id, NULL_INDEX, id);
	if (AurieSuccess(status)) {
		CallBuiltin("variable_instance_set", { id, name, (double)val });
	}
}

inline static void variable_instance_set(CInstance* instance_id, const char* name, const char* val) {
	RValue id;
	AurieStatus status = g_ModuleInterface->GetBuiltin("id", instance_id, NULL_INDEX, id);
	if (AurieSuccess(status)) {
		CallBuiltin("variable_instance_set", { id, name, val });
	}
}

inline static RValue variable_global_get(const char* name) {
	return CallBuiltin("variable_global_get", { name });
}

inline static void variable_global_set(const char* name, const char* value) {
	CallBuiltin("variable_global_set", { name, value });
}

inline static void variable_global_set(const char* name, double value) {
	CallBuiltin("variable_global_set", { name, value });
}

inline static RValue struct_get(RValue variable, const char* name) {
	return CallBuiltin("struct_get", { variable, name });
}

inline static std::string_view array_get(RValue variable, double index) {
	return CallBuiltin("array_get", { variable, index }).AsString();
}

inline static void array_set(RValue variable, double index, const char* value) {
	CallBuiltin("array_set", { variable, index, value });
}

inline static void draw_sprite(const char* sprite, double subimg, double x, double y) {
	CallBuiltin("draw_sprite", { GetAssetIndexFromName(sprite), subimg, x, y });
}

inline static void draw_sprite_ext(const char* sprite, double subimg, double x, double y, double xscale, double yscale, double rot, double color, double alpha) {
	CallBuiltin("draw_sprite_ext", { GetAssetIndexFromName(sprite), subimg, x, y, xscale, yscale, rot, color, alpha });
}

inline static void draw_rectangle(double x1, double y1, double x2, double y2, bool outline) {
	CallBuiltin("draw_rectangle", { x1, y1, x2, y2, outline });
}

inline static void draw_button(double x1, double y1, double x2, double y2, bool up) {
	CallBuiltin("draw_button", { x1, y1, x2, y2, up });
}

inline static void draw_set_font(const char* font) {
	CallBuiltin("draw_set_font", { GetAssetIndexFromName(font) });
}

inline static void draw_set_halign(double halign) {
	CallBuiltin("draw_set_halign", { halign });
}

inline static void draw_set_valign(double valign) {
	CallBuiltin("draw_set_valign", { valign });
}

inline static void draw_set_color(double col) {
	CallBuiltin("draw_set_color", { col });
}

inline static void draw_set_alpha(double alpha) {
	CallBuiltin("draw_set_alpha", { alpha });
}

inline static void draw_text(double x, double y, const char* string) {
	CallBuiltin("draw_text", { x, y, string });
}

inline static void audio_play_sound(const char* name, double priority, bool loop) {
	CallBuiltin("audio_play_sound", { GetAssetIndexFromName(name), priority, loop });
}

inline static RValue ds_map_find_value(RValue id, const char* key) {
	return CallBuiltin("ds_map_find_value", { id, key });
}

inline static void array_resize(RValue array_index, double new_size) {
	CallBuiltin("array_resize", { array_index, new_size });
}

inline static int device_mouse_x_to_gui(double device) {
	return CallBuiltin("device_mouse_x_to_gui", { device }).AsReal();
}

inline static int device_mouse_y_to_gui(double device) {
	return CallBuiltin("device_mouse_y_to_gui", { device }).AsReal();
}

inline static bool device_mouse_check_button_pressed(double device, double button) {
	return CallBuiltin("device_mouse_check_button_pressed", { device, button }).AsBool();
}

// Blacklist menu variables
static bool blacklist_selected = false;
static bool blacklist_open = false;
static bool in_char_select = true;
static int last_selected_char = 0;
static int blacklist_error_timer = 0;
std::vector<int> indexes_to_remove;
std::vector<int> gen_lengths;
static int total_chars = 0;
std::vector<std::string> character_vector = {};
RValue yyrv_mouse_x;
RValue yyrv_mouse_y;
static bool mouse_moving = false;

static int GetCharIndexLimit(const char* Direction) {
	int char_index = 0;
	if (Direction == "down") {
		int gens_to_index = gen_lengths.size() - 1;

	}
}

static bool SetBlacklist(CInstance* Self) {
	if (indexes_to_remove.size() == static_cast<unsigned long long>(total_chars - 1)) {
		if (g_Config.m_UseFavorites.m_BoolValue == true) PrintError(__FILE__, __LINE__, "You can't favorite zero characters!");
		else PrintError(__FILE__, __LINE__, "You can't blacklist all characters!");
		return false;
	}

	// Create default array of 0 to total_chars
	std::vector<int> orig_random_char_array(total_chars - 1);
	std::iota(orig_random_char_array.begin(), orig_random_char_array.end(), 0);

	// Get random available characters array
	// [0, 1, 2, 3, ...]
	RValue random_char_array = variable_instance_get(Self, "randomAvailableCharacters");

	// Construct new random available characters array
	size_t new_size = orig_random_char_array.size() - indexes_to_remove.size();
	array_resize(random_char_array, new_size);
	int curr_index = 0;
	for (int i = 0; i < orig_random_char_array.size(); i++) {
		auto it = std::find(indexes_to_remove.begin(), indexes_to_remove.end(), orig_random_char_array[i]);
		if (it == indexes_to_remove.end()) {
			if (curr_index < new_size) {
				random_char_array[curr_index] = orig_random_char_array[i];
				curr_index++;
			} else {
				PrintError(__FILE__, __LINE__, "curr_index is out of bounds!");
			}
		}
	}

	std::vector<std::string> new_blacklist = {};
	for (int i = 0; i < character_vector.size(); i++) {
		auto it = std::find(indexes_to_remove.begin(), indexes_to_remove.end(), i);
		if (it != indexes_to_remove.end()) {
			new_blacklist.push_back(character_vector[i]);
		}
	}

	g_Config.m_Blacklist.m_StringArrayValue = new_blacklist;

	std::string file_name = FormatString(g_ModName) + ".config.json";
	std::ofstream configFile("modconfigs/" + file_name);
	json data = g_Config;
	if (configFile.is_open()) {
		configFile << std::setw(4) << data << std::endl;
		configFile.close();
	} else {
		PrintError(__FILE__, __LINE__, "[%s] - Error opening config file \"%s\"", g_ModName, file_name.c_str());
	}

	return true;
}

static std::string GetFileName(const char* File) {
	std::string s_file_name(File);
	size_t last_slash_pos = s_file_name.find_last_of("\\");
	if (last_slash_pos != std::string::npos && last_slash_pos != s_file_name.length()) {
		s_file_name = s_file_name.substr(last_slash_pos + 1);
	}
	return s_file_name;
}

static uint32_t FrameNumber = 0;

/*
		Frame Callback Event
*/
static AurieStatus FrameCallback(
	IN FWFrame& FrameContext
) {
	UNREFERENCED_PARAMETER(FrameContext);

	FrameNumber++;
	// Tell the core the handler was successful.
	return AURIE_SUCCESS;
}

/*
		Code Callback Events
*/

// gml_Object_obj_CharSelect_Create_0
void CharSelectCreateAfter(CodeEventArgs& Args) {
	CInstance* Self = std::get<0>(Args);

	// Get random character map value
	RValue character_data = variable_global_get("characterData");
	RValue random = ds_map_find_value(character_data, "random");

	// Add random character to end of character array
	RValue by_gen_array = variable_instance_get(Self, "charListByGen");
	int last_gen_index = by_gen_array.length() - 1;
	RValue last_gen_array = by_gen_array[last_gen_index];
	int last_gen_length = last_gen_array.length();

	array_resize(last_gen_array, last_gen_length + 1);
	last_gen_array[last_gen_length] = random;

	// Set random slot to last index of array
	total_chars = 0;
	gen_lengths.clear();
	int gen_length = 0;
	for (int i = 0; i < last_gen_index + 1; i++) {
		gen_length = by_gen_array[i].length();
		total_chars += gen_length;
		gen_lengths.push_back(gen_length);
	}

	variable_instance_set(Self, "randomSelectSlot", total_chars - 1);

	// Get character list
	// ["ame", "gura", "ina", ...]
	RValue character_list = variable_global_get("characterList");
	character_vector.clear();
	for (int i = 0; i < total_chars; i++) {
		character_vector.push_back(character_list[i].AsString().data());
	}

	// Find which numbers to remove
	// [0, 6, 17, 30, ...]
	indexes_to_remove.clear();
	for (int i = 0; i < g_Config.m_Blacklist.m_StringArrayValue.size(); i++) {
		auto it = std::find(character_vector.begin(), character_vector.end(), g_Config.m_Blacklist.m_StringArrayValue[i]);

		if (it != character_vector.end()) {
			int index = std::distance(character_vector.begin(), it);
			indexes_to_remove.push_back(index);
		} else {
			PrintError(__FILE__, __LINE__, "\"%s\" not found!", g_Config.m_Blacklist.m_StringArrayValue[i]);
		}
	}
	SetBlacklist(Self);
}

// gml_Object_obj_InputManager_Step_0
void InputManagerStepAfter(CodeEventArgs& Args) {
	CInstance* Self = std::get<0>(Args);
	mouse_moving = variable_instance_get(Self, "mouseMoving").AsBool();
}

// gml_Object_obj_CharSelect_Step_0
void CharSelectStepAfter(CodeEventArgs& Args) {
	CInstance* Self = std::get<0>(Args);
	int mouse_x = device_mouse_x_to_gui(0) / 2;
	int mouse_y = device_mouse_y_to_gui(0) / 2;
	bool blacklist_hovered = mouse_x > 484 && mouse_x < 548 && mouse_y > 11 && mouse_y < 32;
	if (mouse_moving == true) {
		if (blacklist_hovered) {
			if (blacklist_selected == false) {
				blacklist_selected = true;
				variable_instance_set(Self, "selectingGen", -1);
				audio_play_sound("snd_charSelectWoosh", 0, false);
			}
		} else {
			if (blacklist_selected == true) {
				blacklist_selected = false;
			}
		}
	}
	bool left_mouse_pressed = device_mouse_check_button_pressed(0, 1);
	if (left_mouse_pressed == 1 && blacklist_hovered && blacklist_selected) {
		if (blacklist_open == false) {
			audio_play_sound("snd_menu_confirm", 0, false);
			blacklist_open = true;
		} else if (blacklist_open == true) {
			if (SetBlacklist(Self)) {
				audio_play_sound("snd_menu_back", 0, false);
				blacklist_open = false;
				blacklist_error_timer = 0;
			} else {
				audio_play_sound("snd_alert", 0, false);
				blacklist_error_timer = 60 * 3;
			}
		}
	}
}

// gml_Object_obj_CharSelect_Draw_0	// 640x360 viewport size
void CharSelectDrawAfter(CodeEventArgs& Args) {
	int margin_w = 85;

	if (blacklist_open) {
		// Blacklist Header
		draw_set_color(16777215);
		draw_rectangle(0 + margin_w, 10, 640 - margin_w, 33, false);
		// Blacklist Header Text
		if (blacklist_error_timer > 0) {
			blacklist_error_timer--;
			draw_set_color(255);
			draw_set_alpha(1.00);
			draw_text(320, 17, g_Config.m_UseFavorites.m_BoolValue == false ? "ERROR: CAN'T BLACKLIST ALL CHARACTERS!" : "ERROR: CAN'T FAVORITE ZERO CHARACTERS!");
		} else {
			draw_set_color(16367178);
			draw_text(320, 17, g_Config.m_UseFavorites.m_BoolValue == false ? "EDITING BLACKLIST..." : "EDITING FAVORITES...");
		}
		// Blacklist Body
		draw_set_color(0);
		draw_set_alpha(0.25);
		draw_rectangle(0 + margin_w, 0 + 34, 640 - margin_w, 360 - 150, false);
		// Blacklist Body Outline
		draw_set_color(16777215);
		draw_set_alpha(1.00);
		draw_rectangle(0 + margin_w, 0 + 10, 640 - margin_w, 360 - 150, true);

		// If current location is a char to be removed, draw sprite at location
		draw_set_color(255);
		draw_set_alpha(0.50);
		int char_port_height = 38;
		int vert_spacing = 4;
		int char_port_y = 40;
		int stat_x = 110;
		int stat_y = 25;
		int spacing_y = 19;
		int char_x = 180;
		int overall_index = 0;
		for (int i = 0; i < gen_lengths.size(); i++) {
			for (int j = 0; j < gen_lengths[i]; j++) {
				auto it = std::find(indexes_to_remove.begin(), indexes_to_remove.end(), overall_index);
				if (it != indexes_to_remove.end()) {
					int push_in_x = ((6 - gen_lengths[i]) * 23);
					draw_rectangle(
						((char_x + (j * 46)) + 3) + push_in_x,
						char_port_y + (i * (char_port_height + vert_spacing)),
						((char_x + push_in_x) + (j * 46)) + 45,
						(char_port_y + (i * (char_port_height + vert_spacing))) + (char_port_height - 1),
						false
					);
				}
				overall_index++;
			}
		}

	}

	if (in_char_select == true) {
		// Blacklist Button
		draw_set_color(0);
		draw_set_alpha(0.25);
		draw_button(484, 11, 548, 32, !blacklist_selected);
		// Blacklist Button Text
		draw_set_color(16777215);
		draw_set_alpha(1.00);
		draw_text(517, 17, g_Config.m_UseFavorites.m_BoolValue == false ? "BLACKLIST" : "FAVORITES");
	}

	draw_set_color(16777215);
	draw_set_alpha(1.00);
}

/*
		Script Callback Functions
*/

// gml_Script_Up_gml_Object_obj_CharSelect_Create_0
RValue& UpCharSelectBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args) {
	int selecting_gen = variable_instance_get(Self, "selectingGen").AsReal();
	if (selecting_gen == 0) {
		variable_instance_set(Self, "selectingGen", -1);
		blacklist_selected = true;
		audio_play_sound("snd_charSelectWoosh", 0, false);
		g_CmInterface->CancelOriginalFunction();
	}
	return ReturnValue;
};

// gml_Script_Down_gml_Object_obj_CharSelect_Create_0
RValue& DownCharSelectBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args) {
	int selecting_gen = variable_instance_get(Self, "selectingGen").AsReal();
	if (selecting_gen == -1) {
		variable_instance_set(Self, "selectingGen", 0);
		int selecting_char = variable_instance_get(Self, "selectingChar").AsReal();
		variable_instance_set(Self, "selectedCharacter", selecting_char);
		blacklist_selected = false;
		audio_play_sound("snd_charSelectWoosh", 0, false);
		g_CmInterface->CancelOriginalFunction();
	}
	return ReturnValue;
};

// gml_Script_Left_gml_Object_obj_CharSelect_Create_0
RValue& LeftCharSelectBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args) {
	int selecting_gen = variable_instance_get(Self, "selectingGen").AsReal();
	if (selecting_gen == -1) {
		g_CmInterface->CancelOriginalFunction();
	}
	return ReturnValue;
};

// gml_Script_Right_gml_Object_obj_CharSelect_Create_0
RValue& RightCharSelectBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args) {
	int selecting_gen = variable_instance_get(Self, "selectingGen").AsReal();
	if (selecting_gen == -1) {
		g_CmInterface->CancelOriginalFunction();
	}
	return ReturnValue;
};

// gml_Script_Select_gml_Object_obj_CharSelect_Create_0
RValue& SelectCharSelectBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args) {
	int selecting_gen = variable_instance_get(Self, "selectingGen").AsReal();
	if (selecting_gen == -1) {
		if (blacklist_open == false) {
			audio_play_sound("snd_menu_confirm", 0, false);
			blacklist_open = true;
		} else if (blacklist_open == true) {
			if (SetBlacklist(Self)) {
				audio_play_sound("snd_menu_back", 0, false);
				blacklist_open = false;
				blacklist_error_timer = 0;
			} else {
				audio_play_sound("snd_alert", 0, false);
				blacklist_error_timer = 60 * 3;
			}
		}
		g_CmInterface->CancelOriginalFunction();
	} else if (blacklist_open == true) {
		int selected_character = variable_instance_get(Self, "selectedCharacter").AsReal();
		if (selected_character == total_chars - 1) {
			g_CmInterface->CancelOriginalFunction();
			return ReturnValue;
		}
		auto it = std::find(indexes_to_remove.begin(), indexes_to_remove.end(), selected_character);
		if (it == indexes_to_remove.end()) {				// if not in blacklist, add
			indexes_to_remove.push_back(selected_character);
		} else {										// if in blacklist, remove
			indexes_to_remove.erase(std::remove(indexes_to_remove.begin(), indexes_to_remove.end(), selected_character), indexes_to_remove.end());
		}
		audio_play_sound("snd_menu_confirm", 0, false);
		g_CmInterface->CancelOriginalFunction();
	} else {
		// Check if a character is selected
		RValue char_selected = variable_global_get("charSelected");
		if (char_selected.m_Object != Self) {
			in_char_select = false;
		}
	}
	return ReturnValue;
};

// gml_Script_Return_gml_Object_obj_CharSelect_Create_0
RValue& ReturnCharSelectBefore(CInstance* Self, CInstance* Other, RValue& ReturnValue, int numArgs, RValue** Args) {
	if (blacklist_open == true) {
		if (SetBlacklist(Self)) {
			audio_play_sound("snd_menu_back", 0, false);
			blacklist_open = false;
			blacklist_error_timer = 0;
		} else {
			audio_play_sound("snd_alert", 0, false);
			blacklist_error_timer = 60 * 3;
		}
		g_CmInterface->CancelOriginalFunction();
	} else {
		// Check if a character is selected
		RValue char_selected = variable_global_get("charSelected");
		if (char_selected.m_Kind == 0 && char_selected.AsReal() == -1) {
			in_char_select = true;
		}
	}
	return ReturnValue;
};

EXPORTED AurieStatus ModuleInitialize(
	IN AurieModule* Module,
	IN const fs::path& ModulePath
) {
	UNREFERENCED_PARAMETER(ModulePath);

	AurieStatus status = AURIE_SUCCESS;

	// Gets a handle to the interface exposed by YYTK
	// You can keep this pointer for future use, as it will not change unless YYTK is unloaded.
	status = ObGetInterface(
		"YYTK_Main",
		(AurieInterfaceBase*&)(g_ModuleInterface)
	);

	// If we can't get the interface, we fail loading.
	if (!AurieSuccess(status))
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;

	Print(CM_LIGHTGREEN, "[%s] - Hello from PluginEntry!", g_ModName);

	status = ObGetInterface(
		"callbackManager",
		(AurieInterfaceBase*&)(g_CmInterface)
	);

	if (!AurieSuccess(status)) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to get Callback Manager Interface! Make sure that CallbackManagerMod.dll is located in the mods/Aurie directory.", g_ModName);
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	Print(CM_LIGHTGREEN, "[%s] - Callback Manager Interface loaded!", g_ModName);

	status = ObGetInterface(
		"InfiCore",
		(AurieInterfaceBase*&)(g_IcInterface)
	);

	if (!AurieSuccess(status)) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to get InfiCore Interface! Make sure that InfiCore.dll is located in the mods/Aurie directory.", g_ModName);
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	Print(CM_LIGHTGREEN, "[%s] - InfiCore Interface loaded!", g_ModName);

	// Create callback for Frame Events
	status = g_ModuleInterface->CreateCallback(
		Module,
		EVENT_FRAME,
		FrameCallback,
		0
	);

	if (!AurieSuccess(status)) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback!", g_ModName);
	}

	if (g_HasConfig == true) {
		// Load mod config file or create one if there isn't one already.
		const wchar_t* dir_name = L"modconfigs";

		if (GetFileAttributes(dir_name) == INVALID_FILE_ATTRIBUTES) {
			if (CreateDirectory(dir_name, NULL)) {
				Print(CM_LIGHTGREEN, "[%s] - Directory \"modconfigs\" created!", g_ModName);
			} else {
				PrintError(__FILE__, __LINE__, "Failed to create the modconfigs directory. Error code: %lu", GetLastError());
				return AURIE_ACCESS_DENIED;
			}
		}
		
		std::string file_name = FormatString(g_ModName) + ".config.json";
		std::ifstream config_file("modconfigs/" + file_name);
		json data;
		if (config_file.is_open() == false) {	// no config file
			GenerateConfig(file_name);
		} else {
			try {
				data = json::parse(config_file);
			} catch (json::parse_error& e) {
				PrintError(__FILE__, __LINE__, "Message: %s\nException ID: %d\nByte Position of Error: %u", e.what(), e.id, (unsigned)e.byte);
				return AURIE_FILE_PART_NOT_FOUND;
			}

			g_Config = data.template get<ModConfig>();
		}
		Print(CM_LIGHTGREEN, "[%s] - %s loaded successfully!", g_ModName, file_name.c_str());

		// Register function pointer for InfiCore to reload config on change
		std::string mod_file_name = FormatString(g_ModName) + ".dll";
		g_IcInterface->SetReadConfigCallback(mod_file_name, []() {
			std::string file_name = FormatString(g_ModName) + ".config.json";
			std::ifstream config_file("modconfigs/" + file_name);
			json data;
			if (config_file.is_open() == false) {	// no config file
				GenerateConfig(file_name);
			} else {
				try {
					data = json::parse(config_file);
				} catch (json::parse_error& e) {
					PrintError(__FILE__, __LINE__, "Message: %s\nException ID: %d\nByte Position of Error: %u", e.what(), e.id, (unsigned)e.byte);
					return AURIE_FILE_PART_NOT_FOUND;
				}

				g_Config = data.template get<ModConfig>();
			}
			Print(CM_LIGHTGREEN, "[InfiCore] - Settings updated for %s!", g_ModName);
		});
	}

	/*
			Code Event Hooks
	*/
	if (!AurieSuccess(g_CmInterface->RegisterCodeEventCallback(g_ModName, "gml_Object_obj_CharSelect_Create_0", nullptr, CharSelectCreateAfter))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterCodeEventCallback(g_ModName, "gml_Object_obj_InputManager_Step_0", nullptr, InputManagerStepAfter))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Object_obj_InputManager_Step_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterCodeEventCallback(g_ModName, "gml_Object_obj_CharSelect_Step_0", nullptr, CharSelectStepAfter))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Object_obj_CharSelect_Step_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterCodeEventCallback(g_ModName, "gml_Object_obj_CharSelect_Draw_0", nullptr, CharSelectDrawAfter))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Object_obj_CharSelect_Draw_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	/*
			Script Function Hooks
	*/
	if (!AurieSuccess(g_CmInterface->RegisterScriptFunctionCallback(g_ModName, "gml_Script_Up_gml_Object_obj_CharSelect_Create_0", UpCharSelectBefore, nullptr, nullptr))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Script_Up_gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterScriptFunctionCallback(g_ModName, "gml_Script_Down_gml_Object_obj_CharSelect_Create_0", DownCharSelectBefore, nullptr, nullptr))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Script_Down_gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterScriptFunctionCallback(g_ModName, "gml_Script_Left_gml_Object_obj_CharSelect_Create_0", LeftCharSelectBefore, nullptr, nullptr))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Script_Left_gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterScriptFunctionCallback(g_ModName, "gml_Script_Right_gml_Object_obj_CharSelect_Create_0", RightCharSelectBefore, nullptr, nullptr))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Script_Right_gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterScriptFunctionCallback(g_ModName, "gml_Script_Select_gml_Object_obj_CharSelect_Create_0", SelectCharSelectBefore, nullptr, nullptr))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Script_Select_gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}
	if (!AurieSuccess(g_CmInterface->RegisterScriptFunctionCallback(g_ModName, "gml_Script_Return_gml_Object_obj_CharSelect_Create_0", ReturnCharSelectBefore, nullptr, nullptr))) {
		PrintError(__FILE__, __LINE__, "[%s] - Failed to register callback for %s", g_ModName, "gml_Script_Return_gml_Object_obj_CharSelect_Create_0");
		return AURIE_MODULE_DEPENDENCY_NOT_RESOLVED;
	}

	Print(CM_LIGHTGREEN, "[%s] - Everything initialized successfully!", g_ModName);

	return AURIE_SUCCESS;
}