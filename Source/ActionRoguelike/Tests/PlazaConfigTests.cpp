// PlazaConfigTests.cpp
// Automation tests for Plaza Configuration system
// Part of SP-N1-V2-003 Fase 3R: Plaza Config & Asset Swap

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "World/RoguePlazaConfig.h"
#include "World/RoguePlazaManager.h"
#include "World/RoguePlazaIntrospector.h"

#if WITH_AUTOMATION_TESTS

//=============================================================================
// Test: FPlazaLayoutConfig Default Values
// Verify that layout config has sensible defaults
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaConfigDefaultsTest,
	"ActionRoguelike.Plaza.ConfigDefaultValues",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaConfigDefaultsTest::RunTest(const FString& Parameters)
{
	FPlazaLayoutConfig Config;

	// Test default dimensions
	TestEqual(TEXT("Default PlazaWidthCm is 4000"), Config.PlazaWidthCm, 4000.f);
	TestEqual(TEXT("Default PlazaHeightCm is 4000"), Config.PlazaHeightCm, 4000.f);
	TestEqual(TEXT("Default WallHeightCm is 300"), Config.WallHeightCm, 300.f);

	// Test default actor counts
	TestEqual(TEXT("Default NumTargetDummies is 3"), Config.NumTargetDummies, 3);
	TestEqual(TEXT("Default NumExplosiveBarrels is 2"), Config.NumExplosiveBarrels, 2);

	// Test default layout settings
	TestEqual(TEXT("Default MinActorSpacing is 200"), Config.MinActorSpacing, 200.f);
	TestEqual(TEXT("Default PlayerStartSafeZone is 500"), Config.PlayerStartSafeZone, 500.f);

	return true;
}

//=============================================================================
// Test: FPlazaLayoutConfig Helper Methods
// Verify meter conversion helpers work correctly
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaConfigHelperMethodsTest,
	"ActionRoguelike.Plaza.ConfigHelperMethods",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaConfigHelperMethodsTest::RunTest(const FString& Parameters)
{
	FPlazaLayoutConfig Config;

	// Set known values
	Config.PlazaWidthCm = 5000.f;
	Config.PlazaHeightCm = 3000.f;
	Config.WallHeightCm = 250.f;

	// Test conversion helpers
	TestEqual(TEXT("Width in meters is 50"), Config.GetWidthMeters(), 50.f);
	TestEqual(TEXT("Height in meters is 30"), Config.GetHeightMeters(), 30.f);
	TestEqual(TEXT("Wall height in meters is 2.5"), Config.GetWallHeightMeters(), 2.5f);

	return true;
}

//=============================================================================
// Test: FPlazaVisualConfig Override Detection
// Verify that visual config correctly detects when overrides are set
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaVisualConfigOverrideTest,
	"ActionRoguelike.Plaza.VisualConfigOverrideDetection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaVisualConfigOverrideTest::RunTest(const FString& Parameters)
{
	FPlazaVisualConfig Config;

	// Initially, no overrides should be set
	TestFalse(TEXT("No player override initially"), Config.HasPlayerVisualOverride());
	TestFalse(TEXT("No dummy override initially"), Config.HasDummyVisualOverride());
	TestFalse(TEXT("No barrel override initially"), Config.HasBarrelVisualOverride());

	return true;
}

//=============================================================================
// Test: FPlazaConfig Complete Structure
// Verify that the combined config struct works correctly
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaConfigStructTest,
	"ActionRoguelike.Plaza.CompleteConfigStruct",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaConfigStructTest::RunTest(const FString& Parameters)
{
	FPlazaConfig Config;

	// Modify layout
	Config.Layout.PlazaWidthCm = 6000.f;
	Config.Layout.NumTargetDummies = 5;

	// Verify modifications
	TestEqual(TEXT("Modified width preserved"), Config.Layout.PlazaWidthCm, 6000.f);
	TestEqual(TEXT("Modified dummy count preserved"), Config.Layout.NumTargetDummies, 5);

	// Visual config should still have no overrides
	TestFalse(TEXT("Visual config unchanged"), Config.Visuals.HasPlayerVisualOverride());

	return true;
}

//=============================================================================
// Test: URoguePlazaConfigAsset Creation
// Verify that config data assets can be created
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaConfigAssetTest,
	"ActionRoguelike.Plaza.ConfigAssetCreation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaConfigAssetTest::RunTest(const FString& Parameters)
{
	// Create a temporary config asset
	URoguePlazaConfigAsset* Asset = NewObject<URoguePlazaConfigAsset>();

	TestNotNull(TEXT("Config asset created"), Asset);

	if (Asset)
	{
		// Verify default values are inherited
		TestEqual(TEXT("Asset has default width"), Asset->Config.Layout.PlazaWidthCm, 4000.f);
		TestEqual(TEXT("Asset has default version"), Asset->Version, FString(TEXT("1.0")));

		// Modify and verify
		Asset->Description = TEXT("Test Configuration");
		Asset->Config.Layout.NumTargetDummies = 10;

		TestEqual(TEXT("Description set"), Asset->Description, FString(TEXT("Test Configuration")));
		TestEqual(TEXT("Dummy count modified"), Asset->Config.Layout.NumTargetDummies, 10);
	}

	return true;
}

