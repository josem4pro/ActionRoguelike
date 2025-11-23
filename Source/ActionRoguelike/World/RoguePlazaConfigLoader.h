// RoguePlazaConfigLoader.h
// JSON-based configuration loader for Plaza Playground
// Part of SP-N1-V2-004 Fase 4R: Plaza Playground Config Runner

#pragma once

#include "CoreMinimal.h"
#include "World/RoguePlazaConfig.h"

/**
 * Static utility class for loading plaza configuration from JSON files
 *
 * This class allows non-programmers to configure the plaza by editing
 * simple JSON files instead of touching C++ code or the Unreal Editor.
 *
 * Usage:
 *   FPlazaConfig Config;
 *   if (FRoguePlazaConfigLoader::LoadPresetFromFile("/path/to/preset.json", Config))
 *   {
 *       // Use Config with ARoguePlazaManager
 *   }
 *
 * JSON Format (PlazaPreset):
 *   {
 *     "name": "My Plaza",
 *     "layout": {
 *       "width_m": 40,
 *       "height_m": 40,
 *       "wall_height_m": 3,
 *       "num_target_dummies": 10,
 *       "num_explosive_barrels": 3
 *     },
 *     "visuals": {
 *       "player_skin": "Default",
 *       "theme": "daylight"
 *     }
 *   }
 */
class ACTIONROGUELIKE_API FRoguePlazaConfigLoader
{
public:
	/**
	 * Load a plaza preset from a JSON file
	 * @param FilePath Absolute path to the JSON file
	 * @param OutConfig Filled with the loaded configuration
	 * @return true if load succeeded, false otherwise
	 */
	static bool LoadPresetFromFile(const FString& FilePath, FPlazaConfig& OutConfig);

	/**
	 * Load a plaza preset from a JSON string
	 * @param JsonString JSON content as string
	 * @param OutConfig Filled with the loaded configuration
	 * @return true if parse succeeded, false otherwise
	 */
	static bool LoadPresetFromString(const FString& JsonString, FPlazaConfig& OutConfig);

	/**
	 * Resolve the preset path from command line arguments or use default
	 * Checks for: -PlazaPreset=/path/to/preset.json
	 * @return Path to preset file (empty if not specified and no default found)
	 */
	static FString ResolvePresetPathFromCmdLine();

	/**
	 * Get the default preset path
	 * @return Path to default preset in the Presets folder
	 */
	static FString GetDefaultPresetPath();

	/**
	 * List all available presets in the Presets folder
	 * @return Array of preset file paths
	 */
	static TArray<FString> GetAvailablePresets();

	/**
	 * Save a configuration to a JSON file
	 * @param FilePath Absolute path to write to
	 * @param Config Configuration to save
	 * @param PresetName Human-readable name for the preset
	 * @param Description Optional description
	 * @return true if save succeeded
	 */
	static bool SavePresetToFile(const FString& FilePath, const FPlazaConfig& Config,
		const FString& PresetName, const FString& Description = TEXT(""));

private:
	/** Parse layout section from JSON object */
	static bool ParseLayoutConfig(const TSharedPtr<class FJsonObject>& JsonObject, FPlazaLayoutConfig& OutLayout);

	/** Parse visuals section from JSON object */
	static bool ParseVisualConfig(const TSharedPtr<class FJsonObject>& JsonObject, FPlazaVisualConfig& OutVisuals);

	/** Get the base path for presets (relative to project) */
	static FString GetPresetsBasePath();
};
