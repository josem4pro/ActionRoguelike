// RoguePlazaManager.cpp
// Implementation of plaza configuration manager
// Part of SP-N1-V2-003 Fase 3R: Plaza Config & Asset Swap

#include "World/RoguePlazaManager.h"
#include "World/RoguePlazaConfig.h"
#include "ActionSystem/RogueAttributeSet.h"
#include "World/RogueTargetDummy.h"
#include "World/RogueExplosiveBarrel.h"
#include "Player/RoguePlayerCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RoguePlazaManager)

DEFINE_LOG_CATEGORY(LogPlazaManager);

ARoguePlazaManager::ARoguePlazaManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ARoguePlazaManager::BeginPlay()
{
	Super::BeginPlay();

	// Load config from asset if specified
	if (ConfigAsset)
	{
		SetPlazaConfigFromAsset(ConfigAsset);
	}

	if (bAutoApplyOnBeginPlay)
	{
		FPlazaConfigResult Result = ApplyConfiguration();
		if (Result.bSuccess)
		{
			UE_LOG(LogPlazaManager, Log, TEXT("Plaza configuration applied successfully"));
		}
		else
		{
			UE_LOG(LogPlazaManager, Warning, TEXT("Plaza configuration failed: %s"), *Result.ErrorMessage);
		}
	}
}

void ARoguePlazaManager::SetPlazaConfig(const FPlazaConfig& InConfig)
{
	PlazaConfig = InConfig;
	UE_LOG(LogPlazaManager, Log, TEXT("Plaza config set: %dx%d cm, %d dummies, %d barrels"),
		(int32)PlazaConfig.Layout.PlazaWidthCm,
		(int32)PlazaConfig.Layout.PlazaHeightCm,
		PlazaConfig.Layout.NumTargetDummies,
		PlazaConfig.Layout.NumExplosiveBarrels);
}

void ARoguePlazaManager::SetPlazaConfigFromAsset(URoguePlazaConfigAsset* InConfigAsset)
{
	if (InConfigAsset)
	{
		ConfigAsset = InConfigAsset;
		PlazaConfig = InConfigAsset->Config;
		UE_LOG(LogPlazaManager, Log, TEXT("Plaza config loaded from asset: %s (v%s)"),
			*InConfigAsset->Description, *InConfigAsset->Version);
	}
}

FPlazaConfigResult ARoguePlazaManager::ApplyConfiguration()
{
	FPlazaConfigResult Result;

	LogCurrentConfig();

	// Apply layout (actor counts)
	FPlazaConfigResult LayoutResult = ApplyLayoutConfig();
	Result.DummiesSpawned = LayoutResult.DummiesSpawned;
	Result.BarrelsSpawned = LayoutResult.BarrelsSpawned;

	// Apply visuals (asset swaps)
	FPlazaConfigResult VisualResult = ApplyVisualConfig();
	Result.VisualsApplied = VisualResult.VisualsApplied;

	Result.bSuccess = true;

	UE_LOG(LogPlazaManager, Log, TEXT("Configuration applied: %d dummies spawned, %d barrels spawned, %d visuals applied"),
		Result.DummiesSpawned, Result.BarrelsSpawned, Result.VisualsApplied);

	return Result;
}

FPlazaConfigResult ARoguePlazaManager::ApplyLayoutConfig()
{
	FPlazaConfigResult Result;

	Result.DummiesSpawned = EnsureDummyCount();
	Result.BarrelsSpawned = EnsureBarrelCount();
	Result.bSuccess = true;

	return Result;
}

FPlazaConfigResult ARoguePlazaManager::ApplyVisualConfig()
{
	FPlazaConfigResult Result;
	Result.bSuccess = true;

	// Apply player visuals
	if (PlazaConfig.Visuals.HasPlayerVisualOverride())
	{
		UWorld* World = GetWorld();
		if (World)
		{
			for (TActorIterator<ARoguePlayerCharacter> It(World); It; ++It)
			{
				if (ApplyPlayerVisualConfig(*It))
				{
					Result.VisualsApplied++;
				}
			}
		}
	}

	// Apply dummy visuals
	if (PlazaConfig.Visuals.HasDummyVisualOverride())
	{
		for (ARogueTargetDummy* Dummy : GetAllDummies())
		{
			if (ApplyDummyVisualConfig(Dummy))
			{
				Result.VisualsApplied++;
			}
		}
	}

	// Apply barrel visuals
	if (PlazaConfig.Visuals.HasBarrelVisualOverride())
	{
		for (ARogueExplosiveBarrel* Barrel : GetAllBarrels())
		{
			if (ApplyBarrelVisualConfig(Barrel))
			{
				Result.VisualsApplied++;
			}
		}
	}

	return Result;
}

