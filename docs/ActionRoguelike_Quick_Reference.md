# ActionRoguelike - Quick Reference Guide

## Modular Structure Overview

| Módulo | Responsabilidad | Clases Clave |
|--------|-----------------|-------------|
| **ActionSystem** | Acciones, efectos, atributos | URogueAction, URogueActionComponent, RogueAttributeSet |
| **Projectiles** | Proyectiles (actor-based + data-oriented) | ARogueProjectile*, URogueProjectilesSubsystem |
| **AI** | IA, Behavior Trees, comportamiento enemigo | ARogueAICharacter, ARogueAIController, BTNodes |
| **Player** | Personaje jugador, input, UI player | ARoguePlayerCharacter, ARoguePlayerController, ARoguePlayerState |
| **Animation** | AnimInstance, AnimNotifies melee | URogueAnimInstance, URogueAnimNotifyState_Melee |
| **Pickups** | Power-ups, créditos, items | ARoguePickupActor* |
| **Core** | GameMode, GameState, interfaces | ARogueGameModeBase, ARogueGameState, IRogueGameplayInterface |
| **SaveSystem** | Guardado y carga de progreso | URogueSaveGame, URogueSaveGameSubsystem |
| **UI** | HUD, Widgets | ARogueHUD, URogueWorldUserWidget |
| **World** | Actores del mundo (cofres, barriles) | ARogueTreasureChest, ARogueExplosiveBarrel |
| **Performance** | Optimización (pooling, LOD, tick) | URogueActorPoolingSubsystem, URogueSignificanceManager |
| **Development** | Herramientas de desarrollo | ARogueCheatManager, RogueDeveloperSettings |

---

## Class Hierarchy - Key Inheritance Chains

### Action System Hierarchy
```
URogueAction (base)
├── URogueActionEffect
│   └── URogueActionEffect_Thorns
└── URogueAction_ProjectileAttack
```

### Character Hierarchy
```
ACharacter
├── ARoguePlayerCharacter (+ IGenericTeamAgentInterface)
└── ARogueAICharacter (+ IGenericTeamAgentInterface + IRogueSignificanceInterface)
```

### Projectile Hierarchy
```
AActor (+ IRogueActorPoolingInterface)
└── ARogueProjectile
    ├── ARogueProjectile_Magic (radius damage)
    ├── ARogueProjectile_Dash (teleport)
    └── ARogueProjectile_Blackhole (attract)
```

### Pickup Hierarchy
```
AActor (+ IRogueGameplayInterface)
└── ARoguePickupActor
    ├── ARoguePickupActor_HealthPotion
    ├── ARoguePickupActor_Credits
    └── ARoguePickupActor_GrantAction
```

### World Actors
```
ARogueTreasureChest (+ IRogueGameplayInterface)
ARogueExplosiveBarrel
ARogueTargetDummy
```

---

## Core Systems - Flow Diagrams

### Action Execution Flow
```
1. AddAction(ActionClass)
   ↓
2. StartActionByName(tag)
   ├─ CanStart() check
   ├─ Add tags to ActiveGameplayTags
   ├─ Call StartAction_Implementation()
   └─ Broadcast OnActionStarted
   
3. StopActionByName(tag)
   ├─ Call StopAction_Implementation()
   ├─ Remove tags
   └─ Broadcast OnActionStopped
```

### Damage Flow
```
Attack Action executes
   ↓
Projectile hits target
   ↓
OnActorHit() → ApplyAttributeChange(Health, -damage)
   ↓
FRogueAttribute.OnAttributeChanged broadcast
   ↓
Listeners notified (Character, AI, Barrel, etc)
   ↓
If Health <= 0 → Death (montage, loot, credits)
```

### AI Behavior
```
ARogueAIController
   ├─ Runs BehaviorTree
   ├─ UAIPerceptionComponent updates target
   └─ Blackboard synced
   
BehaviorTree
   ├─ BTTask: CheckAttackRange
   ├─ BTTask: StartAction (melee/ranged)
   └─ BTService: CheckHealth (heal/flee)
   
ARogueAICharacter
   └─ Executes Actions (Melee, Ranged, Heal)
```

