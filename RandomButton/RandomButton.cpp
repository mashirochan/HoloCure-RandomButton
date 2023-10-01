#define YYSDK_PLUGIN
#include "RandomButton.hpp"	// Include our header
#include "ModInfo.h"
#include <Windows.h>    // Include Windows's mess.
#include <vector>       // Include the STL vector.
#include <unordered_map>
#include <functional>
#include <iostream>
#include <fstream>
#include "json.hpp"
#include "Utils/MH/MinHook.h"
#include "Features/API/Internal.hpp"
using json = nlohmann::json;

static struct Version {
	int major = VERSION_MAJOR;
	int minor = VERSION_MINOR;
	int build = VERSION_BUILD;
} version;

static struct Mod {
	Version version;
	const char* name = MOD_NAME;
} mod;

// Config variables
static struct Config {
	bool debugEnabled = false;
	std::vector<std::string> blacklist = {};
} config;

void to_json(json& j, const Config& c) {
	j = json{
		{ "debugEnabled", c.debugEnabled },
		{ "blacklist", c.blacklist }
	};
}

void from_json(const json& j, Config& c) {
	try {
		j.at("debugEnabled").get_to(c.debugEnabled);
		j.at("blacklist").get_to(c.blacklist);
	} catch (const json::out_of_range& e) {
		PrintError(__FILE__, __LINE__, "%s", e.what());
		std::string fileName = formatString(std::string(mod.name)) + "-config.json";
		GenerateConfig(fileName);
	}
}

std::string formatString(const std::string& input) {
	std::string formattedString = input;

	for (char& c : formattedString) {
		c = std::tolower(c);
	}

	for (char& c : formattedString) {
		if (c == ' ') {
			c = '-';
		}
	}

	return formattedString;
}

void GenerateConfig(std::string fileName) {
	json data = config;

	std::ofstream configFile("modconfigs/" + fileName);
	if (configFile.is_open()) {
		PrintMessage(CLR_DEFAULT, "[%s v%d.%d.%d] - Config file \"%s\" created!", mod.name, mod.version.major, mod.version.minor, mod.version.build, fileName.c_str());
		configFile << std::setw(4) << data << std::endl;
		configFile.close();
	} else {
		PrintError(__FILE__, __LINE__, "[%s v%d.%d.%d] - Error opening config file \"%s\"", mod.name, mod.version.major, mod.version.minor, mod.version.build, fileName.c_str());
	}
}

// Blacklist menu variables
static bool blacklistSelected = false;
static bool blacklistOpen = false;
static bool inCharSelect = true;
static int blacklistErrorTimer = 0;
std::vector<int> indexesToRemove;
std::vector<int> genLengths;
static int totalChars = 0;
std::vector<std::string> characterList = {};
RValue* yyrv_newRandCharArray = nullptr;

bool SetBlacklist(CInstance* Self, CInstance* Other) {
	if (indexesToRemove.size() == totalChars - 1) {
		PrintError(__FILE__, __LINE__, "You can't blacklist all characters!");
		return false;
	}

	// Create default array of 0 to totalChars
	std::vector<int> origRandomCharArray(totalChars - 1);
	std::iota(origRandomCharArray.begin(), origRandomCharArray.end(), 0);

	// Get random available characters array
	// [0, 1, 2, 3, ...]
	YYRValue yyrv_randomCharArray;
	CallBuiltin(yyrv_randomCharArray, "variable_instance_get", Self, Other, { (long long)Self->i_id, "randomAvailableCharacters" });

	// Construct new random available characters array
	size_t newSize = origRandomCharArray.size() - indexesToRemove.size();
	yyrv_newRandCharArray = new RValue[newSize];
	int currIndex = 0;
	for (int i = 0; i < origRandomCharArray.size(); i++) {
		auto it = std::find(indexesToRemove.begin(), indexesToRemove.end(), origRandomCharArray[i]);
		if (it == indexesToRemove.end()) {
			if (currIndex < newSize) {
				yyrv_newRandCharArray[currIndex].Kind = 0;
				yyrv_newRandCharArray[currIndex].Real = origRandomCharArray[i];
				currIndex++;
			} else {
				PrintError(__FILE__, __LINE__, "currIndex is out of bounds!");
			}
		}
	}
	yyrv_randomCharArray.RefArray->m_Array = yyrv_newRandCharArray;
	yyrv_randomCharArray.RefArray->length = newSize;

	std::vector<std::string> newBlacklist = {};
	for (int i = 0; i < characterList.size(); i++) {
		auto it = std::find(indexesToRemove.begin(), indexesToRemove.end(), i);
		if (it != indexesToRemove.end()) {
			newBlacklist.push_back(characterList[i]);
		}
	}

	config.blacklist = newBlacklist;

	std::string fileName = formatString(std::string(mod.name)) + "-config.json";
	std::ofstream configFile("modconfigs/" + fileName);
	json data = config;

	if (configFile.is_open()) {
		configFile << std::setw(4) << data << std::endl;
		configFile.close();
	} else {
		PrintError(__FILE__, __LINE__, "[%s v%d.%d.%d] - Error opening config file \"%s\"", mod.name, mod.version.major, mod.version.minor, mod.version.build, fileName.c_str());
	}

	return true;
}

