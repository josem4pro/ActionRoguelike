# ActionRoguelike - Plan de Complejidad Progresiva

**Fecha**: 2025-11-22
**Proyecto**: ActionRoguelike (Tom Looman - UE5.6)
**Proposito**: Definir 5 niveles progresivos de complejidad para aprendizaje estructurado

---

## 1. RESUMEN EJECUTIVO

Este documento define un plan de complejidad progresiva para ActionRoguelike, clasificando sus 15 sistemas en tres categorias (FUNDAMENTAL, INTERMEDIO, AVANZADO) y distribuyendolos en 5 niveles de aprendizaje.

**Objetivo**: Permitir que un estudiante/desarrollador pueda:
1. Comenzar con un juego minimo funcional (Plaza)
2. Agregar complejidad de forma incremental
3. Entender dependencias entre sistemas
4. Llegar al proyecto completo con comprension total

---

## 2. MATRIZ DE COMPLEJIDAD DE SISTEMAS

### 2.1 Clasificacion de los 15 Sistemas

| # | Sistema | Clasificacion | Justificacion |
|---|---------|---------------|---------------|
| 1 | Third-Person Movement + Enhanced Input | **FUNDAMENTAL** | Sin movimiento no hay juego |
| 2 | Basic Projectile System (ARogueProjectile_Magic) | **FUNDAMENTAL** | Mecanica core de ataque |
| 3 | AttributeComponent (Health, HealthMax, AttackDamage) | **FUNDAMENTAL** | Sin salud no hay game over |
| 4 | Action System (URogueAction, URogueActionComponent) | **INTERMEDIO** | Abstraccion elegante pero no critica para MVP |
| 5 | ActionEffects (URogueActionEffect, Thorns, Burning) | **INTERMEDIO** | Profundidad de gameplay, no esencial |
| 6 | GameplayTags (SharedGameplayTags) | **INTERMEDIO** | Organizacion, no funcionalidad core |
| 7 | UI/HUD (RogueHUD, RogueWorldUserWidget, RogueEffectSlotWidget) | **INTERMEDIO** | UX importante pero juego funciona sin UI compleja |
| 8 | SaveGame System (RogueSaveGame, RogueSaveGameSubsystem) | **INTERMEDIO** | Persistencia, no critico para sesion unica |
| 9 | AI con BT + EQS (ARogueAICharacter, ARogueAIController, Custom BTNodes) | **INTERMEDIO** | Enemigos pueden ser stateless inicialmente |
| 10 | GameMode avanzado (spawning economico, DataTables/DataAssets) | **INTERMEDIO** | Spawn manual es suficiente para MVP |
| 11 | Asset Manager & async loading | **AVANZADO** | Optimizacion, no funcionalidad |
| 12 | Multiplayer/Replication (Server-Authoritative, RPCs, FFastArraySerializer) | **AVANZADO** | Complejidad significativa, opcional |
| 13 | Performance (Actor Pooling, Significance Manager, Animation Budget) | **AVANZADO** | Optimizacion para escala |
| 14 | World Actors (Chest, Barrel, Pickups) | **INTERMEDIO** | Interactividad del mundo |
| 15 | Data-Oriented Projectiles (URogueProjectilesSubsystem) | **AVANZADO** | Optimizacion experimental |

### 2.2 Resumen por Categoria

| Categoria | Sistemas | Total |
|-----------|----------|-------|
| **FUNDAMENTAL** | 1, 2, 3 | 3 |
| **INTERMEDIO** | 4, 5, 6, 7, 8, 9, 10, 14 | 8 |
| **AVANZADO** | 11, 12, 13, 15 | 4 |

---

## 3. DEFINICION DE LOS 5 NIVELES PROGRESIVOS

---

### NIVEL 1: PLAZA (Hola Mundo Jugable)

**Metafora**: Una plaza vacia donde puedes caminar y practicar combate basico.

#### 3.1.1 Objetivo Pedagogico
- Entender el Character base de UE5
- Configurar Enhanced Input System
- Implementar sistema de atributos minimo (Health)
- Crear projectil basico funcional
- Experimentar el loop: atacar -> recibir dano -> morir

#### 3.1.2 Sistemas INCLUIDOS

