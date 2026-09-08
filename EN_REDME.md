# ASoVtetra_v0.5.1 README (English)

- Japanese version is available here: README.md
- Download  
You can download the latest version from the Releases page:  
https://github.com/SwishmarShell/ASoVtetra_version0.5.1/releases/latest


# ASoVtetra_version0.5.1

## Version
Current version: **v0.5.1**


## Features of This Version

- Load PNG images and render them directly on the Windows desktop.  
  (Recommended PNG bit depth: **32-bit**)

- Multiple PNG images can be loaded as **layers**, allowing composite rendering.

- The main drawing window can be moved and resized freely.  
  Each PNG keeps its original size and is drawn from a fixed origin point.

- Press **F1** to switch the drawing window into “fixed mode”.
  - The window becomes **TopMost** and is rendered using Direct2D layered rendering.
  - The white background window disappears, leaving only the PNG composite visible.
  - Clicking on the displayed image will pass mouse events through to windows behind it.
    (The image becomes non-interactive.)

- Press **F2** to open the **Settings** window.


## How to Use

### 1. Open the Settings Window
Press **F2** to open the Settings window.  
(Recommended: close the Settings window after configuration.)

### 2. Add Layers
Click **“Add Layer”** to create a new layer.

Each layer provides:
- Layer name  
- “Load PNG” button  
- “Alias ON” checkbox  
- Move layer forward/backward  
- Delete layer  

### 3. Move and Resize the Drawing Window
You can freely move and resize the drawing window.  
PNG sizes do not change.

### 4. Fix the Drawing Window (F1)
Press **F1** to fix the window:
- The composite PNG image is drawn at the top of the desktop.
- The background window disappears.
- Transparent PNGs will show the desktop behind them.
- Mouse clicks pass through the image to underlying windows.

### 5. Save Project
Click **“Save Project”** to store all layers and PNGs into a folder.  
(Please prepare a folder beforehand — this will be improved in future versions.)

Folder structure:

>             //YourProjectFolder<br>
>             　/ config.ini<br>
>             　/ bitmaps<br>
>             　/ layers<br>
>             　　/ layer(PNG_FileName).png<br>
>             　※ All referenced images are copied to a single folder for easier management.
> <br>

### 6. Load Project
Click **“Load Project”** and select a saved project folder.
- All layers and PNGs are restored.
- A small “Load Folder” dialog will appear; click OK to continue.


## Improvements in v0.5.1

- Project saving changed to **folder-based format**  
  (`config.ini`, `layers/`, `bitmaps/`)
- Fixed an issue where images were not visible immediately after loading.
- Improved Direct2D initialization timing for stable rendering.
- Removed outdated bitmap cache issues.
- Internal code structure cleaned and prepared for future extensions.


## Example Use Cases

- Keep your original logo always visible during desktop capture.
- Freely position and fix character images in OBS scenes.
- Display motivational character images on your desktop without interfering with work.
- Combine multiple transparent PNG parts to create layered character expressions.
- Carry your project folder and application to another PC and continue working.


## System Requirements & How to Start

### Recommended Environment
- Windows 11 64-bit (x64)
- Windows 11 32-bit (x86)

### How to Start
- Download the appropriate ZIP file from the Releases page.
- Extract the ZIP.
- Run **ASoVtetra_version0.5.1.exe** inside the folder.

A sample project folder is included for testing.


## License

This project is released under the **MIT License**.  
Redistribution, modification, and commercial use are permitted under the conditions of the MIT license.

Copyright (c) 2026 Swishmar.Shell


## CREDIT

If you use this project, a credit such as  
**“Based on work by Swishmar.Shell”**  
would be appreciated.


## Notes

- Developed on Windows 11 (x64).  
  Operation on Windows 10 or other environments is not guaranteed.

- Future versions may include ONNX Runtime features, which will require 64-bit OS.
