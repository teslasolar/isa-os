#!/usr/bin/env python3
"""
ISA-OS Auto-Index Generator
Scans source files and auto-generates comprehensive index
"""

import os
import re
from pathlib import Path
from datetime import datetime

# Patterns to extract metadata
MODULE_REGISTER_PATTERN = r'MODULE_REGISTER\s*\(\s*(\w+)\s*,\s*"([^"]+)"\s*,\s*MODULE_TYPE_(\w+)\s*,\s*ISA_L(\d+)'
SYSCALL_PATTERN = r'(?:static\s+)?u32\s+sys_(\w+)\s*\('
FUNCTION_PATTERN = r'^(?:static\s+)?(?:void|i32|u32|u64|bool)\s+(\w+)\s*\('

class ISAOSIndexer:
    def __init__(self, root_dir='.'):
        self.root = Path(root_dir)
        self.modules = []
        self.syscalls = []
        self.drivers = []
        self.protocols = []
        self.files = []
        self.stats = {
            'total_files': 0,
            'total_lines': 0,
            'c_files': 0,
            'asm_files': 0,
            'header_files': 0
        }

    def scan_file(self, filepath):
        """Extract metadata from a source file"""
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                lines = content.count('\n')

                self.stats['total_lines'] += lines

                # Extract module registrations
                for match in re.finditer(MODULE_REGISTER_PATTERN, content, re.MULTILINE):
                    name, desc, mod_type, level = match.groups()
                    self.modules.append({
                        'name': name,
                        'description': desc,
                        'type': mod_type,
                        'level': f'L{level}' if level != '0' else 'Core',
                        'file': str(filepath.relative_to(self.root))
                    })

                # Extract syscalls
                for match in re.finditer(SYSCALL_PATTERN, content, re.MULTILINE):
                    syscall_name = match.group(1)
                    self.syscalls.append({
                        'name': syscall_name,
                        'file': str(filepath.relative_to(self.root))
                    })

                return lines
        except Exception as e:
            print(f"Error scanning {filepath}: {e}")
            return 0

    def scan_directory(self):
        """Scan entire directory tree"""
        for ext, key in [('.c', 'c_files'), ('.asm', 'asm_files'), ('.h', 'header_files')]:
            files = list(self.root.glob(f'**/*{ext}'))
            # Exclude build directory
            files = [f for f in files if 'build' not in str(f)]
            self.stats[key] = len(files)

            for filepath in files:
                self.scan_file(filepath)
                self.files.append({
                    'path': str(filepath.relative_to(self.root)),
                    'size': filepath.stat().st_size,
                    'type': ext[1:]
                })

        self.stats['total_files'] = sum([
            self.stats['c_files'],
            self.stats['asm_files'],
            self.stats['header_files']
        ])

    def generate_index_md(self):
        """Generate INDEX.md with auto-discovered content"""
        output = []

        output.append("# ISA-OS Comprehensive Index")
        output.append(f"\n*Auto-generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}*\n")

        output.append("## 📊 Statistics\n")
        output.append("```")
        output.append(f"Total Files:        {self.stats['total_files']}")
        output.append(f"  C source files:   {self.stats['c_files']}")
        output.append(f"  Assembly files:   {self.stats['asm_files']}")
        output.append(f"  Header files:     {self.stats['header_files']}")
        output.append(f"Total Lines:        {self.stats['total_lines']:,}")
        output.append(f"Registered Modules: {len(self.modules)}")
        output.append(f"System Calls:       {len(self.syscalls)}")
        output.append("```\n")

        # Modules
        if self.modules:
            output.append("## 🧩 Registered Modules\n")
            output.append("| Module | Type | Level | Description | File |")
            output.append("|--------|------|-------|-------------|------|")
            for mod in sorted(self.modules, key=lambda x: x['name']):
                output.append(f"| {mod['name']} | {mod['type']} | {mod['level']} | {mod['description']} | `{mod['file']}` |")
            output.append("")

        # Syscalls
        if self.syscalls:
            output.append("## 🔧 System Calls\n")
            output.append("| Syscall | File |")
            output.append("|---------|------|")
            for sc in sorted(self.syscalls, key=lambda x: x['name']):
                output.append(f"| sys_{sc['name']} | `{sc['file']}` |")
            output.append("")

        # File tree
        output.append("## 📂 File Tree\n")

        # Group files by directory
        dirs = {}
        for f in self.files:
            dir_name = str(Path(f['path']).parent)
            if dir_name not in dirs:
                dirs[dir_name] = []
            dirs[dir_name].append(f)

        for dir_name in sorted(dirs.keys()):
            output.append(f"\n### {dir_name}/\n")
            for f in sorted(dirs[dir_name], key=lambda x: x['path']):
                filename = Path(f['path']).name
                size_kb = f['size'] / 1024
                output.append(f"- **{filename}** ({size_kb:.1f} KB)")

        output.append("\n## 📚 Documentation")
        output.append("\n- [README.md](README.md) - Main overview")
        output.append("- [QUICKSTART.md](QUICKSTART.md) - 5-minute setup")
        output.append("- [ARCHITECTURE.md](ARCHITECTURE.md) - Technical architecture")
        output.append("- [MODULES.md](MODULES.md) - Module system guide")
        output.append("- [INDEX.md](INDEX.md) - This file (auto-generated)")

        output.append("\n## 🔍 Quick Search")
        output.append("\n### By Feature")
        output.append("- **ISA-95 Levels**: L0 (Field), L1 (Control), L2 (SCADA), L3 (MES), L4 (ERP)")
        output.append("- **Protocols**: Modbus, PROFINET, EtherCAT, CAN, OPC UA, MQTT")
        output.append("- **Interfaces**: SPI, I2C, UART")
        output.append("- **Standards**: ISA-95, ISA-88, ISA-18.2, 21 CFR Part 11")

        output.append("\n### By Type")
        drivers = [m for m in self.modules if m['type'] == 'DRIVER']
        protocols = [m for m in self.modules if m['type'] == 'PROTOCOL']
        services = [m for m in self.modules if m['type'] == 'SERVICE']

        if drivers:
            output.append(f"\n**Drivers ({len(drivers)})**: " + ", ".join([d['name'] for d in drivers]))
        if protocols:
            output.append(f"\n**Protocols ({len(protocols)})**: " + ", ".join([p['name'] for p in protocols]))
        if services:
            output.append(f"\n**Services ({len(services)})**: " + ", ".join([s['name'] for s in services]))

        output.append("\n---\n")
        output.append("*This index is automatically generated from source code.*")
        output.append("*Run `./scripts/generate_index.py` to update.*")

        return "\n".join(output)

    def generate_module_list(self):
        """Generate modules list for documentation"""
        output = []
        output.append("# Module Registry\n")
        output.append(f"*Auto-discovered: {len(self.modules)} modules*\n")

        by_type = {}
        for mod in self.modules:
            mod_type = mod['type']
            if mod_type not in by_type:
                by_type[mod_type] = []
            by_type[mod_type].append(mod)

        for mod_type in sorted(by_type.keys()):
            output.append(f"\n## {mod_type}S\n")
            for mod in sorted(by_type[mod_type], key=lambda x: x['name']):
                output.append(f"### {mod['name']}")
                output.append(f"- **Description**: {mod['description']}")
                output.append(f"- **Level**: {mod['level']}")
                output.append(f"- **File**: `{mod['file']}`\n")

        return "\n".join(output)

if __name__ == '__main__':
    indexer = ISAOSIndexer()
    indexer.scan_directory()

    # Generate INDEX.md
    index_content = indexer.generate_index_md()
    with open('INDEX.md', 'w') as f:
        f.write(index_content)
    print(f"✅ Generated INDEX.md ({len(index_content)} bytes)")

    # Generate module list
    module_list = indexer.generate_module_list()
    with open('docs/MODULE_REGISTRY.md', 'w') as f:
        f.write(module_list)
    print(f"✅ Generated MODULE_REGISTRY.md")

    print(f"\n📊 Statistics:")
    print(f"  Files:   {indexer.stats['total_files']}")
    print(f"  Lines:   {indexer.stats['total_lines']:,}")
    print(f"  Modules: {len(indexer.modules)}")
    print(f"  Syscalls: {len(indexer.syscalls)}")