### Replication Flow
```
Server-Authoritative:
├─ GameMode (server only)
├─ GameState (replicated)
├─ Character (server + client prediction)
├─ ActionComponent (replicated)
│  ├─ Actions array
│  ├─ RepData (bIsRunning, Instigator)
│  └─ ActiveGameplayTags
├─ ProjectileData (FFastArraySerializer)
└─ PlayerState
```

---

## Key Data Structures

### FRogueAttribute
```cpp
struct FRogueAttribute
{
    float Base;        // Permanent: leveling, permanent items
    float Modifier;    // Temporary: buffs, equipped items
    
    float GetValue()   // Returns Base + Modifier (clamped >= 0)
    OnAttributeChanged // Delegate broadcast on change
};
```

### AttributeSets Hierarchy
```
FRogueAttributeSet (abstract)
├── FRogueHealthAttributeSet
│   ├── Health
│   └── HealthMax
├── FRogueSurvivorAttributeSet (for player)
│   └── + AttackDamage
└── FRogueMonsterAttributeSet (for enemies)
    └── + AttackDamage
```

### Data-Oriented Projectile Structures
```cpp
struct FProjectileConfig      // Replicated metadata
{
    FVector InitialPosition;
    FVector InitialDirection;
    URogueProjectileData* ConfigDataAsset;
    AActor* InstigatorActor;
    uint32 ID;
    FHitResult Hit;
};

struct FProjectileInstance    // Per-frame data
{
    FVector Position;
    FVector Velocity;
    uint32 ID;
};

// Replication: FFastArraySerializer (delta serialization)
struct FProjectileConfigArray : public FFastArraySerializer
{
    TArray<FProjectileConfig> Items;
};
```

---

## GameplayTags System

### Built-in Tags (SharedGameplayTags.h)
```
Action.Dash
Action.PrimaryAttack
Action.SecondaryAttack
Action.Sprint
Action.Parry
Action.MagicProjectile
Action.Stunned
Action.Burning
Enemies.Melee
Enemies.Ranged
```

### Tag Usage
- **GrantsTags**: Applied while action runs
- **BlockedTags**: Prevent action from starting
- **ActiveGameplayTags**: Container of active tags on character

---

## Interfaces

| Interface | Purpose | Implementations |
|-----------|---------|-----------------|
| `IRogueGameplayInterface` | Interactable actors | Pickups, Chests |
| `IRogueActionSystemInterface` | Has ActionComponent | Player, AI Characters |
| `IRogueActorPoolingInterface` | Supports pooling | Projectiles |
| `IRogueSignificanceInterface` | LOD support | AI Characters |
| `IGenericTeamAgentInterface` | Team system | Player & AI Characters |

---

## Module Dependencies

```
Core/
  ├─ GameMode: Spawn logic, credits, EQS
  ├─ GameState: Replicated state, projectile data
  └─ Interfaces: Gameplay contract definitions

ActionSystem/
  ├─ Used by: Player, AI, World actors, Projectiles
  └─ Dependencies: GameplayTags, AttributeSet

Projectiles/
  ├─ Data-Oriented: URogueProjectilesSubsystem
  └─ Actor-Based: ARogueProjectile hierarchy

Player/
  ├─ Components: ActionComponent, Camera, Input
  └─ Dependencies: ActionSystem, Enhanced Input

AI/
  ├─ Behavior Tree: Custom nodes + EQS
  ├─ Perception: AIPerceptionComponent
  └─ Actions: MeleeAttack, RangedAttack

Performance/
  ├─ Actor Pooling: URogueActorPoolingSubsystem
  ├─ Significance: Distance-based LOD
  └─ Animation Budgeting: Via plugin callback

SaveSystem/
  ├─ SaveGame object: Player + Actor data
  └─ Subsystem: Orchestrates save/load
```

---

## Networking Model

### Authority Rules
- **Server-Authoritative**: GameMode, GameState, Combat validation
- **Client-Predicted**: Player movement
- **Replicated**: Characters, Actions, Attributes, Projectiles