| Sistema | Implementacion en Nivel 1 |
|---------|---------------------------|
| Third-Person Movement | **COMPLETO** - ARoguePlayerCharacter con SpringArm+Camera |
| Enhanced Input | **COMPLETO** - Move, Look, Jump, PrimaryAttack |
| Basic Projectile | **SIMPLIFICADO** - ARogueProjectile_Magic sin pooling |
| AttributeComponent | **SIMPLIFICADO** - Solo Health/HealthMax, sin modifiers |

#### 3.1.3 Sistemas EXCLUIDOS o DESACTIVADOS

| Sistema | Estado | Razon |
|---------|--------|-------|
| Action System completo | EXCLUIDO | Usar input directo a funciones |
| ActionEffects | EXCLUIDO | Sin buffs/debuffs |
| GameplayTags | MINIMO | Solo tags basicos hardcoded |
| UI/HUD | MINIMO | Solo health bar en HUD |
| SaveGame | EXCLUIDO | Sin persistencia |
| AI con BT + EQS | EXCLUIDO | Usar target dummy estatico |
| GameMode avanzado | EXCLUIDO | Spawn manual en nivel |
| Asset Manager | EXCLUIDO | Carga sincrona |
| Multiplayer | EXCLUIDO | Solo singleplayer |
| Performance systems | EXCLUIDO | Sin optimizacion |
| World Actors | PARCIAL | Solo ARogueTargetDummy |
| Data-Oriented Projectiles | EXCLUIDO | Actor-based unicamente |

#### 3.1.4 Archivos Activos en Nivel 1

```
Source/ActionRoguelike/
├── Player/
│   ├── RoguePlayerCharacter.h|cpp (SIMPLIFICADO)
│   ├── RoguePlayerController.h|cpp (MINIMO)
│   └── RoguePlayerData.h|cpp (solo inputs)
├── ActionSystem/
│   ├── RogueActionComponent.h|cpp (solo atributos, sin actions)
│   └── RogueAttributeSet.h (solo FRogueHealthAttributeSet)
├── Projectiles/
│   ├── RogueProjectile.h|cpp (base)
│   └── RogueProjectile_Magic.h|cpp
├── Core/
│   └── RogueGameModeBase.h|cpp (STUB vacio)
├── World/
│   └── RogueTargetDummy.h|cpp
└── SharedGameplayTags.h|cpp (MINIMO)
```

#### 3.1.5 Dependencias Criticas
- Ninguna dependencia externa compleja
- Enhanced Input plugin habilitado
- Niagara para VFX de projectiles (opcional, puede usar Cascade)

#### 3.1.6 Riesgos / Gotchas

| Riesgo | Mitigacion |
|--------|------------|
| Enhanced Input requiere IMC configurado | Crear IMC minimo en constructor |
| Health a 0 no tiene efecto sin lógica | Implementar OnHealthChanged -> DestroyActor o GameOver |
| Projectiles atraviesan todo | Configurar collision profiles correctamente |
| Sin respawn el juego se detiene | Mostrar pantalla de Game Over simple |

#### 3.1.7 Criterios de Aceptacion Nivel 1
- [ ] Personaje se mueve con WASD y mira con mouse
- [ ] Click dispara projectil que viaja en direccion de la camara
- [ ] Target Dummy recibe dano y muestra hit flash
- [ ] Player tiene barra de salud visible
- [ ] Al llegar a 0 HP se muestra "Game Over"

---

### NIVEL 2: PLAZA VIVA (Entorno con Vida)

**Metafora**: La plaza ahora tiene vendedores, objetos y visitantes hostiles.

#### 3.2.1 Objetivo Pedagogico
- Introducir IA enemiga basica (sin BT, solo state machine simple)
- Sistema de pickups funcional
- Interaccion con objetos del mundo (Chest, Barrel)
- UI mejorada con widgets en mundo 3D

#### 3.2.2 Sistemas INCLUIDOS (incrementales)

| Sistema | Implementacion en Nivel 2 |
|---------|---------------------------|
| AI basica | ARogueAICharacter con IA SIMPLE (MoveTo + Attack sin BT) |
| World Actors | ARogueTreasureChest, ARogueExplosiveBarrel |
| Pickups | ARoguePickupActor_HealthPotion |
| UI mejorada | URogueWorldUserWidget para health bars 3D |
| IRogueGameplayInterface | Interfaz de interaccion (E key) |