// Function hooks
using FNScriptData = CScript * (*)(int);
FNScriptData scriptList = nullptr;
TRoutine assetGetIndexFunc;

YYTKStatus MmGetScriptData(FNScriptData& outScript) {
#ifdef _WIN64

	uintptr_t FuncCallPattern = FindPattern("\xE8\x00\x00\x00\x00\x33\xC9\x0F\xB7\xD3", "x????xxxxx", 0, 0);

	if (!FuncCallPattern)
		return YYTK_INVALIDRESULT;

	uintptr_t Relative = *reinterpret_cast<uint32_t*>(FuncCallPattern + 1);
	Relative = (FuncCallPattern + 5) + Relative;

	if (!Relative)
		return YYTK_INVALIDRESULT;

	outScript = reinterpret_cast<FNScriptData>(Relative);

	return YYTK_OK;
#else
	return YYTK_UNAVAILABLE;
#endif
}

void Hook(void* NewFunc, void* TargetFuncPointer, void** pfnOriginal, const char* Name) {
	if (TargetFuncPointer) {
		auto Status = MH_CreateHook(TargetFuncPointer, NewFunc, pfnOriginal);
		if (Status != MH_OK)
			PrintMessage(
				CLR_RED,
				"Failed to hook function %s (MH Status %s) in %s at line %d",
				Name,
				MH_StatusToString(Status),
				__FILE__,
				__LINE__
			);
		else
			MH_EnableHook(TargetFuncPointer);

		PrintMessage(CLR_GRAY, "- &%s = 0x%p", Name, TargetFuncPointer);
	} else {
		PrintMessage(
			CLR_RED,
			"Failed to hook function %s (address not found) in %s at line %d",
			Name,
			__FILE__,
			__LINE__
		);
	}
};

void HookScriptFunction(const char* scriptFunctionName, void* detourFunction, void** origScript) {
	RValue Result;
	RValue arg{};
	arg.Kind = VALUE_STRING;
	arg.String = RefString::Alloc(scriptFunctionName, strlen(scriptFunctionName));
	assetGetIndexFunc(&Result, nullptr, nullptr, 1, &arg);

	int scriptFunctionIndex = static_cast<int>(Result.Real) - 100000;

	CScript* CScript = scriptList(scriptFunctionIndex);

	Hook(
		detourFunction,
		(void*)(CScript->s_pFunc->pScriptFunc),
		origScript,
		scriptFunctionName
	);
}

typedef YYRValue* (*ScriptFunc)(CInstance* Self, CInstance* Other, YYRValue* ReturnValue, int numArgs, YYRValue** Args);

// gml_Script_Up_gml_Object_obj_CharSelect_Create_0
ScriptFunc origUpCharSelectScript = nullptr;
YYRValue* UpCharSelectFuncDetour(CInstance* Self, CInstance* Other, YYRValue* ReturnValue, int numArgs, YYRValue** Args) {
	YYRValue yyrv_selectingGen;
	YYRValue result;
	CallBuiltin(yyrv_selectingGen, "variable_instance_get", Self, Other, { (long long)Self->i_id, "selectingGen" });
	if (static_cast<int>(yyrv_selectingGen) == 0) {
		CallBuiltin(result, "variable_instance_set", Self, Other, { (long long)Self->i_id, "selectingGen", (double)-1 });
		blacklistSelected = true;
		CallBuiltin(result, "audio_play_sound", Self, Other, { (long long)171, (double)0, false }); // 171 = snd_charSelectWoosh
	}
	YYRValue* res = origUpCharSelectScript(Self, Other, ReturnValue, numArgs, Args);
	return res;
};

// gml_Script_Down_gml_Object_obj_CharSelect_Create_0
ScriptFunc origDownCharSelectScript = nullptr;
YYRValue* DownCharSelectFuncDetour(CInstance* Self, CInstance* Other, YYRValue* ReturnValue, int numArgs, YYRValue** Args) {
	YYRValue* res = nullptr;
	YYRValue yyrv_selectingGen;
	YYRValue result;
	CallBuiltin(yyrv_selectingGen, "variable_instance_get", Self, Other, { (long long)Self->i_id, "selectingGen" });
	if (static_cast<int>(yyrv_selectingGen) == -1) {
		CallBuiltin(result, "variable_instance_set", Self, Other, { (long long)Self->i_id, "selectingGen", (double)0 });
		blacklistSelected = false;
		CallBuiltin(result, "audio_play_sound", Self, Other, { (long long)171, (double)0, false });
		return res;
	}
	res = origDownCharSelectScript(Self, Other, ReturnValue, numArgs, Args);
	return res;
};