### RPC Patterns
```cpp
UFUNCTION(Server, Reliable)
void ServerStartAction(AActor* Instigator, FGameplayTag ActionName);

UFUNCTION(Client, Unreliable)
void ClientOnSeenBy(ARogueAICharacter* SeenByPawn);

UFUNCTION(NetMulticast, Unreliable)
void MulticastPlayAttackFX();
```

### Replication Variables
```cpp
UPROPERTY(Replicated)                    // Always replicated
UPROPERTY(ReplicatedUsing=OnRep_XXX)     // With callback
UPROPERTY(Transient)                     // Never replicated
UPROPERTY(NotReplicated)                 // Explicit no replication
```

---

## Build Configuration

### Dependencies
```
PublicDependencyModules: [
    Core, CoreUObject, Engine, InputCore,
    AIModule, GameplayTasks, UMG, GameplayTags,
    OnlineSubsystem, DeveloperSettings,
    SignificanceManager, EnhancedInput, Niagara,
    CoreOnline, NetCore
]

PrivateDependencyModules: [
    MoviePlayer, RenderCore, AnimationBudgetAllocator
]
```

### Enabled Plugins
```
- SignificanceManager (LOD by distance)
- OnlineSubsystemSteam (Steam integration)
- StateTree (state machine)
- AnimationBudgetAllocator (animation LOD)
- Iris (advanced replication)
```

---

## Performance Optimizations

| Technique | Implementation | Benefit |
|-----------|-----------------|---------|
| Actor Pooling | URogueActorPoolingSubsystem | Avoid spawn/destroy overhead |
| Data-Oriented Projectiles | URogueProjectilesSubsystem | Reduced memory, centralized ticking |
| Significance Manager | Distance-based LOD | Throttle AI updates by distance |
| Animation Budgeting | Animation Budget Allocator plugin | Reduce skeletal mesh updates far away |
| PSO Precaching | r.PSOPrecaching=1 | Avoid shader compilation stutter |
| Fast Array Replication | FFastArraySerializer | Delta replication only (bandwidth efficient) |
| Async Asset Loading | Asset Manager | Non-blocking enemy spawn |

---

## Quick Start: Adding Features

### Add New Action
1. Derive from `URogueAction`
2. Override `StartAction_Implementation()` and `StopAction_Implementation()`
3. Set `ActivationTag` in editor
4. Add to character's `DefaultActions` array or grant dynamically

### Add New Enemy Type
1. Create BP from `ARogueAICharacter`
2. Create `URogueMonsterData` asset
3. Add entry to `MonsterTable` (DataTable in GameMode)
4. Configure actions and stats in MonsterData

### Add New Pickup
1. Derive from `ARoguePickupActor`
2. Override `Interact_Implementation()`
3. Place in level or configure GameMode to spawn

### Add New Visual Effect
1. Create Niagara System
2. Assign to action, projectile, or world actor
3. Trigger from action or anim notify

---

## Debug Utilities

### Console Commands
```
game.drawdebugmelee 1      // Visualize melee overlap shapes
stat AnimationBudgetAllocator  // Animation LOD stats
a.Budget.Debug.Enabled 1   // Animation budget debug
stat psocache              // PSO caching stats
```

### Cheat Manager Commands
- Implemented in `ARogueCheatManager`
- Access via `console` in game (tilde key)
- Examples: spawn enemies, test features, etc.

---

## File Structure Reference