#### 3.2.3 Sistemas Promovidos de Nivel 1

| Sistema | Cambio |
|---------|--------|
| AttributeComponent | Agregar AttackDamage |
| GameMode | Agregar OnActorKilled() para tracking |
| Player | Agregar URogueInteractionComponent |

#### 3.2.4 Archivos Nuevos en Nivel 2

```
Source/ActionRoguelike/
├── AI/
│   ├── RogueAICharacter.h|cpp (SIMPLIFICADO - sin BT)
│   └── RogueAIController.h|cpp (MoveTo simple)
├── World/
│   ├── RogueTreasureChest.h|cpp
│   └── RogueExplosiveBarrel.h|cpp
├── Pickups/
│   ├── RoguePickupActor.h|cpp
│   └── RoguePickupActor_HealthPotion.h|cpp
├── Player/
│   └── RogueInteractionComponent.h|cpp
├── Core/
│   └── RogueGameplayInterface.h|cpp
└── UI/
    └── RogueWorldUserWidget.h|cpp
```

#### 3.2.5 Dependencias Criticas
- AIModule para AAIController basico
- Collision channels para interaccion

#### 3.2.6 Riesgos / Gotchas

| Riesgo | Mitigacion |
|--------|------------|
| IA sin BT puede ser muy tonta | Implementar FSM simple con 3 estados: Idle, Chase, Attack |
| Pickups sin respawn agotan recursos | Implementar timer de respawn simple |
| Barril explota infinitamente | Usar flag bHasExploded para single explosion |
| Health bars flotan incorrectamente | Usar WidgetComponent con Screen space |

#### 3.2.7 Criterios de Aceptacion Nivel 2
- [ ] Enemigos aparecen y persiguen al jugador
- [ ] Enemigos atacan cuando estan cerca
- [ ] Cofre se abre con E y da recompensa
- [ ] Barril explota al recibir dano
- [ ] Pocion restaura salud
- [ ] Enemigos muestran barra de salud sobre ellos
- [ ] Matar enemigo incrementa contador

---

### NIVEL 3: BARRIO (Sensacion Roguelike)

**Metafora**: Un barrio con reglas, economia y progresion.

#### 3.3.1 Objetivo Pedagogico
- Action System completo (la estrella arquitectonica del proyecto)
- Efectos con duracion (Burning, Thorns)
- SaveGame basico para persistencia
- GameplayTags como sistema de organizacion
- UI de buffs/debuffs activos
- AI con Behavior Trees basicos

#### 3.3.2 Sistemas INCLUIDOS (incrementales)

| Sistema | Implementacion en Nivel 3 |
|---------|---------------------------|
| Action System | URogueAction completo con CanStart/StartAction/StopAction |
| ActionEffects | URogueActionEffect, URogueActionEffect_Thorns |
| GameplayTags | SharedGameplayTags completo |
| SaveGame | URogueSaveGame + URogueSaveGameSubsystem |
| AI con BT | Behavior Tree basico (Chase -> Attack -> Flee at low HP) |
| UI de efectos | URogueEffectSlotWidget |
| Credits system | ARoguePlayerState.Credits |

#### 3.3.3 Archivos Nuevos en Nivel 3

```
Source/ActionRoguelike/
├── ActionSystem/
│   ├── RogueAction.h|cpp (COMPLETO)
│   ├── RogueActionEffect.h|cpp
│   ├── RogueActionEffect_Thorns.h|cpp
│   └── RogueAction_ProjectileAttack.h|cpp
├── AI/
│   ├── RogueBTTask_RangedAttack.h|cpp
│   ├── RogueBTService_CheckHealth.h|cpp
│   ├── RogueBTService_CheckAttackRange.h|cpp
│   └── RogueAction_MinionMeleeAttack.h|cpp
├── SaveSystem/
│   ├── RogueSaveGame.h|cpp
│   ├── RogueSaveGameSubsystem.h|cpp
│   └── RogueSaveGameSettings.h|cpp
├── Player/
│   └── RoguePlayerState.h|cpp (con Credits)
├── Pickups/
│   ├── RoguePickupActor_Credits.h|cpp
│   └── RoguePickupActor_GrantAction.h|cpp
└── UI/
    └── RogueEffectSlotWidget.h|cpp
```

