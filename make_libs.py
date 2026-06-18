"""Gera import libraries (.a) a partir dos DLLs do OBS instalado."""
import subprocess, os

DLLTOOL   = r"C:\msys64\mingw64\bin\dlltool.exe"
OBS_BIN   = r"C:\Program Files\obs-studio\bin\64bit"
WORK_DIR  = r"C:\Users\Denis\Desktop\obs_nrs\obs_include"

os.makedirs(WORK_DIR, exist_ok=True)

for dll_name, lib_name in [
    ("obs.dll",              "libobs.a"),
    ("obs-frontend-api.dll", "libobs-frontend-api.a"),
]:
    dll  = os.path.join(OBS_BIN,  dll_name)
    defp = os.path.join(WORK_DIR, lib_name.replace(".a", ".def"))
    lib  = os.path.join(WORK_DIR, lib_name)

    print(f"Processando {dll_name}...")

    r1 = subprocess.run([DLLTOOL, "--output-def", defp, dll],
                        capture_output=True, text=True)
    if r1.returncode != 0:
        print(f"  ERRO .def: {r1.stderr}"); continue

    r2 = subprocess.run([DLLTOOL, "-d", defp, "-l", lib],
                        capture_output=True, text=True)
    if r2.returncode == 0:
        print(f"  OK -> {lib}")
    else:
        print(f"  ERRO .a: {r2.stderr}")

print("Concluído.")
