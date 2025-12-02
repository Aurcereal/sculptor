## Design Document:

#### Introduction
I plan to create a sculpting tool using marching cubes and WebGPU.  I'm motivated to explore marching cubes in a sculpting context and create a fun tool.

#### Goal
I intend to create a simple sculpting tool in WebGPU that can be used on the web.

#### Inspiration/reference:
- [This Terraforming video by Sebastion Lague](https://www.youtube.com/watch?v=vTMEdHcKgM4)
- [This Marching Squares Article by Jamie Wong](https://jamie-wong.com/2014/08/19/metaballs-and-marching-squares/)
- [Astroneer](http://store.steampowered.com/app/361420/ASTRONEER/), a game that makes heavy use of terraforming.

#### Specification:
I plan on the project featuring a draw brush that allows for the creation of geometry as well as a subtract brush that deletes geometry.  I hope to have different types of brushes as well.  I may also allow the user to use different visual materials.

#### Techniques:
I will almost definitely use marching cubes, but I may use other techniques that I'm not currently aware of.

#### Design:
<img width="484" height="553" alt="image" src="https://github.com/user-attachments/assets/6f7b39cf-6aac-40d7-b936-06c56b3efd62" />

#### Timeline:
- Week 1: Setup WebGPU, Research Terraforming/Sculpting, Implement Basic Marching Cubes
- Week 2: Finish/fix implementation if necessary, add basic sculpting
- Week 3: Troubleshoot, add UI and a few different brushes
- Week 4: Polish, troubleshoot, test building, and potentially add different visual materials

Submit your Design doc as usual via pull request against this repository.

## Milestone 1:

For this milestone, I mostly just setup WebGPU and classes for drawing and using compute shaders with it.  Setting up and learning WebGPU took me more time than I expected.  

## Milestone 2:

For this milestone, I set up compute shaders for generating and updating the 3D scalar field as well as compute shaders for generating the mesh from the field (marching cubes) and a compute shader for raycasting to the surface.  The user can edit geometry with their mouse.  

https://github.com/user-attachments/assets/c61c4dfe-7189-4275-b309-5f1b5789c22c

I plan on adding UI for changing the brush size and power, as well as adding color and smooth normals.  I plan on having extra operations like mirror, twist around an axis, and sdf-like domain repetition.  I'm not sure if I'll have time to add the sdf-like operations for the final submission.

## Final Submission:

### Final Results

[Slideshow Version](https://docs.google.com/presentation/d/1K8j5KwaZzlcoo3jWt414LCfCrM3Et9RGUvkCsfZc7_M/edit?usp=sharing)

#### Overview

The final result is a sculpting tool made with WebGPU and C++.  The user is able to use a variety of brushes and operations to sculpt their model.  All of the mesh operations are done with compute shaders.

![](ShowcaseMedia/broccoli.png)

#### Implementation Details

The model the user creates is represented by a 3D scalar field in a 3D texture.  From the 3D texture, we create a mesh using marching cubes implemented inside a compute shader.  Every edit the user makes is to the 3D scalar field, which then changes the mesh generated.

![](ShowcaseMedia/stripey.png)
![](ShowcaseMedia/redCharacter.png)

#### Functionality

Because the model is represented with a scalar field, we can take advantage of SDF-like brushes and operations.  The user can use different brush shapes like a sphere, cube, and cone; as well as more procedural shapes like sphere fractal, gyroid, voronoi… etc.  There’s also a mirror mode and a twisting brush.

![](ShowcaseMedia/sdflike.png)
![](ShowcaseMedia/voronoi.png)
![](ShowcaseMedia/twirlSpherePattern.png)

#### Possible Future Features

- Brushes & Operations
  - A stretching operation that allows the user to click and drag geometry to stretch it.
  - Brush that makes pipes/other repeated objects (different types of skin or fur or grass)
  - Can draw roughness/metallic as well as albedo (maybe PBR)
- Optimizations
  - Divide the mesh into 3D chunks and only regenerate the part of the mesh that’s changed.  
  - Only dispatch the scalar field updating compute shader in places where there’ll be changes.

![](ShowcaseMedia/ralsei.png)

### Post Mortem



## Final submission (due 12/1)
Time to polish! Spen this last week of your project using your generator to produce beautiful output. Add textures, tune parameters, play with colors, play with camera animation. Take the feedback from class critques and use it to take your project to the next level.

Submission:
- Push all your code / files to your repository
- Come to class ready to present your finished project
- Update your README with two sections 
  - final results with images and a live demo if possible
  - post mortem: how did your project go overall? Did you accomplish your goals? Did you have to pivot?

## Topic Suggestions

### Create a generator in Houdini

### A CLASSIC 4K DEMO
- In the spirit of the demo scene, create an animation that fits into a 4k executable that runs in real-time. Feel free to take inspiration from the many existing demos. Focus on efficiency and elegance in your implementation.
- Example: 
  - [cdak by Quite & orange](https://www.youtube.com/watch?v=RCh3Q08HMfs&list=PLA5E2FF8E143DA58C)

### A RE-IMPLEMENTATION
- Take an academic paper or other pre-existing project and implement it, or a portion of it.
- Examples:
  - [2D Wavefunction Collapse Pokémon Town](https://gurtd.github.io/566-final-project/)
  - [3D Wavefunction Collapse Dungeon Generator](https://github.com/whaoran0718/3dDungeonGeneration)
  - [Reaction Diffusion](https://github.com/charlesliwang/Reaction-Diffusion)
  - [WebGL Erosion](https://github.com/LanLou123/Webgl-Erosion)
  - [Particle Waterfall](https://github.com/chloele33/particle-waterfall)
  - [Voxelized Bread](https://github.com/ChiantiYZY/566-final)

### A FORGERY
Taking inspiration from a particular natural phenomenon or distinctive set of visuals, implement a detailed, procedural recreation of that aesthetic. This includes modeling, texturing and object placement within your scene. Does not need to be real-time. Focus on detail and visual accuracy in your implementation.
- Examples:
  - [The Shrines](https://github.com/byumjin/The-Shrines)
  - [Watercolor Shader](https://github.com/gracelgilbert/watercolor-stylization)
  - [Sunset Beach](https://github.com/HanmingZhang/homework-final)
  - [Sky Whales](https://github.com/WanruZhao/CIS566FinalProject)
  - [Snail](https://www.shadertoy.com/view/ld3Gz2)
  - [Journey](https://www.shadertoy.com/view/ldlcRf)
  - [Big Hero 6 Wormhole](https://2.bp.blogspot.com/-R-6AN2cWjwg/VTyIzIQSQfI/AAAAAAAABLA/GC0yzzz4wHw/s1600/big-hero-6-disneyscreencaps.com-10092.jpg)

### A GAME LEVEL
- Like generations of game makers before us, create a game which generates an navigable environment (eg. a roguelike dungeon, platforms) and some sort of goal or conflict (eg. enemy agents to avoid or items to collect). Aim to create an experience that will challenge players and vary noticeably in different playthroughs, whether that means procedural dungeon generation, careful resource management or an interesting AI model. Focus on designing a system that is capable of generating complex challenges and goals.
- Examples:
  - [Rhythm-based Mario Platformer](https://github.com/sgalban/platformer-gen-2D)
  - [Pokémon Ice Puzzle Generator](https://github.com/jwang5675/Ice-Puzzle-Generator)
  - [Abstract Exploratory Game](https://github.com/MauKMu/procedural-final-project)
  - [Tiny Wings](https://github.com/irovira/TinyWings)
  - Spore
  - Dwarf Fortress
  - Minecraft
  - Rogue

### AN ANIMATED ENVIRONMENT / MUSIC VISUALIZER
- Create an environment full of interactive procedural animation. The goal of this project is to create an environment that feels responsive and alive. Whether or not animations are musically-driven, sound should be an important component. Focus on user interactions, motion design and experimental interfaces.
- Examples:
  - [The Darkside](https://github.com/morganherrmann/thedarkside)
  - [Music Visualizer](https://yuruwang.github.io/MusicVisualizer/)
  - [Abstract Mesh Animation](https://github.com/mgriley/cis566_finalproj)
  - [Panoramical](https://www.youtube.com/watch?v=gBTTMNFXHTk)
  - [Bound](https://www.youtube.com/watch?v=aE37l6RvF-c)

### YOUR OWN PROPOSAL
- You are of course welcome to propose your own topic . Regardless of what you choose, you and your team must research your topic and relevant techniques and come up with a detailed plan of execution. You will meet with some subset of the procedural staff before starting implementation for approval.
