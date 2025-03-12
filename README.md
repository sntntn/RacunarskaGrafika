# Project: Goldfish and Parachute Worms
A project for computer graphics.
The project skeleton [project base](https://github.com/matf-racunarska-grafika/project_base) was used, along with materials from the [LearnOpenGL](https://github.com/matf-racunarska-grafika/LearnOpenGL.git) repository.
Mandatory topics have been implemented.

### Additional Topics:

From Group A:
- [x] Cubemaps (Skybox)

From Group B:
- [x] HDR (exposure)


# Instructions
1. `git clone https://github.com/sntntn/RacunarskaGrafika`
2. Open CLion -> Open -> path/to/my/project_base
3. The main function is located in `src/main.cpp`
4. ALT+SHIFT+F9 -> project_base -> run


## Description
The goal is to activate all parachute worms and feed them to our goldfish.
With each worm eaten, the fish's glow becomes stronger until it eats the last worm.
Once the last worm is eaten, the mystical box below the fish transforms into a golden coin (which allows us to restart).
Each time a worm is activated, its accompanying star is also activated.

---
### Controls
---
`ESC` Terminates the project execution

#### Camera movement:
- `W` `A` `S` `D`
- Moving the mouse rotates the camera
- Mouse scroll zooms in and out

#### Toggle Features:
`Space` Prints camera coordinates
`B` Activates the first parachute worm
`N` Activates the second parachute worm
`M` Activates the third parachute worm
`R` RESTART (enabled when the coin appears)

`P` Blinn
`O` SpotLight

`1` HDR
`F` BLOOM

`Q` Increase Exposure
`E` Decrease Exposure

---
### ImGui:
`I` Opens/closes the ImGui window

## Resources:
[Coin](https://rigmodels.com/model.php?view=Coin-3d-model__I89O58TBZ353I4X9ANHTRFF5K&searchkeyword=coin&manualsearch=1)
[Parachute](https://rigmodels.com/model.php?view=Parachute_Worm-3d-model__Q41SJLO72KVS0UJF2D1DM2OXO&searchkeyword=parachute&manualsearch=1)
[Star](https://rigmodels.com/model.php?view=Star_Fish-3d-model__O5T6WV158SFXN8YLU5HYHWW1Q&searchkeyword=star&manualsearch=1)
[Fish](https://rigmodels.com/model.php?view=Fish-3d-model__L5GWQCNY48U447XE7EQDWOFVC&searchkeyword=fish&manualsearch=1)

### [YouTube Video Presentation without HDR](https://www.youtube.com/watch?v=PqqQ0k7qxVU)
### [YouTube Video Presentation with HDR](https://www.youtube.com/watch?v=22H2bCjXMRk)
### [YouTube Video Presentation with HDR and BLOOM](https://www.youtube.com/watch?v=DAeUchHD-hc)

**Author:** Vukašin Marković 312/2019

