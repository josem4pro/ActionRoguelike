# ActionRoguelike - Mapa Arquitectónico Completo

**Versión del Engine**: 5.6  
**Última actualización**: 2025-11-22  
**Autor del Proyecto**: Tom Looman (tomlooman.com)  
**Propósito**: Plataforma educativa profesional de desarrollo en C++ y Unreal Engine

---

## 1. VISIÓN GENERAL DEL PROYECTO

### 1.1 Descripción
Action Roguelike es un juego de acción roguelike en tercera persona que demuestra patrones profesionales de desarrollo en Unreal Engine 5 con C++. El proyecto implementa un sistema de acciones similar a GAS (Gameplay Ability System) pero más simplificado y educativo.

### 1.2 Características Principales
- Sistema de Acciones (Action System) - similar a GAS pero más simple
- Combate melee basado en AnimNotifies y overlap detection
- Sistema de Atributos (Health, HealthMax, AttackDamage)
- AI con Behavior Trees y Environment Query System (EQS)
- Sistema de Guardado y Carga (SaveGame)
- Multiplayer compatible (replicación de red)
- Projectiles data-oriented (sin Actors, pure data arrays)
- Asset Manager para carga asíncrona
- Performance Optimization (Significance Manager, Animation Budget Allocator)
- UI con UMG
- Enhanced Input System

### 1.3 Estructura Modular
```
Source/ActionRoguelike/
├── ActionSystem/        (Sistema de acciones y efectos)
├── AI/                 (IA, Behavior Trees, EQS)
├── Animation/          (AnimInstance, AnimNotifies)
├── Core/              (GameMode, GameState, interfaces)
├── Development/        (Cheat manager, developer settings)
├── Performance/        (Actor pooling, Significance, Tickables)
├── Pickups/           (PowerUps - Health, Credits, Actions)
├── Player/            (PlayerCharacter, Controller, State)
├── Projectiles/       (Projectiles, Movement, Data)
├── SaveSystem/        (SaveGame, SaveGameSubsystem)
├── UI/                (HUD, Widgets)
└── World/             (Level actors - Chest, Barrel, etc)
```

---

## 2. MÓDULO: ACTION SYSTEM

### 2.1 Descripción
Sistema de acciones que gestiona todas las habilidades, efectos y capacidades de los actores. Es similar a GAS pero más simplificado para propósitos educativos.

### 2.2 Clases Principales

#### **URogueAction** (Base class - Abstract concept)
- **Archivo**: `ActionSystem/RogueAction.h|cpp`
- **Hereda de**: `UObject`
- **Propósito**: Base para todas las acciones que pueden ejecutarse
- **Responsabilidades**:
  - Control del ciclo de vida de acciones (CanStart, StartAction, StopAction)
  - Manejo de GameplayTags (GrantsTags, BlockedTags)
  - Replicación de red (FActionRepData)
  - Icono para UI
  - bAutoStart para iniciar automáticamente al agregarse
- **Miembros Clave**:
  ```cpp
  FGameplayTagContainer GrantsTags;      // Tags que activa mientras corre
  FGameplayTagContainer BlockedTags;     // Tags que impiden inicio
  FActionRepData RepData;                // Replicación: bIsRunning, Instigator
  FGameplayTag ActivationTag;            // ID único para activación
  bool bAutoStart;                       // Inicia automáticamente
  ```
- **Métodos Virtuales**:
  - `bool CanStart_Implementation(AActor* Instigator)`
  - `void StartAction_Implementation(AActor* Instigator)`
  - `void StopAction_Implementation(AActor* Instigator)`

#### **URogueActionComponent** (Core component)
- **Archivo**: `ActionSystem/RogueActionComponent.h|cpp`
- **Hereda de**: `UActorComponent`
- **Propósito**: Componente que gestiona todas las acciones de un actor
- **Responsabilidades**:
  - Mantener lista de acciones activas
  - Inicializar acciones por defecto
  - Gestionar atributos (Health, etc)
  - Broadcast de eventos cuando acciones inician/detienen
  - Soporte de red (replicación)
- **Miembros Clave**:
  ```cpp
  TArray<URogueAction*> Actions;                    // Todas las acciones
  TMap<FGameplayTag, URogueAction*> CachedActions; // Lookup rápido
  FGameplayTagContainer ActiveGameplayTags;        // Tags activos
  FInstancedStruct AttributeSet;                   // Datos de atributos
  TMap<FGameplayTag, FRogueAttribute*> AttributeCache;
  ```
- **Métodos Públicos**:
  - `void AddAction(AActor* Instigator, TSubclassOf<URogueAction> ActionClass)`
  - `void RemoveAction(URogueAction* ActionToRemove)`
  - `URogueAction* GetAction(TSubclassOf<URogueAction> ActionClass)`
  - `bool StartActionByName(AActor* Instigator, FGameplayTag ActionName)`
  - `bool StopActionByName(AActor* Instigator, FGameplayTag ActionName)`
  - `bool ApplyAttributeChange(...)` - Modifica atributos
  - `FRogueAttribute* GetAttribute(FGameplayTag InAttributeTag)`
- **Eventos**:
  - `OnActionStarted` - Broadcast cuando acción inicia
  - `OnActionStopped` - Broadcast cuando acción termina

#### **URogueActionEffect** (Efecto base)
- **Archivo**: `ActionSystem/RogueActionEffect.h|cpp`
- **Hereda de**: `URogueAction`
- **Propósito**: Base para efectos con duración y ticks periódicos
- **Responsabilidades**:
  - Manejo de duración del efecto
  - Ejecución periódica (ticks cada X segundos)
  - Limpieza con timers
- **Miembros Clave**:
  ```cpp
  float Duration = 0.0f;      // Cuanto dura el efecto
  float Period = 0.0f;        // Cada cuanto tiempo se aplica
  FTimerHandle PeriodHandle;  // Timer para ticks
  FTimerHandle DurationHandle;// Timer para fin
  ```
- **Métodos Virtuales**:
  - `void ExecutePeriodicEffect_Implementation(AActor* Instigator)` - Llamado cada Period

#### **URogueActionEffect_Thorns**
- **Archivo**: `ActionSystem/RogueActionEffect_Thorns.h|cpp`
- **Hereda de**: `URogueActionEffect`
- **Propósito**: Efecto que refleja daño back al atacante
- **Responsabilidades**: Implementar lógica de reflejo de daño

#### **URogueAction_ProjectileAttack**
- **Archivo**: `ActionSystem/RogueAction_ProjectileAttack.h|cpp`
- **Hereda de**: `URogueAction`
- **Propósito**: Acción para disparar projectiles
- **Responsabilidades**:
  - Jugar animación de ataque
  - Calcular dirección del projectile (desde cámara/sweeps)
  - Instanciar o poolear projectiles
  - Soporte para data-oriented projectiles