// gml_Script_Left_gml_Object_obj_CharSelect_Create_0
ScriptFunc origLeftCharSelectScript = nullptr;
YYRValue* LeftCharSelectFuncDetour(CInstance* Self, CInstance* Other, YYRValue* ReturnValue, int numArgs, YYRValue** Args) {
	YYRValue* res = nullptr;
	YYRValue yyrv_selectingGen;
	CallBuiltin(yyrv_selectingGen, "variable_instance_get", Self, Other, { (long long)Self->i_id, "selectingGen" });
	if (static_cast<int>(yyrv_selectingGen) == -1) {
		return res;
	}
	res = origLeftCharSelectScript(Self, Other, ReturnValue, numArgs, Args);
	return res;
};

// gml_Script_Right_gml_Object_obj_CharSelect_Create_0
ScriptFunc origRightCharSelectScript = nullptr;
YYRValue* RightCharSelectFuncDetour(CInstance* Self, CInstance* Other, YYRValue* ReturnValue, int numArgs, YYRValue** Args) {
	YYRValue* res = nullptr;
	YYRValue yyrv_selectingGen;
	CallBuiltin(yyrv_selectingGen, "variable_instance_get", Self, Other, { (long long)Self->i_id, "selectingGen" });
	if (static_cast<int>(yyrv_selectingGen) == -1) {
		return res;
	}
	res = origRightCharSelectScript(Self, Other, ReturnValue, numArgs, Args);
	return res;
};

// gml_Script_Select_gml_Object_obj_CharSelect_Create_0
ScriptFunc origSelectCharSelectScript = nullptr;
YYRValue* SelectCharSelectFuncDetour(CInstance* Self, CInstance* Other, YYRValue* ReturnValue, int numArgs, YYRValue** Args) {
	YYRValue* res = nullptr;
	YYRValue yyrv_selectingGen;
	CallBuiltin(yyrv_selectingGen, "variable_instance_get", Self, Other, { (long long)Self->i_id, "selectingGen" });
	if (static_cast<int>(yyrv_selectingGen) == -1) {
		YYRValue result;
		if (blacklistOpen == false) {
			CallBuiltin(result, "audio_play_sound", Self, Other, { (long long)76, (double)0, false }); // 76 = snd_menu_confirm
			blacklistOpen = true;
		} else if (blacklistOpen == true) {
			if (SetBlacklist(Self, Other)) {
				CallBuiltin(result, "audio_play_sound", Self, Other, { (long long)55, (double)0, false }); // 55 = snd_menu_back
				blacklistOpen = false;
				blacklistErrorTimer = 0;
			} else {
				CallBuiltin(result, "audio_play_sound", Self, Other, { (long long)83, (double)0, false }); // 83 = snd_alert
				blacklistErrorTimer = 60 * 3;
			}
		}
		return res;
	} else if (blacklistOpen == true) {
		YYRValue yyrv_selectedselectedCharacter;
		CallBuiltin(yyrv_selectedselectedCharacter, "variable_instance_get", Self, Other, { (long long)Self->i_id, "selectedCharacter" });
		int selectedCharacter = static_cast<int>(yyrv_selectedselectedCharacter);
		if (selectedCharacter == totalChars - 1) return res;
		auto it = std::find(indexesToRemove.begin(), indexesToRemove.end(), selectedCharacter);
		if (it == indexesToRemove.end()) {				// if not in blacklist, add
			indexesToRemove.push_back(selectedCharacter);
		} else {										// if in blacklist, remove
			indexesToRemove.erase(std::remove(indexesToRemove.begin(), indexesToRemove.end(), selectedCharacter), indexesToRemove.end());
		}
		return res;
	} else {
		res = origSelectCharSelectScript(Self, Other, ReturnValue, numArgs, Args);
		// Check if a character is selected
		YYRValue yyrv_charSelected;
		CallBuiltin(yyrv_charSelected, "variable_global_get", Self, Other, { "charSelected" });
		if (yyrv_charSelected.Object != Self) {
			inCharSelect = false;
		}
		return res;
	}
};

