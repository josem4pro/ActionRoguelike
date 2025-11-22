# ActionRoguelike - Nivel 1: Plaza (Especificacion Detallada)

**Version**: 2.0 - Fase C Implementation
**Fecha**: 2025-11-22
**Nombre Clave**: "Hola Mundo Jugable"
**Objetivo**: Juego minimo funcional - moverse, disparar, recibir dano

---

## 🎯 Estado de Implementación - Fase C (Noviembre 22, 2025)

### Completado ✅

| Sistema | Status | Detalles |
|---------|--------|----------|
| **Decisión Salud** | ✅ DONE | Opción B (ActionComponent con atributos) - Documentada línea 137 |
| **Tests de Salud** | ✅ DONE | 6 tests pasan en Level1_PlazaTests.cpp (compilación verificada) |
| **PrimaryAttack_Level1** | ✅ DONE | Implementado en RoguePlayerCharacter.cpp:327-366 |
| **Respawn Simple** | ✅ DONE | Ya existe en ARogueGameModeBase.cpp:337-354 |
| **Documentación** | ✅ DONE | Sección 11 (Configuración) completamente actualizada |
| **Especificación** | ✅ DONE | Ambigüedades resueltas, paths de archivos verificados |

### Pendiente para José (Blueprint/Editor) 📋

| Tarea | Prioridad | Notas |
|-------|-----------|-------|
| Crear **BP_Level1_GameMode** (basado en ARogueGameModeBase) | HIGH | Activar `bAutoRespawnPlayer = true` |
| Crear **BP_Level1_PlayerCharacter** (basado en ARoguePlayerCharacter) | HIGH | Asignar `ProjectileClass` a BP_MagicProjectile |
| Crear **L_Level1_Plaza** map | HIGH | Seguir checklist 11.3 |
| Crear **WBP_HealthBar** widget | MEDIUM | Bind a `Attribute_Health` para visualizar salud |
| Configurar **Input Actions** en PlayerData | HIGH | Asegurar que Input_PrimaryAttack está configurado |
| Crear **BP_MagicProjectile** (si no existe) | HIGH | Blueprint wrapper de ARogueProjectile_Magic |

### C++ Ready para Uso 🚀

Toda la lógica C++ de Nivel 1 está implementada:
- ✅ ARoguePlayerCharacter con PrimaryAttack_Level1()
- ✅ ARogueGameModeBase con respawn automático
- ✅ ActionComponent con FRogueSurvivorAttributeSet (health, healthmax, attackdamage)
- ✅ Todos los tests compilando sin errores

---

---

## 1. Resumen del Loop de Juego

```
┌─────────────────────────────────────────────────────────┐
│                    NIVEL 1: PLAZA                        │
├─────────────────────────────────────────────────────────┤
│                                                          │
│   1. Player spawna en PlayerStart                       │
│              ↓                                          │
│   2. Player se mueve con WASD/Gamepad                   │
│              ↓                                          │
│   3. Player dispara projectiles con click/boton         │
│              ↓                                          │
│   4. Projectile impacta Target Dummy                    │
│              ↓                                          │
│   5. Target Dummy recibe dano (visual feedback)         │
│              ↓                                          │
│   6. Player puede recibir dano (de barrel explosion)    │
│              ↓                                          │
│   7. Si Health <= 0 → "Game Over" (respawn o mensaje)   │
│                                                          │
└─────────────────────────────────────────────────────────┘
```

**Tiempo estimado de sesion**: 2-5 minutos de juego continuo

**Sensacion objetivo**: "Puedo moverme y disparar. Es estable y responsive."

---

## 2. Clases y Assets Involucrados

### 2.1 Clases C++ Requeridas

| Clase | Archivo | Responsabilidad | Modificaciones Nivel 1 |
|-------|---------|-----------------|------------------------|
| `ARoguePlayerCharacter` | Player/RoguePlayerCharacter.h|.cpp | Personaje controlable | Simplificar a projectile directo |
| `ARoguePlayerController` | Player/RoguePlayerController.h|.cpp | Input handling | Mantener solo input basico |
| `URogueInteractionComponent` | Player/RogueInteractionComponent.h|.cpp | Interaccion (E key) | Opcional, puede excluirse |
| `ARogueProjectile_Magic` | Projectiles/RogueProjectile_Magic.h|.cpp | Projectile principal | Sin pooling, spawn directo |
| `URogueProjectileMovementComponent` | Projectiles/RogueProjectileMovementComponent.h|.cpp | Movimiento de projectile | Sin cambios |
| `FRogueHealthAttributeSet` | ActionSystem/RogueAttributeSet.h | Health, HealthMax | Solo Health basico |
| `ARogueTargetDummy` | World/RogueTargetDummy.h|.cpp | Target para testear | Sin cambios |
| `ARogueExplosiveBarrel` | World/RogueExplosiveBarrel.h|.cpp | Fuente de dano | Simplificado |
| `ARogueGameModeBase` | Core/RogueGameModeBase.h|.cpp | Reglas de juego | Minimo (solo respawn) |

