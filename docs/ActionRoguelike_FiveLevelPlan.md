# ActionRoguelike - Plan de 5 Niveles Progresivos

**Version**: 1.0
**Fecha**: 2025-11-22
**Proyecto**: ActionRoguelike (Tom Looman) - UE 5.6
**Proposito**: Destilar el proyecto en 5 niveles de complejidad progresiva

---

## Indice

1. [Matriz de Complejidad](#1-matriz-de-complejidad)
2. [Los 5 Niveles](#2-los-5-niveles)
3. [Estrategia Git & Worktrees](#3-estrategia-git--worktrees)
4. [Plan TDD por Nivel](#4-plan-tdd-por-nivel)
5. [Como Iterar Este Plan](#5-como-iterar-este-plan)

---

## 1. Matriz de Complejidad

### 1.1 Clasificacion de Sistemas

| Sistema | Categoria | Justificacion |
|---------|-----------|---------------|
| Third-Person Movement + Enhanced Input | **FUNDAMENTAL** | Sin movimiento no hay juego |
| Basic Projectile System | **FUNDAMENTAL** | Mecanica core de ataque |
| AttributeComponent (Health) | **FUNDAMENTAL** | Sin salud no hay game over |
| Action System (URogueAction) | INTERMEDIO | Abstraccion elegante pero no critica para MVP |
| ActionEffects (Thorns, Burning) | INTERMEDIO | Profundidad de gameplay, no esencial |
| GameplayTags | INTERMEDIO | Organizacion, no funcionalidad core |
| UI/HUD | INTERMEDIO | Juego funciona sin UI compleja |
| SaveGame System | INTERMEDIO | No critico para sesion unica |
| AI con BT + EQS | INTERMEDIO | Enemigos pueden ser simples inicialmente |
| GameMode avanzado (economia) | INTERMEDIO | Spawn manual suficiente para MVP |
| World Actors (Chest, Barrel) | INTERMEDIO | Interactividad del mundo |
| Asset Manager & async loading | **AVANZADO** | Optimizacion, no funcionalidad |
| Multiplayer/Replication | **AVANZADO** | Complejidad significativa, opcional |
| Performance (Pooling, Significance) | **AVANZADO** | Optimizacion para escala |
| Data-Oriented Projectiles | **AVANZADO** | Optimizacion experimental |

### 1.2 Dependencias Entre Sistemas

```
Movement + Input
    |
    v
Basic Projectile <-- AttributeComponent (Health)
    |
    v
Action System (abstraccion de projectile y abilities)
    |
    +---> ActionEffects (requiere Action System)
    |
    +---> AI con BT (usa Action System para ataques)
    |
    v
GameMode avanzado (usa AI + Actions)
    |
    v
Multiplayer/Replication (replica todo lo anterior)
    |
    v
Performance Systems (optimiza todo lo anterior)
```

---

## 2. Los 5 Niveles

### Nivel 1: Plaza (Hola Mundo Jugable)

**Objetivo Pedagogico**: Juego minimo funcional - moverse, disparar, recibir dano, "game over"

**Sistemas INCLUIDOS**:
- Third-Person Movement + Enhanced Input
- Basic Projectile (ARogueProjectile_Magic)
- AttributeComponent (Health, HealthMax)
- HUD minimo (barra de vida basica)
- Target Dummy (para testear dano)

**Sistemas EXCLUIDOS**:
- Action System (ataque directo sin abstraccion)
- AI compleja (sin BT, sin EQS)
- SaveGame
- Multiplayer
- Todos los sistemas de Performance

**Archivos Clave**:
```
Source/ActionRoguelike/
├── Player/RoguePlayerCharacter.cpp|.h
├── Player/RoguePlayerController.cpp|.h
├── Projectiles/RogueProjectile_Magic.cpp|.h
├── ActionSystem/RogueAttributeSet.h (solo FRogueHealthAttributeSet)
└── World/RogueTargetDummy.cpp|.h
```

**Resultado Esperado**:
Un mini-juego super estable donde el jugador puede moverse, disparar proyectiles a un target dummy, y ver su barra de vida.

**Riesgos/Gotchas**:
- El projectile actual usa ActionComponent; puede requerir simplificacion temporal
- Enhanced Input puede tener cambios de comportamiento en UE 5.7

---

### Nivel 2: Plaza Viva

**Objetivo Pedagogico**: Anadir vida al entorno - enemigos basicos que se mueven hacia el jugador

**Sistemas INCLUIDOS** (adicional a Nivel 1):
- ARogueAICharacter (sin BT complejo, solo perseguir)
- ARogueAIController (simplificado)
- Pickups (Health, Credits)
- World Actors (Chest, Barrel)
- URogueInteractionComponent
- URogueWorldUserWidget (health bars 3D)

**Sistemas EXCLUIDOS**:
- BT completo (solo MoveTo basico)
- EQS
- Action System completo
- SaveGame
- Multiplayer

**Archivos Clave Adicionales**:
```
Source/ActionRoguelike/
├── AI/RogueAICharacter.cpp|.h
├── AI/RogueAIController.cpp|.h
├── Pickups/RoguePickupActor*.cpp|.h
├── World/RogueTreasureChest.cpp|.h
├── World/RogueExplosiveBarrel.cpp|.h
├── Player/RogueInteractionComponent.cpp|.h
└── UI/RogueWorldUserWidget.cpp|.h
```

**Resultado Esperado**:
Enemigos basicos que persiguen al jugador, pickups funcionales, interaccion con cofres y barriles.

**Riesgos/Gotchas**:
- AI simplificada puede requerir stubbing del BehaviorTree
- Health bars requieren URogueWorldUserWidget funcionando

---

### Nivel 3: Barrio (Sensacion Roguelike)

**Objetivo Pedagogico**: Action System completo, persistencia, sensacion de roguelike

**Sistemas INCLUIDOS** (adicional a Nivel 2):
- URogueAction completo
- URogueActionComponent
- URogueActionEffect (Thorns, Burning)
- Dash, Blackhole abilities
- GameplayTags (SharedGameplayTags)
- SaveGame System
- BT basico para AI (roam, chase, attack)
- UI completa (HUD, effect slots)

**Sistemas EXCLUIDOS**:
- Multiplayer/Replication
- GameMode economico (spawn manual)
- EQS avanzado
- Performance systems

**Archivos Clave Adicionales**:
```
Source/ActionRoguelike/
├── ActionSystem/RogueAction.cpp|.h
├── ActionSystem/RogueActionComponent.cpp|.h
├── ActionSystem/RogueActionEffect*.cpp|.h
├── ActionSystem/RogueAction_ProjectileAttack.cpp|.h
├── Projectiles/RogueProjectile_Dash.cpp|.h
├── Projectiles/RogueProjectile_Blackhole.cpp|.h
├── AI/RogueBTTask_*.cpp|.h
├── AI/RogueBTService_*.cpp|.h
├── SaveSystem/RogueSaveGame*.cpp|.h
├── SharedGameplayTags.cpp|.h
└── UI/RogueEffectSlotWidget.cpp|.h
```

**Resultado Esperado**:
Juego con multiples habilidades, efectos temporales, AI con comportamiento variado, y capacidad de guardar/cargar.

**Riesgos/Gotchas**:
- Transicion a URogueAction requiere refactor de projectile spawning
- SaveGame requiere UPROPERTY(SaveGame) correctamente configurado
- GameplayTags requieren configuracion en DefaultGameplayTags.ini

---

### Nivel 4: Distrito Conectado (Multiplayer)

**Objetivo Pedagogico**: Introducir networking y datos externos

**Sistemas INCLUIDOS** (adicional a Nivel 3):
- Server-Authoritative model
- RPCs (ServerStartAction, etc.)
- Replicacion de Actions, Attributes
- FFastArraySerializer para projectiles
- ARogueGameState (datos replicados)
- GameMode con economia (SpawnCredit, DataTables)
- Asset Manager async loading
- EQS para spawn locations

**Sistemas EXCLUIDOS**:
- Performance systems (Pooling, Significance)
- Data-Oriented Projectiles

**Archivos Clave Adicionales**:
```
Source/ActionRoguelike/
├── Core/RogueGameModeBase.cpp|.h (economia)
├── Core/RogueGameState.cpp|.h (replicacion)
├── Core/RogueMonsterData.cpp|.h (DataAsset)
├── Player/RoguePlayerState.cpp|.h (credits replicados)
└── Player/RoguePlayerData.cpp|.h (DataAsset)
```

**Configuracion Requerida**:
```ini
# DefaultEngine.ini
[OnlineSubsystem]
DefaultPlatformService=Steam
```

**Resultado Esperado**:
Juego jugable en multiplayer (Listen Server o Dedicated), con spawning economico de enemigos.

**Riesgos/Gotchas**:
- Replicacion requiere revisar TODOS los UPROPERTY(Replicated)
- Authority checks criticos para evitar cheating
- Asset Manager puede causar race conditions si no se maneja async correctamente

---

### Nivel 5: Ciudad Completa (Proyecto Original)

**Objetivo Pedagogico**: ActionRoguelike completo con todas las optimizaciones

**Sistemas INCLUIDOS** (adicional a Nivel 4):
- URogueActorPoolingSubsystem
- IRogueActorPoolingInterface
- URogueSignificanceManager
- IRogueSignificanceInterface
- Animation Budget Allocator integration
- URogueTickablesSubsystem
- Data-Oriented Projectiles (URogueProjectilesSubsystem)
- PSO Precaching

**Archivos Clave Adicionales**:
```
Source/ActionRoguelike/
├── Performance/RogueActorPoolingSubsystem.cpp|.h
├── Performance/RogueActorPoolingInterface.cpp|.h
├── Performance/RogueSignificanceManager.cpp|.h
├── Performance/RogueSignificanceComponent.cpp|.h
├── Performance/RogueTickablesSubsystem.cpp|.h
└── Projectiles/RogueProjectilesSubsystem.cpp|.h
```

**Configuracion Requerida**:
```ini
# DefaultEngine.ini
[/Script/SignificanceManager.SignificanceManager]
SignificanceManagerClassName=/Script/ActionRoguelike.RogueSignificanceManager

# DefaultScalability.ini
[AnimationQuality@0]
a.Budget.BudgetMs=1.0

[AnimationQuality@3]
a.Budget.BudgetMs=4.0
```

**Resultado Esperado**:
El proyecto ActionRoguelike completo, identico a la rama main, con todas las optimizaciones funcionando.

**Riesgos/Gotchas**:
- Pooling requiere que projectiles implementen IRogueActorPoolingInterface
- Significance LOD requiere calibracion de distancias
- Animation Budget puede causar pop-in visual si mal configurado

---

## 3. Estrategia Git & Worktrees

### 3.1 Diagrama de Ramas

```
master (intacto - fuente de verdad)
   |
   +-- city-level1-plaza .................. (juego minimo)
   |      |
   |      +-- city-level2-plaza-viva ...... (enemigos basicos)
   |             |
   |             +-- city-level3-barrio ... (Action System)
   |                    |
   |                    +-- city-level4-distrito-conectado (multiplayer)
   |                           |
   |                           +-- city-level5-ciudad-completa (completo)
```

### 3.2 Comandos de Setup

```bash
# Ejecutar desde /home/jose/Repositorios/ActionRoguelike

# 1. Crear ramas en orden de herencia
git branch city-level1-plaza master
git branch city-level2-plaza-viva city-level1-plaza
git branch city-level3-barrio city-level2-plaza-viva
git branch city-level4-distrito-conectado city-level3-barrio
git branch city-level5-ciudad-completa city-level4-distrito-conectado

# 2. Crear worktrees
git worktree add ../ActionRoguelike-level1-plaza city-level1-plaza
git worktree add ../ActionRoguelike-level2-plaza-viva city-level2-plaza-viva
git worktree add ../ActionRoguelike-level3-barrio city-level3-barrio
git worktree add ../ActionRoguelike-level4-distrito city-level4-distrito-conectado
git worktree add ../ActionRoguelike-level5-completo city-level5-ciudad-completa

# 3. Verificar
git worktree list
```

### 3.3 Estructura Resultante

```
/home/jose/Repositorios/
    ActionRoguelike/                    # master (no tocar)
    ActionRoguelike-level1-plaza/       # Nivel 1
    ActionRoguelike-level2-plaza-viva/  # Nivel 2
    ActionRoguelike-level3-barrio/      # Nivel 3
    ActionRoguelike-level4-distrito/    # Nivel 4
    ActionRoguelike-level5-completo/    # Nivel 5 (= master)
```

### 3.4 Flujo de Trabajo

**Propagar cambios hacia arriba (L1 -> L2 -> L3 -> ...):**
```bash
# Commit en level2
cd ActionRoguelike-level2-plaza-viva
git add . && git commit -m "feat: Add basic AI patrol"

# Merge a level3
cd ../ActionRoguelike-level3-barrio
git merge city-level2-plaza-viva

# Merge a level4
cd ../ActionRoguelike-level4-distrito
git merge city-level3-barrio

# etc.
```

### 3.5 Consideraciones UE5

| Directorio | Se duplica? | Notas |
|------------|-------------|-------|
| Source/ | Si (676 KB) | Codigo C++ |
| Content/ | No* | Git usa hardlinks |
| Intermediate/ | No | En .gitignore |
| Saved/ | No | En .gitignore |

*Content no se duplica hasta que modifiques assets especificos.

**Espacio estimado**: ~1-2 GB adicionales por todos los worktrees.

### 3.6 Advertencias Importantes

- **NUNCA** hacer checkout en el repo principal (usa worktrees)
- **NUNCA** merge de level5 a master (master es upstream)
- **NUNCA** eliminar .git de un worktree
- **SIEMPRE** propagar cambios en orden (1->2->3->4->5)

---

## 4. Plan TDD por Nivel

### 4.1 Resumen de Tests

| Nivel | Escenarios G/W/T | Automation Tests | Manual Checks |
|-------|-----------------|------------------|---------------|
| L1 - Plaza | 10 | 4-6 | 8 |
| L2 - Plaza Viva | 10 | 4-6 | 9 |
| L3 - Barrio | 11 | 6-8 | 10 |
| L4 - Distrito | 10 | 6-8 | 13 |
| L5 - Ciudad | 9 | 4-6 | 10 |
| **TOTAL** | **50** | **24-34** | **50** |

### 4.2 Nivel 1 - Tests Clave

**Given/When/Then:**
| ID | Given | When | Then |
|----|-------|------|------|
| L1-MOV-01 | Player spawneado | Presiona WASD | Character se mueve |
| L1-PROJ-01 | Player con ActionComponent | Presiona ataque | Projectile_Magic spawna |
| L1-HP-01 | Player con Health=100 | Se aplica -25 dano | Health == 75 |
| L1-HP-02 | Player con Health=10 | Se aplica -50 dano | Health == 0 (clamped) |

**Criterio de Aceptacion:**
- [ ] Movimiento responsive (<100ms input lag)
- [ ] Projectiles spawnan sin crash
- [ ] Health system funcional
- [ ] FPS >60 estable

### 4.3 Nivel 2 - Tests Clave

**Given/When/Then:**
| ID | Given | When | Then |
|----|-------|------|------|
| L2-AI-01 | AI con perception | Player en rango | AI obtiene target |
| L2-AI-02 | AI con target | AI tick | AI persigue player |
| L2-PU-01 | Player con Health=50 | Interactua con HealthPotion | Health aumenta |

**Criterio de Aceptacion:**
- [ ] AI persigue y ataca
- [ ] Pickups funcionan y respawnean
- [ ] Health bars visibles

### 4.4 Nivel 3 - Tests Clave

**Given/When/Then:**
| ID | Given | When | Then |
|----|-------|------|------|
| L3-ACT-01 | Player con Dash | Presiona input | Player teleporta |
| L3-ACT-02 | Player con Thorns | Recibe dano | Atacante recibe reflejo |
| L3-SG-01 | Player con credits=500 | WriteSaveGame() | Archivo .sav creado |

**Criterio de Aceptacion:**
- [ ] Todas las actions funcionan
- [ ] SaveGame persiste datos
- [ ] UI completa y responsive

### 4.5 Nivel 4 - Tests Multiplayer

**Given/When/Then:**
| ID | Given | When | Then |
|----|-------|------|------|
| L4-NET-01 | 2 clients conectados | Client2 se mueve | Client1 ve movimiento |
| L4-NET-02 | Client dispara | Server procesa | Todos ven projectile |
| L4-NET-03 | Client desconecta | - | Server no crashea |

**Comandos de Network Testing:**
```bash
# En consola UE5:
Net PktLag=100    # 100ms latencia
Net PktLag=200    # 200ms latencia
Net PktLoss=5     # 5% packet loss
```

**Criterio de Aceptacion:**
- [ ] 4 players simultaneos
- [ ] Funciona con 200ms latencia
- [ ] No desync visible

### 4.6 Nivel 5 - Tests Performance

**Given/When/Then:**
| ID | Given | When | Then |
|----|-------|------|------|
| L5-POOL-01 | Pooling activo | Disparo rapido | Projectiles se reutilizan |
| L5-SIG-01 | AI lejos | Update | LOD alto (menor calidad) |
| L5-PERF-01 | 50+ actors | Gameplay | FPS >30 |

**Comandos de Profiling:**
```bash
# En consola UE5:
stat fps
stat unit
stat memory
stat significancemanager
```

**Criterio de Aceptacion:**
- [ ] Pooling >80% reuse rate
- [ ] FPS >30 en stress test
- [ ] Memoria estable (+/- 100MB en 10min)

### 4.7 Matriz de Regresion

| Test Category | L1 | L2 | L3 | L4 | L5 |
|---------------|----|----|----|----|-----|
| Movement | INIT | MUST | MUST | MUST | MUST |
| Projectile | INIT | MUST | MUST | MUST | MUST |
| Health | INIT | MUST | MUST | MUST | MUST |
| AI | - | INIT | MUST | MUST | MUST |
| Pickups | - | INIT | MUST | MUST | MUST |
| Actions | - | - | INIT | MUST | MUST |
| SaveGame | - | - | INIT | MUST | MUST |
| Multiplayer | - | - | - | INIT | MUST |
| Performance | - | - | - | - | INIT |

---

## 5. Como Iterar Este Plan

### 5.1 Actualizar Especificacion de un Nivel

1. Editar la seccion correspondiente en este documento
2. Actualizar la matriz de complejidad si es necesario
3. Ajustar los tests G/W/T
4. Documentar cambios en el changelog (abajo)

### 5.2 Agregar Feature a un Nivel

1. Identificar a que nivel pertenece (FUNDAMENTAL/INTERMEDIO/AVANZADO)
2. Agregar a la lista de "Sistemas INCLUIDOS" del nivel apropiado
3. Listar archivos clave adicionales
4. Agregar tests correspondientes

### 5.3 Re-ejecutar Analisis con Agentes

Para actualizar el plan completo:

```
[Agente] refactor-planner
[Prompt] Actualizar clasificacion de sistemas para ActionRoguelike con las siguientes nuevas features: [lista]
```

### 5.4 Extender a Nivel 6+

Si se necesitan mas niveles de complejidad:

1. Crear nueva rama: `git branch city-level6-xxx city-level5-ciudad-completa`
2. Crear worktree: `git worktree add ../ActionRoguelike-level6-xxx city-level6-xxx`
3. Agregar seccion en este documento
4. Definir tests especificos

---

## Changelog

| Fecha | Version | Cambios |
|-------|---------|---------|
| 2025-11-22 | 1.0 | Documento inicial con analisis completo |

---

## Documentos Relacionados

- `docs/ActionRoguelike_Architecture_Map.md` - Mapa arquitectonico completo
- `docs/ActionRoguelike_Quick_Reference.md` - Referencia rapida
- `docs/ActionRoguelike_Level1_PlazaSpec.md` - Especificacion detallada del Nivel 1

---

**Generado por**: Claude Code (Orchestrator DevOps)
**Agentes utilizados**: refactor-planner, code-architecture-reviewer, plan-reviewer, general-purpose
**Skills aplicados**: test-driven-development, using-git-worktrees, writing-plans