// gml_Script_Return_gml_Object_obj_CharSelect_Create_0
ScriptFunc origReturnCharSelectScript = nullptr;
YYRValue* ReturnCharSelectFuncDetour(CInstance* Self, CInstance* Other, YYRValue* ReturnValue, int numArgs, YYRValue** Args) {
	YYRValue* res = nullptr;
	YYRValue yyrv_selectingGen;
	CallBuiltin(yyrv_selectingGen, "variable_instance_get", Self, Other, { (long long)Self->i_id, "selectingGen" });
	YYRValue result;
	if (blacklistOpen == true) {
		if (SetBlacklist(Self, Other)) {
			CallBuiltin(result, "audio_play_sound", Self, Other, { (long long)55, (double)0, false }); // 55 = snd_menu_back
			blacklistOpen = false;
			blacklistErrorTimer = 0;
		} else {
			CallBuiltin(result, "audio_play_sound", Self, Other, { (long long)83, (double)0, false }); // 83 = snd_alert
			blacklistErrorTimer = 60 * 3;
		}
		return res;
	} else {
		res = origReturnCharSelectScript(Self, Other, ReturnValue, numArgs, Args);
		// Check if a character is selected
		YYRValue yyrv_charSelected;
		CallBuiltin(yyrv_charSelected, "variable_global_get", Self, Other, { "charSelected" });
		if (static_cast<int>(yyrv_charSelected) == -1) {
			inCharSelect = true;
		}
		return res;
	}
};

inline void CallOriginal(YYTKCodeEvent* pCodeEvent, CInstance* Self, CInstance* Other, CCode* Code, RValue* Res, int Flags) {
	if (!pCodeEvent->CalledOriginal()) {
		pCodeEvent->Call(Self, Other, Code, Res, Flags);
	}
}

std::string GetFileName(const char* File) {
	std::string sFileName(File);
	size_t LastSlashPos = sFileName.find_last_of("\\");
	if (LastSlashPos != std::string::npos && LastSlashPos != sFileName.length()) {
		sFileName = sFileName.substr(LastSlashPos + 1);
	}
	return sFileName;
}

// CallBuiltIn is way too slow to use per frame. Need to investigate if there's a better way to call in built functions.

// We save the CodeCallbackHandler attributes here, so we can unregister the callback in the unload routine.
static CallbackAttributes_t* g_pFrameCallbackAttributes = nullptr;
static CallbackAttributes_t* g_pCodeCallbackAttributes = nullptr;
static uint32_t FrameNumber = 0;

static std::unordered_map<int, const char*> codeIndexToName;
static std::unordered_map<int, std::function<void(YYTKCodeEvent* pCodeEvent, CInstance* Self, CInstance* Other, CCode* Code, RValue* Res, int Flags)>> codeFuncTable;

static const char* playStr = "Play Modded!";
RefString tempVar = RefString(playStr, strlen(playStr), false);
static bool versionTextChanged = false;

// This callback is registered on EVT_PRESENT and EVT_ENDSCENE, so it gets called every frame on DX9 / DX11 games.
YYTKStatus FrameCallback(YYTKEventBase* pEvent, void* OptionalArgument) {
	FrameNumber++;
	// Tell the core the handler was successful.
	return YYTK_OK;
}