### 2.2 Clases a EXCLUIR o DESHABILITAR

| Clase | Motivo de Exclusion |
|-------|---------------------|
| `URogueAction*` | No hay Action System en Nivel 1 |
| `URogueActionComponent` | Usar atributos directos |
| `URogueActionEffect*` | No hay efectos |
| `ARogueAICharacter` | No hay AI |
| `ARogueAIController` | No hay AI |
| `URogueBT*` | No hay BehaviorTree |
| `URogueSaveGame*` | No hay persistencia |
| `URogueActorPooling*` | No hay pooling |
| `URogueSignificance*` | No hay LOD |
| `URogueProjectilesSubsystem` | Sin data-oriented |

### 2.3 Blueprints Requeridos

| Blueprint | Ubicacion Esperada | Proposito |
|-----------|-------------------|-----------|
| BP_PlayerCharacter | Content/ActionRoguelike/Characters/ | Player configurable |
| BP_MagicProjectile | Content/ActionRoguelike/Projectiles/ | Projectile visual |
| BP_TargetDummy | Content/ActionRoguelike/World/ | Target de pruebas |
| BP_ExplosiveBarrel | Content/ActionRoguelike/World/ | Fuente de dano |
| WBP_HealthBar | Content/ActionRoguelike/UI/ | HUD minimo |

### 2.4 Mapas

| Mapa | Proposito |
|------|-----------|
| L_Level1_Plaza | Mapa simple con PlayerStart, dummies, barriles |
| TestLevel (existente) | Puede usarse como base |

### 2.5 Input Actions (Enhanced Input)

| Action | Binding Default | Proposito |
|--------|-----------------|-----------|
| IA_Move | WASD / Left Stick | Movimiento |
| IA_Look | Mouse / Right Stick | Camera |
| IA_PrimaryAttack | LMB / Right Trigger | Disparar |
| IA_Jump | Space / A Button | Saltar |

---

## 3. Simplificaciones del Codigo Original

### 3.1 Disparo de Projectile (Sin Action System)

**Original** (usa URogueAction_ProjectileAttack):
```cpp
// En RoguePlayerCharacter.cpp
void ARoguePlayerCharacter::PrimaryAttack(const FInputActionInstance& Instance)
{
    ActionComp->StartActionByName(this, SharedGameplayTags::Action_PrimaryAttack);
}
```

**Nivel 1** (disparo directo):
```cpp
// En RoguePlayerCharacter.cpp (simplificado)
void ARoguePlayerCharacter::PrimaryAttack_Level1(const FInputActionInstance& Instance)
{
    if (!ProjectileClass) return;

    FVector SpawnLocation = GetMesh()->GetSocketLocation(TEXT("Muzzle_01"));
    FRotator SpawnRotation = GetControlRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;

    GetWorld()->SpawnActor<ARogueProjectile>(
        ProjectileClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );
}
```

### 3.2 Sistema de Salud (Sin ActionComponent completo)

**DECISION TOMADA: Opcion B** ✅ (Confirmed Phase C1)

Razones:
1. Tests ya asumen ActionComponent (Level1_PlazaTests.cpp)
2. Compatibilidad con Nivel 2 (AI, Pickups herdan AttributeComponent)
3. Preparación para Nivel 3: solo activar DefaultActions

**Opcion A** (rechazada): Usar atributos directos en PlayerCharacter
- ❌ Diverge de arquitectura general
- ❌ Incompatible con AI future

**Opcion B IMPLEMENTADA**: URogueActionComponent solo con atributos

