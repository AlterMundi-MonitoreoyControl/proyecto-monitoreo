Import("env")
import os
import subprocess
from datetime import datetime

def get_object_sizes(build_dir, size_cmd):
    """Get sizes of all .o files in the build directory"""
    object_files = []

    for root, dirs, files in os.walk(build_dir):
        for f in files:
            if f.endswith('.o'):
                obj_path = os.path.join(root, f)
                try:
                    result = subprocess.run(
                        [size_cmd, obj_path],
                        capture_output=True,
                        text=True
                    )
                    for line in result.stdout.strip().split('\n'):
                        parts = line.split()
                        if len(parts) >= 4 and parts[0].isdigit():
                            text = int(parts[0])
                            data = int(parts[1])
                            bss = int(parts[2])

                            rel_path = os.path.relpath(obj_path, build_dir)

                            object_files.append({
                                'name': rel_path,
                                'text': text,
                                'data': data,
                                'bss': bss,
                                'flash': text + data,
                                'ram': data + bss,
                            })
                except:
                    pass

    return object_files


def post_build_size(source, target, env):
    """Generate a memory usage report after build"""

    board = env.BoardConfig()
    mcu = board.get("build.mcu", "esp32")

    # Get partition sizes - these match PlatformIO's calculation
    # ESP32 default partition: app = 1310720 (0x140000), total flash = 4MB
    # PlatformIO sets these in the build environment
    flash_max = int(env.BoardConfig().get("upload.maximum_size", 1310720))
    ram_max = int(env.BoardConfig().get("upload.maximum_ram_size", 327680))

    # Total flash chip size (for reference)
    flash_total_str = board.get("upload.flash_size", "4MB")
    flash_total = int(flash_total_str.replace("MB", "")) * 1024 * 1024

    # Get partition scheme name
    partition_scheme = env.GetProjectOption("board_build.partitions", "default")

    firmware_path = str(target[0])
    build_dir = os.path.dirname(firmware_path)

    toolchain_prefix = env.get("TOOLCHAIN_PREFIX", "xtensa-esp32-elf-")
    size_cmd = toolchain_prefix + "size"

    # Use basic size output - this is exactly what PlatformIO uses
    text_size = 0
    data_size = 0
    bss_size = 0

    try:
        result = subprocess.run(
            [size_cmd, firmware_path],
            capture_output=True,
            text=True
        )
        for line in result.stdout.strip().split('\n'):
            parts = line.split()
            if len(parts) >= 4 and parts[0].isdigit():
                text_size = int(parts[0])
                data_size = int(parts[1])
                bss_size = int(parts[2])
    except:
        pass

    # PlatformIO calculation (matches their output exactly):
    # Flash = text + data (program size written to flash)
    # RAM = data + bss (static RAM usage)
    flash_used = text_size + data_size
    ram_used = data_size + bss_size

    flash_percent = (flash_used / flash_max) * 100 if flash_max > 0 else 0
    ram_percent = (ram_used / ram_max) * 100 if ram_max > 0 else 0

    # Calculate remaining flash (total chip - app partition - other partitions estimate)
    # Typical: bootloader=0x8000, partitions=0x8000, nvs=0x5000, otadata=0x2000, spiffs varies
    spiffs_size = flash_total - flash_max - 0x10000  # Rough estimate

    # Get detailed section info for the saved report
    try:
        result_A = subprocess.run(
            [size_cmd, "-A", firmware_path],
            capture_output=True,
            text=True
        )
        size_output_A = result_A.stdout
    except:
        size_output_A = ""

    # Get object file sizes
    object_sizes = get_object_sizes(build_dir, size_cmd)
    objects_by_flash = sorted(object_sizes, key=lambda x: x['flash'], reverse=True)
    objects_by_ram = sorted(object_sizes, key=lambda x: x['ram'], reverse=True)

    env_name = env.get("PIOENV", "unknown")
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    git_version = subprocess.run(['git', 'rev-parse', 'HEAD'], capture_output=True, text=True).stdout.strip()
    # Build progress bars (20 chars wide)
    def make_bar(percent):
        filled = min(20, int(percent / 5))
        return "=" * filled + " " * (20 - filled)

    flash_bar = make_bar(flash_percent)
    ram_bar = make_bar(ram_percent)

    report = f"""
================================================================================
                        BUILD MEMORY REPORT
================================================================================
Environment: {env_name}
Git version: {git_version}
MCU:         {mcu}
Timestamp:   {timestamp}
Firmware:    {os.path.basename(firmware_path)}
Partitions:  {partition_scheme}
--------------------------------------------------------------------------------

MEMORY USAGE (matches PlatformIO):
  Flash: [{flash_bar}] {flash_percent:5.1f}%  (used {flash_used:,} / {flash_max:,} bytes)
  RAM:   [{ram_bar}] {ram_percent:5.1f}%  (used {ram_used:,} / {ram_max:,} bytes)

--------------------------------------------------------------------------------
FLASH BREAKDOWN:
  .text (code):      {text_size:>12,} bytes
  .data (init vars): {data_size:>12,} bytes
  ─────────────────────────────────
  Program size:      {flash_used:>12,} bytes
  App partition:     {flash_max:>12,} bytes
  Free in partition: {flash_max - flash_used:>12,} bytes

RAM BREAKDOWN:
  .data (init vars): {data_size:>12,} bytes
  .bss (zero vars):  {bss_size:>12,} bytes
  ─────────────────────────────────
  Static RAM:        {ram_used:>12,} bytes
  Total DRAM:        {ram_max:>12,} bytes
  Free (static):     {ram_max - ram_used:>12,} bytes

--------------------------------------------------------------------------------
FLASH CHIP LAYOUT ({flash_total_str} total):
  Total flash:       {flash_total:>12,} bytes
  App partition:     {flash_max:>12,} bytes ({flash_max*100/flash_total:.1f}%)
  Other (NVS/OTA/FS):{flash_total - flash_max:>12,} bytes ({(flash_total-flash_max)*100/flash_total:.1f}%)

================================================================================
"""

    # Top flash consumers
    blame_flash = "\n🔥 TOP FLASH CONSUMERS (who to blame):\n"
    blame_flash += "-" * 80 + "\n"
    blame_flash += f"{'#':<3} {'FILE':<50} {'FLASH':>10} {'%':>7}\n"
    blame_flash += "-" * 80 + "\n"

    total_obj_flash = sum(o['flash'] for o in object_sizes)
    for i, obj in enumerate(objects_by_flash[:15], 1):
        name = obj['name']
        if len(name) > 48:
            name = "..." + name[-45:]
        pct = (obj['flash'] / total_obj_flash * 100) if total_obj_flash > 0 else 0
        blame_flash += f"{i:<3} {name:<50} {obj['flash']:>10,} {pct:>6.1f}%\n"

    blame_flash += "-" * 80 + "\n"

    # Top RAM consumers
    blame_ram = "\n💾 TOP RAM CONSUMERS:\n"
    blame_ram += "-" * 80 + "\n"
    blame_ram += f"{'#':<3} {'FILE':<50} {'RAM':>10} {'%':>7}\n"
    blame_ram += "-" * 80 + "\n"

    total_obj_ram = sum(o['ram'] for o in object_sizes)
    for i, obj in enumerate(objects_by_ram[:10], 1):
        if obj['ram'] == 0:
            continue
        name = obj['name']
        if len(name) > 48:
            name = "..." + name[-45:]
        pct = (obj['ram'] / total_obj_ram * 100) if total_obj_ram > 0 else 0
        blame_ram += f"{i:<3} {name:<50} {obj['ram']:>10,} {pct:>6.1f}%\n"

    blame_ram += "-" * 80 + "\n"

    # Print to console
    print(report)
    print(blame_flash)
    print(blame_ram)

    # Full report for file
    full_report = report

    full_report += "\n" + "=" * 80 + "\n"
    full_report += "                    COMPLETE OBJECT FILE ANALYSIS\n"
    full_report += "=" * 80 + "\n\n"

    full_report += "BY FLASH USAGE:\n"
    full_report += "-" * 95 + "\n"
    full_report += f"{'#':<4} {'FILE':<55} {'TEXT':>10} {'DATA':>10} {'FLASH':>10}\n"
    full_report += "-" * 95 + "\n"

    for i, obj in enumerate(objects_by_flash, 1):
        if obj['flash'] == 0:
            continue
        full_report += f"{i:<4} {obj['name']:<55} {obj['text']:>10,} {obj['data']:>10,} {obj['flash']:>10,}\n"

    full_report += "\n\nBY RAM USAGE:\n"
    full_report += "-" * 95 + "\n"
    full_report += f"{'#':<4} {'FILE':<55} {'DATA':>10} {'BSS':>10} {'RAM':>10}\n"
    full_report += "-" * 95 + "\n"

    for i, obj in enumerate(objects_by_ram, 1):
        if obj['ram'] == 0:
            continue
        full_report += f"{i:<4} {obj['name']:<55} {obj['data']:>10,} {obj['bss']:>10,} {obj['ram']:>10,}\n"

    full_report += "\n\nDETAILED ELF SECTIONS:\n"
    full_report += "-" * 80 + "\n"
    full_report += size_output_A

    # Save to file
    report_dir = os.path.join(env.subst("$PROJECT_DIR"), "build_reports")
    os.makedirs(report_dir, exist_ok=True)

    report_file = os.path.join(report_dir, f"size_report_{env_name}.txt")
    with open(report_file, "w") as f:
        f.write(full_report)

    print(f"\n📄 Full report saved to: {report_file}")

# Register the post-build action
env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", post_build_size)
