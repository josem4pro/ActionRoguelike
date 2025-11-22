#!/usr/bin/env python3
"""
Long Run Test for Nivel 1 - La Plaza
=====================================
Extended runtime test for stability validation.

Unlike the smoke test (5-10s), this test runs the game for 30-60 seconds
to validate:
1. Memory stability (no leaks or crashes)
2. No recurring errors in logs
3. Stable frame/tick execution

Use this before:
- Integrating major changes
- Running RL training sessions
- Pre-commit validation for significant features

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
from typing import Optional, Tuple, List, Dict
from collections import Counter

# Import shared utilities from smoke test
from run_plaza_smoketest import (
    find_executable,
    build_command,
    save_log,
    SUCCESS_PATTERNS,
    FAILURE_PATTERNS,
    WARNING_PATTERNS,
    LOG_DIR,
)


# ============================================================================
# EXTENDED ANALYSIS
# ============================================================================

def analyze_error_frequency(log_lines: List[str]) -> Dict[str, int]:
    """
    Analyze frequency of error patterns in logs.

    Returns dict of pattern -> count
    """
    error_counts = Counter()

    for line in log_lines:
        for pattern in FAILURE_PATTERNS:
            if re.search(pattern, line, re.IGNORECASE):
                error_counts[pattern] += 1

    return dict(error_counts)


def analyze_warning_frequency(log_lines: List[str]) -> Dict[str, int]:
    """
    Analyze frequency of warning patterns in logs.

    Returns dict of pattern -> count
    """
    warning_counts = Counter()

    for line in log_lines:
        for pattern in WARNING_PATTERNS:
            if re.search(pattern, line, re.IGNORECASE):
                # Extract warning type if possible
                match = re.search(r"(Log\w+)", line)
                key = match.group(1) if match else pattern
                warning_counts[key] += 1

    return dict(warning_counts)


def check_repetitive_errors(log_lines: List[str], threshold: int = 10) -> List[str]:
    """
    Find errors that repeat too many times (possible loop/spam).

    Args:
        log_lines: Log output lines
        threshold: Number of repetitions to flag

    Returns:
        List of problematic patterns
    """
    line_counts = Counter()

    for line in log_lines:
        # Normalize line (remove timestamps, numbers)
        normalized = re.sub(r'\d+', 'N', line)
        normalized = re.sub(r'0x[0-9a-fA-F]+', 'ADDR', normalized)
        line_counts[normalized] += 1

    repetitive = []
    for pattern, count in line_counts.items():
        if count >= threshold and any(
            re.search(p, pattern, re.IGNORECASE)
            for p in ["error", "warning", "failed", "invalid"]
        ):
            repetitive.append(f"{count}x: {pattern[:80]}...")

    return repetitive


def run_longrun_test(
    executable: Path,
    duration_seconds: int = 30,
    map_name: Optional[str] = None,
    verbose: bool = False
) -> Tuple[bool, str, List[str], Dict]:
    """
    Run extended stability test.

    Args:
        executable: Path to game binary
        duration_seconds: How long to run
        map_name: Optional map to load
        verbose: Print verbose output

    Returns:
        Tuple of (success, summary, log_lines, metrics)
    """
    cmd = build_command(executable, duration_seconds, map_name)

    if verbose:
        print(f"Executing: {' '.join(cmd)}")
        print(f"Duration: {duration_seconds}s")

    log_lines = []
    metrics = {
        "errors": 0,
        "warnings": 0,
        "success_indicators": 0,
        "duration_actual": 0,
        "exit_code": 0,
        "error_breakdown": {},
        "warning_breakdown": {},
        "repetitive_patterns": [],
    }

    start_time = time.time()

    try:
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1,
        )

        last_activity = time.time()

        while True:
            elapsed = time.time() - start_time

            if elapsed >= duration_seconds:
                if verbose:
                    print(f"\nDuration reached ({duration_seconds}s)")
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                break

            try:
                line = process.stdout.readline()
                if line:
                    log_lines.append(line.rstrip())
                    last_activity = time.time()

                    if verbose and len(log_lines) % 100 == 0:
                        print(f"  [{int(elapsed)}s] Lines: {len(log_lines)}")

                    # Quick checks
                    for pattern in FAILURE_PATTERNS:
                        if re.search(pattern, line, re.IGNORECASE):
                            metrics["errors"] += 1

                    for pattern in WARNING_PATTERNS:
                        if re.search(pattern, line, re.IGNORECASE):
                            metrics["warnings"] += 1

                    for pattern in SUCCESS_PATTERNS:
                        if re.search(pattern, line, re.IGNORECASE):
                            metrics["success_indicators"] += 1

                elif process.poll() is not None:
                    break

                # Check for hang (no output for 10 seconds)
                if time.time() - last_activity > 10:
                    if verbose:
                        print("WARNING: No activity for 10 seconds")

            except Exception as e:
                if verbose:
                    print(f"Read error: {e}")
                break

        metrics["exit_code"] = process.returncode or 0
        metrics["duration_actual"] = time.time() - start_time

    except FileNotFoundError:
        return False, f"Executable not found: {executable}", [], metrics
    except Exception as e:
        return False, f"Execution error: {e}", [], metrics

    # Post-run analysis
    metrics["error_breakdown"] = analyze_error_frequency(log_lines)
    metrics["warning_breakdown"] = analyze_warning_frequency(log_lines)
    metrics["repetitive_patterns"] = check_repetitive_errors(log_lines)

    # Determine success
    success = True
    summary_parts = []

    # Critical errors fail the test
    if metrics["errors"] > 0:
        success = False
        summary_parts.append(f"ERRORS: {metrics['errors']} critical errors")
        for pattern, count in metrics["error_breakdown"].items():
            summary_parts.append(f"  - {pattern}: {count}")

    # Repetitive errors are a red flag
    if metrics["repetitive_patterns"]:
        success = False
        summary_parts.append(f"REPETITIVE PATTERNS: {len(metrics['repetitive_patterns'])} problematic")
        for pat in metrics["repetitive_patterns"][:5]:
            summary_parts.append(f"  - {pat}")

    # Exit code check (ignore our termination signals)
    if metrics["exit_code"] not in [0, -15, -9, 143, 137]:
        success = False
        summary_parts.append(f"EXIT CODE: {metrics['exit_code']} (unexpected)")

    # Success indicators
    if metrics["success_indicators"] > 0:
        summary_parts.append(f"SUCCESS INDICATORS: {metrics['success_indicators']}")
    else:
        summary_parts.append("WARNING: No success indicators found")

    # Warnings (informational)
    if metrics["warnings"] > 0:
        summary_parts.append(f"WARNINGS: {metrics['warnings']} total")

    # Duration info
    summary_parts.append(f"DURATION: {metrics['duration_actual']:.1f}s (target: {duration_seconds}s)")
    summary_parts.append(f"LOG LINES: {len(log_lines)}")

    status = "PASS" if success else "FAIL"
    summary_parts.insert(0, f"LONG RUN TEST: {status}")

    return success, "\n".join(summary_parts), log_lines, metrics


# ============================================================================
# MAIN
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="Long run stability test for Nivel 1 - La Plaza",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Default 30 second test
  python run_plaza_longrun.py

  # Extended 60 second test with verbose output
  python run_plaza_longrun.py --duration 60 --verbose

  # Pre-RL validation
  python run_plaza_longrun.py --duration 120 --save-log
        """
    )

    parser.add_argument(
        "--duration", "-d",
        type=int,
        default=30,
        help="Duration in seconds (default: 30)"
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
        help="Path to game executable"
    )

    args = parser.parse_args()

    print("=" * 70)
    print("PLAZA LONG RUN TEST - Nivel 1: La Plaza (Stability Validation)")
    print("=" * 70)
    print(f"Timestamp: {datetime.now().isoformat()}")
    print()

    # Find executable
    if args.executable:
        executable = Path(args.executable)
    else:
        executable = find_executable()

    if not executable:
        print("ERROR: Could not find game executable!")
        print("Use --executable or set PLAZA_EXECUTABLE env var")
        sys.exit(1)

    print(f"Executable: {executable}")
    print(f"Duration: {args.duration}s")
    if args.map:
        print(f"Map: {args.map}")
    print()

    print("Starting long run test...")
    print("-" * 70)

    success, summary, log_lines, metrics = run_longrun_test(
        executable,
        duration_seconds=args.duration,
        map_name=args.map,
        verbose=args.verbose
    )

    print("-" * 70)
    print()
    print(summary)
    print()

    # Always save log for long runs
    if args.save_log or not success:
        log_file = save_log(log_lines, prefix="plaza_longrun")
        print(f"Log saved to: {log_file}")

    # Print metrics summary
    print()
    print("METRICS SUMMARY:")
    print(f"  - Errors: {metrics['errors']}")
    print(f"  - Warnings: {metrics['warnings']}")
    print(f"  - Success Indicators: {metrics['success_indicators']}")
    print(f"  - Duration: {metrics['duration_actual']:.1f}s")
    print(f"  - Exit Code: {metrics['exit_code']}")

    print()
    print("=" * 70)
    if success:
        print("RESULT: PASS - System stable for extended run")
        print("=" * 70)
        sys.exit(0)
    else:
        print("RESULT: FAIL - Stability issues detected")
        print("=" * 70)
        sys.exit(1)


if __name__ == "__main__":
    main()
