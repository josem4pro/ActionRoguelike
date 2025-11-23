// RoguePlazaIntrospector.h
// Introspection tool for discovering and documenting plaza world contents
// Part of SP-N1-V2-003 Fase 3R: Plaza Config & Asset Swap

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoguePlazaIntrospector.generated.h"

/**
 * Data structure for storing information about discovered actors
 */
USTRUCT(BlueprintType)
struct FPlazaActorInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString ActorName;

	UPROPERTY(BlueprintReadOnly)
	FString ClassName;

	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	UPROPERTY(BlueprintReadOnly)
	FRotator Rotation;

	UPROPERTY(BlueprintReadOnly)
	FVector Scale;

	UPROPERTY(BlueprintReadOnly)
	FString MeshAssetPath;

	UPROPERTY(BlueprintReadOnly)
	FString MaterialAssetPath;

	UPROPERTY(BlueprintReadOnly)
	FString SkeletonAssetPath;

	UPROPERTY(BlueprintReadOnly)
	FString Notes;
};

/**
 * Summary of the plaza layout dimensions and actor counts
 */
USTRUCT(BlueprintType)
struct FPlazaLayoutSummary
{
	GENERATED_BODY()

	// Dimensions calculated from floor bounds
	UPROPERTY(BlueprintReadOnly)
	float PlazaWidthCm = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float PlazaHeightCm = 0.f;

	// Actor counts
	UPROPERTY(BlueprintReadOnly)
	int32 NumTargetDummies = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NumExplosiveBarrels = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NumPlayerStarts = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NumStaticMeshActors = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NumLights = 0;

	// Bounds
	UPROPERTY(BlueprintReadOnly)
	FVector MinBounds;

	UPROPERTY(BlueprintReadOnly)
	FVector MaxBounds;
};

/**
 * Actor that performs introspection of the plaza map and outputs
 * detailed information about all actors and layout configuration.
 *
 * Can be spawned in BeginPlay or triggered via console command.
 * Outputs to log and optionally to a file.
 */
UCLASS()
class ACTIONROGUELIKE_API ARoguePlazaIntrospector : public AActor
{
	GENERATED_BODY()

public:
	ARoguePlazaIntrospector();

	virtual void BeginPlay() override;

	/**
	 * Perform full introspection of the current world.
	 * @param bLogToConsole If true, outputs results to UE_LOG
	 * @return Summary of discovered layout
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Introspection")
	FPlazaLayoutSummary PerformIntrospection(bool bLogToConsole = true);

	/**
	 * Get all discovered actor information
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Introspection")
	TArray<FPlazaActorInfo> GetDiscoveredActors() const { return DiscoveredActors; }

	/**
	 * Log detailed info about a specific actor class
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Introspection")
	void LogActorsByClass(TSubclassOf<AActor> ActorClass);

	/**
	 * Generate a formatted report string suitable for documentation
	 */
	UFUNCTION(BlueprintCallable, Category = "Plaza|Introspection")
	FString GenerateMarkdownReport() const;

protected:
	// If true, automatically runs introspection on BeginPlay
	UPROPERTY(EditAnywhere, Category = "Plaza|Introspection")
	bool bAutoRunOnBeginPlay = false;

	// Store discovered actors
	UPROPERTY(BlueprintReadOnly, Category = "Plaza|Introspection")
	TArray<FPlazaActorInfo> DiscoveredActors;

	// Store the layout summary
	UPROPERTY(BlueprintReadOnly, Category = "Plaza|Introspection")
	FPlazaLayoutSummary LayoutSummary;

	// Helper to extract mesh path from actor
	FString GetMeshAssetPath(AActor* Actor) const;

	// Helper to extract skeleton path from skeletal mesh component
	FString GetSkeletonPath(AActor* Actor) const;

	// Helper to classify an actor and add notes
	FString ClassifyActor(AActor* Actor) const;
};