- **Miembros Clave**:
  ```cpp
  URogueProjectileData* ProjectileConfig;
  TSubclassOf<AActor> ProjectileClass;
  UAnimMontage* AttackAnim;
  UNiagaraSystem* CastingEffect;
  USoundBase* CastingSound;
  bool bSupportsDataOrientedProjectiles;
  float SweepRadius;          // Para targeting bajo crosshair
  float SweepDistanceFallback;
  ```

#### **RogueAttributeSet - Estructura de Atributos**
- **Archivo**: `ActionSystem/RogueAttributeSet.h`
- **Tipo**: Struct (UStruct)
- **Propósito**: Definir sistema de atributos similar a GAS
- **Conceptos**:
  - Base (valor permanente): +Strength de level up, items permanentes
  - Modifier (temporal): Buffs, debuffs, items equipados
  - Valor final: `Base + Modifier` (clamped to 0)
- **Jerarquía**:
  ```
  FRogueAttributeSet (base abstract)
  ├── FRogueHealthAttributeSet (Health, HealthMax) - base para world actors
  ├── FRogueSurvivorAttributeSet (+ AttackDamage) - para player
  └── FRogueMonsterAttributeSet (+ AttackDamage) - para enemigos
  ```
- **Modificaciones**:
  ```cpp
  enum EAttributeModifyType
  {
      AddModifier,   // Suma al Modifier temporal
      AddBase,       // Suma al Base permanente
      OverrideBase   // Reemplaza Base
  }
  ```

### 2.3 Flujo de Ejecución - Acción Simple
```
1. AddAction() → Crea instancia de URogueAction
2. StartActionByName() → Valida tags, llama CanStart()
3. Si CanStart() = true:
   - Agrega tags a actor.ActiveGameplayTags
   - Llama StartAction_Implementation()
   - Broadcast OnActionStarted
4. StopActionByName() → Llama StopAction_Implementation()
   - Remueve tags
   - Broadcast OnActionStopped
```

### 2.4 Replicación de Red
- `URogueActionComponent` está replicado
- `FActionRepData` se replica (bIsRunning, Instigator)
- Acciones se replican en array `Actions`
- Los cambios de atributos se pueden invocar como RPCs

---

## 3. MÓDULO: PROJECTILES

### 3.1 Descripción
Sistema de projectiles con dos implementaciones:
1. **Actor-based**: Cada projectile es un AActor (tradicional)
2. **Data-oriented**: Projectiles como puro data en subsystem (experimental, optimizado)

### 3.2 Clases Principales

#### **ARogueProjectile** (Base Actor)
- **Archivo**: `Projectiles/RogueProjectile.h|cpp`
- **Hereda de**: `AActor + IRogueActorPoolingInterface`
- **Propósito**: Base para todos los projectiles actor-based
- **Responsabilidades**:
  - Componentes: Sphere (collision), Movement, Niagara (VFX), Audio
  - Detectar impactos y explotar
  - Reproducir efectos de impacto (VFX, sonido, shake cámara)
  - Soporte para actor pooling (reutilización)
- **Componentes**:
  ```cpp
  USphereComponent* SphereComp;
  URogueProjectileMovementComponent* MoveComp;
  UNiagaraComponent* NiagaraLoopComp;
  UAudioComponent* AudioComp;
  ```
- **Métodos Virtuales**:
  - `void OnActorHit(...)` - Callback de colisión
  - `void Explode_Implementation()` - Efecto de impacto
  - `void PoolBeginPlay_Implementation()` - Al sacar del pool
  - `void PoolEndPlay_Implementation()` - Al volver al pool

#### **ARogueProjectile_Magic**
- **Archivo**: `Projectiles/RogueProjectile_Magic.h|cpp`
- **Hereda de**: `ARogueProjectile`
- **Propósito**: Projectile mágico que aplica daño en área
- **Responsabilidades**: Explosion radius damage en impacto

#### **ARogueProjectile_Dash**
- **Archivo**: `Projectiles/RogueProjectile_Dash.h|cpp`
- **Hereda de**: `ARogueProjectile`
- **Propósito**: Projectile de dash - teleporta al instigador
- **Responsabilidades**:
  - Delay antes de detonar
  - Teleportar instigador a posición de impacto

#### **ARogueProjectile_Blackhole**
- **Archivo**: `Projectiles/RogueProjectile_Blackhole.h|cpp`
- **Hereda de**: `ARogueProjectile`
- **Propósito**: Projectile que atrae objetos
- **Responsabilidades**: Physics-based attraction

#### **URogueProjectileData** (DataAsset)
- **Archivo**: `Projectiles/RogueProjectileData.h|cpp`
- **Hereda de**: `UDataAsset`
- **Propósito**: Configuración de projectiles, reutilizable
- **Miembros**:
  ```cpp
  UNiagaraSystem* ProjectileEffect;   // VFX en vuelo
  UNiagaraSystem* ImpactEffect;       // VFX en impacto
  float InitialSpeed = 2000.0f;
  float Lifespan = 10.0f;
  float DamageCoefficient = 100;      // % del AttackDamage del instigador
  ```

#### **URogueProjectileMovementComponent**
- **Archivo**: `Projectiles/RogueProjectileMovementComponent.h|cpp`
- **Hereda de**: `UProjectileMovementComponent`
- **Propósito**: Movimiento personalizado de projectiles
- **Responsabilidades**: Lógica de movimiento específica

#### **URogueProjectilesSubsystem** (WorldSubsystem - Data-Oriented)
- **Archivo**: `Projectiles/RogueProjectilesSubsystem.h|cpp`
- **Hereda de**: `UTickableWorldSubsystem`
- **Propósito**: Gestionar projectiles como datos puros (sin Actors)
- **Responsabilidades**:
  - Mantener arrays de datos: `TArray<FProjectileInstance>`
  - Actualizar posiciones cada frame (Tick)
  - Detectar colisiones
  - Spawnar efectos de impacto
  - Replicación vía GameState
- **Estructuras**:
  ```cpp
  struct FProjectileInstance
  {
      FVector Position;   // Posición actual
      FVector Velocity;   // Velocidad
      uint32 ID;          // Para tracking
  };
  ```
- **Métodos**:
  - `int32 CreateProjectile(...)` - Crea nuevo projectile
  - `void RemoveProjectileID(uint32 IdToRemove)`
  - `virtual void Tick(float DeltaTime)` - Actualiza posiciones
  - `void SpawnImpactFX(...)`

### 3.3 Replicación de Projectiles Data-Oriented
- Datos replicados en `ARogueGameState.ProjectileData`
- Usa `FFastArraySerializer` para delta replication
- `FProjectileConfig` contiene metadata del projectile
- `FProjectileInstance` contiene datos por-frame

---

## 4. MÓDULO: AI

### 4.1 Descripción
Sistema de IA con Behavior Trees y EQS para control del comportamiento enemigo.

### 4.2 Clases Principales