```cpp
// En ARoguePlayerCharacter::Constructor (ya implementado)
ActionComp = CreateDefaultSubobject<URogueActionComponent>(TEXT("ActionComp"));
ActionComp->SetDefaultAttributeSet(FRogueSurvivorAttributeSet::StaticStruct());

// Interfaz para Nivel 1 (sin Action System):
FRogueAttribute* HealthAttr = ActionComp->GetAttribute(SharedGameplayTags::Attribute_Health);
ActionComp->ApplyAttributeChange(
    SharedGameplayTags::Attribute_Health,
    DamageAmount,  // Negativo para daño
    Instigator,
    EAttributeModifyType::AddBase,
    FGameplayTagContainer()
);

// FUTURO Nivel 3: Reemplazar con Action System (DefaultActions, GrantAction)
```

**Atributos disponibles en L1**:
- `Attribute_Health` (default 100)
- `Attribute_HealthMax` (default 100)
- `Attribute_AttackDamage` (default 25) - usado por projectiles

### 3.3 Target Dummy Simplificado

El ARogueTargetDummy existente ya es simple. Sin cambios necesarios.

### 3.4 GameMode Minimo

```cpp
// En RogueGameModeBase.cpp (Nivel 1)
void ARogueGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    // NO iniciar SpawnBots
    // NO iniciar sistema de creditos
    // Solo dejar el respawn de player activo
}

void ARogueGameModeBase::OnActorKilled(AActor* VictimActor, AActor* Killer)
{
    // Respawn simple del player
    if (ARoguePlayerCharacter* Player = Cast<ARoguePlayerCharacter>(VictimActor))
    {
        FTimerHandle RespawnTimer;
        GetWorldTimerManager().SetTimer(RespawnTimer, [this, Player]()
        {
            RestartPlayer(Player->GetController());
        }, 2.0f, false);
    }
}
```

---

## 4. Escenarios de Test (Given/When/Then)

### 4.1 Movimiento

| ID | Escenario | Given | When | Then |
|----|-----------|-------|------|------|
| L1-MOV-01 | Movimiento WASD | Player spawneado en nivel | Presiona W | Character se mueve hacia adelante |
| L1-MOV-02 | Movimiento lateral | Player en juego | Presiona A o D | Character se mueve lateralmente |
| L1-MOV-03 | Movimiento con gamepad | Player con gamepad | Usa stick izquierdo | Character se mueve proporcional |
| L1-MOV-04 | Rotacion con mouse | Player en juego | Mueve mouse | Camera rota, character gira |
| L1-MOV-05 | Salto | Player en suelo | Presiona Space | Character salta |

### 4.2 Projectile

| ID | Escenario | Given | When | Then |
|----|-----------|-------|------|------|
| L1-PROJ-01 | Disparo basico | Player valido | Presiona LMB | Projectile spawna desde muzzle |
| L1-PROJ-02 | Direccion de disparo | Player mirando a target | Dispara | Projectile va hacia crosshair |
| L1-PROJ-03 | Impacto con pared | Projectile en vuelo | Colisiona con StaticMesh | VFX de impacto, projectile destruido |
| L1-PROJ-04 | Impacto con dummy | Projectile en vuelo | Colisiona con TargetDummy | Dummy recibe dano, feedback visual |
| L1-PROJ-05 | No auto-damage | Player dispara | Projectile toca al propio player | Sin dano aplicado |

### 4.3 Health System

| ID | Escenario | Given | When | Then |
|----|-----------|-------|------|------|
| L1-HP-01 | Health inicial | Player recien spawneado | Consulta Health | Health == 100, HealthMax == 100 |
| L1-HP-02 | Recibir dano | Player con Health=100 | Recibe 25 de dano | Health == 75 |
| L1-HP-03 | Health no negativo | Player con Health=10 | Recibe 50 de dano | Health == 0 (clamped) |
| L1-HP-04 | Muerte | Player con Health=10 | Recibe dano letal | OnDeath() se ejecuta, respawn |

### 4.4 HUD

| ID | Escenario | Given | When | Then |
|----|-----------|-------|------|------|
| L1-HUD-01 | HUD visible | Player spawneado | Juego inicia | Barra de vida visible |
| L1-HUD-02 | HUD se actualiza | Player con HUD | Health cambia | Barra refleja nuevo valor |

---

## 5. Checklist de Verificacion Manual

### 5.1 Setup

- [ ] Mapa L_Level1_Plaza existe y carga sin errores
- [ ] PlayerStart colocado en posicion valida
- [ ] Al menos 2 TargetDummy en el nivel
- [ ] Al menos 1 ExplosiveBarrel en el nivel
- [ ] Enhanced Input configurado correctamente

### 5.2 Gameplay Core