bool ARoguePlazaManager::ApplyPlayerVisualConfig(ARoguePlayerCharacter* Player)
{
	if (!Player)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("ApplyPlayerVisualConfig: null player"));
		return false;
	}

	if (!PlazaConfig.Visuals.HasPlayerVisualOverride())
	{
		UE_LOG(LogPlazaManager, Verbose, TEXT("ApplyPlayerVisualConfig: no override configured"));
		return false;
	}

	// Load skeletal mesh if needed
	USkeletalMesh* NewMesh = PlazaConfig.Visuals.PlayerSkeletalMesh.LoadSynchronous();
	if (!NewMesh)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("ApplyPlayerVisualConfig: failed to load mesh"));
		return false;
	}

	// Get the mesh component
	USkeletalMeshComponent* MeshComp = Player->GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("ApplyPlayerVisualConfig: player has no mesh component"));
		return false;
	}

	// Apply the mesh swap
	MeshComp->SetSkeletalMesh(NewMesh);
	UE_LOG(LogPlazaManager, Log, TEXT("ApplyPlayerVisualConfig: swapped player mesh to %s"), *NewMesh->GetName());

	// Apply anim class if specified
	if (!PlazaConfig.Visuals.PlayerAnimClass.IsNull())
	{
		UClass* AnimClass = PlazaConfig.Visuals.PlayerAnimClass.LoadSynchronous();
		if (AnimClass)
		{
			MeshComp->SetAnimInstanceClass(AnimClass);
			UE_LOG(LogPlazaManager, Log, TEXT("ApplyPlayerVisualConfig: set anim class to %s"), *AnimClass->GetName());
		}
	}

	return true;
}

bool ARoguePlazaManager::ApplyDummyVisualConfig(ARogueTargetDummy* Dummy)
{
	if (!Dummy)
	{
		return false;
	}

	if (!PlazaConfig.Visuals.HasDummyVisualOverride())
	{
		return false;
	}

	USkeletalMesh* NewMesh = PlazaConfig.Visuals.DummySkeletalMesh.LoadSynchronous();
	if (!NewMesh)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("ApplyDummyVisualConfig: failed to load mesh"));
		return false;
	}

	USkeletalMeshComponent* MeshComp = Dummy->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("ApplyDummyVisualConfig: dummy has no skeletal mesh"));
		return false;
	}

	MeshComp->SetSkeletalMesh(NewMesh);
	UE_LOG(LogPlazaManager, Log, TEXT("ApplyDummyVisualConfig: swapped dummy mesh to %s"), *NewMesh->GetName());

	// Apply anim class if specified
	if (!PlazaConfig.Visuals.DummyAnimClass.IsNull())
	{
		UClass* AnimClass = PlazaConfig.Visuals.DummyAnimClass.LoadSynchronous();
		if (AnimClass)
		{
			MeshComp->SetAnimInstanceClass(AnimClass);
		}
	}

	return true;
}

bool ARoguePlazaManager::ApplyBarrelVisualConfig(ARogueExplosiveBarrel* Barrel)
{
	if (!Barrel)
	{
		return false;
	}

	if (!PlazaConfig.Visuals.HasBarrelVisualOverride())
	{
		return false;
	}

	UStaticMesh* NewMesh = PlazaConfig.Visuals.BarrelStaticMesh.LoadSynchronous();
	if (!NewMesh)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("ApplyBarrelVisualConfig: failed to load mesh"));
		return false;
	}

	UStaticMeshComponent* MeshComp = Barrel->FindComponentByClass<UStaticMeshComponent>();
	if (!MeshComp)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("ApplyBarrelVisualConfig: barrel has no static mesh"));
		return false;
	}

	MeshComp->SetStaticMesh(NewMesh);
	UE_LOG(LogPlazaManager, Log, TEXT("ApplyBarrelVisualConfig: swapped barrel mesh to %s"), *NewMesh->GetName());

	return true;
}

void ARoguePlazaManager::GetCurrentActorCounts(int32& OutDummies, int32& OutBarrels) const
{
	OutDummies = GetAllDummies().Num();
	OutBarrels = GetAllBarrels().Num();
}