// This callback is registered on EVT_CODE_EXECUTE, so it gets called every game function call.
YYTKStatus CodeCallback(YYTKEventBase* pEvent, void* OptionalArgument) {
	YYTKCodeEvent* pCodeEvent = dynamic_cast<decltype(pCodeEvent)>(pEvent);

	std::tuple<CInstance*, CInstance*, CCode*, RValue*, int> args = pCodeEvent->Arguments();

	CInstance* Self = std::get<0>(args);
	CInstance* Other = std::get<1>(args);
	CCode* Code = std::get<2>(args);
	RValue* Res = std::get<3>(args);
	int			Flags = std::get<4>(args);

	if (!Code->i_pName) {
		return YYTK_INVALIDARG;
	}

	if (codeFuncTable.count(Code->i_CodeIndex) != 0) {
		codeFuncTable[Code->i_CodeIndex](pCodeEvent, Self, Other, Code, Res, Flags);
	} else // Haven't cached the function in the table yet. Run the if statements and assign the function to the code index
	{
		codeIndexToName[Code->i_CodeIndex] = Code->i_pName;
		if (_strcmpi(Code->i_pName, "gml_Object_obj_TitleScreen_Create_0") == 0) {
			auto TitleScreen_Create_0 = [](YYTKCodeEvent* pCodeEvent, CInstance* Self, CInstance* Other, CCode* Code, RValue* Res, int Flags) {
				if (versionTextChanged == false) {
					YYRValue yyrv_version;
					CallBuiltin(yyrv_version, "variable_global_get", Self, Other, { "version" });
					if (config.debugEnabled) PrintMessage(CLR_AQUA, "[%s:%d] variable_global_get : version", GetFileName(__FILE__).c_str(), __LINE__);
					if (yyrv_version.operator std::string().find("Modded") == std::string::npos) {
						std::string moddedVerStr = yyrv_version.operator std::string() + " (Modded)";
						CallBuiltin(yyrv_version, "variable_global_set", Self, Other, { "version", moddedVerStr.c_str() });
						if (config.debugEnabled) PrintMessage(CLR_TANGERINE, "[%s:%d] variable_global_set : version", GetFileName(__FILE__).c_str(), __LINE__);
					}
					versionTextChanged = true;
				}

				for (int i = 0; i < config.blacklist.size(); i++) {
					if (config.debugEnabled) PrintMessage(CLR_BRIGHTPURPLE, "blacklist[%d] = %s", i, config.blacklist[i].c_str());
				}

				CallOriginal(pCodeEvent, Self, Other, Code, Res, Flags);
			};
			TitleScreen_Create_0(pCodeEvent, Self, Other, Code, Res, Flags);
			codeFuncTable[Code->i_CodeIndex] = TitleScreen_Create_0;
		}
		else if (_strcmpi(Code->i_pName, "gml_Object_obj_TextController_Create_0") == 0) {
			auto TextController_Create_0 = [](YYTKCodeEvent* pCodeEvent, CInstance* Self, CInstance* Other, CCode* Code, RValue* Res, int Flags) {
				CallOriginal(pCodeEvent, Self, Other, Code, Res, Flags);
				YYRValue yyrv_textContainer;
				CallBuiltin(yyrv_textContainer, "variable_global_get", Self, Other, { "TextContainer" });
				if (config.debugEnabled) PrintMessage(CLR_AQUA, "[%s:%d] variable_global_get : TextContainer", GetFileName(__FILE__).c_str(), __LINE__);
				YYRValue yyrv_titleButtons;
				CallBuiltin(yyrv_titleButtons, "struct_get", Self, Other, { yyrv_textContainer, "titleButtons" });
				if (config.debugEnabled) PrintMessage(CLR_AQUA, "[%s:%d] struct_get : titleButtons", GetFileName(__FILE__).c_str(), __LINE__);
				YYRValue yyrv_eng;
				CallBuiltin(yyrv_eng, "struct_get", Self, Other, { yyrv_titleButtons, "eng" });
				if (config.debugEnabled) PrintMessage(CLR_AQUA, "[%s:%d] struct_get : eng", GetFileName(__FILE__).c_str(), __LINE__);
				if (std::string(yyrv_eng.RefArray->m_Array[0].String->Get()).find("Modded") == std::string::npos) {
					yyrv_eng.RefArray->m_Array[0].String = &tempVar;
					if (config.debugEnabled) PrintMessage(CLR_TANGERINE, "[%s:%d] variable_global_set : eng[0]", GetFileName(__FILE__).c_str(), __LINE__);
				}
			};
			TextController_Create_0(pCodeEvent, Self, Other, Code, Res, Flags);
			codeFuncTable[Code->i_CodeIndex] = TextController_Create_0;
		}
		else if (_strcmpi(Code->i_pName, "gml_Object_obj_CharSelect_Create_0") == 0) {
			auto CharSelect_Create_0 = [](YYTKCodeEvent* pCodeEvent, CInstance* Self, CInstance* Other, CCode* Code, RValue* Res, int Flags) {
				CallOriginal(pCodeEvent, Self, Other, Code, Res, Flags);

				// Get random character map value
				YYRValue yyrv_characterData;
				CallBuiltin(yyrv_characterData, "variable_global_get", Self, Other, { "characterData" });
				if (config.debugEnabled) PrintMessage(CLR_AQUA, "[%s:%d] variable_global_get : characterData", GetFileName(__FILE__).c_str(), __LINE__);
				YYRValue yyrv_random;
				CallBuiltin(yyrv_random, "ds_map_find_value", Self, Other, { yyrv_characterData, "random" });
				if (config.debugEnabled) PrintMessage(CLR_AQUA, "[%s:%d] ds_map_find_value : yyrv_characterData, \"random\"", GetFileName(__FILE__).c_str(), __LINE__);

				// Add random character to end of character array
				YYRValue yyrv_byGenArray;
				CallBuiltin(yyrv_byGenArray, "variable_instance_get", Self, Other, { (long long)Self->i_id, "charListByGen" });
				if (config.debugEnabled) PrintMessage(CLR_AQUA, "[%s:%d] variable_instance_get : charListByGen", GetFileName(__FILE__).c_str(), __LINE__);
				int yyrv_lastGenIndex = yyrv_byGenArray.RefArray->length - 1;
				YYRValue yyrv_lastGenArray;
				yyrv_lastGenArray.Kind = 2;
				yyrv_lastGenArray.RefArray = yyrv_byGenArray.RefArray->m_Array[yyrv_lastGenIndex].RefArray;
				if (config.debugEnabled) PrintMessage(CLR_GRAY, "[%s:%d] array_get : yyrv_byGenArray, yyrv_lastGenIndex", GetFileName(__FILE__).c_str(), __LINE__);
				int lastGenLength = yyrv_lastGenArray.RefArray->length;
				RValue* yyrv_newArray = new RValue[lastGenLength + 1];
				for (int i = 0; i < lastGenLength; i++) {
					yyrv_newArray[i] = yyrv_lastGenArray.RefArray->m_Array[i];
				}
				yyrv_lastGenArray.RefArray->m_Array = yyrv_newArray;
				yyrv_lastGenArray.RefArray->length = lastGenLength + 1;
				yyrv_lastGenArray.RefArray->m_Array[lastGenLength] = yyrv_random;

				// Set random slot to last index of array
				totalChars = 0;
				genLengths.clear();
				int genLength = 0;
				for (int i = 0; i < yyrv_byGenArray.RefArray->length; i++) {
					genLength = yyrv_byGenArray.RefArray->m_Array[i].RefArray->length;
					totalChars += genLength;
					genLengths.push_back(genLength);
				}
				YYRValue yyrv_randomSelectSlotIndex;
				yyrv_randomSelectSlotIndex.Kind = VALUE_REAL;
				yyrv_randomSelectSlotIndex.Real = totalChars - 1;
				YYRValue yyrv_result;
				CallBuiltin(yyrv_result, "variable_instance_set", Self, Other, { (long long)Self->i_id, "randomSelectSlot", yyrv_randomSelectSlotIndex });
				if (config.debugEnabled) PrintMessage(CLR_TANGERINE, "[%s:%d] variable_instance_set : \"randomSelectSlot\", yyrv_randomSelectSlotIndex", GetFileName(__FILE__).c_str(), __LINE__);

				// Get character list
				// ["ame", "gura", "ina", ...]
				YYRValue yyrv_characterList;
				CallBuiltin(yyrv_characterList, "variable_global_get", Self, Other, { "characterList" });
				characterList.clear();
				for (int i = 0; i < totalChars; i++) {
					characterList.push_back(std::string(yyrv_characterList.RefArray->m_Array[i].String->Get()));
				}

				// Find which numbers to remove
				// [0, 6, 17, 30, ...]
				indexesToRemove.clear();
				for (int i = 0; i < config.blacklist.size(); i++) {
					auto it = std::find(characterList.begin(), characterList.end(), config.blacklist[i]);

					if (it != characterList.end()) {
						int index = std::distance(characterList.begin(), it);
						indexesToRemove.push_back(index);
						if (config.debugEnabled) PrintMessage(CLR_BRIGHTPURPLE, "\"%s\" found at %d", config.blacklist[i], index);
					} else {
						PrintError(__FILE__, __LINE__, "\"%s\" not found!", config.blacklist[i]);
					}
				}
				SetBlacklist(Self, Other);
			};
			CharSelect_Create_0(pCodeEvent, Self, Other, Code, Res, Flags);
			codeFuncTable[Code->i_CodeIndex] = CharSelect_Create_0;
		}
		else if (_strcmpi(Code->i_pName, "gml_Object_obj_CharSelect_Draw_0") == 0) {	// 640x360 viewport size
			auto CharSelect_Draw_0 = [](YYTKCodeEvent* pCodeEvent, CInstance* Self, CInstance* Other, CCode* Code, RValue* Res, int Flags) {
				CallOriginal(pCodeEvent, Self, Other, Code, Res, Flags);
				int marginW = 85;
				YYRValue yyrv_result;
				

				if (blacklistOpen) {
					// Blacklist Header
					CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)16777215 }); // white
					CallBuiltin(yyrv_result, "draw_rectangle", Self, Other, { (long long)(0 + marginW), (long long)(10), (long long)(640 - marginW), (long long)(33), false });
					// Blacklist Header Text
					if (blacklistErrorTimer > 0) {
						blacklistErrorTimer--;
						CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)255 }); // red
						CallBuiltin(yyrv_result, "draw_set_alpha", Self, Other, { (double)1.00 });
						CallBuiltin(yyrv_result, "draw_text", Self, Other, { (long long)320, (long long)17, "ERROR: CAN'T BLACKLIST ALL CHARACTERS!" });
					} else {
						CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)16367178 }); // blue 4abef9
						CallBuiltin(yyrv_result, "draw_text", Self, Other, { (long long)320, (long long)17, "EDITING BLACKLIST..." });
					}
					// Blacklist Body
					CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)0 });
					CallBuiltin(yyrv_result, "draw_set_alpha", Self, Other, { (double)0.25 });
					CallBuiltin(yyrv_result, "draw_rectangle", Self, Other, { (long long)(0 + marginW), (long long)(0 + 34), (long long)(640 - marginW), (long long)(360 - 150), false });
					// Blacklist Body Outline
					CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)16777215 }); // white
					CallBuiltin(yyrv_result, "draw_set_alpha", Self, Other, { (double)1.00 });
					CallBuiltin(yyrv_result, "draw_rectangle", Self, Other, { (long long)(0 + marginW), (long long)(10), (long long)(640 - marginW), (long long)(360 - 150), true });

					// If current location is a char to be removed, draw sprite at location
					CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)255 }); // red
					CallBuiltin(yyrv_result, "draw_set_alpha", Self, Other, { (double)0.50 });
					int charPortHeight = 38;
					int vertSpacing = 4;
					int charPortY = 40;
					int statX = 110;
					int statY = 25;
					int spacingY = 19;
					int charX = 180;
					int overallIndex = 0;
					for (int i = 0; i < genLengths.size(); i++) {
						for (int j = 0; j < genLengths[i]; j++) {
							auto it = std::find(indexesToRemove.begin(), indexesToRemove.end(), overallIndex);
							if (it != indexesToRemove.end()) {
								int pushInX = ((6 - genLengths[i]) * 23);
								CallBuiltin(yyrv_result, "draw_rectangle", Self, Other, {
									(long long)(((charX + (j * 46)) + 3) + pushInX),
									(long long)(charPortY + (i * (charPortHeight + vertSpacing))),
									(long long)(((charX + pushInX) + (j * 46)) + 45),
									(long long)((charPortY + (i * (charPortHeight + vertSpacing))) + (charPortHeight - 1)),
									false
								});
							}
							overallIndex++;
						}
					}
					
				}

				if (inCharSelect == true) {
					// Blacklist Button
					CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)0 });
					CallBuiltin(yyrv_result, "draw_set_alpha", Self, Other, { (double)0.25 });
					CallBuiltin(yyrv_result, "draw_button", Self, Other, { (long long)484, (long long)11, (long long)548, (long long)32, !blacklistSelected });
					// Blacklist Button Text
					CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)16777215 }); // white
					CallBuiltin(yyrv_result, "draw_set_alpha", Self, Other, { (double)1.00 });
					CallBuiltin(yyrv_result, "draw_text", Self, Other, { (long long)517, (long long)17, "BLACKLIST" });
				}

				CallBuiltin(yyrv_result, "draw_set_color", Self, Other, { (long long)16777215 }); // white
				CallBuiltin(yyrv_result, "draw_set_alpha", Self, Other, { (double)1.00 });
			};
			CharSelect_Draw_0(pCodeEvent, Self, Other, Code, Res, Flags);
			codeFuncTable[Code->i_CodeIndex] = CharSelect_Draw_0;
		}
		else
		{
			auto UnmodifiedFunc = [](YYTKCodeEvent* pCodeEvent, CInstance* Self, CInstance* Other, CCode* Code, RValue* Res, int Flags) {
				CallOriginal(pCodeEvent, Self, Other, Code, Res, Flags);
			};
			UnmodifiedFunc(pCodeEvent, Self, Other, Code, Res, Flags);
			codeFuncTable[Code->i_CodeIndex] = UnmodifiedFunc;
		}
	}
	// Tell the core the handler was successful.
	return YYTK_OK;
}