#### **ARogueAICharacter**
- **Archivo**: `AI/RogueAICharacter.h|cpp`
- **Hereda de**: `ACharacter + IGenericTeamAgentInterface + IRogueSignificanceInterface`
- **Propósito**: Personaje de IA que es controlado por BehaviorTree
- **Responsabilidades**:
  - Poseer ActionComponent para ejecutar acciones
  - Mostrar barra de salud (widget)
  - Hit flash effect en daño
  - Reducción de trabajo de animación según LOD (Animation Budget)
  - Soporte para Significance LOD system
- **Componentes**:
  ```cpp
  URogueActionComponent* ActionComp;
  UAudioComponent* AttackSoundComp;
  UNiagaraComponent* AttackParticleComp;
  URogueWorldUserWidget* ActiveHealthBar;
  ```
- **Métodos**:
  - `AActor* GetTargetActor() const` - Retorna target actual
  - `void SignificanceLODChanged(int32 NewLOD)` - Para LOD system
  - `void OnReduceAnimationWork(...)` - Throttling de animaciones

#### **ARogueAIController**
- **Archivo**: `AI/RogueAIController.h|cpp`
- **Hereda de**: `AAIController`
- **Propósito**: Controla ARogueAICharacter via Behavior Tree
- **Responsabilidades**:
  - Ejecutar behavior tree
  - Percepción (UAIPerceptionComponent)
  - Actualizar blackboard cuando target cambia
- **Miembros**:
  ```cpp
  UBehaviorTree* BehaviorTree;
  UAIPerceptionComponent* PerceptionComp;
  ```

#### **Behavior Tree Nodes Personalizados**

**URogueBTTask_StartAction**
- **Archivo**: `AI/RogueBTTask_StartAction.h|cpp`
- **Hereda de**: `UBTTaskNode`
- **Propósito**: Nodo que inicia una acción en la IA
- **Uso**: En BT para iniciar ataque melee o ranged

**URogueBTService_StartAction**
- **Archivo**: `AI/RogueBTService_StartAction.h|cpp`
- **Hereda de**: `UBTService`
- **Propósito**: Service que mantiene acción ejecutándose

**URogueBTService_CheckAttackRange**
- **Archivo**: `AI/RogueBTService_CheckAttackRange.h|cpp`
- **Hereda de**: `UBTService`
- **Propósito**: Verifica si target está en rango de ataque

**URogueBTService_CheckHealth**
- **Archivo**: `AI/RogueBTService_CheckHealth.h|cpp`
- **Hereda de**: `UBTService`
- **Propósito**: Verifica salud y decide si huir/sanar

**URogueBTTask_RangedAttack**
- **Archivo**: `AI/RogueBTTask_RangedAttack.h|cpp`
- **Hereda de**: `UBTTaskNode`
- **Propósito**: Tarea para ataque ranged

**URogueBTTask_HealSelf**
- **Archivo**: `AI/RogueBTTask_HealSelf.h|cpp`
- **Hereda de**: `UBTTaskNode`
- **Propósito**: Tarea para que IA se cure

#### **Acciones de IA**

**URogueAction_MinionMeleeAttack**
- **Archivo**: `AI/RogueAction_MinionMeleeAttack.h|cpp`
- **Hereda de**: `URogueAction`
- **Propósito**: Acción melee para minions
- **Responsabilidades**:
  - Juega AnimMontage de ataque
  - Escucha OnMeleeOverlap desde AnimInstance
  - Aplica daño a overlaps detectados

**URogueAction_MinionRangedAttack**
- **Archivo**: `AI/RogueAction_MinionRangedAttack.h|cpp`
- **Hereda de**: `URogueAction`
- **Propósito**: Acción ranged para minions
- **Responsabilidades**: Dispara projectiles

### 4.3 Gameplay Tags para AI
```
"Action.Stunned"      - IA no puede atacar
"Action.Burning"      - Efecto de quemadura
"Action.Thorns"       - Refleja daño
etc.
```

---

## 5. MÓDULO: PLAYER

### 5.1 Descripción
Sistema de jugador: personaje, input, control y estado.

### 5.2 Clases Principales

#### **ARoguePlayerCharacter**
- **Archivo**: `Player/RoguePlayerCharacter.h|cpp`
- **Hereda de**: `ACharacter + IGenericTeamAgentInterface`
- **Propósito**: Personaje jugador controlable
- **Responsabilidades**:
  - Enhanced Input System (nuevo sistema de input)
  - Cámara con Spring Arm
  - ActionComponent para ejecutar acciones
  - Hit flash effect
  - Perception stimuli para que IA nos vea
- **Componentes**:
  ```cpp
  USpringArmComponent* SpringArmComp;
  UCameraComponent* CameraComp;
  URogueActionComponent* ActionComp;
  UAIPerceptionStimuliSourceComponent* PerceptionStimuliComp;
  UAudioComponent* AttackSoundsComp;
  ```
- **Enhanced Input Callbacks**:
  - `Move(const FInputActionInstance& Instance)`
  - `LookMouse(const FInputActionValue& InputValue)`
  - `LookStick(const FInputActionValue& InputValue)`
  - `StartActionByTag(...)` - Inicia acción con tag
  - `StopActionByTag(...)` - Detiene acción
- **Features**:
  - Detección de crosshair target para mejor targeting de projectiles
  - Async line traces para encontrar target
  - Widget para mostrar cuando IA nos ve (ClientOnSeenBy)

#### **ARoguePlayerController**
- **Archivo**: `Player/RoguePlayerController.h|cpp`
- **Hereda de**: `APlayerController`
- **Propósito**: Control del jugador
- **Responsabilidades**:
  - Interacción (E key)
  - Detect gamepad vs mouse input
  - Inicializar UI en BeginPlayingState
- **Componentes**:
  ```cpp
  URogueInteractionComponent* InteractionComp;
  ```
- **Métodos**:
  - `bool IsUsingGamepad() const`
  - `void PrimaryInteract()`
- **Eventos**:
  - `OnPlayerStateReceived` - Cuando PlayerState está disponible

#### **ARoguePlayerState**
- **Archivo**: `Player/RoguePlayerState.h|cpp`
- **Hereda de**: `APlayerState`
- **Propósito**: Estado del jugador que persiste
- **Responsabilidades**:
  - Mantener créditos
  - Record personal (mejor tiempo)
  - Integración con SaveGame
- **Miembros**:
  ```cpp
  int32 Credits;
  float PersonalRecordTime;
  ```
- **Métodos**:
  - `int32 GetCredits() const`
  - `void AddCredits(int32 Delta)`
  - `bool TryRemoveCredits(int32 Delta)` - Para gastar créditos
  - `bool UpdatePersonalRecord(float NewTime)`
- **Eventos**:
  - `OnCreditsChanged` - Cuando cambian créditos
  - `OnRecordTimeChanged` - Cuando se actualiza record