```
Source/ActionRoguelike/
├── ActionSystem/
│   ├── RogueAction.h|cpp (base)
│   ├── RogueActionComponent.h|cpp (manager)
│   ├── RogueActionEffect.h|cpp (periodic effects)
│   ├── RogueActionEffect_Thorns.h|cpp
│   ├── RogueAction_ProjectileAttack.h|cpp
│   ├── RogueActionSystemInterface.h|cpp
│   └── RogueAttributeSet.h (FRogueAttribute, attribute sets)
│
├── AI/
│   ├── RogueAICharacter.h|cpp
│   ├── RogueAIController.h|cpp
│   ├── RogueBTTask_*.h|cpp (7 custom BT nodes)
│   ├── RogueBTService_*.h|cpp (3 custom BT services)
│   ├── RogueAction_MinionMeleeAttack.h|cpp
│   └── RogueAction_MinionRangedAttack.h|cpp
│
├── Animation/
│   ├── RogueAnimInstance.h|cpp
│   ├── RogueAnimNotifyState_Melee.h|cpp
│   └── RogueCurveAnimSubsystem.h|cpp
│
├── Core/
│   ├── RogueGameModeBase.h|cpp (enemy spawn, credits, EQS)
│   ├── RogueGameState.h|cpp (replicated state)
│   ├── RogueGameplayInterface.h|cpp (interaction contract)
│   ├── RogueGameplayFunctionLibrary.h|cpp
│   ├── RogueMonsterData.h|cpp (enemy config asset)
│   ├── RogueGameViewportClient.h|cpp
│   └── RogueLoadingScreenSubsystem.h|cpp
│
├── Player/
│   ├── RoguePlayerCharacter.h|cpp (controllable character)
│   ├── RoguePlayerController.h|cpp (input + interaction)
│   ├── RoguePlayerState.h|cpp (credits, records)
│   ├── RoguePlayerData.h|cpp (input actions asset)
│   └── RogueInteractionComponent.h|cpp (E key system)
│
├── Projectiles/
│   ├── RogueProjectile.h|cpp (base actor)
│   ├── RogueProjectile_Magic.h|cpp
│   ├── RogueProjectile_Dash.h|cpp
│   ├── RogueProjectile_Blackhole.h|cpp
│   ├── RogueProjectileData.h|cpp (config asset)
│   ├── RogueProjectileMovementComponent.h|cpp
│   └── RogueProjectilesSubsystem.h|cpp (data-oriented)
│
├── Pickups/
│   ├── RoguePickupActor.h|cpp (base)
│   ├── RoguePickupActor_HealthPotion.h|cpp
│   ├── RoguePickupActor_Credits.h|cpp
│   └── RoguePickupActor_GrantAction.h|cpp
│
├── SaveSystem/
│   ├── RogueSaveGame.h|cpp (container)
│   ├── RogueSaveGameSubsystem.h|cpp (manager)
│   └── RogueSaveGameSettings.h|cpp (config)
│
├── UI/
│   ├── RogueHUD.h|cpp
│   ├── RogueWorldUserWidget.h|cpp (world space UI)
│   └── RogueEffectSlotWidget.h|cpp (buff display)
│
├── World/
│   ├── RogueTreasureChest.h|cpp
│   ├── RogueExplosiveBarrel.h|cpp
│   └── RogueTargetDummy.h|cpp
│
├── Performance/
│   ├── RogueActorPoolingInterface.h|cpp (contract)
│   ├── RogueActorPoolingSubsystem.h|cpp (manager)
│   ├── RogueSignificanceInterface.h|cpp (contract)
│   ├── RogueSignificanceManager.h|cpp (LOD mgr)
│   ├── RogueSignificanceComponent.h|cpp
│   ├── RogueSignificanceSettings.h|cpp
│   └── RogueTickablesSubsystem.h|cpp
│
├── Development/
│   ├── RogueCheatManager.h|cpp
│   └── RogueDeveloperSettings.h|cpp
│
├── SharedGameplayTags.h|cpp (tag definitions)
├── ActionRoguelike.h|cpp (module header)
└── ActionRoguelike.Build.cs (build config)
```

---

## Project Statistics

- **Total Classes**: 60+ public classes (ACTIONROGUELIKE_API)
- **Modules**: 14 functional modules
- **Interfaces**: 5 core interfaces
- **Subsystems**: 6 custom subsystems
- **Source Files**: ~126 (.h + .cpp)
- **Engine Version**: UE 5.6
- **C++ Standard**: Modern (C++17+)

---

## External References

- **Original Author**: Tom Looman (tomlooman.com)
- **Purpose**: Professional C++ & UE5 education
- **License**: Paragon assets under UE EULA (game code is free for learning)
- **GitHub**: https://github.com/tomlooman/ActionRoguelike

---

**Document Generated**: 2025-11-22  
**Total Architecture Lines**: 1423  
**Quick Reference Guide**: Complete system overview for rapid navigation