#### 3.3.4 Dependencias Criticas
- GameplayTags module
- GameplayTasks module (para BT)
- Behavior Tree assets en Content

#### 3.3.5 Riesgos / Gotchas

| Riesgo | Mitigacion |
|--------|------------|
| Actions pueden quedarse corriendo al morir | Implementar StopAllActions() en death |
| SaveGame corrompe datos si struct cambia | Usar versionado en USaveGame |
| BT no funciona sin Blackboard | Crear Blackboard con keys: TargetActor, Health, bInRange |
| Tags collision (mismo tag para cosas distintas) | Usar jerarquia: Action.PrimaryAttack, Status.Stunned |

#### 3.3.6 Criterios de Aceptacion Nivel 3
- [ ] Acciones usan sistema CanStart/StartAction/StopAction
- [ ] Tags bloquean acciones correctamente (stunned no puede atacar)
- [ ] Efecto Thorns refleja dano al atacante
- [ ] Guardar juego preserva posicion, creditos, acciones desbloqueadas
- [ ] Cargar juego restaura estado correctamente
- [ ] AI usa BT para tomar decisiones (huye con poca vida)
- [ ] UI muestra buffs/debuffs activos con duracion

---

### NIVEL 4: DISTRITO CONECTADO (Multiplayer y Datos)

**Metafora**: Distritos conectados que comparten informacion y recursos.

#### 3.4.1 Objetivo Pedagogico
- Arquitectura Server-Authoritative
- Replicacion de acciones y atributos
- RPCs (Server, Client, Multicast)
- GameMode avanzado con spawning economico
- DataTables y Asset Manager
- EQS para ubicacion de spawns

#### 3.4.2 Sistemas INCLUIDOS (incrementales)

| Sistema | Implementacion en Nivel 4 |
|---------|---------------------------|
| Multiplayer | Replicacion completa de ActionComponent |
| GameMode avanzado | Spawn credits, MonsterTable, EQS |
| Asset Manager | Async loading de URogueMonsterData |
| EQS | SpawnBotQuery, PowerupSpawnQuery |
| RPCs | Server/Client para acciones, Multicast para FX |

#### 3.4.3 Cambios de Replicacion

```cpp
// ActionComponent - agregar replicacion
UPROPERTY(ReplicatedUsing=OnRep_Actions)
TArray<TObjectPtr<URogueAction>> Actions;

UFUNCTION(Server, Reliable)
void ServerStartAction(AActor* Instigator, FGameplayTag ActionName);

UFUNCTION(Server, Reliable)
void ServerStopAction(AActor* Instigator, FGameplayTag ActionName);
```

#### 3.4.4 Archivos Nuevos/Modificados en Nivel 4

```
Source/ActionRoguelike/
├── Core/
│   ├── RogueGameModeBase.h|cpp (COMPLETO con EQS)
│   ├── RogueGameState.h|cpp
│   └── RogueMonsterData.h|cpp
├── ActionSystem/
│   └── RogueActionComponent.cpp (agregar RPCs)
└── AI/
    └── RogueBTTask_StartAction.h|cpp
```

#### 3.4.5 Dependencias Criticas
- OnlineSubsystem (Steam recomendado)
- AIModule con EQS
- DataTable assets en Content

#### 3.4.6 Riesgos / Gotchas

| Riesgo | Mitigacion |
|--------|------------|
| RPCs no llegan si no hay HasAuthority check | Siempre verificar authority antes de ejecutar |
| Actions corren en cliente y servidor duplicado | Solo servidor ejecuta, cliente predice |
| EQS query falla sin Navmesh | Generar navmesh en nivel |
| Async load no completa antes de spawn | Usar callbacks OnMonsterLoaded |
| PlayerState no disponible inmediatamente | Esperar OnPlayerStateReceived |

#### 3.4.7 Criterios de Aceptacion Nivel 4
- [ ] 2 jugadores pueden conectarse al servidor
- [ ] Acciones de un jugador se ven en el otro
- [ ] Dano se calcula solo en servidor
- [ ] Enemigos aparecen via EQS en posiciones validas
- [ ] Spawning respeta economia de creditos
- [ ] MonsterData se carga asincrónicamente

---

### NIVEL 5: CIUDAD COMPLETA (Proyecto Original)

**Metafora**: La metropolis completa con toda la infraestructura optimizada.

