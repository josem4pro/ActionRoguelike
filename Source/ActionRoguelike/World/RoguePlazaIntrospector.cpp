// RoguePlazaIntrospector.cpp
// Implementation of plaza introspection tool
// Part of SP-N1-V2-003 Fase 3R: Plaza Config & Asset Swap

#include "World/RoguePlazaIntrospector.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/LightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "ActionSystem/RogueAttributeSet.h"
#include "World/RogueTargetDummy.h"
#include "World/RogueExplosiveBarrel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePlazaIntrospector)

DEFINE_LOG_CATEGORY_STATIC(LogPlazaIntrospector, Log, All);

ARoguePlazaIntrospector::ARoguePlazaIntrospector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARoguePlazaIntrospector::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoRunOnBeginPlay)
	{
		PerformIntrospection(true);
	}
}

FPlazaLayoutSummary ARoguePlazaIntrospector::PerformIntrospection(bool bLogToConsole)
{
	DiscoveredActors.Empty();
	LayoutSummary = FPlazaLayoutSummary();

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogPlazaIntrospector, Error, TEXT("No valid world for introspection"));
		return LayoutSummary;
	}

	// Initialize bounds with extreme values
	LayoutSummary.MinBounds = FVector(MAX_FLT, MAX_FLT, MAX_FLT);
	LayoutSummary.MaxBounds = FVector(-MAX_FLT, -MAX_FLT, -MAX_FLT);

	if (bLogToConsole)
	{
		UE_LOG(LogPlazaIntrospector, Log, TEXT(""));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("========================================"));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("PLAZA INTROSPECTION REPORT"));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Map: %s"), *World->GetMapName());
		UE_LOG(LogPlazaIntrospector, Log, TEXT("========================================"));
	}

	// Iterate through all actors in the world
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == this)
		{
			continue;
		}

		// Create actor info
		FPlazaActorInfo Info;
		Info.ActorName = Actor->GetName();
		Info.ClassName = Actor->GetClass()->GetName();
		Info.Location = Actor->GetActorLocation();
		Info.Rotation = Actor->GetActorRotation();
		Info.Scale = Actor->GetActorScale3D();
		Info.MeshAssetPath = GetMeshAssetPath(Actor);
		Info.SkeletonAssetPath = GetSkeletonPath(Actor);
		Info.Notes = ClassifyActor(Actor);

		// Update bounds based on actor location
		LayoutSummary.MinBounds = LayoutSummary.MinBounds.ComponentMin(Info.Location);
		LayoutSummary.MaxBounds = LayoutSummary.MaxBounds.ComponentMax(Info.Location);

		// Count specific actor types
		if (Actor->IsA<ARogueTargetDummy>())
		{
			LayoutSummary.NumTargetDummies++;
		}
		else if (Actor->IsA<ARogueExplosiveBarrel>())
		{
			LayoutSummary.NumExplosiveBarrels++;
		}
		else if (Actor->IsA<APlayerStart>())
		{
			LayoutSummary.NumPlayerStarts++;
		}
		else if (Actor->IsA<AStaticMeshActor>())
		{
			LayoutSummary.NumStaticMeshActors++;
		}

		// Check for lights
		if (Actor->FindComponentByClass<ULightComponent>())
		{
			LayoutSummary.NumLights++;
		}

		DiscoveredActors.Add(Info);
	}

	// Calculate plaza dimensions from bounds
	FVector BoundsSize = LayoutSummary.MaxBounds - LayoutSummary.MinBounds;
	LayoutSummary.PlazaWidthCm = BoundsSize.X;
	LayoutSummary.PlazaHeightCm = BoundsSize.Y;

	if (bLogToConsole)
	{
		UE_LOG(LogPlazaIntrospector, Log, TEXT(""));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("--- LAYOUT SUMMARY ---"));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Estimated Plaza Size: %.1f x %.1f cm (%.1f x %.1f m)"),
			LayoutSummary.PlazaWidthCm, LayoutSummary.PlazaHeightCm,
			LayoutSummary.PlazaWidthCm / 100.f, LayoutSummary.PlazaHeightCm / 100.f);
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Min Bounds: (%.1f, %.1f, %.1f)"),
			LayoutSummary.MinBounds.X, LayoutSummary.MinBounds.Y, LayoutSummary.MinBounds.Z);
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Max Bounds: (%.1f, %.1f, %.1f)"),
			LayoutSummary.MaxBounds.X, LayoutSummary.MaxBounds.Y, LayoutSummary.MaxBounds.Z);
		UE_LOG(LogPlazaIntrospector, Log, TEXT(""));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("--- ACTOR COUNTS ---"));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Player Starts: %d"), LayoutSummary.NumPlayerStarts);
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Target Dummies: %d"), LayoutSummary.NumTargetDummies);
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Explosive Barrels: %d"), LayoutSummary.NumExplosiveBarrels);
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Static Mesh Actors: %d"), LayoutSummary.NumStaticMeshActors);
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Lights: %d"), LayoutSummary.NumLights);
		UE_LOG(LogPlazaIntrospector, Log, TEXT("Total Discovered Actors: %d"), DiscoveredActors.Num());

		UE_LOG(LogPlazaIntrospector, Log, TEXT(""));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("--- KEY ACTORS DETAIL ---"));

		// Log details of key actors
		for (const FPlazaActorInfo& Info : DiscoveredActors)
		{
			// Only log "interesting" actors
			if (Info.Notes.Contains(TEXT("KEY")) || Info.Notes.Contains(TEXT("IMPORTANT")))
			{
				UE_LOG(LogPlazaIntrospector, Log, TEXT(""));
				UE_LOG(LogPlazaIntrospector, Log, TEXT("[%s]"), *Info.ActorName);
				UE_LOG(LogPlazaIntrospector, Log, TEXT("  Class: %s"), *Info.ClassName);
				UE_LOG(LogPlazaIntrospector, Log, TEXT("  Location: (%.1f, %.1f, %.1f)"),
					Info.Location.X, Info.Location.Y, Info.Location.Z);
				UE_LOG(LogPlazaIntrospector, Log, TEXT("  Scale: (%.2f, %.2f, %.2f)"),
					Info.Scale.X, Info.Scale.Y, Info.Scale.Z);
				if (!Info.MeshAssetPath.IsEmpty())
				{
					UE_LOG(LogPlazaIntrospector, Log, TEXT("  Mesh: %s"), *Info.MeshAssetPath);
				}
				if (!Info.SkeletonAssetPath.IsEmpty())
				{
					UE_LOG(LogPlazaIntrospector, Log, TEXT("  Skeleton: %s"), *Info.SkeletonAssetPath);
				}
				if (!Info.Notes.IsEmpty())
				{
					UE_LOG(LogPlazaIntrospector, Log, TEXT("  Notes: %s"), *Info.Notes);
				}
			}
		}

		UE_LOG(LogPlazaIntrospector, Log, TEXT(""));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("========================================"));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("END INTROSPECTION REPORT"));
		UE_LOG(LogPlazaIntrospector, Log, TEXT("========================================"));
	}

	return LayoutSummary;
}

