#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "plugin_sdk/vstsdk2.4/public.sdk/source/vst2.x/audioeffectx.h"

typedef AEffect* (*PluginEntryProc) (audioMasterCallback audioMaster);
void* hInstance;

HINSTANCE GetMyModuleHandle()
{
	MEMORY_BASIC_INFORMATION mbi;
	VirtualQuery(GetMyModuleHandle, &mbi, sizeof(mbi));
	return (HINSTANCE)(mbi.AllocationBase);
}

struct PluginLoader
{
	void* module;

	PluginLoader() : module(0)
	{
	}

	~PluginLoader()
	{
		if (module)
		{
			// Sloppy, no cleanup after loading
			//FreeLibrary((HMODULE)module);
		}
	}

	bool loadLibrary(const char* fileName)
	{
		module = LoadLibrary(fileName);
		return module != 0;
	}

	PluginEntryProc getMainEntry()
	{
		PluginEntryProc mainProc = 0;
		mainProc = (PluginEntryProc)GetProcAddress((HMODULE)module, "VSTPluginMain");
		if (!mainProc)
			mainProc = (PluginEntryProc)GetProcAddress((HMODULE)module, "main");
		return mainProc;
	}
};

//-------------------------------------------------------------------------------------------------------
audioMasterCallback audioMasterOrig = nullptr;

std::vector<VstInt32> ignoredOpcodes = { 7, 37 };

const char* opToName (VstInt32 op)
{
    if (op == audioMasterAutomate) return "audioMasterAutomate";
    if (op == audioMasterVersion) return "audioMasterVersion";
    if (op == audioMasterCurrentId) return "audioMasterCurrentId";
    if (op == audioMasterIdle) return "audioMasterIdle";
    if (op == audioMasterPinConnected) return "audioMasterPinConnected";
    if (op == audioMasterWantMidi) return "audioMasterWantMidi";
    if (op == audioMasterGetTime) return "audioMasterGetTime";
    if (op == audioMasterProcessEvents) return "audioMasterProcessEvents";
    if (op == audioMasterSetTime) return "audioMasterSetTime";
    if (op == audioMasterTempoAt) return "audioMasterTempoAt";
    if (op == audioMasterGetNumAutomatableParameters) return "audioMasterGetNumAutomatableParameters";
    if (op == audioMasterGetParameterQuantization) return "audioMasterGetParameterQuantization";
    if (op == audioMasterIOChanged) return "audioMasterIOChanged";
    if (op == audioMasterNeedIdle) return "audioMasterNeedIdle";
    if (op == audioMasterSizeWindow) return "audioMasterSizeWindow";
    if (op == audioMasterGetSampleRate) return "audioMasterGetSampleRate";
    if (op == audioMasterGetBlockSize) return "audioMasterGetBlockSize";
    if (op == audioMasterGetInputLatency) return "audioMasterGetInputLatency";
    if (op == audioMasterGetOutputLatency) return "audioMasterGetOutputLatency";
    if (op == audioMasterGetPreviousPlug) return "audioMasterGetPreviousPlug";
    if (op == audioMasterGetNextPlug) return "audioMasterGetNextPlug";
    if (op == audioMasterWillReplaceOrAccumulate) return "audioMasterWillReplaceOrAccumulate";
    if (op == audioMasterGetCurrentProcessLevel) return "audioMasterGetCurrentProcessLevel";
    if (op == audioMasterGetAutomationState) return "audioMasterGetAutomationState";
    if (op == audioMasterOfflineStart) return "audioMasterOfflineStart";
    if (op == audioMasterOfflineRead) return "audioMasterOfflineRead";
    if (op == audioMasterOfflineWrite) return "audioMasterOfflineWrite";
    if (op == audioMasterOfflineGetCurrentPass) return "audioMasterOfflineGetCurrentPass";
    if (op == audioMasterOfflineGetCurrentMetaPass) return "audioMasterOfflineGetCurrentMetaPass";
    if (op == audioMasterSetOutputSampleRate) return "audioMasterSetOutputSampleRate";
    if (op == audioMasterGetOutputSpeakerArrangement) return "audioMasterGetOutputSpeakerArrangement";
    if (op == audioMasterGetVendorString) return "audioMasterGetVendorString";
    if (op == audioMasterGetProductString) return "audioMasterGetProductString";
    if (op == audioMasterGetVendorVersion) return "audioMasterGetVendorVersion";
    if (op == audioMasterVendorSpecific) return "audioMasterVendorSpecific";
    if (op == audioMasterSetIcon) return "audioMasterSetIcon";
    if (op == audioMasterCanDo) return "audioMasterCanDo";
    if (op == audioMasterGetLanguage) return "audioMasterGetLanguage";
    if (op == audioMasterOpenWindow) return "audioMasterOpenWindow";
    if (op == audioMasterCloseWindow) return "audioMasterCloseWindow";
    if (op == audioMasterGetDirectory) return "audioMasterGetDirectory";
    if (op == audioMasterUpdateDisplay) return "audioMasterUpdateDisplay";
    if (op == audioMasterBeginEdit) return "audioMasterBeginEdit";
    if (op == audioMasterEndEdit) return "audioMasterEndEdit";
    if (op == audioMasterOpenFileSelector) return "audioMasterOpenFileSelector";
    if (op == audioMasterCloseFileSelector) return "audioMasterCloseFileSelector";
    if (op == audioMasterEditFile) return "audioMasterEditFile";
    if (op == audioMasterGetChunkFile) return "audioMasterGetChunkFile";
    if (op == audioMasterGetInputSpeakerArrangement) return "audioMasterGetInputSpeakerArrangement";

    return "";
}