#### 3.5.1 Objetivo Pedagogico
- Optimizaciones de produccion
- Actor Pooling para objetos frecuentes
- Significance Manager para LOD de AI
- Animation Budget Allocator
- Data-Oriented Projectiles (experimental)
- PSO Precaching y shader warmup
- FFastArraySerializer para replicacion eficiente

#### 3.5.2 Sistemas INCLUIDOS (incrementales)

| Sistema | Implementacion en Nivel 5 |
|---------|---------------------------|
| Actor Pooling | URogueActorPoolingSubsystem para projectiles |
| Significance Manager | LOD para ARogueAICharacter |
| Animation Budget | Plugin habilitado, throttling |
| Data-Oriented Projectiles | URogueProjectilesSubsystem |
| FFastArraySerializer | FProjectileConfigArray en GameState |
| Development Tools | URogueCheatManager, URogueDeveloperSettings |

#### 3.5.3 Archivos Nuevos en Nivel 5

```
Source/ActionRoguelike/
├── Performance/
│   ├── RogueActorPoolingSubsystem.h|cpp
│   ├── RogueActorPoolingInterface.h|cpp
│   ├── RogueSignificanceManager.h|cpp
│   ├── RogueSignificanceComponent.h|cpp
│   ├── RogueSignificanceInterface.h|cpp
│   ├── RogueSignificanceSettings.h|cpp
│   └── RogueTickablesSubsystem.h|cpp
├── Projectiles/
│   ├── RogueProjectilesSubsystem.h|cpp
│   └── RogueProjectileData.h|cpp
├── Development/
│   ├── RogueCheatManager.h|cpp
│   └── RogueDeveloperSettings.h|cpp
├── Core/
│   ├── RogueGameViewportClient.h|cpp
│   └── RogueLoadingScreenSubsystem.h|cpp
└── Animation/
    ├── RogueAnimInstance.h|cpp
    ├── RogueAnimNotifyState_Melee.h|cpp
    └── RogueCurveAnimSubsystem.h|cpp
```

#### 3.5.4 Dependencias Criticas
- SignificanceManager plugin
- AnimationBudgetAllocator plugin
- Iris plugin (networking avanzado)
- MoviePlayer module (loading screens)

#### 3.5.5 Riesgos / Gotchas

| Riesgo | Mitigacion |
|--------|------------|
| Pooling devuelve actor en estado invalido | Implementar PoolBeginPlay/PoolEndPlay correctamente |
| Significance desactiva AI necesaria | Configurar buckets apropiados (nunca LOD 2 a < 2000 units) |
| Data-Oriented projectiles no colisionan | Tick manual de collision spheres |
| FFastArraySerializer marca todo dirty | Solo marcar elementos cambiados |
| Animation Budget mata animaciones importantes | Excluir player de budget |

#### 3.5.6 Criterios de Aceptacion Nivel 5
- [ ] 100+ projectiles sin caida de FPS (pooling)
- [ ] 50+ enemigos con LOD dinamico (significance)
- [ ] Animaciones se throttlean a distancia
- [ ] Data-Oriented projectiles funcionan en MP
- [ ] Loading screen custom aparece en transiciones
- [ ] Cheats funcionan en Development builds
- [ ] PSO precaching elimina shader stutters

---

## 4. MAPA DE DEPENDENCIAS ENTRE NIVELES

```
┌─────────────────────────────────────────────────────────────────┐
│                         NIVEL 5                                  │
│                    CIUDAD COMPLETA                               │
│  [Actor Pooling] [Significance] [Data-Oriented] [Animation Budget]│
│                           ▲                                      │
│                           │                                      │
├─────────────────────────────────────────────────────────────────┤
│                         NIVEL 4                                  │
│                   DISTRITO CONECTADO                             │
│     [Multiplayer] [GameMode+EQS] [Asset Manager] [RPCs]         │
│                           ▲                                      │
│                           │                                      │
├─────────────────────────────────────────────────────────────────┤
│                         NIVEL 3                                  │
│                         BARRIO                                   │
│  [Action System] [ActionEffects] [SaveGame] [BT] [GameplayTags] │
│                           ▲                                      │
│                           │                                      │
├─────────────────────────────────────────────────────────────────┤
│                         NIVEL 2                                  │
│                       PLAZA VIVA                                 │
│     [AI Simple] [World Actors] [Pickups] [Interaction] [UI 3D]  │
│                           ▲                                      │
│                           │                                      │
├─────────────────────────────────────────────────────────────────┤
│                         NIVEL 1                                  │
│                          PLAZA                                   │
│     [Movement] [Enhanced Input] [Projectile] [Health/Damage]    │
└─────────────────────────────────────────────────────────────────┘
```

