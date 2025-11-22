# Runtime Tests - Nivel 1: La Plaza

**Versión**: 1.0
**Fecha**: 22 Noviembre 2025
**Engine**: Unreal Engine 5.7
**Fase**: D - Runtime & RL Harness

---

## Overview

Este documento describe los **tests de runtime** para Nivel 1 - La Plaza. Estos tests validan que el ejecutable del juego (Game target) arranque correctamente y se mantenga estable, **sin usar el Editor de Unreal**.

### Diferencia entre tipos de tests

| Tipo | Descripción | Requiere Editor | Ubicación |
|------|-------------|-----------------|-----------|
| **C++ Automation Tests** | Tests unitarios que validan lógica C++ | Sí (EditorContext) | `Source/ActionRoguelike/Tests/` |
| **Runtime Tests** | Validan el binario compilado | **NO** | `tests/runtime/` |
| **Functional Tests** | Blueprints de testing en mapa | Sí | `Content/Tests/` |

Los **Runtime Tests** son la única opción para validación headless en CI/CD sin licencia de Editor.

---

## Requisitos

### Sistema

- **Python**: 3.10+ (recomendado 3.12)
- **OS**: Linux (Ubuntu 22.04+) o Windows 10/11
- **RAM**: Mínimo 8GB para ejecutar el juego
- **GPU**: Opcional (tests usan -NullRHI para headless)

### Binario del juego

El binario del juego debe estar compilado como **Game target** (no Editor):

```bash
# Compilar Game target en Linux
/opt/UnrealEngine-5.7/Engine/Build/BatchFiles/Linux/Build.sh \
    ActionRoguelike Linux Development \
    -Project=/path/to/ActionRoguelike.uproject

# Resultado esperado
Binaries/Linux/ActionRoguelike
```

### Rutas esperadas del binario

Los tests buscan el ejecutable en este orden:

1. Variable de entorno `PLAZA_EXECUTABLE`
2. `Binaries/Linux/ActionRoguelike` (relativo al repo root)
3. `Binaries/Win64/ActionRoguelike.exe` (Windows)

---

## Tests Disponibles

### 1. Smoke Test (`run_plaza_smoketest.py`)

**Propósito**: Validación rápida de que el juego arranca sin errores críticos.

**Duración**: 5-10 segundos

**Uso**:

```bash
# Desde el directorio del repo
cd /home/jose/Repositorios/ActionRoguelike

# Test básico
python tests/runtime/run_plaza_smoketest.py

# Con timeout extendido y verbose
python tests/runtime/run_plaza_smoketest.py --timeout 15 --verbose

# Guardar log
python tests/runtime/run_plaza_smoketest.py --save-log

# Usar ejecutable específico
python tests/runtime/run_plaza_smoketest.py --executable /path/to/ActionRoguelike
```

**Criterios de éxito**:

- ✅ Exit code 0 (o señales de terminación esperadas)
- ✅ Log contiene indicadores de éxito (LogInit, LogLoad, etc.)
- ✅ Log NO contiene errores fatales

**Criterios de fallo**:

- ❌ Exit code no esperado
- ❌ Presencia de "Fatal error", "Ensure condition failed", etc.
- ❌ Excepciones no manejadas

### 2. Long Run Test (`run_plaza_longrun.py`)

**Propósito**: Validación de estabilidad extendida antes de entrenar RL o integrar cambios grandes.

**Duración**: 30-120 segundos (configurable)

**Uso**:

```bash
# Test de 30 segundos (default)
python tests/runtime/run_plaza_longrun.py

# Test extendido de 60 segundos
python tests/runtime/run_plaza_longrun.py --duration 60

# Pre-RL validation (2 minutos)
python tests/runtime/run_plaza_longrun.py --duration 120 --save-log --verbose
```

**Análisis adicional**:

- Frecuencia de errores por tipo
- Patrones repetitivos (posibles memory leaks o loops)
- Estabilidad de framerate (via logs)

**Cuándo usar**:

- Antes de sesiones de RL training
- Después de cambios significativos en C++
- Como parte de CI/CD pre-merge

---

## Flags del ejecutable

Los tests lanzan el juego con estos flags:

