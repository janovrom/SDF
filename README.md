# Deferred shading using SDF for CTU course PGR2

## Overview
This work focuses on creating deferred shading which would combine rasterized objects and objects (ie. terrain) created by raymarching (specifically sphere tracing). 
Each of the deferred shading passes consists of two steps - rasterization and raymarching. All passes are computed each frame.
1. Shadow pass 
    * Generate shadow maps for point lights
    * Generate shadow maps for directional lights
2. Geometry pass - render position, depth, normals and color
    * Rasterize scene
    * Raymarch scene
3. Light pass - render to intermediate texture with blending 1-to-1
    * Render sphere for each point light - light is computed only on fragments that pass stencil and depth test for this sphere
    * Render screen-space quad for each directional light - add shadows from skybox using raymarching
4. Final pass
    * Blit intermediate texture to default framebuffer
5. Intermediate pass
    * Display shadow maps, color, normal, depth, position textures from geometry pass and show noise texture

More on wiki.