### 4.1 Dependencias Criticas por Transicion

| Transicion | Dependencia Critica | Riesgo si Falta |
|------------|---------------------|-----------------|
| 1 -> 2 | IRogueGameplayInterface | Interaccion no funciona |
| 1 -> 2 | Collision Channels | Pickups/Chests no detectan player |
| 2 -> 3 | FGameplayTagContainer | Actions no pueden usar tags |
| 2 -> 3 | URogueAction base | Sistema de acciones incompleto |
| 3 -> 4 | GetLifetimeReplicatedProps | Variables no replican |
| 3 -> 4 | ROLE_Authority checks | Logica corre duplicada |
| 4 -> 5 | IRogueActorPoolingInterface | Pooling no funciona |
| 4 -> 5 | SignificanceManager plugin | LOD no disponible |

---

## 5. REFACTORS SUGERIDOS POR TRANSICION

### 5.1 Transicion Nivel 1 -> Nivel 2

#### Codigo que se ACTIVA
```cpp
// Activar en RoguePlayerController
UPROPERTY(VisibleAnywhere)
TObjectPtr<URogueInteractionComponent> InteractionComp;

// Activar en RogueGameModeBase
virtual void OnActorKilled(AActor* VictimActor, AActor* Killer);
```

#### Refactors Necesarios
| Refactor | Descripcion | Impacto |
|----------|-------------|---------|
| Extraer IRogueGameplayInterface | Crear interfaz para todos los interactuables | MEDIO |
| Agregar AIController base | Clase simple para IA sin BT | BAJO |
| Collision Channel "Interactable" | Canal dedicado para overlap de interaccion | BAJO |

#### Posibles Breaking Changes
- Input mapping debe incluir "Interact" action
- WorldUserWidget requiere attachment point configurado

---

### 5.2 Transicion Nivel 2 -> Nivel 3

#### Codigo que se ACTIVA
```cpp
// En RogueActionComponent - antes solo atributos, ahora acciones completas
UPROPERTY(EditAnywhere, Category = "Actions")
TArray<TSubclassOf<URogueAction>> DefaultActions;

// Sistema de Save
UPROPERTY(SaveGame)  // Marcar variables que persisten
int32 Credits;
```

#### Refactors Necesarios
| Refactor | Descripcion | Impacto |
|----------|-------------|---------|
| Migrar attacks a URogueAction | Input ya no llama directamente a SpawnProjectile | ALTO |
| Extraer atributos a FInstancedStruct | Permitir attribute sets intercambiables | ALTO |
| Implementar tag blocking | CanStart() verifica BlockedTags | MEDIO |
| StopAllActions() en death | Cleanup de acciones al morir | BAJO |

#### Posibles Breaking Changes
- **CRITICO**: Todo input de ataque debe pasar por StartActionByName()
- SaveGame struct changes requieren migration code
- BT assets deben existir en Content antes de iniciar

---

### 5.3 Transicion Nivel 3 -> Nivel 4

#### Codigo que se ACTIVA
```cpp
// Replicacion en ActionComponent
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

UFUNCTION(Server, Reliable)
void ServerStartAction(AActor* Instigator, FGameplayTag ActionName);
```

#### Refactors Necesarios
| Refactor | Descripcion | Impacto |
|----------|-------------|---------|
| Authority checks en todas las funciones de logica | if(HasAuthority()) para server-only | CRITICO |
| RPCs para acciones | ServerStartAction/ServerStopAction | ALTO |
| OnRep functions | OnRep_Actions para sync de clientes | ALTO |
| Separar cosmetic vs gameplay | FX en cliente, logica en server | MEDIO |

#### Posibles Breaking Changes
- **CRITICO**: Sin authority checks, logica corre duplicada
- PlayerState debe estar replicado antes de usar
- EQS queries deben tener navmesh generado

---

### 5.4 Transicion Nivel 4 -> Nivel 5