#### **URoguePlayerData** (DataAsset)
- **Archivo**: `Player/RoguePlayerData.h|cpp`
- **Hereda de**: `UDataAsset`
- **Propósito**: Configuración del jugador
- **Contenido**:
  ```cpp
  UInputAction* Input_Move;
  UInputAction* Input_LookMouse;
  UInputAction* Input_LookStick;
  UInputAction* Input_Jump;
  UInputAction* Input_Sprint;
  UInputAction* Input_Dash;
  UInputAction* Input_PrimaryAttack;
  UInputAction* Input_SecondaryAttack;
  ```

#### **URogueInteractionComponent**
- **Archivo**: `Player/RogueInteractionComponent.h|cpp`
- **Hereda de**: `UActorComponent`
- **Propósito**: Sistema de interacción (E key)
- **Responsabilidades**:
  - Detectar actores interactuables nearby
  - Mostrar prompt de interacción
  - Ejecutar interacción al presionar E

---

## 6. MÓDULO: ANIMATION

### 6.1 Descripción
Sistema de animación: AnimInstance y AnimNotifies personalizados.

### 6.2 Clases Principales

#### **URogueAnimInstance**
- **Archivo**: `Animation/RogueAnimInstance.h|cpp`
- **Hereda de**: `UAnimInstance`
- **Propósito**: AnimBlueprint que sincroniza datos de juego
- **Responsabilidades**:
  - Sincronizar bIsStunned con ActionComponent tags
  - Broadcast OnMeleeOverlap a través de AnimNotify
  - Reproducir sonidos de footstep en AnimNotifies
- **Miembros**:
  ```cpp
  bool bIsStunned;
  URogueActionComponent* ActionComp;
  FOnAnimNofify_MeleeOverlap OnMeleeOverlap; // Delegate
  ```
- **Métodos**:
  - `void NativeInitializeAnimation()` - Setup
  - `void NativeUpdateAnimation(float DeltaSeconds)` - Update cada frame
  - `bool HandleNotify(const FAnimNotifyEvent& AnimNotifyEvent)` - CustomNotifies

#### **URogueAnimNotifyState_Melee**
- **Archivo**: `Animation/RogueAnimNotifyState_Melee.h|cpp`
- **Hereda de**: `UAnimNotifyState`
- **Propósito**: AnimNotify que detecta overlaps melee durante animación
- **Responsabilidades**:
  - Mientras AnimNotify está activo, hace overlap checks cada frame
  - Broadcast resultado a OnMeleeOverlap
  - El Action_MinionMeleeAttack escucha esto y aplica daño
- **Métodos**:
  - `void NotifyBegin(...)` - Cuando notify inicia
  - `void NotifyTick(...)` - Cada frame del notify
  - `void NotifyEnd(...)` - Cuando notify termina

#### **URogueCurveAnimSubsystem**
- **Archivo**: `Animation/RogueCurveAnimSubsystem.h|cpp`
- **Hereda de**: `UTickableWorldSubsystem`
- **Propósito**: Sistema para animar valores via UCurveFloat
- **Responsabilidades**: Interpolación de curvas para animaciones personalizadas

---

## 7. MÓDULO: PICKUPS

### 7.1 Descripción
Sistema de recolectables: poder-ups, créditos, items.

### 7.2 Clases Principales

#### **ARoguePickupActor** (Base)
- **Archivo**: `Pickups/RoguePickupActor.h|cpp`
- **Hereda de**: `AActor + IRogueGameplayInterface`
- **Propósito**: Base para todos los pickups
- **Responsabilidades**:
  - Mostrar/esconder mesh
  - Respawn después de cierto tiempo
  - Auto pickup al caminar sobre (opcional)
  - Interacción manual (E key)
- **Componentes**:
  ```cpp
  USphereComponent* SphereComp;
  UStaticMeshComponent* MeshComp;
  ```
- **Miembros**:
  ```cpp
  bool bIsActive = true;
  bool bCanAutoPickup = false;
  float RespawnTime = 10.0f;
  ```

#### **ARoguePickupActor_HealthPotion**
- **Archivo**: `Pickups/RoguePickupActor_HealthPotion.h|cpp`
- **Hereda de**: `ARoguePickupActor`
- **Propósito**: Pickup de salud
- **Responsabilidades**: Restaura health al recoger

#### **ARoguePickupActor_Credits**
- **Archivo**: `Pickups/RoguePickupActor_Credits.h|cpp`
- **Hereda de**: `ARoguePickupActor`
- **Propósito**: Pickup de créditos
- **Responsabilidades**: Agrega créditos a PlayerState

#### **ARoguePickupActor_GrantAction**
- **Archivo**: `Pickups/RoguePickupActor_GrantAction.h|cpp`
- **Hereda de**: `ARoguePickupActor`
- **Propósito**: Pickup que otorga nueva acción
- **Responsabilidades**: Agrega acción a ActionComponent

---

## 8. MÓDULO: CORE

### 8.1 Descripción
Sistemas centrales: GameMode, GameState, interfaces, utilidades.

### 8.2 Clases Principales

#### **ARogueGameModeBase**
- **Archivo**: `Core/RogueGameModeBase.h|cpp`
- **Hereda de**: `AGameModeBase`
- **Propósito**: Lógica de juego - spawning de enemigos, créditos, powerups
- **Responsabilidades**:
  - Spawnear bots enemigos con EQS
  - Gestionar créditos disponibles para spawn
  - Spawnear powerups en inicio de partida
  - Auto-respawn de jugador (opcional)
  - Actor Pooling setup
- **Miembros Clave**:
  ```cpp
  UDataTable* MonsterTable;           // Tabla de enemigos disponibles
  UEnvQuery* SpawnBotQuery;           // EQS para ubicación spawn
  UCurveFloat* SpawnCreditCurve;      // Curva de créditos por tiempo
  float AvailableSpawnCredit = 0;     // Créditos actuales
  int32 InitialSpawnCredit = 50;      // Créditos iniciales
  UEnvQuery* PowerupSpawnQuery;       // EQS para powerups
  TArray<TSubclassOf<AActor>> PowerupClasses;
  TMap<TSubclassOf<AActor>, int32> ActorPoolClasses; // Actor pooling
  ```
- **Métodos**:
  - `void StartSpawningBots()`
  - `void OnBotSpawnQueryCompleted(...)` - Callback de EQS
  - `void OnMonsterLoaded(...)` - Asset async loaded
  - `virtual void OnActorKilled(AActor* VictimActor, AActor* Killer)` - Event cuando muere actor
  - `void RequestPrimedActors()` - Solicita actores al pool

#### **ARogueGameState**
- **Archivo**: `Core/RogueGameState.h|cpp`
- **Hereda de**: `AGameStateBase`
- **Propósito**: Estado del juego replicado
- **Responsabilidades**:
  - Mantener datos de projectiles data-oriented
  - Replicación via FFastArraySerializer
- **Miembros**:
  ```cpp
  FProjectileConfigArray ProjectileData; // Replicado
  ```
- **Métodos**:
  - `void ServerCreateProjectile(...)` - Crea projectile data-oriented