void ARoguePlazaIntrospector::LogActorsByClass(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		return;
	}

	UE_LOG(LogPlazaIntrospector, Log, TEXT("--- Actors of class: %s ---"), *ActorClass->GetName());

	for (const FPlazaActorInfo& Info : DiscoveredActors)
	{
		if (Info.ClassName == ActorClass->GetName())
		{
			UE_LOG(LogPlazaIntrospector, Log, TEXT("  %s at (%.1f, %.1f, %.1f)"),
				*Info.ActorName, Info.Location.X, Info.Location.Y, Info.Location.Z);
		}
	}
}

FString ARoguePlazaIntrospector::GenerateMarkdownReport() const
{
	FString Report;

	Report += TEXT("# Plaza Atom Manifest\n\n");
	Report += TEXT("**Generated automatically by RoguePlazaIntrospector**\n\n");

	Report += TEXT("## 1. Vista para Humanos\n\n");
	Report += FString::Printf(TEXT("- **Tamaño estimado de la plaza**: %.1f x %.1f metros\n"),
		LayoutSummary.PlazaWidthCm / 100.f, LayoutSummary.PlazaHeightCm / 100.f);
	Report += FString::Printf(TEXT("- **Player Starts**: %d\n"), LayoutSummary.NumPlayerStarts);
	Report += FString::Printf(TEXT("- **Target Dummies (maniquíes)**: %d\n"), LayoutSummary.NumTargetDummies);
	Report += FString::Printf(TEXT("- **Barriles explosivos**: %d\n"), LayoutSummary.NumExplosiveBarrels);
	Report += FString::Printf(TEXT("- **Luces**: %d\n"), LayoutSummary.NumLights);
	Report += TEXT("\n");

	Report += TEXT("## 2. Vista Técnica\n\n");
	Report += TEXT("| Actor | Clase | Ubicación (X,Y,Z) | Asset Path | Notas |\n");
	Report += TEXT("|-------|-------|-------------------|------------|-------|\n");

	for (const FPlazaActorInfo& Info : DiscoveredActors)
	{
		if (Info.Notes.Contains(TEXT("KEY")) || Info.Notes.Contains(TEXT("IMPORTANT")))
		{
			Report += FString::Printf(TEXT("| %s | %s | (%.0f, %.0f, %.0f) | %s | %s |\n"),
				*Info.ActorName,
				*Info.ClassName,
				Info.Location.X, Info.Location.Y, Info.Location.Z,
				Info.MeshAssetPath.IsEmpty() ? TEXT("-") : *Info.MeshAssetPath,
				*Info.Notes);
		}
	}

	Report += TEXT("\n## 3. Dimensiones del Mundo\n\n");
	Report += FString::Printf(TEXT("- **Bounds Mínimos**: (%.1f, %.1f, %.1f)\n"),
		LayoutSummary.MinBounds.X, LayoutSummary.MinBounds.Y, LayoutSummary.MinBounds.Z);
	Report += FString::Printf(TEXT("- **Bounds Máximos**: (%.1f, %.1f, %.1f)\n"),
		LayoutSummary.MaxBounds.X, LayoutSummary.MaxBounds.Y, LayoutSummary.MaxBounds.Z);

	return Report;
}