#### Codigo que se ACTIVA
```cpp
// Actor Pooling
static AActor* SpawnActorPooled(const UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform, ESpawnActorCollisionHandlingMethod SpawnHandling);

// Significance
virtual void SignificanceLODChanged(int32 NewLOD) override;
```

#### Refactors Necesarios
| Refactor | Descripcion | Impacto |
|----------|-------------|---------|
| Reemplazar SpawnActor con SpawnActorPooled | Projectiles y enemigos | ALTO |
| Implementar IRogueActorPoolingInterface | PoolBeginPlay/PoolEndPlay | ALTO |
| Configurar Significance buckets | Distancias para LOD 0/1/2 | MEDIO |
| Data-Oriented projectiles | Migrar de Actor a FProjectileInstance | MUY ALTO |

#### Posibles Breaking Changes
- Pooled actors NO llaman BeginPlay/EndPlay normal
- Significance puede desactivar ticks - verificar que logica critica no dependa de tick
- Data-Oriented projectiles rompen compatibilidad con Actor-based - usar feature flag

---

## 6. ESTRATEGIA DE IMPLEMENTACION RECOMENDADA

### 6.1 Enfoque Feature Flags

Para manejar la complejidad progresiva sin branches separados:

```cpp
// En RogueDeveloperSettings.h
UPROPERTY(EditDefaultsOnly, Category="Progressive Complexity")
int32 ComplexityLevel = 5; // 1-5

// En codigo
if (GetDefault<URogueDeveloperSettings>()->ComplexityLevel >= 3)
{
    // Usar Action System completo
    ActionComp->StartActionByName(this, SharedGameplayTags::Action_PrimaryAttack);
}
else
{
    // Usar ataque directo (Nivel 1-2)
    SpawnProjectileDirect();
}
```

### 6.2 Checklist de Validacion por Nivel

#### Nivel 1 Checklist
- [ ] Build compila sin errores
- [ ] Player se mueve correctamente
- [ ] Projectil se dispara y colisiona
- [ ] Health disminuye al recibir dano
- [ ] Game Over aparece al morir

#### Nivel 2 Checklist
- [ ] AI persigue al player
- [ ] Pickups funcionan
- [ ] Chest se abre con E
- [ ] Barrel explota
- [ ] Health bars 3D visibles

#### Nivel 3 Checklist
- [ ] Actions se inician/detienen correctamente
- [ ] Tags bloquean acciones
- [ ] Save/Load funciona
- [ ] BT ejecuta comportamiento
- [ ] UI de buffs funciona

#### Nivel 4 Checklist
- [ ] Conexion cliente-servidor funciona
- [ ] Acciones replican correctamente
- [ ] Spawning EQS funciona
- [ ] No hay desync de estado

#### Nivel 5 Checklist
- [ ] Pooling reduce allocations
- [ ] Significance reduce load
- [ ] Animation budget funciona
- [ ] No hay memory leaks
- [ ] Performance targets met

---

## 7. METRICAS DE EXITO

| Nivel | Metrica | Target |
|-------|---------|--------|
| 1 | Tiempo para juego funcional | < 4 horas |
| 2 | Tiempo para agregar IA | < 3 horas |
| 3 | Tiempo para Action System | < 6 horas |
| 4 | Tiempo para multiplayer basico | < 8 horas |
| 5 | FPS con 100 projectiles | > 60 FPS |
| 5 | FPS con 50 enemigos | > 60 FPS |
| 5 | Memory con pooling vs sin | < 50% uso |

---

## 8. CONCLUSION

Este plan de complejidad progresiva permite:

1. **Aprendizaje Estructurado**: Comenzar simple, agregar complejidad gradualmente
2. **Comprension de Dependencias**: Entender por que cada sistema existe
3. **Refactoring Consciente**: Saber que cambiar en cada transicion
4. **Reduccion de Riesgo**: Cada nivel es un checkpoint estable
5. **Flexibilidad**: Feature flags permiten activar/desactivar sistemas

**Recomendacion Final**: Implementar cada nivel completamente antes de avanzar. Resistir la tentacion de "saltarse" al nivel 5 - cada nivel construye sobre el anterior y la comprension acumulada es invaluable.

---

**Documento generado**: 2025-11-22
**Basado en**: ActionRoguelike Architecture Map v1.0
**Autor del analisis**: Claude Code (refactoring specialist)