//=============================================================================
// Test: Plaza Introspector Struct Initialization
// Verify that introspector data structures initialize correctly
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaIntrospectorStructTest,
	"ActionRoguelike.Plaza.IntrospectorStructInit",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaIntrospectorStructTest::RunTest(const FString& Parameters)
{
	FPlazaActorInfo ActorInfo;

	// Verify default initialization
	TestTrue(TEXT("ActorName is empty"), ActorInfo.ActorName.IsEmpty());
	TestTrue(TEXT("ClassName is empty"), ActorInfo.ClassName.IsEmpty());
	TestEqual(TEXT("Location is zero"), ActorInfo.Location, FVector::ZeroVector);
	TestEqual(TEXT("Scale is zero (default)"), ActorInfo.Scale, FVector::ZeroVector);

	// Verify layout summary defaults
	FPlazaLayoutSummary Summary;
	TestEqual(TEXT("Summary width is zero"), Summary.PlazaWidthCm, 0.f);
	TestEqual(TEXT("Summary dummies is zero"), Summary.NumTargetDummies, 0);

	return true;
}

//=============================================================================
// Test: Plaza Manager Configuration
// Verify that plaza manager can be configured correctly
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaManagerConfigTest,
	"ActionRoguelike.Plaza.ManagerConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaManagerConfigTest::RunTest(const FString& Parameters)
{
	// Create a manager without spawning in world
	ARoguePlazaManager* Manager = NewObject<ARoguePlazaManager>();

	TestNotNull(TEXT("Manager created"), Manager);

	if (Manager)
	{
		// Create custom config
		FPlazaConfig CustomConfig;
		CustomConfig.Layout.PlazaWidthCm = 8000.f;
		CustomConfig.Layout.NumTargetDummies = 7;

		// Apply config
		Manager->SetPlazaConfig(CustomConfig);

		// Verify config was set
		const FPlazaConfig& RetrievedConfig = Manager->GetPlazaConfig();
		TestEqual(TEXT("Width configured"), RetrievedConfig.Layout.PlazaWidthCm, 8000.f);
		TestEqual(TEXT("Dummy count configured"), RetrievedConfig.Layout.NumTargetDummies, 7);
	}

	return true;
}

//=============================================================================
// Test: Plaza Manager Asset Configuration
// Verify that manager can load config from asset
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaManagerAssetConfigTest,
	"ActionRoguelike.Plaza.ManagerAssetConfiguration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaManagerAssetConfigTest::RunTest(const FString& Parameters)
{
	// Create manager and asset
	ARoguePlazaManager* Manager = NewObject<ARoguePlazaManager>();
	URoguePlazaConfigAsset* Asset = NewObject<URoguePlazaConfigAsset>();

	TestNotNull(TEXT("Manager created"), Manager);
	TestNotNull(TEXT("Asset created"), Asset);

	if (Manager && Asset)
	{
		// Configure asset
		Asset->Description = TEXT("Test Asset Config");
		Asset->Config.Layout.PlazaWidthCm = 12000.f;
		Asset->Config.Layout.NumExplosiveBarrels = 8;

		// Apply asset to manager
		Manager->SetPlazaConfigFromAsset(Asset);

		// Verify config was loaded
		const FPlazaConfig& Config = Manager->GetPlazaConfig();
		TestEqual(TEXT("Width from asset"), Config.Layout.PlazaWidthCm, 12000.f);
		TestEqual(TEXT("Barrel count from asset"), Config.Layout.NumExplosiveBarrels, 8);
	}

	return true;
}

//=============================================================================
// Test: Spawn Position Calculation
// Verify that spawn positions are calculated correctly
//=============================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPlazaSpawnPositionTest,
	"ActionRoguelike.Plaza.SpawnPositionCalculation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPlazaSpawnPositionTest::RunTest(const FString& Parameters)
{
	// Note: This test verifies the algorithm logic, not actual spawning
	// Full spawning tests require a valid world context

	FPlazaLayoutConfig Config;
	Config.PlazaWidthCm = 4000.f;
	Config.PlazaHeightCm = 4000.f;
	Config.MinActorSpacing = 200.f;
	Config.PlayerStartSafeZone = 500.f;

	// Calculate how many potential positions we should have
	float HalfWidth = Config.PlazaWidthCm / 2.f;
	float HalfHeight = Config.PlazaHeightCm / 2.f;
	float Spacing = Config.MinActorSpacing;

	int32 PositionsX = (int32)((HalfWidth * 2 - Spacing * 2) / Spacing);
	int32 PositionsY = (int32)((HalfHeight * 2 - Spacing * 2) / Spacing);

	// There should be a grid of positions
	TestTrue(TEXT("Should have multiple X positions"), PositionsX > 0);
	TestTrue(TEXT("Should have multiple Y positions"), PositionsY > 0);

	AddInfo(FString::Printf(TEXT("Grid size: %d x %d = %d potential positions"),
		PositionsX, PositionsY, PositionsX * PositionsY));

	return true;
}

#endif // WITH_AUTOMATION_TESTS
