"""
Baixa os headers necessários do OBS 32.0.1 do GitHub
e gera as import libraries para compilação do plugin.
"""
import urllib.request
import zipfile
import io
import os
import subprocess

OBS_TAG = "32.0.1"
OBS_ZIP = f"https://github.com/obsproject/obs-studio/archive/refs/tags/{OBS_TAG}.zip"
INCLUDE_DIR = r"C:\Users\Denis\Desktop\obs_nrs\obs_include"
MSYS2_DLLTOOL = r"C:\msys64\mingw64\bin\dlltool.exe"
OBS_BIN = r"C:\Program Files\obs-studio\bin\64bit"

# Prefixos de headers que queremos extrair
HEADER_PREFIXES = [
    f"obs-studio-{OBS_TAG}/libobs/",
    f"obs-studio-{OBS_TAG}/UI/obs-frontend-api/",
]

print(f"Baixando fonte do OBS {OBS_TAG} (apenas headers)...")
print("Isso pode demorar 1-2 minutos...")

req = urllib.request.Request(OBS_ZIP, headers={"User-Agent": "Mozilla/5.0"})
with urllib.request.urlopen(req, timeout=120) as response:
    data = response.read()
    print(f"Download concluído: {len(data)//1024//1024} MB")

print("Extraindo headers...")
os.makedirs(INCLUDE_DIR, exist_ok=True)

with zipfile.ZipFile(io.BytesIO(data)) as z:
    extracted = 0
    for name in z.namelist():
        if not name.endswith(".h"):
            continue
        # Verifica se é um header que nos interessa
        for prefix in HEADER_PREFIXES:
            if name.startswith(prefix):
                # Calcula o caminho de destino preservando a estrutura
                rel = name[len(f"obs-studio-{OBS_TAG}/"):]
                dest = os.path.join(INCLUDE_DIR, rel)
                os.makedirs(os.path.dirname(dest), exist_ok=True)
                with z.open(name) as src, open(dest, "wb") as dst:
                    dst.write(src.read())
                extracted += 1
                break

print(f"Extraídos {extracted} headers")

# Gera import libraries a partir dos DLLs instalados
print("\nGerando import libraries dos DLLs do OBS...")

for dll_name, lib_name in [("obs.dll", "libobs.a"), ("obs-frontend-api.dll", "libobs-frontend-api.a")]:
    dll_path = os.path.join(OBS_BIN, dll_name)
    def_path = os.path.join(INCLUDE_DIR, lib_name.replace(".a", ".def"))
    lib_path = os.path.join(INCLUDE_DIR, lib_name)

    if not os.path.exists(dll_path):
        print(f"AVISO: {dll_path} não encontrado")
        continue

    # Extrai símbolos exportados do DLL com dumpbin via PowerShell
    print(f"  Processando {dll_name}...")
    result = subprocess.run(
        ["powershell", "-Command",
         f"(& '{{0}}' /exports '{dll_path}' 2>&1)".format(
             r"C:\Windows\System32\dllhost.exe"  # placeholder
         )],
        capture_output=True, text=True
    )

    # Usa gendef via PowerShell para gerar .def
    gendef_result = subprocess.run(
        [r"C:\msys64\mingw64\bin\gendef.exe" if os.path.exists(r"C:\msys64\mingw64\bin\gendef.exe") else "gendef",
         dll_path],
        capture_output=True, text=True, cwd=INCLUDE_DIR
    )

    # gendef não existe, usa dlltool direto com --output-def
    result2 = subprocess.run(
        [MSYS2_DLLTOOL, "--output-def", def_path, dll_path],
        capture_output=True, text=True
    )

    if result2.returncode != 0:
        print(f"  ERRO ao gerar .def: {result2.stderr}")
        continue

    # Gera a import lib a partir do .def
    result3 = subprocess.run(
        [MSYS2_DLLTOOL, "-d", def_path, "-l", lib_path, "--target-bfd=pe-x86-64"],
        capture_output=True, text=True
    )

    if result3.returncode == 0:
        print(f"  {lib_name} gerado com sucesso")
    else:
        print(f"  ERRO ao gerar {lib_name}: {result3.stderr}")

print("\nPronto! Agora pode compilar o plugin.")