// Create an entry routine - it must be named exactly this, and must accept these exact arguments.
// It must also be declared DllExport (notice how the other functions are not).
DllExport YYTKStatus PluginEntry(YYTKPlugin* PluginObject) {
	// Set the unload routine
	PluginObject->PluginUnload = PluginUnload;

	// Print a message to the console
	PrintMessage(CLR_DEFAULT, "[%s v%d.%d.%d] - Hello from PluginEntry!", mod.name, mod.version.major, mod.version.minor, mod.version.build);

	PluginAttributes_t* PluginAttributes = nullptr;

	// Get the attributes for the plugin - this is an opaque structure, as it may change without any warning.
	// If Status == YYTK_OK (0), then PluginAttributes is guaranteed to be valid (non-null).
	if (YYTKStatus Status = PmGetPluginAttributes(PluginObject, PluginAttributes)) {
		PrintError(__FILE__, __LINE__, "[%s v%d.%d.%d] - PmGetPluginAttributes failed with 0x%x", mod.name, mod.version.major, mod.version.minor, mod.version.build, Status);
		return YYTK_FAIL;
	}

	// Register a callback for frame events
	YYTKStatus Status = PmCreateCallback(
		PluginAttributes,					// Plugin Attributes
		g_pFrameCallbackAttributes,				// (out) Callback Attributes
		FrameCallback,						// The function to register as a callback
		static_cast<EventType>(EVT_PRESENT | EVT_ENDSCENE), // Which events trigger this callback
		nullptr								// The optional argument to pass to the function
	);

	if (Status) {
		PrintError(__FILE__, __LINE__, "[%s v%d.%d.%d] - PmCreateCallback failed with 0x%x", mod.name, mod.version.major, mod.version.minor, mod.version.build, Status);
		return YYTK_FAIL;
	}

	// Register a callback for frame events
	Status = PmCreateCallback(
		PluginAttributes,					// Plugin Attributes
		g_pCodeCallbackAttributes,			// (out) Callback Attributes
		CodeCallback,						// The function to register as a callback
		static_cast<EventType>(EVT_CODE_EXECUTE), // Which events trigger this callback
		nullptr								// The optional argument to pass to the function
	);

	if (Status) {
		PrintError(__FILE__, __LINE__, "[%s v%d.%d.%d] - PmCreateCallback failed with 0x%x", mod.name, mod.version.major, mod.version.minor, mod.version.build, Status);
		return YYTK_FAIL;
	}

	if (HAS_CONFIG == true) {
		// Load mod config file or create one if there isn't one already.
		const wchar_t* dirName = L"modconfigs";

		if (GetFileAttributes(dirName) == INVALID_FILE_ATTRIBUTES) {
			if (CreateDirectory(dirName, NULL)) {
				PrintMessage(CLR_GREEN, "[%s v%d.%d.%d] - Directory \"modconfigs\" created!", mod.name, mod.version.major, mod.version.minor, mod.version.build);
			} else {
				PrintError(__FILE__, __LINE__, "Failed to create the modconfigs directory. Error code: %lu", GetLastError());
				return YYTK_FAIL;
			}
		}

		std::string fileName = formatString(std::string(mod.name)) + "-config.json";
		std::ifstream configFile("modconfigs/" + fileName);
		json data;
		if (configFile.is_open() == false) {	// no config file
			GenerateConfig(fileName);
		} else {
			try {
				data = json::parse(configFile);
			} catch (json::parse_error& e) {
				PrintError(__FILE__, __LINE__, "Message: %s\nException ID: %d\nByte Position of Error: %u", e.what(), e.id, (unsigned)e.byte);
				return YYTK_FAIL;
			}

			config = data.template get<Config>();
		}
		PrintMessage(CLR_GREEN, "[%s v%d.%d.%d] - %s loaded successfully!", mod.name, mod.version.major, mod.version.minor, mod.version.build, fileName.c_str());
	}

	// Function hooks
	GetFunctionByName("asset_get_index", assetGetIndexFunc);
	MH_Initialize();
	MmGetScriptData(scriptList);
	
	HookScriptFunction("gml_Script_Up_gml_Object_obj_CharSelect_Create_0", (void*)&UpCharSelectFuncDetour, (void**)&origUpCharSelectScript);
	HookScriptFunction("gml_Script_Down_gml_Object_obj_CharSelect_Create_0", (void*)&DownCharSelectFuncDetour, (void**)&origDownCharSelectScript);
	HookScriptFunction("gml_Script_Left_gml_Object_obj_CharSelect_Create_0", (void*)&LeftCharSelectFuncDetour, (void**)&origLeftCharSelectScript);
	HookScriptFunction("gml_Script_Right_gml_Object_obj_CharSelect_Create_0", (void*)&RightCharSelectFuncDetour, (void**)&origRightCharSelectScript);
	HookScriptFunction("gml_Script_Select_gml_Object_obj_CharSelect_Create_0", (void*)&SelectCharSelectFuncDetour, (void**)&origSelectCharSelectScript);
	HookScriptFunction("gml_Script_Return_gml_Object_obj_CharSelect_Create_0", (void*)&ReturnCharSelectFuncDetour, (void**)&origReturnCharSelectScript);

	// Off it goes to the core.
	return YYTK_OK;
}

