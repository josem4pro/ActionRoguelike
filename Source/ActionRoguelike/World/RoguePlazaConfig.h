// RoguePlazaConfig.h
// Configuration structures for parametric plaza generation
// Part of SP-N1-V2-003 Fase 3R: Plaza Config & Asset Swap

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RoguePlazaConfig.generated.h"

class USkeletalMesh;
class UAnimInstance;

/**
 * Layout configuration for the plaza
 * Defines physical dimensions and actor quantities
 */
USTRUCT(BlueprintType)
struct ACTIONROGUELIKE_API FPlazaLayoutConfig
{
	GENERATED_BODY()

	// --- Dimensions ---

	/** Width of the plaza floor in centimeters (X axis) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions", meta = (ClampMin = "1000", ClampMax = "50000"))
	float PlazaWidthCm = 4000.f;

	/** Height/Depth of the plaza floor in centimeters (Y axis) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions", meta = (ClampMin = "1000", ClampMax = "50000"))
	float PlazaHeightCm = 4000.f;

	/** Height of surrounding walls in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dimensions", meta = (ClampMin = "100", ClampMax = "1000"))
	float WallHeightCm = 300.f;

	// --- Actor Quantities ---

	/** Number of target dummies to spawn/ensure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actors", meta = (ClampMin = "0", ClampMax = "20"))
	int32 NumTargetDummies = 3;

	/** Number of explosive barrels to spawn/ensure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actors", meta = (ClampMin = "0", ClampMax = "20"))
	int32 NumExplosiveBarrels = 2;

	// --- Spawn Layout ---

	/** Minimum distance between spawned actors in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (ClampMin = "100", ClampMax = "1000"))
	float MinActorSpacing = 200.f;

	/** Safe zone around PlayerStart where no hazards spawn (centimeters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout", meta = (ClampMin = "0", ClampMax = "2000"))
	float PlayerStartSafeZone = 500.f;

	// --- Helpers ---

	/** Get plaza width in meters */
	float GetWidthMeters() const { return PlazaWidthCm / 100.f; }

	/** Get plaza height in meters */
	float GetHeightMeters() const { return PlazaHeightCm / 100.f; }

	/** Get wall height in meters */
	float GetWallHeightMeters() const { return WallHeightCm / 100.f; }
};

/**
 * Visual configuration for asset swapping
 * Defines which meshes and materials to use for key actors
 */
USTRUCT(BlueprintType)
struct ACTIONROGUELIKE_API FPlazaVisualConfig
{
	GENERATED_BODY()

	// --- Player Visuals ---

	/** Override skeletal mesh for player character */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TSoftObjectPtr<USkeletalMesh> PlayerSkeletalMesh;

	/** Override animation class for player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TSoftClassPtr<UAnimInstance> PlayerAnimClass;

	// --- Target Dummy Visuals ---

	/** Override skeletal mesh for target dummies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetDummy")
	TSoftObjectPtr<USkeletalMesh> DummySkeletalMesh;

	/** Override animation class for dummies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TargetDummy")
	TSoftClassPtr<UAnimInstance> DummyAnimClass;

	// --- Barrel Visuals ---

	/** Override static mesh for explosive barrels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barrel")
	TSoftObjectPtr<UStaticMesh> BarrelStaticMesh;

	// --- Material Overrides ---

	/** Override material for floor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> FloorMaterial;

	/** Override material for walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TSoftObjectPtr<UMaterialInterface> WallMaterial;

	// --- Helper Methods ---

	/** Check if player visual override is set */
	bool HasPlayerVisualOverride() const { return !PlayerSkeletalMesh.IsNull(); }

	/** Check if dummy visual override is set */
	bool HasDummyVisualOverride() const { return !DummySkeletalMesh.IsNull(); }

	/** Check if barrel visual override is set */
	bool HasBarrelVisualOverride() const { return !BarrelStaticMesh.IsNull(); }
};

/**
 * Complete plaza configuration combining layout and visuals
 */
USTRUCT(BlueprintType)
struct ACTIONROGUELIKE_API FPlazaConfig
{
	GENERATED_BODY()

	/** Layout configuration (dimensions, quantities) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FPlazaLayoutConfig Layout;

	/** Visual configuration (asset swapping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FPlazaVisualConfig Visuals;
};

/**
 * Data Asset for storing plaza configurations
 * Can be created in editor and referenced by GameMode or Level Blueprints
 */
UCLASS(BlueprintType)
class ACTIONROGUELIKE_API URoguePlazaConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The plaza configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FPlazaConfig Config;

	/** Human-readable description of this configuration */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metadata")
	FString Description;

	/** Version identifier for tracking changes */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Metadata")
	FString Version = TEXT("1.0");
};