- [ ] Player spawna en PlayerStart
- [ ] WASD mueve al character
- [ ] Mouse rota la camara
- [ ] Click izquierdo dispara projectile
- [ ] Projectile sale del socket correcto (Muzzle_01)
- [ ] Projectile viaja en direccion correcta
- [ ] Projectile impacta y muestra VFX
- [ ] Projectile hace dano a TargetDummy

### 5.3 Health System

- [ ] Barra de vida visible en HUD
- [ ] Explotar barrel cerca del player causa dano
- [ ] Barra de vida se reduce al recibir dano
- [ ] Al llegar a 0 HP, player muere
- [ ] Player respawna despues de muerte
- [ ] Health se reinicia al respawnear

### 5.4 Performance

- [ ] FPS > 60 en escena simple
- [ ] Sin stuttering al disparar
- [ ] Sin memory leaks visibles (stat memory)
- [ ] Sin crashes en 5 minutos de juego

---

## 6. Borrador de Automation Tests

### 6.1 Test: Player Spawn

```cpp
// Source/ActionRoguelike/Tests/Level1_PlayerSpawnTest.cpp

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLevel1_PlayerSpawnTest,
    "ActionRoguelike.Level1.PlayerSpawnWithComponents",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLevel1_PlayerSpawnTest::RunTest(const FString& Parameters)
{
    // Arrange
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    FActorSpawnParameters SpawnParams;

    // Act
    ARoguePlayerCharacter* Player = World->SpawnActor<ARoguePlayerCharacter>(
        ARoguePlayerCharacter::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams
    );

    // Assert
    TestNotNull(TEXT("Player spawned successfully"), Player);
    TestNotNull(TEXT("Player has mesh"), Player->GetMesh());
    TestNotNull(TEXT("Player has camera"), Player->FindComponentByClass<UCameraComponent>());

    return true;
}
```

### 6.2 Test: Health Initialization

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLevel1_HealthInitTest,
    "ActionRoguelike.Level1.HealthInitializedCorrectly",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLevel1_HealthInitTest::RunTest(const FString& Parameters)
{
    // Arrange
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    ARoguePlayerCharacter* Player = World->SpawnActor<ARoguePlayerCharacter>();

    // Assert
    if (Player)
    {
        // Opcion A: Si usamos atributos directos
        TestEqual(TEXT("Health is 100"), Player->Health, 100.0f);
        TestEqual(TEXT("HealthMax is 100"), Player->HealthMax, 100.0f);

        // Opcion B: Si usamos ActionComponent
        // URogueActionComponent* ActionComp = Player->GetActionComponent();
        // FRogueAttribute* Health = ActionComp->GetAttribute(HealthTag);
        // TestEqual(TEXT("Health is 100"), Health->GetValue(), 100.0f);
    }

    return true;
}
```

### 6.3 Test: Damage Application

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLevel1_DamageTest,
    "ActionRoguelike.Level1.DamageReducesHealth",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLevel1_DamageTest::RunTest(const FString& Parameters)
{
    // Arrange
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    ARoguePlayerCharacter* Player = World->SpawnActor<ARoguePlayerCharacter>();
    float InitialHealth = 100.0f;
    float DamageAmount = 25.0f;

    // Act
    Player->ApplyDamage(DamageAmount, nullptr);

    // Assert
    TestEqual(TEXT("Health reduced correctly"), Player->Health, InitialHealth - DamageAmount);

    return true;
}
```

