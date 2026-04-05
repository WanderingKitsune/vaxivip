#!/usr/bin/env python3
import os
import sys

sys.path.insert(0, os.path.dirname(__file__))
from file_list import TARGET_EXTENSIONS, EXCLUDE_DIRS

DEFAULT_EXTENSIONS = TARGET_EXTENSIONS

class colors:
    def __init__(self):
        if not sys.stdout.isatty() and not os.getenv('FORCE_COLOR'):
            self.RESET = self.GREEN = self.YELLOW = self.RED = self.BLUE = ''
        else:
            self.RESET = '\033[0m'
            self.GREEN = '\033[92m'
            self.YELLOW = '\033[93m'
            self.RED = '\033[91m'
            self.BLUE = '\033[94m'

colors = colors()

def clean_file(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
        
        new_lines = []
        for i, line in enumerate(lines):
            new_line = line.rstrip()
            
            if i == len(lines) - 1 and line.endswith('\n\n'):
                new_line = line.rstrip()
            
            new_lines.append(new_line + '\n')
        
        if lines != new_lines:
            while new_lines and new_lines[-1] == '\n':
                new_lines.pop()
            
            with open(file_path, 'w', encoding='utf-8') as f:
                f.writelines(new_lines)
            return True
        return False
    except Exception as e:
        print(f"Error: {file_path}: {e}")
        return None

def is_target_file(file_path, extensions):
    return any(file_path.endswith(ext) for ext in extensions)

def list_files(directory, extensions, exclude_dirs=None):
    if exclude_dirs is None:
        exclude_dirs = EXCLUDE_DIRS
    
    file_count = 0
    for root, dirs, files in os.walk(directory):
        dirs[:] = [d for d in dirs if not d.startswith('.') and d not in exclude_dirs]
        
        for file in files:
            if is_target_file(file, extensions):
                file_path = os.path.join(root, file)
                if clean_file(file_path):
                    file_count += 1
                    print(file_path)
    return file_count

def process_directory(directory, extensions, exclude_dirs=None, quiet=False):
    if exclude_dirs is None:
        exclude_dirs = EXCLUDE_DIRS
    
    processed = 0
    success = 0
    error = 0
    
    for root, dirs, files in os.walk(directory):
        dirs[:] = [d for d in dirs if not d.startswith('.') and d not in exclude_dirs]
        
        for file in files:
            if is_target_file(file, extensions):
                file_path = os.path.join(root, file)
                result = clean_file(file_path)
                
                if result is None:
                    error += 1
                elif result:
                    processed += 1
                    if not quiet:
                        print(f"{colors.YELLOW}[MODIFIED]{colors.RESET} {file_path}")
                else:
                    success += 1
                    if not quiet:
                        print(f"{colors.GREEN}[PASS]   {colors.RESET} {file_path}")
    
    return processed, success, error

def main():
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Remove trailing spaces from source files',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog='''
Examples:
  %(prog)s -l                      List files that need processing in current directory
  %(prog)s -l -d src/              List files in src/ directory
  %(prog)s -r                      Process and clean files in current directory
  %(prog)s -r -d src/             Process files in src/ directory
  %(prog)s -r -e .hpp .cpp         Process only .hpp and .cpp files
  %(prog)s -r -d src/ -e .hpp      Process .hpp files in src/ directory
        '''
    )
    
    mode_group = parser.add_mutually_exclusive_group(required=True)
    mode_group.add_argument('-l', '--list', action='store_true', help='List files that need processing')
    mode_group.add_argument('-r', '--run', action='store_true', help='Process and clean files')
    
    parser.add_argument('-d', '--dir', type=str, default='.', help='Directory to process (default: current directory)')
    parser.add_argument('-e', '--ext', nargs='+', default=DEFAULT_EXTENSIONS, help='File extensions to process')
    parser.add_argument('-q', '--quiet', action='store_true', help='Quiet mode (only show count, no file list)')
    parser.add_argument('-v', '--verbose', action='store_true', help='Show file list (default for -r)')
    
    args = parser.parse_args()
    
    if args.list:
        count = list_files(args.dir, args.ext)
        print(f"\n{colors.BLUE}Total: {count} file(s){colors.RESET}")
    else:
        modified, success, error = process_directory(args.dir, args.ext, EXCLUDE_DIRS, args.quiet)
        if not args.quiet:
            print(f"\n{colors.YELLOW}Modified: {modified}{colors.RESET}  {colors.GREEN}Success: {success}{colors.RESET}  {colors.RED}Error: {error}{colors.RESET}")
        else:
            print(f"Modified: {modified}  Success: {success}  Error: {error}")

if __name__ == '__main__':
    main()