#### **IRogueGameplayInterface** (Interface)
- **Archivo**: `Core/RogueGameplayInterface.h|cpp`
- **Propósito**: Interfaz para actores interactuables
- **Métodos**:
  - `void OnActorLoaded()` - Después de cargar del SaveGame
  - `FText GetInteractText(AController* InstigatorController)` - Prompt
  - `void Interact(AController* InstigatorController)` - Ejecutar interacción
- **Implementadores**: ARoguePickupActor, ARogueTreasureChest, etc

#### **IRogueActionSystemInterface** (Interface)
- **Archivo**: `ActionSystem/RogueActionSystemInterface.h|cpp`
- **Propósito**: Interfaz para actores que tienen ActionComponent
- **Métodos**:
  - `URogueActionComponent* GetActionComponent() const`
- **Implementadores**: ARoguePlayerCharacter, ARogueAICharacter

#### **URogueGameplayFunctionLibrary**
- **Archivo**: `Core/RogueGameplayFunctionLibrary.h|cpp`
- **Hereda de**: `UBlueprintFunctionLibrary`
- **Propósito**: Utilidades globales accesibles desde Blueprint

#### **URogueMonsterData** (DataAsset)
- **Archivo**: `Core/RogueMonsterData.h|cpp`
- **Hereda de**: `UPrimaryDataAsset`
- **Propósito**: Configuración de enemigos para GameMode
- **Miembros**:
  ```cpp
  TSubclassOf<AActor> MonsterClass;      // Qué clase spawear
  TArray<TSubclassOf<URogueAction>> Actions; // Acciones iniciales
  UTexture2D* Icon;                       // Para UI
  ```
- **Asset Manager**: Usa `PrimaryAssetId("Monsters", ...)`

#### **RogueGameViewportClient**
- **Archivo**: `Core/RogueGameViewportClient.h|cpp`
- **Hereda de**: `UGameViewportClient`
- **Propósito**: Custom viewport client
- **Responsabilidades**: Setup de pantalla de carga personalizada

#### **RogueLoadingScreenSubsystem**
- **Archivo**: `Core/RogueLoadingScreenSubsystem.h|cpp`
- **Hereda de**: `UEngineSubsystem`
- **Propósito**: Manejar pantalla de carga personalizada

#### **FMonsterInfoRow** (Struct DataTable)
- **Archivo**: `Core/RogueGameModeBase.h`
- **Hereda de**: `FTableRowBase`
- **Propósito**: Fila en tabla de monstruos
- **Miembros**:
  ```cpp
  FPrimaryAssetId MonsterId;
  float Weight;          // Probabilidad relativa
  float SpawnCost;       // Créditos que cuesta
  float KillReward;      // Créditos otorgados al matar
  ```

---

## 9. MÓDULO: SAVE SYSTEM

### 9.1 Descripción
Sistema de guardado y carga de progreso del juego.

### 9.2 Clases Principales

#### **URogueSaveGame**
- **Archivo**: `SaveSystem/RogueSaveGame.h|cpp`
- **Hereda de**: `USaveGame`
- **Propósito**: Contenedor de datos a guardar
- **Responsabilidades**:
  - Guardar datos de jugadores
  - Guardar transformadas de actores
  - Guardar estado genérico de actores
- **Miembros**:
  ```cpp
  TArray<FPlayerSaveData> SavedPlayers;
  TMap<FName, FActorSaveData> SavedActorMap; // Actores por nivel
  ```
- **Métodos**:
  - `FPlayerSaveData* GetPlayerData(APlayerState* PlayerState)`

#### **FPlayerSaveData** (Struct)
- **Propósito**: Datos de jugador a persistir
- **Miembros**:
  ```cpp
  FString PlayerID;               // Steam ID, etc
  int32 Credits;
  float PersonalRecordTime;
  FVector Location;              // Posición al guardar
  FRotator Rotation;             // Rotación al guardar
  bool bResumeAtTransform;       // ¿Restaurar posición?
  ```

#### **FActorSaveData** (Struct)
- **Propósito**: Datos de actor a persistir
- **Miembros**:
  ```cpp
  FName ActorName;               // Identificador
  FTransform Transform;           // Posición/rotación/escala
  TArray<uint8> ByteData;        // Datos binarios (SaveGame marked vars)
  ```

#### **URogueSaveGameSubsystem**
- **Archivo**: `SaveSystem/RogueSaveGameSubsystem.h|cpp`
- **Hereda de**: `UWorldSubsystem`
- **Propósito**: Gestionar guardado/carga de juego
- **Responsabilidades**:
  - Buscar actores con IRogueGameplayInterface
  - Llamar SavePlayerState en PlayerStates
  - Llamar OnActorLoaded después de cargar
  - Handle async file I/O

#### **URogueSaveGameSettings**
- **Archivo**: `SaveSystem/RogueSaveGameSettings.h|cpp`
- **Hereda de**: `UDeveloperSettings`
- **Propósito**: Configuración de save game global

---

## 10. MÓDULO: UI

### 10.1 Descripción
Sistema de interfaz de usuario con UMG.

### 10.2 Clases Principales

#### **ARogueHUD**
- **Archivo**: `UI/RogueHUD.h|cpp`
- **Hereda de**: `AHUD`
- **Propósito**: HUD del juego
- **Responsabilidades**:
  - Toggle pause menu
- **Miembros**:
  ```cpp
  TSubclassOf<UUserWidget> PauseMenuClass;
  UUserWidget* PauseMenuInstance;
  ```

#### **URogueWorldUserWidget**
- **Archivo**: `UI/RogueWorldUserWidget.h|cpp`
- **Hereda de**: `UUserWidget`
- **Propósito**: Widget que se posiciona en el mundo 3D
- **Responsabilidades**:
  - Health bars sobre enemigos
  - Spotted indicators
  - Tooltips en mundo

#### **URogueEffectSlotWidget**
- **Archivo**: `UI/RogueEffectSlotWidget.h|cpp`
- **Hereda de**: `UUserWidget`
- **Propósito**: Widget para mostrar buffs/debuffs activos
- **Responsabilidades**:
  - Display íconos de efectos
  - Mostrar duración remaining

---

## 11. MÓDULO: WORLD

### 11.1 Descripción
Actores especiales del mundo: cofres, barriles explosivos, dummies.

### 11.2 Clases Principales

#### **ARogueTreasureChest**
- **Archivo**: `World/RogueTreasureChest.h|cpp`
- **Hereda de**: `AActor + IRogueGameplayInterface`
- **Propósito**: Cofre del tesoro interactuable
- **Responsabilidades**:
  - Interacción (abrir con E)
  - Animar apertura con curve
  - Reproducir efectos
  - Soporte SaveGame
- **Componentes**:
  ```cpp
  UStaticMeshComponent* BaseMesh;
  UStaticMeshComponent* LidMesh;
  UNiagaraComponent* OpenChestEffect;
  UAudioComponent* OpenChestSound;
  ```