void ARoguePlazaManager::LogCurrentConfig() const
{
	UE_LOG(LogPlazaManager, Log, TEXT(""));
	UE_LOG(LogPlazaManager, Log, TEXT("========================================"));
	UE_LOG(LogPlazaManager, Log, TEXT("PLAZA CONFIGURATION"));
	UE_LOG(LogPlazaManager, Log, TEXT("========================================"));
	UE_LOG(LogPlazaManager, Log, TEXT("Layout:"));
	UE_LOG(LogPlazaManager, Log, TEXT("  Plaza Size: %.1f x %.1f cm (%.1f x %.1f m)"),
		PlazaConfig.Layout.PlazaWidthCm, PlazaConfig.Layout.PlazaHeightCm,
		PlazaConfig.Layout.GetWidthMeters(), PlazaConfig.Layout.GetHeightMeters());
	UE_LOG(LogPlazaManager, Log, TEXT("  Wall Height: %.1f cm (%.1f m)"),
		PlazaConfig.Layout.WallHeightCm, PlazaConfig.Layout.GetWallHeightMeters());
	UE_LOG(LogPlazaManager, Log, TEXT("  Target Dummies: %d"), PlazaConfig.Layout.NumTargetDummies);
	UE_LOG(LogPlazaManager, Log, TEXT("  Explosive Barrels: %d"), PlazaConfig.Layout.NumExplosiveBarrels);
	UE_LOG(LogPlazaManager, Log, TEXT("  Min Actor Spacing: %.1f cm"), PlazaConfig.Layout.MinActorSpacing);
	UE_LOG(LogPlazaManager, Log, TEXT("  Player Safe Zone: %.1f cm"), PlazaConfig.Layout.PlayerStartSafeZone);

	UE_LOG(LogPlazaManager, Log, TEXT("Visuals:"));
	UE_LOG(LogPlazaManager, Log, TEXT("  Player Override: %s"),
		PlazaConfig.Visuals.HasPlayerVisualOverride() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogPlazaManager, Log, TEXT("  Dummy Override: %s"),
		PlazaConfig.Visuals.HasDummyVisualOverride() ? TEXT("Yes") : TEXT("No"));
	UE_LOG(LogPlazaManager, Log, TEXT("  Barrel Override: %s"),
		PlazaConfig.Visuals.HasBarrelVisualOverride() ? TEXT("Yes") : TEXT("No"));

	int32 CurrentDummies, CurrentBarrels;
	GetCurrentActorCounts(CurrentDummies, CurrentBarrels);
	UE_LOG(LogPlazaManager, Log, TEXT("Current State:"));
	UE_LOG(LogPlazaManager, Log, TEXT("  Dummies in world: %d (config: %d)"), CurrentDummies, PlazaConfig.Layout.NumTargetDummies);
	UE_LOG(LogPlazaManager, Log, TEXT("  Barrels in world: %d (config: %d)"), CurrentBarrels, PlazaConfig.Layout.NumExplosiveBarrels);
	UE_LOG(LogPlazaManager, Log, TEXT("========================================"));
}

TArray<FVector> ARoguePlazaManager::CalculateSpawnPositions(int32 NumPositions) const
{
	TArray<FVector> Positions;

	if (NumPositions <= 0)
	{
		return Positions;
	}

	FVector PlayerStartLoc = GetPlayerStartLocation();

	// Calculate grid for distributing actors
	float HalfWidth = PlazaConfig.Layout.PlazaWidthCm / 2.f;
	float HalfHeight = PlazaConfig.Layout.PlazaHeightCm / 2.f;
	float Spacing = PlazaConfig.Layout.MinActorSpacing;

	// Create a grid of potential positions
	TArray<FVector> PotentialPositions;
	for (float X = -HalfWidth + Spacing; X < HalfWidth - Spacing; X += Spacing)
	{
		for (float Y = -HalfHeight + Spacing; Y < HalfHeight - Spacing; Y += Spacing)
		{
			FVector Pos(X, Y, 0.f);
			if (!IsInSafeZone(Pos, PlayerStartLoc))
			{
				PotentialPositions.Add(Pos);
			}
		}
	}

	// Shuffle and pick requested number
	for (int32 i = PotentialPositions.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		PotentialPositions.Swap(i, j);
	}

	int32 NumToReturn = FMath::Min(NumPositions, PotentialPositions.Num());
	for (int32 i = 0; i < NumToReturn; i++)
	{
		Positions.Add(PotentialPositions[i]);
	}

	return Positions;
}

