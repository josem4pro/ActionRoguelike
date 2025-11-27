// RoguePlazaConfigLoader.cpp
// Implementation of JSON-based configuration loader
// Part of SP-N1-V2-004 Fase 4R: Plaza Playground Config Runner

#include "World/RoguePlazaConfigLoader.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlazaConfigLoader, Log, All);

FString FRoguePlazaConfigLoader::GetPresetsBasePath()
{
	// Presets are stored relative to project root in:
	// Niveles/Nivel-1-La-Plaza/2.0-version/Presets/
	return FPaths::Combine(FPaths::ProjectDir(), TEXT("Niveles/Nivel-1-La-Plaza/2.0-version/Presets"));
}

FString FRoguePlazaConfigLoader::GetDefaultPresetPath()
{
	return FPaths::Combine(GetPresetsBasePath(), TEXT("PlazaPreset_Default.json"));
}

FString FRoguePlazaConfigLoader::ResolvePresetPathFromCmdLine()
{
	FString PresetPath;

	// Check for command line argument: -PlazaPreset=/path/to/preset.json
	if (FParse::Value(FCommandLine::Get(), TEXT("-PlazaPreset="), PresetPath))
	{
		UE_LOG(LogPlazaConfigLoader, Log, TEXT("Preset path from command line: %s"), *PresetPath);

		// If relative path, make it relative to project dir
		if (FPaths::IsRelative(PresetPath))
		{
			PresetPath = FPaths::Combine(FPaths::ProjectDir(), PresetPath);
		}

		return PresetPath;
	}

	// Check for environment variable: PLAZA_PRESET_PATH
	FString EnvPresetPath = FPlatformMisc::GetEnvironmentVariable(TEXT("PLAZA_PRESET_PATH"));
	if (!EnvPresetPath.IsEmpty())
	{
		UE_LOG(LogPlazaConfigLoader, Log, TEXT("Preset path from environment: %s"), *EnvPresetPath);

		if (FPaths::IsRelative(EnvPresetPath))
		{
			EnvPresetPath = FPaths::Combine(FPaths::ProjectDir(), EnvPresetPath);
		}

		return EnvPresetPath;
	}

	// No explicit preset specified - return default
	FString DefaultPath = GetDefaultPresetPath();
	if (FPaths::FileExists(DefaultPath))
	{
		UE_LOG(LogPlazaConfigLoader, Log, TEXT("Using default preset: %s"), *DefaultPath);
		return DefaultPath;
	}

	UE_LOG(LogPlazaConfigLoader, Warning, TEXT("No preset path specified and default not found"));
	return FString();
}

TArray<FString> FRoguePlazaConfigLoader::GetAvailablePresets()
{
	TArray<FString> Presets;
	FString BasePath = GetPresetsBasePath();

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.FindFiles(Presets, *BasePath, TEXT(".json"));

	UE_LOG(LogPlazaConfigLoader, Log, TEXT("Found %d presets in %s"), Presets.Num(), *BasePath);

	return Presets;
}

bool FRoguePlazaConfigLoader::LoadPresetFromFile(const FString& FilePath, FPlazaConfig& OutConfig)
{
	// Validate file exists
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogPlazaConfigLoader, Error, TEXT("Preset file not found: %s"), *FilePath);
		return false;
	}

	// Read file contents
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogPlazaConfigLoader, Error, TEXT("Failed to read preset file: %s"), *FilePath);
		return false;
	}

	UE_LOG(LogPlazaConfigLoader, Log, TEXT("Loading preset from: %s"), *FilePath);

	return LoadPresetFromString(JsonString, OutConfig);
}