- **Miembros**:
  ```cpp
  UCurveFloat* LidAnimCurve;
  bool bLidOpened;
  ```

#### **ARogueExplosiveBarrel**
- **Archivo**: `World/RogueExplosiveBarrel.h|cpp`
- **Hereda de**: `AActor`
- **Propósito**: Barril que explota por daño
- **Responsabilidades**:
  - Contar hits
  - Explotar después de X hits
  - Aplicar daño radial y fuerza física
- **Componentes**:
  ```cpp
  URogueActionComponent* ActionComp;  // Para atributos
  UStaticMeshComponent* MeshComp;
  URadialForceComponent* ForceComp;
  UNiagaraComponent* ExplosionComp;
  UNiagaraComponent* FlamesFXComp;
  ```
- **Lógica**: Escucha health damage via ActionComponent, cuenta hits

#### **ARogueTargetDummy**
- **Archivo**: `World/RogueTargetDummy.h|cpp`
- **Hereda de**: `AActor`
- **Propósito**: Target para testing (no daño, reset)

---

## 12. MÓDULO: PERFORMANCE

### 12.1 Descripción
Optimización de performance: actor pooling, LOD system, tickables.

### 12.2 Clases Principales

#### **IRogueActorPoolingInterface** (Interface)
- **Archivo**: `Performance/RogueActorPoolingInterface.h|cpp`
- **Propósito**: Interfaz para actores que soportan pooling
- **Métodos**:
  - `void PoolBeginPlay_Implementation()` - Cuando sale del pool
  - `void PoolEndPlay_Implementation()` - Cuando vuelve al pool

#### **URogueActorPoolingSubsystem**
- **Archivo**: `Performance/RogueActorPoolingSubsystem.h|cpp`
- **Hereda de**: `UWorldSubsystem`
- **Propósito**: Gestionar pool de actores (reutilización)
- **Responsabilidades**:
  - Mantener pool de actores libres: `TMap<TSubclassOf<AActor>, FActorPool>`
  - `SpawnActorPooled()` - Obtiene del pool o crea
  - `ReleaseToPool()` - Devuelve al pool
- **Beneficio**: Evita spawn/destroy costoso, reusa memoria
- **Métodos Públicos**:
  - `static AActor* SpawnActorPooled(...)` - Factory method
  - `static bool ReleaseToPool(AActor* Actor)`
  - `void PrimeActorPool(TSubclassOf<AActor>, int32 Amount)` - Pre-crear actores

#### **IRogueSignificanceInterface** (Interface)
- **Archivo**: `Performance/RogueSignificanceInterface.h|cpp`
- **Propósito**: Interfaz para actores que soportan LOD by distance
- **Métodos**:
  - `void SignificanceLODChanged(int32 NewLOD)` - Called cuando LOD cambia
- **Implementadores**: ARogueAICharacter

#### **URogueSignificanceManager**
- **Archivo**: `Performance/RogueSignificanceManager.h|cpp`
- **Hereda de**: `USignificanceManager`
- **Propósito**: Sistema de LOD personalizado
- **Responsabilidades**:
  - Categorizar actores por distancia
  - Calcular "significancia" (importancia)
  - Assign LOD level (0=high detail, 2=low detail)
  - Callback a IRogueSignificanceInterface
- **Setup**: Configurado en DefaultEngine.ini

#### **URogueSignificanceComponent**
- **Archivo**: `Performance/RogueSignificanceComponent.h|cpp`
- **Hereda de**: `UActorComponent`
- **Propósito**: Componente para aplicar significance a un actor

#### **URogueSignificanceSettings**
- **Archivo**: `Performance/RogueSignificanceSettings.h|cpp`
- **Hereda de**: `UDeveloperSettings`
- **Propósito**: Configuración de significance manager

#### **URogueTickablesSubsystem**
- **Archivo**: `Performance/RogueTickablesSubsystem.h|cpp`
- **Hereda de**: `UWorldSubsystem`
- **Propósito**: Gestionar objetos que hacen tick (UTickableGameObject)

---

## 13. MÓDULO: DEVELOPMENT

### 13.1 Descripción
Herramientas de desarrollo: cheat manager, developer settings.

### 13.2 Clases Principales

#### **ARogueCheatManager**
- **Archivo**: `Development/RogueCheatManager.h|cpp`
- **Hereda de**: `UCheatManager`
- **Propósito**: Comandos de consola para development
- **Responsabilidades**:
  - Debug commands
  - Spawn enemies
  - Test features
- **Métodos**: `UFUNCTION(exec, ...)` para comandos de consola

#### **URogueDeveloperSettings**
- **Archivo**: `Development/RogueDeveloperSettings.h|cpp`
- **Hereda de**: `UDeveloperSettings`
- **Propósito**: Configuración de desarrollo
- **Responsabilidades**:
  - Settings editables solo en editor
  - Debug features on/off

---

## 14. DEPENDENCIAS DE MÓDULOS

### 14.1 Mapa de Dependencias

