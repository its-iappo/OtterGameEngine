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


def find_glslc():
    for root, _, files in os.walk(ROOT_DIR):
        for f in files:
            if f.lower() in ("glslc", "glslc.exe"):
                return os.path.join(root, f)
    return None


def compile_and_copy_shaders(preset):
    shader_src_dir = os.path.join(ROOT_DIR, "OtterEngine", "Shaders")
    shader_build_dir = os.path.join(BUILD_DIR, preset, "OtterEngine", "Shaders")

    studio_output_dir = os.path.join(BUILD_DIR, preset, "OtterStudio")

    debug_studio_dir = os.path.join(studio_output_dir, "Debug")
    release_studio_dir = os.path.join(studio_output_dir, "Release")

    shader_out_dirs = [
        os.path.join(debug_studio_dir, "Shaders"),
        os.path.join(release_studio_dir, "Shaders"),
        os.path.join(studio_output_dir, "Shaders")  # Fallback
    ]

    os.makedirs(shader_build_dir, exist_ok=True)
    for out_dir in shader_out_dirs:
        os.makedirs(out_dir, exist_ok=True)

    glslc = find_glslc()

    if not glslc:
        print("[ERROR] glslc not found in PATH. Install via vcpkg or Vulkan SDK.")
        sys.exit(1)
    else:
        print(f"[INFO] Found glslc at: {glslc}")

    for root, _, files in os.walk(shader_src_dir):
        for f in files:
            if f.endswith((".vert", ".frag")):
                src_path = os.path.join(root, f)
                rel_path = os.path.relpath(src_path, shader_src_dir)

                dst_build_path = os.path.join(shader_build_dir, rel_path + ".spv")
                os.makedirs(os.path.dirname(dst_build_path), exist_ok=True)

                print(f"[INFO] Compiling {rel_path}")
                run_cmd(f"\"{glslc}\" \"{src_path}\" -o \"{dst_build_path}\"")

                for out_dir in shader_out_dirs:
                    dst_runtime = os.path.join(out_dir, rel_path + ".spv")
                    os.makedirs(os.path.dirname(dst_runtime), exist_ok=True)
                    shutil.copy(dst_build_path, dst_runtime)
                    print(f"[INFO] Copied to -> {dst_runtime}")

                dst_source = os.path.join(shader_src_dir, rel_path + ".spv")
                shutil.copy(dst_build_path, dst_source)
                print(f"[INFO] Copied to source -> {dst_source}")


def copy_resources(preset):
    resources_src = os.path.join(ROOT_DIR, "OtterEngine", "Resources")
    studio_output_dir = os.path.join(BUILD_DIR, preset, "OtterStudio")

    output_dirs = [
        os.path.join(studio_output_dir, "Debug", "Resources"),
        os.path.join(studio_output_dir, "Release", "Resources"),
        os.path.join(studio_output_dir, "Resources")  # Fallback
    ]

    for output_dir in output_dirs:
        if os.path.exists(resources_src):
            os.makedirs(output_dir, exist_ok=True)
            print(f"[INFO] Copying resources to: {output_dir}")

            for item in os.listdir(resources_src):
                src_path = os.path.join(resources_src, item)
                dst_path = os.path.join(output_dir, item)

                if os.path.isfile(src_path):
                    shutil.copy2(src_path, dst_path)
                elif os.path.isdir(src_path):
                    shutil.copytree(src_path, dst_path, dirs_exist_ok=True)


def main():
    parser = argparse.ArgumentParser(description='OtterGameEngine Setup Tool')
    parser.add_argument('--preset', type=str, default='x64-debug', help='CMake preset name (default: "x64-debug")')
    parser.add_argument('--configure-only', action='store_true', help='Run CMake configuration only')
    parser.add_argument('--build-only', action='store_true', help='Build without configuring')
    parser.add_argument('--clean', action='store_true', help='Clean the build directory before starting')
    parser.add_argument('--recompile-shaders', action='store_true', help='Recompile shaders without rebuilding the whole project')
    parser.add_argument('--copy-resources', action='store_true', help='Copy resources to output directory')
    # parser.add_argument('--full-rebuild', action='store_true', help='Rebuild entire project, recompile shaders and copy all resources')
    args = parser.parse_args()

    should_recompile_shaders = args.recompile_shaders or args.clean
    should_copy_resources = args.copy_resources or args.clean

    if args.clean:
        clean_build()

    if args.configure_only:
        configure(args.preset)
    elif args.build_only:
        build(args.preset)
    else:
        configure(args.preset)
        build(args.preset)

    if should_recompile_shaders:
        compile_and_copy_shaders(args.preset)

    if should_copy_resources:
        copy_resources(args.preset)

    print(f'[INFO] Process complete.')


if __name__ == '__main__':
    main()