bool FRoguePlazaConfigLoader::LoadPresetFromString(const FString& JsonString, FPlazaConfig& OutConfig)
{
	// Parse JSON
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
	{
		UE_LOG(LogPlazaConfigLoader, Error, TEXT("Failed to parse JSON preset"));
		return false;
	}

	// Get preset name for logging
	FString PresetName;
	if (JsonObject->TryGetStringField(TEXT("name"), PresetName))
	{
		UE_LOG(LogPlazaConfigLoader, Log, TEXT("Parsing preset: %s"), *PresetName);
	}

	// Parse layout section
	const TSharedPtr<FJsonObject>* LayoutObject;
	if (JsonObject->TryGetObjectField(TEXT("layout"), LayoutObject))
	{
		if (!ParseLayoutConfig(*LayoutObject, OutConfig.Layout))
		{
			UE_LOG(LogPlazaConfigLoader, Warning, TEXT("Failed to parse layout config, using defaults"));
		}
	}
	else
	{
		UE_LOG(LogPlazaConfigLoader, Warning, TEXT("No 'layout' section in preset, using defaults"));
	}

	// Parse visuals section
	const TSharedPtr<FJsonObject>* VisualsObject;
	if (JsonObject->TryGetObjectField(TEXT("visuals"), VisualsObject))
	{
		if (!ParseVisualConfig(*VisualsObject, OutConfig.Visuals))
		{
			UE_LOG(LogPlazaConfigLoader, Warning, TEXT("Failed to parse visual config, using defaults"));
		}
	}

	UE_LOG(LogPlazaConfigLoader, Log, TEXT("Preset loaded: %.0fx%.0f m, %d dummies, %d barrels"),
		OutConfig.Layout.GetWidthMeters(),
		OutConfig.Layout.GetHeightMeters(),
		OutConfig.Layout.NumTargetDummies,
		OutConfig.Layout.NumExplosiveBarrels);

	return true;
}

bool FRoguePlazaConfigLoader::ParseLayoutConfig(const TSharedPtr<FJsonObject>& JsonObject, FPlazaLayoutConfig& OutLayout)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	// Width (in meters, convert to cm)
	double WidthM;
	if (JsonObject->TryGetNumberField(TEXT("width_m"), WidthM))
	{
		OutLayout.PlazaWidthCm = static_cast<float>(WidthM * 100.0);
	}

	// Height (in meters, convert to cm)
	double HeightM;
	if (JsonObject->TryGetNumberField(TEXT("height_m"), HeightM))
	{
		OutLayout.PlazaHeightCm = static_cast<float>(HeightM * 100.0);
	}

	// Wall height (in meters, convert to cm)
	double WallHeightM;
	if (JsonObject->TryGetNumberField(TEXT("wall_height_m"), WallHeightM))
	{
		OutLayout.WallHeightCm = static_cast<float>(WallHeightM * 100.0);
	}

	// Number of dummies
	int32 NumDummies;
	if (JsonObject->TryGetNumberField(TEXT("num_target_dummies"), NumDummies))
	{
		OutLayout.NumTargetDummies = NumDummies;
	}

	// Number of barrels
	int32 NumBarrels;
	if (JsonObject->TryGetNumberField(TEXT("num_explosive_barrels"), NumBarrels))
	{
		OutLayout.NumExplosiveBarrels = NumBarrels;
	}

	// Min actor spacing (in meters, convert to cm)
	double MinSpacingM;
	if (JsonObject->TryGetNumberField(TEXT("min_actor_spacing_m"), MinSpacingM))
	{
		OutLayout.MinActorSpacing = static_cast<float>(MinSpacingM * 100.0);
	}

	// Player safe zone (in meters, convert to cm)
	double SafeZoneM;
	if (JsonObject->TryGetNumberField(TEXT("player_safe_zone_m"), SafeZoneM))
	{
		OutLayout.PlayerStartSafeZone = static_cast<float>(SafeZoneM * 100.0);
	}

	return true;
}

