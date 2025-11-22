#!/usr/bin/env python3
"""
Smoke Test for Nivel 1 - La Plaza
==================================
Runtime test that validates the UE5 game executable starts correctly.

This test:
1. Locates the ActionRoguelike game binary (NOT the editor)
2. Launches it with appropriate flags
3. Monitors logs for startup success/failure indicators
4. Reports pass/fail based on exit code and log analysis

NO EDITOR REQUIRED - Works purely with compiled Game target.

Reference: docs/ActionRoguelike_Level1_RuntimeTests.md
"""

import os
import sys
import subprocess
import time
import argparse
import re
from pathlib import Path
from datetime import datetime
from typing import Optional, Tuple, List


# ============================================================================
# CONFIGURATION
# ============================================================================

# Default paths to search for the game binary
DEFAULT_BINARY_PATHS = [
    # Linux paths
    "Binaries/Linux/ActionRoguelike",
    "../Binaries/Linux/ActionRoguelike",
    "../../Binaries/Linux/ActionRoguelike",
    # Windows paths (for completeness)
    "Binaries/Win64/ActionRoguelike.exe",
    "../Binaries/Win64/ActionRoguelike.exe",
]

# Log output directory
LOG_DIR = Path(__file__).parent / "logs"

# Success indicators in logs
SUCCESS_PATTERNS = [
    r"LogInit.*Engine is starting",
    r"LogLoad.*Took .* seconds to LoadMap",
    r"LogWorld.*Bringing World.*up for play",
    r"LogGameMode",
]

# Failure indicators in logs
FAILURE_PATTERNS = [
    r"Fatal error",
    r"Ensure condition failed",
    r"Unhandled exception",
    r"Assertion failed",
    r"Crash in runnable thread",
    r"Critical error:",
    r"SIGABRT",
    r"SIGSEGV",
]

# Warning patterns (don't fail, but report)
WARNING_PATTERNS = [
    r"Warning:",
    r"LogError:",
]


# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

def find_executable(env_var: str = "PLAZA_EXECUTABLE") -> Optional[Path]:
    """
    Find the game executable.

    Priority:
    1. Environment variable PLAZA_EXECUTABLE
    2. Search in default paths relative to repo root

    Returns:
        Path to executable or None if not found
    """
    # Check environment variable first
    if env_var in os.environ:
        path = Path(os.environ[env_var])
        if path.exists() and path.is_file():
            return path
        print(f"WARNING: {env_var}={path} does not exist")

    # Find repo root (look for .uproject file)
    script_dir = Path(__file__).parent.resolve()
    repo_root = None

    # Walk up to find repo root
    current = script_dir
    for _ in range(5):  # Max 5 levels up
        uproject = list(current.glob("*.uproject"))
        if uproject:
            repo_root = current
            break
        current = current.parent

    if not repo_root:
        # Try common relative paths
        repo_root = script_dir.parent.parent  # tests/runtime -> repo root

    # Search default paths
    for rel_path in DEFAULT_BINARY_PATHS:
        full_path = repo_root / rel_path
        if full_path.exists() and full_path.is_file():
            return full_path.resolve()

    return None


def build_command(executable: Path, timeout_seconds: int = 10, map_name: Optional[str] = None) -> List[str]:
    """
    Build the command line to launch the game.

    Args:
        executable: Path to game binary
        timeout_seconds: How long to let the game run
        map_name: Optional specific map to load

    Returns:
        List of command arguments
    """
    cmd = [str(executable)]

    # Core flags for headless/testing execution
    cmd.extend([
        "-log",                    # Enable logging
        "-stdout",                 # Output to stdout
        "-FullStdOutLogOutput",    # Full log output
        "-nosplash",               # No splash screen
        "-NullRHI",                # No rendering (headless)
        "-NoSound",                # No audio
        "-unattended",             # Non-interactive
        "-nopause",                # Don't pause on errors
    ])

    # Resolution for any UI that might render
    cmd.extend([
        "-ResX=800",
        "-ResY=600",
    ])

    # Optional map specification
    if map_name:
        cmd.append(map_name)

    return cmd


