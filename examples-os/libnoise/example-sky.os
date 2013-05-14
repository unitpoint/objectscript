require "lib"

// This texture map is made up two layers.  The bottom layer is a wavy water
// texture.  The top layer is a cloud texture.  These two layers are
// combined together to create the final texture map.

// Lower layer: water texture
// --------------------------

// Base of the water texture.  The Voronoi polygons generate the waves.  At
// the center of the polygons, the values are at their lowest.  At the edges
// of the polygons, the values are at their highest.  The values smoothly
// change between the center and the edges of the polygons, producing a
// wave-like effect.
var baseWater = NoiseVoronoi {
	seed = 0,
	frequency = 8.0,
	distance = true,
	displacement = 0.0,
}

// Stretch the waves along the z axis.
var baseStretchedWater = NoiseScalePoint {
	sourceModule = baseWater,
	xScale = 1,
	yScale = 1,
	zScale = 3,
}

// Smoothly perturb the water texture for more realism.
var finalWater = NoiseTurbulence {
	sourceModule = baseStretchedWater,
	seed = 1,
	frequency = 8.0,
	power = 1.0 / 32.0,
	roughness = 1,
}

// Upper layer: cloud texture
// --------------------------

// Base of the cloud texture.  The billowy noise produces the basic shape
// of soft, fluffy clouds.
var cloudBase = NoiseBillow {
	seed = 2,
	frequency = 2.0,
	persistence = 0.375,
	lacunarity = 2.12109375,
	octaveCount = 4,
	noiseQuality = NOISE_QUALITY_BEST,
}

// Perturb the cloud texture for more realism.
var finalClouds = NoiseTurbulence {
	sourceModule = cloudBase,
	seed = 3,
	frequency = 16.0,
	power = 1.0 / 64.0,
	roughness = 2,
}

var TEXTURE_HEIGHT = 256

function createTextureColorLayer1(renderer){
  // Create a water palette with varying shades of blue.
  renderer.clearGradient()
  renderer.addGradientPoint(-1.00, [ 48,  64, 192, 255])
  renderer.addGradientPoint( 0.50, [ 96, 192, 255, 255])
  renderer.addGradientPoint( 1.00, [255, 255, 255, 255])
}

function createTextureColorLayer2(renderer){
  // Create an entirely white palette with varying alpha (transparency) values
  // for the clouds.  These transparent values allows the water to show
  // through.
  renderer.clearGradient()
  renderer.addGradientPoint(-1.00, [255, 255, 255,   0])
  renderer.addGradientPoint(-0.50, [255, 255, 255,   0])
  renderer.addGradientPoint( 1.00, [255, 255, 255, 255])
}

function renderTexture(lowerNoiseMap, upperNoiseMap, filename, seamless){
	// Create the color gradients for the lower texture.
	var textureRenderer = NoiseRenderer()
	createTextureColorLayer1(textureRenderer)

	// Set up us the texture renderer and pass the lower noise map to it.
	var destTexture = NoiseImage()
	textureRenderer.attrs {
		sourceNoiseMap = lowerNoiseMap,
		destImage = destTexture,
		wrap = seamless,
		light = true,
		lightAzimuth = 135.0,
		lightElev = 60.0,
		lightContrast = 2.0,
		lightColor = [255, 255, 255, 0],
	}
	// Render the texture.
	textureRenderer.render()

	// Create the color gradients for the upper texture.
	createTextureColorLayer2(textureRenderer);

	// Set up us the texture renderer and pass the upper noise map to it.  Also
	// use the lower texture map as a background so that the upper texture map
	// can be rendered on top of the lower texture map.
	textureRenderer.attrs {
		sourceNoiseMap = upperNoiseMap,
		backgroundImage = destTexture,
		destImage = destTexture,
		light = false,
	}
	
	// Render the texture.
	textureRenderer.render()

	// Write the texture as a Windows bitmap file (*.bmp).
	print "writeBMP"
	destTexture.writeBMP(filename)
}

function createSkyPlanar(lowerNoiseModule, upperNoiseModule, seamless, height){
	var lowerNoiseMap = createPlanarMap(lowerNoiseModule, seamless, height)
	var upperNoiseMap = createPlanarMap(upperNoiseModule, seamless, height)
	renderTexture(lowerNoiseMap, upperNoiseMap, seamless ? "sky-seamless.bmp" : "sky-plane.bmp", seamless)
}

createSkyPlanar(finalWater, finalClouds, false, TEXTURE_HEIGHT)
createSkyPlanar(finalWater, finalClouds, true, TEXTURE_HEIGHT)

function createSkySpherical(lowerNoiseModule, upperNoiseModule, height){
	var lowerNoiseMap = createSphericalMap(lowerNoiseModule, height)
	var upperNoiseMap = createSphericalMap(upperNoiseModule, height)
	renderTexture(lowerNoiseMap, upperNoiseMap, "sky-sphere.bmp")
}

createSkySpherical(finalWater, finalClouds, TEXTURE_HEIGHT)