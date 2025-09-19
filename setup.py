#!/usr/bin/env python3
import argparse
import os
import shutil
import subprocess
import sys

ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
BUILD_DIR = os.path.join(ROOT_DIR, 'out', 'build')

def run_cmd(cmd, cwd=None):
    print(f'>> {cmd}')
    result = subprocess.run(cmd, cwd=cwd, shell=True)
    if result.returncode != 0:
        print(f'[ERROR] Command failed: {cmd}')
        sys.exit(result.returncode)

def clean_build():
    if os.path.exists(BUILD_DIR):
        print(f'[INFO] Cleaning build directory: {BUILD_DIR}')
        shutil.rmtree(BUILD_DIR)
    else:
        print(f'[ERROR] Build directory does not exist: {BUILD_DIR}')

def configure(preset):
    print(f'[INFO] Configuring project with preset: {preset}')
    run_cmd(f'cmake --preset {preset}')

def build(preset):
    print(f'[INFO] Building project with preset: {preset}')
    run_cmd(f'cmake --build --preset {preset}')

def main():
    parser = argparse.ArgumentParser(description='OtterGameEngine Setup Tool')
    parser.add_argument('--preset', type=str, default='x64-debug', help='CMake preset name (default: "x64-debug")')
    parser.add_argument('--configure-only', action='store_true', help='Run CMake configuration only')
    parser.add_argument('--build-only', action='store_true', help='Build without configuring')
    parser.add_argument('--clean', action='store_true', help='Clean the build directory before starting')
    args = parser.parse_args()

    if args.clean:
        clean_build()

    if args.configure_only:
        configure(args.preset)
    elif args.build_only:
        build(args.preset)
    else:
        configure(args.preset)
        build(args.preset)

    print(f'[INFO] Process complete.')

if __name__ == '__main__':
    main()