def run_smoke_test(
    executable: Path,
    timeout_seconds: int = 10,
    map_name: Optional[str] = None,
    verbose: bool = False
) -> Tuple[bool, str, List[str]]:
    """
    Run the smoke test.

    Args:
        executable: Path to game binary
        timeout_seconds: Max time to run
        map_name: Optional map to load
        verbose: Print verbose output

    Returns:
        Tuple of (success: bool, summary: str, log_lines: list)
    """
    cmd = build_command(executable, timeout_seconds, map_name)

    if verbose:
        print(f"Executing: {' '.join(cmd)}")
        print(f"Timeout: {timeout_seconds}s")

    log_lines = []
    errors = []
    warnings = []
    success_indicators = []

    try:
        # Start process
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,  # Line buffered
        )

        start_time = time.time()

        # Read output with timeout
        while True:
            # Check timeout
            elapsed = time.time() - start_time
            if elapsed >= timeout_seconds:
                if verbose:
                    print(f"\nTimeout reached ({timeout_seconds}s)")
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                break

            # Try to read a line (non-blocking would be better but this works)
            try:
                line = process.stdout.readline()
                if line:
                    log_lines.append(line.rstrip())

                    if verbose:
                        print(f"  {line.rstrip()}")

                    # Check for success indicators
                    for pattern in SUCCESS_PATTERNS:
                        if re.search(pattern, line, re.IGNORECASE):
                            success_indicators.append(line.rstrip())

                    # Check for failure indicators
                    for pattern in FAILURE_PATTERNS:
                        if re.search(pattern, line, re.IGNORECASE):
                            errors.append(line.rstrip())

                    # Check for warnings
                    for pattern in WARNING_PATTERNS:
                        if re.search(pattern, line, re.IGNORECASE):
                            warnings.append(line.rstrip())

                elif process.poll() is not None:
                    # Process ended
                    break

            except Exception as e:
                if verbose:
                    print(f"Read error: {e}")
                break

        # Get exit code
        exit_code = process.returncode or 0

    except FileNotFoundError:
        return False, f"Executable not found: {executable}", []
    except Exception as e:
        return False, f"Execution error: {e}", []

    # Analyze results
    success = True
    summary_parts = []

    # Check for critical errors
    if errors:
        success = False
        summary_parts.append(f"ERRORS: {len(errors)} critical errors found")
        for err in errors[:5]:  # Show first 5
            summary_parts.append(f"  - {err[:100]}")

    # Check for success indicators
    if success_indicators:
        summary_parts.append(f"SUCCESS INDICATORS: {len(success_indicators)} found")
    else:
        # No success indicators is suspicious but not fatal
        summary_parts.append("WARNING: No success indicators found in log")

    # Report warnings
    if warnings:
        summary_parts.append(f"WARNINGS: {len(warnings)} warnings")

    # Exit code check
    if exit_code != 0 and exit_code not in [-15, -9, 143, 137]:  # Ignore SIGTERM/SIGKILL from our timeout
        success = False
        summary_parts.append(f"EXIT CODE: {exit_code} (non-zero)")

    # Summary
    status = "PASS" if success else "FAIL"
    summary_parts.insert(0, f"SMOKE TEST: {status}")
    summary_parts.append(f"Total log lines: {len(log_lines)}")

    return success, "\n".join(summary_parts), log_lines


def save_log(log_lines: List[str], prefix: str = "plaza_smoke") -> Path:
    """Save log to file."""
    LOG_DIR.mkdir(parents=True, exist_ok=True)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    log_file = LOG_DIR / f"{prefix}_{timestamp}.log"

    with open(log_file, "w") as f:
        f.write("\n".join(log_lines))

    return log_file


# ============================================================================
# MAIN
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="Smoke test for Nivel 1 - La Plaza (ActionRoguelike)",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Basic smoke test (10 second timeout)
  python run_plaza_smoketest.py

  # Longer test with verbose output
  python run_plaza_smoketest.py --timeout 30 --verbose

  # Use specific executable
  PLAZA_EXECUTABLE=/path/to/ActionRoguelike python run_plaza_smoketest.py
        """
    )

    parser.add_argument(
        "--timeout", "-t",
        type=int,
        default=10,
        help="Timeout in seconds (default: 10)"
    )

    parser.add_argument(
        "--map", "-m",
        type=str,
        default=None,
        help="Specific map to load (optional)"
    )

    parser.add_argument(
        "--verbose", "-v",
        action="store_true",
        help="Verbose output"
    )

    parser.add_argument(
        "--save-log", "-s",
        action="store_true",
        help="Save log to file"
    )

    parser.add_argument(
        "--executable", "-e",
        type=str,
        default=None,
        help="Path to game executable (overrides PLAZA_EXECUTABLE env var)"
    )

    args = parser.parse_args()

    print("=" * 60)
    print("PLAZA SMOKE TEST - Nivel 1: La Plaza")
    print("=" * 60)
    print(f"Timestamp: {datetime.now().isoformat()}")
    print()

    # Find executable
    if args.executable:
        executable = Path(args.executable)
    else:
        executable = find_executable()

    if not executable:
        print("ERROR: Could not find game executable!")
        print()
        print("Please either:")
        print("  1. Set PLAZA_EXECUTABLE environment variable")
        print("  2. Use --executable /path/to/ActionRoguelike")
        print("  3. Run from repository root with compiled game")
        print()
        print("Expected locations:")
        for path in DEFAULT_BINARY_PATHS:
            print(f"  - {path}")
        sys.exit(1)

    print(f"Executable: {executable}")
    print(f"Timeout: {args.timeout}s")
    if args.map:
        print(f"Map: {args.map}")
    print()

    # Run test
    print("Starting smoke test...")
    print("-" * 60)

    success, summary, log_lines = run_smoke_test(
        executable,
        timeout_seconds=args.timeout,
        map_name=args.map,
        verbose=args.verbose
    )

    print("-" * 60)
    print()
    print(summary)
    print()

    # Save log if requested
    if args.save_log or not success:
        log_file = save_log(log_lines)
        print(f"Log saved to: {log_file}")

    # Exit with appropriate code
    print()
    print("=" * 60)
    if success:
        print("RESULT: PASS")
        print("=" * 60)
        sys.exit(0)
    else:
        print("RESULT: FAIL")
        print("=" * 60)
        sys.exit(1)


if __name__ == "__main__":
    main()
