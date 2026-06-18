# Como instalar o Guide Overlay para OBS

## O que é esse plugin?
Uma fonte de vídeo que mostra guias de composição (regra dos terços, mira central, áreas seguras) **visíveis apenas no preview do OBS** — elas não aparecem na gravação nem no stream.

---

## MÉTODO MAIS RÁPIDO — Instalador automático

Na pasta do projeto há scripts para gerar um instalador `.exe` que copia tudo sozinho:

### Opção A — Inno Setup (recomendado)
1. Baixe e instale o [Inno Setup](https://jrsoftware.org/isdl.php)
2. Clique com direito em `installer.iss` → **Compile**
3. O instalador será gerado em `installer_output\obs-guide-overlay-setup-1.0.0.exe`

### Opção B — NSIS
1. Baixe e instale o [NSIS](https://nsis.sourceforge.io/Download)
2. Clique com direito em `installer.nsi` → **Compile NSIS Script**
3. O instalador será gerado como `obs-guide-overlay-setup-1.0.0.exe`

### Opção C — Script automático
Execute `build_installer.bat` — ele tenta compilar com Inno Setup ou NSIS automaticamente.

Ambos criam um instalador que detecta a pasta do OBS e instala a DLL e os arquivos de idioma nos lugares corretos. Também registram desinstalador no Windows.

---

## MÉTODO MAIS FÁCIL — GitHub Actions (sem instalar nada)

A maneira mais simples é deixar o GitHub compilar o plugin por você automaticamente.

### Passo 1 — Criar uma conta no GitHub
1. Acesse https://github.com
2. Clique em **Sign up** e crie uma conta gratuita

### Passo 2 — Criar o repositório com os arquivos do plugin
1. Faça login no GitHub
2. Clique no botão **+** (canto superior direito) → **New repository**
3. Nome do repositório: `obs-guide-overlay`
4. Marque **Public** (obrigatório para Actions gratuito)
5. Clique em **Create repository**

### Passo 3 — Fazer o upload dos arquivos
1. Na página do repositório, clique em **uploading an existing file**
2. Arraste **todos os arquivos e pastas** desta pasta (`obs_nrs`) para a área de upload
3. Clique em **Commit changes**

### Passo 4 — Baixar o plugin compilado
1. No repositório, clique na aba **Actions**
2. Clique no workflow **Build Plugin** mais recente
3. Aguarde ele ficar verde (✓) — leva cerca de 5–10 minutos
4. Role a página para baixo até **Artifacts**
5. Clique em **obs-guide-overlay-windows-x64** para baixar o `.zip`

### Passo 5 — Instalar no OBS
1. Extraia o `.zip` baixado
2. Copie os arquivos para:
   - `obs-guide-overlay.dll` → `C:\Program Files\obs-studio\obs-plugins\64bit\`
   - Pasta `obs-guide-overlay` (com os arquivos de locale) → `C:\Program Files\obs-studio\data\obs-plugins\`
3. Reinicie o OBS

---

## Como usar no OBS após instalar

1. No OBS, clique em **+** na lista de **Fontes (Sources)**
2. Selecione **Guide Overlay (Regras de Composição)**
3. Clique com o botão direito na fonte → **Propriedades** para configurar:
   - **Somente preview**: mantenha marcado — é isso que impede de aparecer na gravação
   - **Regra dos terços**: linhas em grade 3x3
   - **Mira central**: cruz no centro da tela
   - **Área de ação segura (90%)**: borda interna de 5% em cada lado
   - **Área de título segura (80%)**: borda interna de 10% em cada lado
   - **Cor das linhas**: escolha a cor que quiser
   - **Opacidade**: transparência das linhas (0–100%)
   - **Espessura**: espessura das linhas em pixels
4. Redimensione a fonte para cobrir a tela inteira (clique com botão direito → **Transform** → **Fit to screen**)

### Posicione a fonte acima das outras
Na lista de **Fontes**, arraste o **Guide Overlay** para o **topo da lista** para que apareça sobre tudo.

---

## Verificando que não aparece na gravação
1. Inicie uma gravação de teste curta
2. A grade some do preview assim que a gravação começa
3. Assista o arquivo gravado — as linhas **não estarão presentes**

---

## Solução de problemas

| Problema | Solução |
|---|---|
| Plugin não aparece na lista do OBS | Verifique se o `.dll` está em `obs-plugins\64bit` e reinicie o OBS |
| As linhas aparecem na gravação | Na fonte, abra Propriedades e certifique que "Somente preview" está marcado |
| O GitHub Actions falhou | Vá em Actions → clique no workflow com ✗ → veja os logs de erro |
| Cor muito fraca | Aumente a Opacidade nas propriedades da fonte |