VstIntPtr audioMasterFake (AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt)
{
    if (audioMasterOrig)
    {
        auto itr = std::find (ignoredOpcodes.begin(), ignoredOpcodes.end(), opcode);
        if (itr == ignoredOpcodes.end())
        {
            char buff[1024];
            sprintf (buff, "opcode: %d (%s) index: %d intptr: %p, ptr: %p, opt %f\n", opcode, opToName(opcode), index, value, ptr, opt);
            OutputDebugStringA (buff);
        }
        return audioMasterOrig (effect, opcode, index, value, ptr, opt);
    }
    return 0;
}
//-------------------------------------------------------------------------------------------------------
extern "C"
{
	__declspec(dllexport) AEffect* VSTPluginMain(audioMasterCallback audioMaster)
	{
        audioMasterOrig = audioMaster;

		char dllName[512];
		char underlyingDllName[512];
		char vstIdIntString[16];

		HINSTANCE thismodule = GetMyModuleHandle();
		GetModuleFileName(thismodule, dllName, 512);

		int pluginNameLen = int (strlen(dllName) - 24);
		
		// determine underlying dll name
		strncpy(underlyingDllName, dllName, pluginNameLen);
		strcpy(underlyingDllName + pluginNameLen, "dll");
		const char* fullFilePath = underlyingDllName;

		// determine the vstid from the name
		strncpy(vstIdIntString, dllName + pluginNameLen + 10, 10);
		vstIdIntString[10] = 0;
		const int vstId = atoi(vstIdIntString);

		PluginLoader loader;
		if (!loader.loadLibrary(fullFilePath))
		{
			printf("Failed to load VST Plugin library!\n");
			return 0;
		}

		PluginEntryProc mainEntry = loader.getMainEntry();
		if (!mainEntry)
		{
			printf("VST Plugin main entry not found!\n");
			return 0;
		}

		printf("Create effect...\n");
		AEffect* effect = mainEntry(audioMasterFake);
		if (!effect)
		{
			printf("Failed to create effect instance!\n");
			return 0;
		}

		unsigned int uid = effect->uniqueID;
		effect->uniqueID = vstId;
		return effect;
	}

}

extern "C" 
{
	BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
	{
		hInstance = hInst;
		return 1;
	}
}