FString ARoguePlazaIntrospector::GetMeshAssetPath(AActor* Actor) const
{
	if (!Actor)
	{
		return FString();
	}

	// Check for StaticMeshComponent
	if (UStaticMeshComponent* SMC = Actor->FindComponentByClass<UStaticMeshComponent>())
	{
		if (UStaticMesh* Mesh = SMC->GetStaticMesh())
		{
			return Mesh->GetPathName();
		}
	}

	// Check for SkeletalMeshComponent
	if (USkeletalMeshComponent* SkMC = Actor->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (USkeletalMesh* Mesh = SkMC->GetSkeletalMeshAsset())
		{
			return Mesh->GetPathName();
		}
	}

	return FString();
}

FString ARoguePlazaIntrospector::GetSkeletonPath(AActor* Actor) const
{
	if (!Actor)
	{
		return FString();
	}

	if (USkeletalMeshComponent* SkMC = Actor->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (USkeletalMesh* Mesh = SkMC->GetSkeletalMeshAsset())
		{
			if (USkeleton* Skeleton = Mesh->GetSkeleton())
			{
				return Skeleton->GetPathName();
			}
		}
	}

	return FString();
}

FString ARoguePlazaIntrospector::ClassifyActor(AActor* Actor) const
{
	if (!Actor)
	{
		return FString();
	}

	TArray<FString> Tags;

	// Classify by type
	if (Actor->IsA<APlayerStart>())
	{
		Tags.Add(TEXT("KEY:SPAWN"));
		Tags.Add(TEXT("IMPORTANT"));
	}
	else if (Actor->IsA<ARogueTargetDummy>())
	{
		Tags.Add(TEXT("KEY:ENEMY"));
		Tags.Add(TEXT("IMPORTANT"));
		Tags.Add(TEXT("SKELETAL"));
	}
	else if (Actor->IsA<ARogueExplosiveBarrel>())
	{
		Tags.Add(TEXT("KEY:HAZARD"));
		Tags.Add(TEXT("IMPORTANT"));
	}
	else if (Actor->FindComponentByClass<ULightComponent>())
	{
		Tags.Add(TEXT("KEY:LIGHTING"));
		Tags.Add(TEXT("IMPORTANT"));
	}

	// Check if it has skeletal mesh (for asset swap consideration)
	if (Actor->FindComponentByClass<USkeletalMeshComponent>())
	{
		if (!Tags.Contains(TEXT("SKELETAL")))
		{
			Tags.Add(TEXT("SKELETAL"));
		}
	}

	// Check for large static meshes (potential floor/walls)
	if (UStaticMeshComponent* SMC = Actor->FindComponentByClass<UStaticMeshComponent>())
	{
		FVector Scale = Actor->GetActorScale3D();
		if (Scale.X > 5.f || Scale.Y > 5.f)
		{
			Tags.Add(TEXT("KEY:STRUCTURE"));
			Tags.Add(TEXT("IMPORTANT"));
		}
	}

	return FString::Join(Tags, TEXT(", "));
}