bool FRoguePlazaConfigLoader::ParseVisualConfig(const TSharedPtr<FJsonObject>& JsonObject, FPlazaVisualConfig& OutVisuals)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	// Visual config currently uses string identifiers for skins
	// These will be mapped to actual asset paths in future iterations
	// For now, we log them for awareness

	FString PlayerSkin;
	if (JsonObject->TryGetStringField(TEXT("player_skin"), PlayerSkin))
	{
		UE_LOG(LogPlazaConfigLoader, Verbose, TEXT("Player skin: %s"), *PlayerSkin);
		// TODO: Map skin name to actual mesh asset path
	}

	FString DummySkin;
	if (JsonObject->TryGetStringField(TEXT("dummy_skin"), DummySkin))
	{
		UE_LOG(LogPlazaConfigLoader, Verbose, TEXT("Dummy skin: %s"), *DummySkin);
	}

	FString BarrelSkin;
	if (JsonObject->TryGetStringField(TEXT("barrel_skin"), BarrelSkin))
	{
		UE_LOG(LogPlazaConfigLoader, Verbose, TEXT("Barrel skin: %s"), *BarrelSkin);
	}

	FString Theme;
	if (JsonObject->TryGetStringField(TEXT("theme"), Theme))
	{
		UE_LOG(LogPlazaConfigLoader, Verbose, TEXT("Theme: %s"), *Theme);
	}

	return true;
}

bool FRoguePlazaConfigLoader::SavePresetToFile(const FString& FilePath, const FPlazaConfig& Config,
	const FString& PresetName, const FString& Description)
{
	// Create JSON object
	TSharedRef<FJsonObject> RootObject = MakeShared<FJsonObject>();

	RootObject->SetStringField(TEXT("name"), PresetName);
	RootObject->SetStringField(TEXT("description"), Description);
	RootObject->SetStringField(TEXT("version"), TEXT("1.0"));

	// Layout section
	TSharedRef<FJsonObject> LayoutObject = MakeShared<FJsonObject>();
	LayoutObject->SetNumberField(TEXT("width_m"), Config.Layout.GetWidthMeters());
	LayoutObject->SetNumberField(TEXT("height_m"), Config.Layout.GetHeightMeters());
	LayoutObject->SetNumberField(TEXT("wall_height_m"), Config.Layout.GetWallHeightMeters());
	LayoutObject->SetNumberField(TEXT("num_target_dummies"), Config.Layout.NumTargetDummies);
	LayoutObject->SetNumberField(TEXT("num_explosive_barrels"), Config.Layout.NumExplosiveBarrels);
	LayoutObject->SetNumberField(TEXT("min_actor_spacing_m"), Config.Layout.MinActorSpacing / 100.0);
	LayoutObject->SetNumberField(TEXT("player_safe_zone_m"), Config.Layout.PlayerStartSafeZone / 100.0);
	RootObject->SetObjectField(TEXT("layout"), LayoutObject);

	// Visuals section (placeholder)
	TSharedRef<FJsonObject> VisualsObject = MakeShared<FJsonObject>();
	VisualsObject->SetStringField(TEXT("player_skin"), TEXT("Default"));
	VisualsObject->SetStringField(TEXT("dummy_skin"), TEXT("Default"));
	VisualsObject->SetStringField(TEXT("barrel_skin"), TEXT("Default"));
	VisualsObject->SetStringField(TEXT("theme"), TEXT("daylight"));
	RootObject->SetObjectField(TEXT("visuals"), VisualsObject);

	// Serialize to string
	FString OutputString;
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString);

	if (!FJsonSerializer::Serialize(RootObject, Writer))
	{
		UE_LOG(LogPlazaConfigLoader, Error, TEXT("Failed to serialize preset to JSON"));
		return false;
	}

	// Write to file
	if (!FFileHelper::SaveStringToFile(OutputString, *FilePath))
	{
		UE_LOG(LogPlazaConfigLoader, Error, TEXT("Failed to write preset file: %s"), *FilePath);
		return false;
	}

	UE_LOG(LogPlazaConfigLoader, Log, TEXT("Saved preset to: %s"), *FilePath);
	return true;
}