int32 ARoguePlazaManager::EnsureDummyCount()
{
	TArray<ARogueTargetDummy*> Dummies = GetAllDummies();
	int32 CurrentCount = Dummies.Num();
	int32 TargetCount = PlazaConfig.Layout.NumTargetDummies;

	if (CurrentCount >= TargetCount)
	{
		UE_LOG(LogPlazaManager, Log, TEXT("Dummy count OK: %d/%d"), CurrentCount, TargetCount);
		return 0;
	}

	int32 ToSpawn = TargetCount - CurrentCount;
	UE_LOG(LogPlazaManager, Log, TEXT("Need to spawn %d dummies (%d/%d)"), ToSpawn, CurrentCount, TargetCount);

	if (!TargetDummyClass)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("Cannot spawn dummies: TargetDummyClass not set"));
		return 0;
	}

	TArray<FVector> SpawnPositions = CalculateSpawnPositions(ToSpawn);
	int32 Spawned = 0;

	UWorld* World = GetWorld();
	if (World)
	{
		for (const FVector& Pos : SpawnPositions)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			ARogueTargetDummy* NewDummy = World->SpawnActor<ARogueTargetDummy>(
				TargetDummyClass,
				Pos,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (NewDummy)
			{
				Spawned++;
				UE_LOG(LogPlazaManager, Log, TEXT("Spawned dummy at (%.1f, %.1f, %.1f)"),
					Pos.X, Pos.Y, Pos.Z);
			}
		}
	}

	return Spawned;
}

int32 ARoguePlazaManager::EnsureBarrelCount()
{
	TArray<ARogueExplosiveBarrel*> Barrels = GetAllBarrels();
	int32 CurrentCount = Barrels.Num();
	int32 TargetCount = PlazaConfig.Layout.NumExplosiveBarrels;

	if (CurrentCount >= TargetCount)
	{
		UE_LOG(LogPlazaManager, Log, TEXT("Barrel count OK: %d/%d"), CurrentCount, TargetCount);
		return 0;
	}

	int32 ToSpawn = TargetCount - CurrentCount;
	UE_LOG(LogPlazaManager, Log, TEXT("Need to spawn %d barrels (%d/%d)"), ToSpawn, CurrentCount, TargetCount);

	if (!ExplosiveBarrelClass)
	{
		UE_LOG(LogPlazaManager, Warning, TEXT("Cannot spawn barrels: ExplosiveBarrelClass not set"));
		return 0;
	}

	TArray<FVector> SpawnPositions = CalculateSpawnPositions(ToSpawn);
	int32 Spawned = 0;

	UWorld* World = GetWorld();
	if (World)
	{
		for (const FVector& Pos : SpawnPositions)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			ARogueExplosiveBarrel* NewBarrel = World->SpawnActor<ARogueExplosiveBarrel>(
				ExplosiveBarrelClass,
				Pos,
				FRotator::ZeroRotator,
				SpawnParams
			);

			if (NewBarrel)
			{
				Spawned++;
				UE_LOG(LogPlazaManager, Log, TEXT("Spawned barrel at (%.1f, %.1f, %.1f)"),
					Pos.X, Pos.Y, Pos.Z);
			}
		}
	}

	return Spawned;
}

FVector ARoguePlazaManager::GetPlayerStartLocation() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FVector::ZeroVector;
	}

	for (TActorIterator<APlayerStart> It(World); It; ++It)
	{
		return It->GetActorLocation();
	}

	return FVector::ZeroVector;
}

bool ARoguePlazaManager::IsInSafeZone(const FVector& Position, const FVector& PlayerStartLoc) const
{
	float DistSq = FVector::DistSquared2D(Position, PlayerStartLoc);
	float SafeZoneSq = FMath::Square(PlazaConfig.Layout.PlayerStartSafeZone);
	return DistSq < SafeZoneSq;
}

TArray<ARogueTargetDummy*> ARoguePlazaManager::GetAllDummies() const
{
	TArray<ARogueTargetDummy*> Result;

	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ARogueTargetDummy> It(World); It; ++It)
		{
			Result.Add(*It);
		}
	}

	return Result;
}

TArray<ARogueExplosiveBarrel*> ARoguePlazaManager::GetAllBarrels() const
{
	TArray<ARogueExplosiveBarrel*> Result;

	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ARogueExplosiveBarrel> It(World); It; ++It)
		{
			Result.Add(*It);
		}
	}

	return Result;
}