| Flag | Propósito |
|------|-----------|
| `-log` | Habilita logging |
| `-stdout` | Output a stdout |
| `-FullStdOutLogOutput` | Log completo |
| `-nosplash` | Sin splash screen |
| `-NullRHI` | Sin renderizado (headless) |
| `-NoSound` | Sin audio |
| `-unattended` | No interactivo |
| `-nopause` | No pausar en errores |
| `-ResX=800 -ResY=600` | Resolución mínima |

---

## Patrones de log analizados

### Indicadores de éxito

```
LogInit.*Engine is starting
LogLoad.*Took .* seconds to LoadMap
LogWorld.*Bringing World.*up for play
LogGameMode
```

### Indicadores de fallo (críticos)

```
Fatal error
Ensure condition failed
Unhandled exception
Assertion failed
Crash in runnable thread
Critical error:
SIGABRT
SIGSEGV
```

### Warnings (no fallan el test)

```
Warning:
LogError:
```

---

## Integración con CI/CD

### GitHub Actions ejemplo

```yaml
name: Runtime Tests

on: [push, pull_request]

jobs:
  runtime-tests:
    runs-on: self-hosted  # Requiere runner con UE5 compilado
    steps:
      - uses: actions/checkout@v4

      - name: Run Smoke Test
        run: python tests/runtime/run_plaza_smoketest.py --timeout 15

      - name: Run Long Run Test (on main only)
        if: github.ref == 'refs/heads/main'
        run: python tests/runtime/run_plaza_longrun.py --duration 60 --save-log

      - name: Upload logs
        if: failure()
        uses: actions/upload-artifact@v4
        with:
          name: runtime-logs
          path: tests/runtime/logs/
```

---

## Logs

Los logs se guardan en `tests/runtime/logs/`:

```
tests/runtime/logs/
├── plaza_smoke_20251122_163000.log
├── plaza_smoke_20251122_170000.log
└── plaza_longrun_20251122_180000.log
```

**Formato del nombre**: `{test_type}_{YYYYMMDD}_{HHMMSS}.log`

---

## Troubleshooting

### El ejecutable no se encuentra

```
ERROR: Could not find game executable!
```

**Soluciones**:

1. Verificar que el juego está compilado:
   ```bash
   ls -la Binaries/Linux/ActionRoguelike
   ```

2. Usar variable de entorno:
   ```bash
   export PLAZA_EXECUTABLE=/path/to/ActionRoguelike
   ```

3. Usar flag `--executable`:
   ```bash
   python run_plaza_smoketest.py --executable /path/to/ActionRoguelike
   ```

### Test falla inmediatamente

**Posibles causas**:

1. **Permisos**: El binario necesita permisos de ejecución
   ```bash
   chmod +x Binaries/Linux/ActionRoguelike
   ```

2. **Dependencias faltantes**: Verificar librerías de UE5
   ```bash
   ldd Binaries/Linux/ActionRoguelike | grep "not found"
   ```

3. **Content no compilado**: Asegurar que los assets están cooked
   ```bash
   ls Saved/Cooked/
   ```

### "Fatal error" en el log

1. Ver el log completo para contexto
2. Buscar el stack trace
3. Verificar que el mapa por defecto existe
4. Verificar configuración de GameMode

---

## Conexión con RL Proving Grounds

Estos tests de runtime son **prerequisitos** para el entorno de RL:

```
[Runtime Tests] ──validates──> [Game Binary]
                                    │
                              [RL Environment]
                                    │
                              [RL Training]
```

El entorno RL (`la_plaza_env.py` en rl-proving-grounds) usa el mismo binario y depende de:

1. ✅ Smoke test pasando
2. ✅ Long run test estable
3. ✅ Logs analizables para estado del juego

---

## Referencias

- **PlazaSpec**: `docs/ActionRoguelike_Level1_PlazaSpec.md`
- **C++ Tests**: `Source/ActionRoguelike/Tests/Level1_PlazaTests.cpp`
- **RL Environment ADR**: Ver `rl-proving-grounds/LA_PLAZA_ENV_ADR.md`
- **GlobalDocs**: `Niveles/Nivel-1-La-Plaza/GlobalDocs/`

---

## Changelog

### v1.0 (22 Nov 2025)

- ✅ Implementación inicial de smoke test
- ✅ Implementación de long run test
- ✅ Documentación completa
- ✅ Integración con estructura de Fase D

---

**Generado por**: Claude Code - DevOps Orchestrator
**Fase**: D - Runtime & RL Harness
**Status**: 🟢 LISTO