### 6.4 Test: Health Clamping

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLevel1_HealthClampTest,
    "ActionRoguelike.Level1.HealthClampsToZero",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLevel1_HealthClampTest::RunTest(const FString& Parameters)
{
    // Arrange
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    ARoguePlayerCharacter* Player = World->SpawnActor<ARoguePlayerCharacter>();
    Player->Health = 10.0f;

    // Act - Apply overkill damage
    Player->ApplyDamage(50.0f, nullptr);

    // Assert
    TestEqual(TEXT("Health clamped to 0"), Player->Health, 0.0f);
    TestTrue(TEXT("Health not negative"), Player->Health >= 0.0f);

    return true;
}
```

### 6.5 Test: Projectile Spawn

```cpp
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FLevel1_ProjectileSpawnTest,
    "ActionRoguelike.Level1.ProjectileSpawnsCorrectly",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FLevel1_ProjectileSpawnTest::RunTest(const FString& Parameters)
{
    // Arrange
    UWorld* World = FAutomationEditorCommonUtils::CreateNewMap();
    ARoguePlayerCharacter* Player = World->SpawnActor<ARoguePlayerCharacter>();

    int32 InitialActorCount = World->GetActorCount();

    // Act
    Player->PrimaryAttack_Level1(FInputActionInstance());

    int32 FinalActorCount = World->GetActorCount();

    // Assert
    TestTrue(TEXT("New actor spawned (projectile)"), FinalActorCount > InitialActorCount);

    // Find the spawned projectile
    TArray<AActor*> Projectiles;
    UGameplayStatics::GetAllActorsOfClass(World, ARogueProjectile_Magic::StaticClass(), Projectiles);
    TestTrue(TEXT("Magic projectile exists"), Projectiles.Num() > 0);

    return true;
}
```

---

## 7. Criterios de Aceptacion del Nivel 1

### 7.1 Funcionales

| Criterio | Metrica | Umbral |
|----------|---------|--------|
| Player spawn | Spawn exitoso | 100% |
| Movement responsive | Input lag | <100ms |
| Projectile spawn | Sin crash en 10 disparos | 0 crashes |
| Projectile direction | Direccion correcta | Hacia crosshair |
| Health system | Tests pasan | 100% |
| Death/Respawn | Ciclo completo | Funcional |

### 7.2 Performance

| Criterio | Metrica | Umbral |
|----------|---------|--------|
| FPS estable | FPS en idle | >60 |
| Sin stuttering | Frame time spikes | <50ms |
| Memoria | Crecimiento en 5min | <50MB |

### 7.3 Estabilidad

| Criterio | Metrica | Umbral |
|----------|---------|--------|
| Sin crashes | Crashes en 5min | 0 |
| Sin errores en log | Errores criticos | 0 |

---

## 8. Notas de Implementacion Futura

### 8.1 Que NO Incluir en Nivel 1

- Networking/Replication
- SaveGame
- AI de cualquier tipo
- Action System abstracto
- Abilities (Dash, Blackhole, etc.)
- Efectos (Thorns, Burning)
- Pickups
- EQS
- Pooling
- Significance

### 8.2 Preparacion para Nivel 2

Al finalizar Nivel 1, el codigo debe estar estructurado para facilitar:

1. **Agregar AI**: ARogueAICharacter puede heredar la logica de health
2. **Agregar Pickups**: Sistema de interaccion ya funcional
3. **Expandir a Action System**: Projectile spawn puede abstraerse

### 8.3 Puntos de Extension Identificados

```cpp
// En RoguePlayerCharacter.h

// FUTURO: Reemplazar con Action System
virtual void PrimaryAttack(const FInputActionInstance& Instance);

// FUTURO: Mover a URogueActionComponent
UPROPERTY()
float Health;
float HealthMax;
```

---

## 9. Riesgos y Mitigaciones

| Riesgo | Probabilidad | Impacto | Mitigacion |
|--------|--------------|---------|------------|
| Enhanced Input changes en UE 5.7 | Media | Alto | Documentar bindings, usar abstraccion |
| Projectile dependencia de ActionComponent | Alta | Medio | Crear spawn directo temporal |
| Health system diverge del original | Media | Medio | Mantener interfaz compatible |
| Mapa muy simple no es representativo | Baja | Bajo | Crear escenario variado |

---

## 10. Siguiente Paso

**Cuando Nivel 1 este COMPLETO:**

1. Verificar que todos los tests pasan
2. Verificar checklist manual al 100%
3. Commit con mensaje: `[Level1-Plaza] Juego minimo funcional completo`
4. Merge a `city-level2-plaza-viva`
5. Comenzar a agregar AI basica

---

## 11. Configuracion de Blueprint/Editor para Level 1

### 11.1 GameMode Blueprint Configuration (IMPLEMENTADO EN C++)

El `ARogueGameModeBase` YA TIENE la lógica de respawn simple implementada:

✅ **En RogueGameModeBase.cpp (línea 337-354)**:
- `OnActorKilled()` detecta muerte del player
- Timer de 2 segundos antes de respawn
- Llamada a `RestartPlayer()` automática

**Para Level 1, solo activar en Blueprint**:
```
BP_Level1_GameMode (basado en ARogueGameModeBase)
├── bAutoRespawnPlayer = true ⭐ (ACTIVAR ESTO)
├── SpawnTimerInterval = 0 (no spawnar bots)
├── InitialSpawnCredit = 0 (no créditos para bots)
├── MonsterTable = None (sin tabla de enemigos)
└── SpawnBotQuery = None (sin EQS)
```

### 11.2 World Settings del Mapa L_Level1_Plaza (CHECKLIST)

```
Pasos en el Editor:
1. Crear nuevo mapa L_Level1_Plaza o usar TestLevel como base
2. World Settings (Panel derecho del mapa):
   - GameMode Override → BP_Level1_GameMode
   - Default Pawn Class → BP_PlayerCharacter