```
┌─────────────────────────────────────────────────────────────┐
│                    MÓDULOS PRINCIPALES                       │
└─────────────────────────────────────────────────────────────┘

ActionSystem/
  ├─ RogueAction (base)
  ├─ RogueActionComponent (core manager)
  │  └─ RogueAttributeSet (datos)
  ├─ RogueActionEffect (extending RogueAction)
  │  ├─ RogueActionEffect_Thorns
  │  └─ (periodicity, duration)
  └─ RogueAction_ProjectileAttack
     └─ Projectiles/RogueProjectileData

Projectiles/
  ├─ ARogueProjectile (base actor)
  │  ├─ URogueProjectileMovementComponent
  │  ├─ Performance/RogueActorPoolingInterface
  │  └─ Core/RogueGameplayInterface
  ├─ ARogueProjectile_Magic
  ├─ ARogueProjectile_Dash
  ├─ ARogueProjectile_Blackhole
  ├─ URogueProjectileData (DataAsset)
  └─ URogueProjectilesSubsystem (data-oriented)
     └─ Core/RogueGameState (replication)

Player/
  ├─ ARoguePlayerCharacter
  │  ├─ ActionSystem/RogueActionComponent
  │  ├─ Enhanced Input System
  │  ├─ Camera (Spring Arm + Camera)
  │  └─ AI/Perception (UAIPerceptionStimuliSourceComponent)
  ├─ ARoguePlayerController
  │  ├─ URogueInteractionComponent
  │  └─ Player/RoguePlayerState
  ├─ ARoguePlayerState
  │  └─ SaveSystem/RogueSaveGame
  └─ URoguePlayerData (DataAsset)

AI/
  ├─ ARogueAICharacter
  │  ├─ ActionSystem/RogueActionComponent
  │  ├─ Performance/RogueSignificanceInterface
  │  ├─ Performance/RogueActorPoolingInterface
  │  └─ UI/RogueWorldUserWidget (health bar)
  ├─ ARogueAIController
  │  ├─ BehaviorTree
  │  ├─ UAIPerceptionComponent
  │  └─ Blackboard
  ├─ Custom BT Nodes
  │  ├─ URogueBTTask_StartAction
  │  ├─ URogueBTService_CheckAttackRange
  │  ├─ URogueBTService_CheckHealth
  │  ├─ URogueBTTask_RangedAttack
  │  └─ URogueBTTask_HealSelf
  ├─ URogueAction_MinionMeleeAttack
  │  └─ Animation/RogueAnimNotifyState_Melee
  └─ URogueAction_MinionRangedAttack
     └─ Projectiles/

Animation/
  ├─ URogueAnimInstance
  │  └─ ActionSystem/RogueActionComponent
  └─ URogueAnimNotifyState_Melee
     └─ (broadcasts OnMeleeOverlap)

Pickups/
  ├─ ARoguePickupActor (base)
  │  └─ Core/RogueGameplayInterface
  ├─ ARoguePickupActor_HealthPotion
  ├─ ARoguePickupActor_Credits
  └─ ARoguePickupActor_GrantAction

World/
  ├─ ARogueTreasureChest
  │  └─ Core/RogueGameplayInterface
  ├─ ARogueExplosiveBarrel
  │  └─ ActionSystem/RogueActionComponent
  └─ ARogueTargetDummy

Core/
  ├─ ARogueGameModeBase
  │  ├─ Core/RogueMonsterData
  │  ├─ EnvironmentQuery/EQS
  │  ├─ AssetManager
  │  └─ Performance/RogueActorPoolingSubsystem
  ├─ ARogueGameState
  │  ├─ Projectiles/RogueProjectilesSubsystem
  │  └─ FFastArraySerializer (replication)
  ├─ IRogueGameplayInterface (interface)
  ├─ IRogueActionSystemInterface (interface)
  ├─ RogueMonsterData (DataAsset)
  ├─ RogueGameViewportClient
  └─ RogueLoadingScreenSubsystem

SaveSystem/
  ├─ URogueSaveGame
  │  └─ FPlayerSaveData, FActorSaveData
  ├─ URogueSaveGameSubsystem
  │  └─ Core/RogueGameplayInterface
  └─ URogueSaveGameSettings

UI/
  ├─ ARogueHUD
  ├─ URogueWorldUserWidget
  └─ URogueEffectSlotWidget

Performance/
  ├─ IRogueActorPoolingInterface
  ├─ URogueActorPoolingSubsystem
  ├─ IRogueSignificanceInterface
  ├─ URogueSignificanceManager
  ├─ URogueSignificanceComponent
  └─ URogueTickablesSubsystem

Development/
  ├─ ARogueCheatManager
  └─ URogueDeveloperSettings
```

### 14.2 Diagrama de Flujo - Daño y Atributos

```
1. INFLICTING DAMAGE:
   Player/Enemy executes attack action (URogueAction_ProjectileAttack)
       ↓
   Projectile hits target
       ↓
   OnActorHit() broadcasts impact and applies damage via:
       ApplyAttributeChange(Health, -damage, attacker)
       ↓
2. ATTRIBUTE CHANGE:
   RogueActionComponent.ApplyAttributeChange(tag, magnitude, ...)
       ↓
   FRogueAttribute.OnAttributeChanged broadcast
       ↓
   Listeners (listeners register in Blueprint/C++)
       ↓
   ARogueExplosiveBarrel.OnHealthAttributeChanged()
   ARogueAICharacter.OnHealthAttributeChanged()
   ARoguePlayerCharacter.OnHealthAttributeChanged()
   etc.
       ↓
3. ON DEATH:
   OnHealthAttributeChanged detects Health <= 0
       ↓
   Execute death montage
       ↓
   Call GameMode.OnActorKilled()
       ↓
   Award credits, spawn loot, etc.
```

### 14.3 Flujo de Red (Multiplayer)

```
REPLICATION FLOW:
┌─ Server ──────────────────────────── Client(s) ─────────────┐
│                                                              │
│  ActionComponent (Replicated)                              │
│  ├─ Actions array                                          │
│  ├─ RepData (bIsRunning, Instigator)                       │
│  └─ ActiveGameplayTags                                     │
│                                                              │
│  ProjectileData (in GameState, FFastArraySerializer)       │
│  ├─ FProjectileConfig (metadata, replicated once)          │
│  └─ FProjectileConfigArray (delta serialized)              │
│                                                              │
│  PlayerState (Replicated)                                  │
│  ├─ Credits                                                │
│  └─ PersonalRecordTime                                     │
│                                                              │
│  Attributes (change triggered via ApplyAttributeChange)   │
│  └─ Server-side resolution, clients subscribe to events   │
│                                                              │
└────────────────────────────────────────────────────────────┘
```

---

## 15. INTERFACES Y GAMEPLAY TAGS

### 15.1 Interfaces Principales

| Interface | Propósito | Implementadores |
|-----------|-----------|-----------------|
| `IRogueGameplayInterface` | Actores interactuables | ARoguePickupActor, ARogueTreasureChest |
| `IRogueActionSystemInterface` | Acceso a ActionComponent | ARoguePlayerCharacter, ARogueAICharacter |
| `IRogueActorPoolingInterface` | Soporte actor pooling | ARogueProjectile, (custom actors) |
| `IRogueSignificanceInterface` | LOD by distance | ARogueAICharacter |
| `IGenericTeamAgentInterface` | Team system (enemy vs player) | ARoguePlayerCharacter, ARogueAICharacter |

### 15.2 GameplayTags

**Compartidos** (en `SharedGameplayTags.h|cpp`):
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
etc.
```

---

## 16. BUILD CONFIGURATION

### 16.1 Build.cs Dependencies
```cpp
PublicDependencyModuleNames: [
  "Core",
  "CoreUObject", 
  "Engine",
  "InputCore",
  "AIModule",          // BT, BTC, EQS
  "GameplayTasks",
  "UMG",              // UI
  "GameplayTags",     // GameplayTag system
  "OnlineSubsystem",  // Steam, etc
  "DeveloperSettings", // DeveloperSettings
  "SignificanceManager", // Distance-based LOD
  "EnhancedInput",     // New input system
  "Niagara",          // VFX
  "CoreOnline",
  "NetCore"           // Networking
]

PrivateDependencyModuleNames: [
  "MoviePlayer",              // Loading screen
  "RenderCore",               // PSO caching
  "AnimationBudgetAllocator"  // Animation LOD
]
```

### 16.2 Plugins Habilitados
```
- SignificanceManager (LOD system)
- OnlineSubsystemSteam (Steam)
- Text3D (3D text rendering)
- ModelingToolsEditorMode
- StaticMeshEditorModeling
- StateTree (state machine plugin)
- SlateInsights (UI debugging)
- GameplayInsights (gameplay debugging)
- TraceSourceFilters
- AnimationBudgetAllocator
- Iris (advanced replication)
```

---

## 17. ARQUITECTURA DE RED / REPLICACIÓN

### 17.1 Authority Model
```
Server-Authoritative:
- GameMode: Server only (no replica)
- GameState: Server replicates to clients
- PlayerState: Server replicates to owning client primarily
- Character: Server authoritative with client prediction (movement)
- Inventory/Actions: Replicated via components
```

### 17.2 RPC Patterns
```
Server RPCs:
- ServerStartAction()
- ServerStopAction()
- ServerCreateProjectile()

