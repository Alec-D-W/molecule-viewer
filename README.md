# Molecule Viewer

A small C++/OpenGL hobby project for visualizing molecular structures. The current build displays atoms from a PDB file. Volumetric rendering from an OpenDX grid is experimental and may not render correctly.
Customization options are planned for the near future.

## Requirements

- Windows
- Visual Studio 2022 with the Desktop development with C++ workload
- A graphics card and driver supporting OpenGL 3.3 or later

The project uses the Visual Studio solution and includes its header dependencies under `resources/include`.

## Build

1. Open `molecule-viewer.sln` in Visual Studio.
2. Select the `Release` configuration and `x64` platform.
3. Build the solution.

The application loads shaders and model data using paths relative to its working directory. Run it from a directory containing the `shaders/` and `models/` folders described below.

## Configuration

Settings are loaded from `config/config.json` at startup. Keep the `config/` folder beside the executable in a Release package, and run the application with the extracted folder as its working directory. The checked-in example config is at `molecule-viewer/config/config.json`.

Edit the JSON file before starting the application; settings are not reloaded while it is running. Missing settings use built-in defaults, and invalid values cause the application to report a configuration error.

| Section | Settings |
| --- | --- |
| `molecule` | Relative paths to the atom PDB and optional volume DX files |
| `window` | Window `width`, `height`, and `vsync` |
| `camera` | `fovDegrees` and `positionOffset`, which places the camera relative to the loaded molecule's center |
| `rendering` | Initial `showAtoms` and `showVolume` states, volume `volumeStepSize` and `volumeMaxSteps`, plus `sphereSegments`, `sphereRings`, and `sphereRadius` |
| `atomStyles` | Per-element RGB colors in the range 0 to 1; elements not listed use built-in colors |
| `graphics` | Background and lighting colors, directional-light direction, Blinn-Phong `specularColor`, `specularStrength`, and `shininess`, and `msaaSamples` |

Increasing `sphereSegments` or `sphereRings` makes atom spheres smoother but increases geometry and rendering cost. `sphereRadius` scales the sphere mesh and therefore also affects the visible atom sizes. `volumeStepSize` and `volumeMaxSteps` trade volume quality against performance.

## Run a Release

Download and extract the latest Release archive, then run `molecule-viewer.exe` from the extracted directory. The archive must retain this layout:

```text
release/
	molecule-viewer.exe
	config/
		config.json
	models/
		2RT4.pdb
		volmap_7.dx
	shaders/
		model_frag.fs
		model_vert.vs
		volume_frag.fs
		volume_vert.vs
```

The PDB and DX model files are not included in the source repository; they are required at runtime and are distributed in the Release archive.

## Controls

| Input | Action |
| --- | --- |
| `W`, `A`, `S`, `D` | Move the camera |
| Mouse | Look around |
| Mouse wheel | Adjust camera zoom |
| `E` | Toggle atom rendering |
| `V` | Toggle volumetric rendering (experimental) |
| `R` | Reload shaders |
| `Esc` | Exit |

## Model Data

The included structure is PDB entry [2RT4](https://www.rcsb.org/structure/2RT4) ([citation](https://doi.org/10.2210/pdb2RT4/pdb)). The bundled `volmap_7.dx` is a volumetric grid for the same molecule.

To generate a volumetric map from PDB data, [VMD](https://www.ks.uiuc.edu/Research/vmd/) can be used.

## Pictures:
<img width="927" height="679" alt="image" src="https://github.com/user-attachments/assets/15175737-1522-419f-b8a8-a66f29db4074" />
<img width="982" height="695" alt="image" src="https://github.com/user-attachments/assets/5336f817-cc6e-4b59-98b1-0074ab43bb94" />
<img width="1305" height="840" alt="image" src="https://github.com/user-attachments/assets/7b478b24-c476-4fe1-b1d7-ecb4bb3d025c" />

Atom colours:

Hydrogen - white

Carbon - grey

Nitrogen - blue

Oxygen - red

Fluorine - green

Phosphorous - orange

sulfur - yellow

(Colours might change in future updates)