// The routine that gets called on plugin unload.
// Registered in PluginEntry - you should use this to release resources.
YYTKStatus PluginUnload() {
	YYTKStatus Removal = PmRemoveCallback(g_pFrameCallbackAttributes);

	// If we didn't succeed in removing the callback.
	if (Removal != YYTK_OK) {
		PrintError(__FILE__, __LINE__, "[%s v%d.%d.%d] PmRemoveCallback failed with 0x%x", mod.name, mod.version.major, mod.version.minor, mod.version.build, Removal);
	}

	Removal = PmRemoveCallback(g_pCodeCallbackAttributes);

	// If we didn't succeed in removing the callback.
	if (Removal != YYTK_OK) {
		PrintError(__FILE__, __LINE__, "[%s v%d.%d.%d] PmRemoveCallback failed with 0x%x", mod.name, mod.version.major, mod.version.minor, mod.version.build, Removal);
	}

	PrintMessage(CLR_DEFAULT, "[%s v%d.%d.%d] - Goodbye!", mod.name, mod.version.major, mod.version.minor, mod.version.build);

	return YYTK_OK;
}

// Boilerplate setup for a Windows DLL, can just return TRUE.
// This has to be here or else you get linker errors (unless you disable the main method)
BOOL APIENTRY DllMain(
	HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
) {
	return 1;
}