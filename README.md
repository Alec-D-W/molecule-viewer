# Molecule Viewer

A small C++/OpenGL hobby project for visualizing molecular structures. The current build displays atoms from a PDB file. Volumetric rendering from an OpenDX grid is experimental and may not render correctly.

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

## Run a Release

Download and extract the latest Release archive, then run `molecule-viewer.exe` from the extracted directory. The archive must retain this layout:

```text
release/
	molecule-viewer.exe
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
