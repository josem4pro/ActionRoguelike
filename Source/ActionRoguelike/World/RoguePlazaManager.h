// RoguePlazaManager.h
// Manager for applying plaza configuration and asset swapping
// Part of SP-N1-V2-003 Fase 3R: Plaza Config & Asset Swap

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/RoguePlazaConfig.h"
#include "RoguePlazaManager.generated.h"

class ARogueTargetDummy;
class ARogueExplosiveBarrel;
class ARoguePlayerCharacter;
class URoguePlazaConfigAsset;

DECLARE_LOG_CATEGORY_EXTERN(LogPlazaManager, Log, All);

/**
 * Result of applying plaza configuration
 */
USTRUCT(BlueprintType)
struct FPlazaConfigResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	UPROPERTY(BlueprintReadOnly)
	int32 DummiesSpawned = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 BarrelsSpawned = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 VisualsApplied = 0;

	UPROPERTY(BlueprintReadOnly)
	FString ErrorMessage;
};

/**
 * Manager actor that handles plaza configuration application
 *
 * Place in level or spawn from GameMode to:
 * - Enforce actor counts based on FPlazaLayoutConfig
 * - Apply visual overrides via FPlazaVisualConfig
 * - Log current configuration state
 *
 * Usage:
 * 1. Spawn ARoguePlazaManager in level or from GameMode
 * 2. Set PlazaConfig (via property or SetPlazaConfig())
 * 3. Call ApplyConfiguration() or let it auto-apply on BeginPlay
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePlazaManager : public AActor
{
	GENERATED_BODY()

public:
	ARoguePlazaManager();

	virtual void BeginPlay() override;

	// --- Configuration ---

	/**
	 * Set the plaza configuration to apply
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Config")
	void SetPlazaConfig(const FPlazaConfig& InConfig);

	/**
	 * Set configuration from a data asset
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Config")
	void SetPlazaConfigFromAsset(URoguePlazaConfigAsset* ConfigAsset);

	/**
	 * Get current plaza configuration
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Config")
	const FPlazaConfig& GetPlazaConfig() const { return PlazaConfig; }

	// --- Application ---

	/**
	 * Apply the current configuration to the world
	 * Spawns missing actors and applies visual overrides
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Config")
	FPlazaConfigResult ApplyConfiguration();

	/**
	 * Apply only the layout configuration (actor counts)
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Config")
	FPlazaConfigResult ApplyLayoutConfig();

	/**
	 * Apply only the visual configuration (asset swaps)
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Config")
	FPlazaConfigResult ApplyVisualConfig();

	// --- Asset Swap ---

	/**
	 * Apply visual configuration to a specific player character
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|AssetSwap")
	bool ApplyPlayerVisualConfig(ARoguePlayerCharacter* Player);

	/**
	 * Apply visual configuration to a specific target dummy
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|AssetSwap")
	bool ApplyDummyVisualConfig(ARogueTargetDummy* Dummy);

	/**
	 * Apply visual configuration to a specific barrel
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|AssetSwap")
	bool ApplyBarrelVisualConfig(ARogueExplosiveBarrel* Barrel);

	// --- Utility ---

	/**
	 * Count current actors of each type in the world
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Utility")
	void GetCurrentActorCounts(int32& OutDummies, int32& OutBarrels) const;

	/**
	 * Log current configuration to console
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Debug")
	void LogCurrentConfig() const;

	/**
	 * Calculate spawn positions for new actors based on layout config
	 * @param NumPositions Number of positions to calculate
	 * @return Array of valid spawn positions
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Utility")
	TArray<FVector> CalculateSpawnPositions(int32 NumPositions) const;

protected:
	// --- Properties ---

	/** If true, automatically apply configuration on BeginPlay */
	UPROPERTY(EditAnywhere, Category = "Plaza|Config")
	bool bAutoApplyOnBeginPlay = true;

	/** If set, use this data asset for configuration instead of inline config */
	UPROPERTY(EditAnywhere, Category = "Plaza|Config")
	TObjectPtr<URoguePlazaConfigAsset> ConfigAsset;

	/** Inline configuration (used if ConfigAsset is null) */
	UPROPERTY(EditAnywhere, Category = "Plaza|Config", meta = (EditCondition = "ConfigAsset == nullptr"))
	FPlazaConfig PlazaConfig;

	/** Class to spawn for target dummies */
	UPROPERTY(EditAnywhere, Category = "Plaza|Spawning")
	TSubclassOf<ARogueTargetDummy> TargetDummyClass;

	/** Class to spawn for explosive barrels */
	UPROPERTY(EditAnywhere, Category = "Plaza|Spawning")
	TSubclassOf<ARogueExplosiveBarrel> ExplosiveBarrelClass;

	// --- Internal Methods ---

	/** Spawn missing dummies to match config */
	int32 EnsureDummyCount();

	/** Spawn missing barrels to match config */
	int32 EnsureBarrelCount();

	/** Find PlayerStart location for safe zone calculation */
	FVector GetPlayerStartLocation() const;

	/** Check if a position is within the safe zone */
	bool IsInSafeZone(const FVector& Position, const FVector& PlayerStartLoc) const;

	/** Get all target dummies in world */
	TArray<ARogueTargetDummy*> GetAllDummies() const;

	/** Get all explosive barrels in world */
	TArray<ARogueExplosiveBarrel*> GetAllBarrels() const;
};
