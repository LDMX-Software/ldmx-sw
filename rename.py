import os
import re
import argparse

def camel_to_snake(name):
    """Convert CamelCase or mixedCase to snake_case."""
    s1 = re.sub(r'(.)([A-Z][a-z]+)', r'\1_\2', name)
    s2 = re.sub(r'([a-z0-9])([A-Z])', r'\1_\2', s1)
    return s2.lower()

def process_file(filepath, apply=False):
    with open(filepath, "r") as f:
        content = f.read()

    matches = set(re.findall(r'\b([A-Za-z][A-Za-z0-9]*)_\b', content))
    replacements = {}

    for match in matches:
        new_name = camel_to_snake(match) + "_"
        if new_name != match + "_":
            replacements[match + "_"] = new_name

    if replacements:
        print(f"\n{filepath}:")
        for old, new in replacements.items():
            print(f"  {old} → {new}")

        if apply:
            for old, new in replacements.items():
                content = re.sub(rf'\b{old}\b', new, content)
            with open(filepath, "w") as f:
                f.write(content)
            print("  ✅ Changes applied")

def process_directory(root_dir, apply=False):
    for subdir, _, files in os.walk(root_dir):
        for file in files:
            if file.endswith(".cxx"):
                process_file(os.path.join(subdir, file), apply=apply)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Convert trailing-underscore variables to snake_case_.")
    parser.add_argument("root", nargs="?", default=".", help="Root directory to scan (default: current dir)")
    parser.add_argument("--apply", action="store_true", help="Actually apply the changes")
    args = parser.parse_args()

    process_directory(args.root, apply=args.apply)