Client RPCs:
- ClientOnSeenBy() - Cuando IA ve al player

Multicast RPCs:
- MulticastPlayAttackFX()
```

### 17.3 Variable Replication
```cpp
UPROPERTY(Replicated)           // Replicada bidireccional
UPROPERTY(ReplicatedUsing=OnRep_XXX) // Con callback
UPROPERTY(Transient)            // No replicada
UPROPERTY(NotReplicated)        // Explícitamente no rep
```

---

## 18. DATA ASSETS Y CONFIGURACIÓN

### 18.1 DataAssets Principales
| Asset | Propósito |
|-------|----------|
| `URoguePlayerData` | Config de input actions |
| `URogueMonsterData` | Config de enemigos |
| `URogueProjectileData` | Config de projectiles |
| `URogueSaveGameSettings` | Config de save system |
| `RogueDeveloperSettings` | Debug/dev features |

### 18.2 Asset Manager
```
AssetManager Type: "Monsters"
Base Class: URogueMonsterData
Directory: /Game/ActionRoguelike/Monsters
Async Loading: Yes
Cook Rule: AlwaysCook
```

---

## 19. PERFORMANCE OPTIMIZATIONS

### 19.1 Techniques Implementadas

1. **Actor Pooling**
   - Projectiles reutilizados via pool
   - Evita spawn/destroy overhead
   - Pre-primed en GameMode

2. **Data-Oriented Projectiles**
   - Projectiles como datos en arrays (no Actors)
   - Ticking centralizado en URogueProjectilesSubsystem
   - Menos overhead de replicación

3. **Significance Manager**
   - LOD por distancia (AICharacter detail levels)
   - Distance-based LOD buckets

4. **Animation Budget Allocator**
   - Plugin de Epic
   - Throttle skeletal mesh updates lejos
   - Callback: `OnReduceAnimationWork()`
   - CVAR: `a.Budget.BudgetMs`

5. **PSO Precaching**
   - Pre-compile shader permutations
   - Evita stutter en primera carga
   - DefaultEngine.ini: `r.PSOPrecaching=1`

6. **Fast Array Replication**
   - Projectile data usa `FFastArraySerializer`
   - Delta serialization (solo cambios)

7. **Async Asset Loading**
   - MonsterData loaded asynchronously
   - Enemy spawning deferred hasta carga completa

### 19.2 Performance CVARs
```
a.Budget.Enabled=1              // Animation budgeter
a.Budget.BudgetMs=4.0           // MS para animaciones
a.Budget.Debug.Enabled=1        // Ver estadísticas
stat AnimationBudgetAllocator   // Profiling

r.PSOPrecaching=1               // PSO precache
r.ShaderPipelineCache.Enabled=1 // Shader cache

tick.AllowBatchedTicks=1        // Batch ticking
Slate.EnableGlobalInvalidation=1 // UI optimization

net.Iris.UseIrisReplication=1   // Advanced networking
```

---

## 20. MISSING / UNKNOWN ASPECTS

### 20.1 Areas que Requieren Inspección Manual

1. **Blueprint Integration Details**
   - Cuales blueprints usan RoguePlayerCharacter
   - Configuración específica de cada BP enemy
   - Animation montages exactas usadas
   - Status: NO EXAMINADO

2. **Content Examples**
   - Exacta estructura de /Game/ActionRoguelike/
   - Maps (TestLevel, MainMenu, etc)
   - Materiales y assets
   - Status: NO EXAMINADO

3. **Multiplayer Edge Cases**
   - Replicación de atributos en detalles
   - Timing de RPCs
   - Prediction/correction
   - Status: PARCIALMENTE DOCUMENTADO

4. **EQS Queries Exactas**
   - Exacta configuración de SpawnBotQuery
   - Exacta configuración de PowerupSpawnQuery
   - Generators y filters
   - Status: REFERENCIAS SOLO

5. **Animation Blend Space Details**
   - Exacta estructura de blend spaces
   - Transiciones custom
   - Status: ARCHIVO RogueAnimInstance REVISADO pero no animaciones

6. **Audio Implementation**
   - Soundbanks específicos
   - Spatial audio
   - Music management via BP_MusicManager
   - Status: PARCIALMENTE DOCUMENTADO

---

## 21. RESUMEN ESTADÍSTICAS

### 21.1 Métricas del Proyecto

| Métrica | Valor |
|---------|-------|
| Clases públicas (ACTIONROGUELIKE_API) | 60 |
| Módulos funcionales | 14 |
| Interfaces | 5 |
| Structs (UStruct) | ~20+ |
| DataAssets | 3+ |
| Subsystems | 6 |
| Files de código (.h + .cpp) | ~126 |
| Engine Version | 5.6 |

### 21.2 Dependencies Externas
- **Engine Modules**: Core, Engine, AIModule, UMG, GameplayTags, OnlineSubsystem, SignificanceManager, EnhancedInput, Niagara
- **Plugins Principales**: StateTree, SignificanceManager, OnlineSubsystemSteam, AnimationBudgetAllocator, Iris
- **Online**: Steam (OnlineSubsystemSteam)

---

## 22. PUNTOS DE ENTRADA PRINCIPALES

### 22.1 Para Nuevo Desarrollo

**Para agregar nueva acción:**
1. Crear clase derivada de URogueAction
2. Override StartAction_Implementation() y StopAction_Implementation()
3. Asignar ActivationTag
4. Agregar a DefaultActions en RogueActionComponent o en Blueprint

**Para agregar nuevo enemigo:**
1. Crear BP derivado de ARogueAICharacter
2. Crear URogueMonsterData asset
3. Agregar entrada a MonsterTable (DataTable)
4. Especificar actions y config en MonsterData

**Para agregar nuevo UI:**
1. Crear Widget derivado de UUserWidget
2. Para world space: derivar de URogueWorldUserWidget
3. Registrar en HUD o en personaje relevante

---

## CONCLUSIÓN

ActionRoguelike es una codebase educativa bien estructurada que demuestra:
- **Clean Architecture**: Módulos separados, interfaces claras
- **Game Systems**: Action system, attributes, AI, networking
- **Modern UE5**: Enhanced Input, Data-Oriented Programming, Iris replication
- **Performance**: Pooling, LOD, budgeting, PSO caching
- **Multiplayer**: Full network replication support

El proyecto es un excelente referente para aprender patterns profesionales en Unreal Engine C++.