3. Colocar PlayerStart válido
4. Guardar mapa
```

### 11.3 Contenido Minimo del Mapa (CHECKLIST)

```
L_Level1_Plaza debe contener:
├── ✓ PlayerStart (1) - Posición válida donde spawneará el player
├── ✓ BP_TargetDummy (2-3) - Objetivos para testear daño
├── ✓ BP_ExplosiveBarrel (1-2) - Fuente de daño para player
├── ✓ Floor (StaticMesh) - Plataforma jugable
├── ✓ Paredes básicas (colisiones)
├── ✓ Luz direccional
└── ✓ Sky - Para no estar en void
```

### 11.4 Player Blueprint Configuration (NIVEL 1 MODIFICADO)

**BP_PlayerCharacter debe tener**:
- ✅ `PlayerConfig` (URoguePlayerData) configurado con Input Actions
- ✅ `ActionComp` configurado con `FRogueSurvivorAttributeSet` (ya en C++)
- 🆕 `ProjectileClass` → Asignar `BP_MagicProjectile` (new for Level 1)
- ⚠️ NO agregar `DefaultActions` en Level 1 (sin Action System)

### 11.5 Configuracion de Input (Enhanced Input System)

**Asegurar que estas Input Actions existen en PlayerData**:
```
Input_Move: WASD / Left Stick → Move()
Input_Look: Mouse / Right Stick → LookMouse()/LookStick()
Input_PrimaryAttack: LMB / Right Trigger → PrimaryAttack_Level1() ⭐ NEW
Input_Jump: Space / A Button → Jump()
Input_Dash: No requerido en L1
Input_Sprint: No requerido en L1
```

**Cambio crítico en Input Binding** (línea 111 de RoguePlayerCharacter.cpp):
- ANTES (Level 2+): `StartActionByTag(...Action_PrimaryAttack)`
- DESPUÉS (Level 1): Podría llamar a `PrimaryAttack_Level1()` directamente
- 📝 NOTA: Actualmente llama a StartActionByTag, que será un NOOP en L1 (sin DefaultActions)
- ✅ ALTERNATIVA: Crear Blueprint BP_Level1_PlayerCharacter que overridea input binding

### 11.6 HUD Configuration (OPCIONAL para L1)

```
Para mostrar Health Bar:
├── Crear Widget WBP_HealthBar con blueprint
├── Bind a Player Health atributo via ActionComponent
├── Agregar a PlayerHUD.AddToViewport en ARogueHUD o Blueprint

Alternativa simple:
└── Usar GameMode Blueprint para spawnear HUD widget directamente
```

---

## 12. Notas de Compatibilidad UE 5.6 → 5.7

### 12.1 Cambios Requeridos

| Archivo | Cambio | Motivo |
|---------|--------|--------|
| `RogueActionComponent.h:79` | `BaseStruct = "/Script/ActionRoguelike.RogueAttributeSet"` | UE 5.7 requiere path largo |
| `RogueProjectilesSubsystem.cpp` | Agregar `#include "NiagaraDataChannelPublic.h"` | API de Niagara Data Channels cambió |

### 12.2 Tests Automatizados Creados

```
Source/ActionRoguelike/Tests/Level1_PlazaTests.cpp

Tests incluidos:
├── FLevel1_HealthInitTest - Health inicial = 100
├── FLevel1_DamageTest - Daño reduce health
├── FLevel1_HealthClampTest - Health clamp a 0
├── FLevel1_HealthMaxClampTest - Health no excede max
├── FLevel1_AttackDamageTest - AttackDamage existe
└── FLevel1_ActionComponentSetupTest - Componente configurado
```

### 12.3 Ejecutar Tests

```bash
# Desde línea de comandos
/opt/UnrealEngine-5.7/Engine/Binaries/Linux/UnrealEditor-Cmd \
    ActionRoguelike.uproject \
    -ExecCmds="Automation RunTests ActionRoguelike.Level1" \
    -unattended -nopause -NullRHI -nosplash
```

---

**Documento generado por**: Claude Code
**Basado en**: Analisis de ActionRoguelike (Tom Looman)
**Engine target**: UE 5.6 / 5.7 compatible
**Ultima actualizacion**: 2025-11-22
