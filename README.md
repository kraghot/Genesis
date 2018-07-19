# Genesis - Game Development SS18

## Compiling

The code does not depend on any libraries other than those required by Glow,
so the compilation should be relatively straight forward, by simply running 
CMake and builing the project. We recommend that the project is built in release
mode as it provides significant performance improvements.

## Running

The `GlowApp` binary needs to be run from within the `/bin` folder in order to
have access to the external resources.

## Usage

The default ordering of editing to be applied is optimally:
- Keep generating terrains until a satisfying one is found
- Edit the terrain using the height brush
- Recalculate the textures
- Edit the terrain using the texture brush
- Apply the Thermal and Hydraulic erosion
- Set the wind direction
- Turn off Edit mode and enjoy

## Project features

This application offers many features in the area of procedural generation, 
terrain and texture manipulation, as well as biomes and erosion effects.

### Procedural generation
You are starting with a top-down overview of the island. 
Begin with procedurally generating the terrain, by pressing the `Generate random terrain` button. 
Behind the scenes:
 - Interface for noise generators is implemented so that it is easily extensible
 - Multiple noise generators can be mixed together with different properties
 - Perlin noise and DiamondSquare noise are already implemented
 - In the master branch only the DiamondSquare is used as it is enough to get a realistic looking terrain

As a result, you will get a realistic looking island terrain which is round-shaped and without remaining square-shaped heightmap leftovers.
 - Multiple filters can also be used to process the terrain after it has been generated
 - There are 2 filters provided: Square-Shaped and Circular-Shaped Island maks generators which make the terrain shaped like an island
 - The Square-Shaped Island Mask includes a random component so that the mask generated does not look uniform and artificial


After that, you are able to choose between an Edit and Preview mode, which is 
the `Edit Mode` checkbox. In the Edit mode, no meshes are generated, which 
improves performance and gives a better preview of the textures and terrain 
properties for editing. Let's for now stay in Edit Mode.

### Editing
We now decide to manipulate our terrain using the texture brush. For this, we 
choose the desired texture to paint from the `Texture Brush` dropdown menu, as 
well as the Texture Brush from the `Brush Type` dropdown.
If the height brush is used, it is possible to increase or decrease the height 
of the terrain based on the height brush factor (intensity) in the `Height Brush`
text box. Do not forget to recalculate the textures by clicking the `Recalcuate textures` 
button, after using the height brush to get an optimal result.
Use both brushes by clicking the right mouse click.
 - Mouse-Terrain intersection is implemented and used by default by unprojection of the pixel that the cursor is on
 - Ray-Terrain intersection is also implemented and optimized using quadtrees made specifically for heightmaps
 - Smoothing between textures from the center towards the edges of the circle
 - Built with a focus on performance, i.e. only checking vertices which are inside a bounding box around the circle
 - Texture splatting with index maps used for having up to 4 different textures per pixel out of 255 possible ones
 - Optimized recalculation of normals and tangents in real time when height brush is used
 - Scrollwheel control of the circle radius value

 

### Erosion
The user is also able to use 2 types of erosion:
- Thermal Erosion
- Hydraulic Erosion using the droplet model

#### Thermal Erosion
The thermal erosion is implemented using an algorithm with an accent on 
performance. It is able to quickly modify large portions of the terrain and one
pass influences the whole terrain. It is usually sufficient to run the Thermal
Erosion only one time.

#### Hydraulic Erosion
Hydraulic Erosion is implemented using the droplet model which simulates rainfall
and the traversal of each rain drop all over the terrain. It does not influence
the terrain as much as the Thermal Erosion, but it does however add an extra
degree of realism. Using the droplet erosion water bodies such as rivers and
lakes are also simulated as filling in the portions of the terrain where the rain
has flown through the most.

### Biomes
After disabling `Edit Mode`, you are in `Preview mode`, which now renders the 
meshes and gives you a preview of different biomes on the island. You are able
to influence the generation of biomes by either choosing a specific wind direction 
from the `Wind Direction` dropdown menu and pressing the `Set wind direction` button,
or randomly generating it, since biomes depend on the humidity of the terrain 
(wind carries rain).
 - Simulates how much rain from a specific direction falls on the island, considering different terrain properties such as height
 - Textures of different biomes are interpolated
 - Every biome has its own set of different vegetation
 - Every type of vegetation is randomly distributed using Poisson Disc Sampling with multiple constraints such as terrain height or lakes/rivers
 - Trees on steep slopes are rotated towards the positive y axis
 - Every vegetation object has a random scaling component
 - Rendering of meshes is optimized using instancing
 