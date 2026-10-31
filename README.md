# Monitor Utility — Windows x64 VST3 (REAPER)

Este proyecto reconstruye en C++/JUCE el efecto descrito por los archivos originales de Flow Music. No es un exportador oficial de Flow Music: es una implementación nativa independiente basada en el comportamiento DSP visible en `processor.wasm.ts`.

## Incluye
- Bass: filtro low-pass Butterworth de 2.º orden (12 dB/oct), 20–1000 Hz, default 250 Hz.
- Mids: high-pass seguido de low-pass, ambos Butterworth de 2.º orden, 20–2000 Hz y 500–16000 Hz.
- Highs: high-pass Butterworth de 2.º orden, 500–16000 Hz, default 3500 Hz.
- Solo Bass / Solo Mids / Solo Highs (botones conmutables).
- Monitor Mid Mono: suma L+R y entrega la misma señal mono a ambos canales.
- Parámetros guardados en el estado del plugin.
- Atajos 1, 2, 3 y 4 cuando la ventana del plugin tiene el foco.

## Compilación remota (no requiere instalar compiladores localmente)

1. Crea o inicia sesión en GitHub.
2. Crea un repositorio nuevo llamado `MonitorUtility` (puede ser privado).
3. Descomprime este ZIP.
4. En el repositorio, usa **Add file → Upload files** y sube el contenido de esta carpeta (no la carpeta ZIP).
5. Confirma el commit en la rama `main`.
6. En GitHub abre **Actions**. El workflow **Build Windows VST3** se ejecutará automáticamente. También puedes ejecutarlo con **Run workflow**.
7. Cuando termine en verde, abre la ejecución y descarga el artifact `MonitorUtility-Windows-x64-VST3`.
8. Descomprime el ZIP descargado. Copia la carpeta `Monitor Utility.vst3` a:
   `C:\Program Files\Common Files\VST3`
9. Abre REAPER → **Options → Preferences → Plug-ins → VST** → **Re-scan**. Busca `Monitor Utility` en FX.

## Importante
- El build aún no se ha ejecutado: se ejecutará al subir el proyecto a GitHub. No se incluye un binario VST3 precompilado.
- La GUI es una reconstrucción nativa sencilla; los atajos 1–4 alternan los estados (el original usa pulsación momentánea para los tres Solo).
- Los parámetros Solo se implementan como toggles por compatibilidad y facilidad de uso en un plugin estándar. Revisa el comportamiento auditivamente antes de usarlo en sesiones críticas.
- El DSP mantiene la topología de filtros del original, pero el cambio de coeficientes se actualiza por bloque y puede producir artefactos si se mueven los controles durante audio. Es recomendable probarlo y ajustar suavizado antes de distribución.
- El runtime original `dsp-runtime` no fue proporcionado; por ello se reimplementaron los filtros con JUCE DSP. La lógica de medidores del archivo original no se ha replicado en esta primera versión.
- JUCE se distribuye bajo AGPLv3 o licencia comercial. Antes de distribuir o vender este plugin, revisa qué licencia de JUCE corresponde a tu caso: https://juce.com/get-juce/ y https://github.com/juce-framework/JUCE/blob/master/LICENSE.md
- Este repositorio descarga JUCE 8.0.10 automáticamente durante el build. CMake 3.22+ y un toolchain MSVC son necesarios en el runner; el workflow usa `windows-latest`.

## Fuentes originales
Los archivos `editor.tsx` y `processor.wasm.ts` se conservan en `OriginalFlowFiles` como referencia. `params.json` documenta los parámetros.
