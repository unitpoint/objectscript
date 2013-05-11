require "lib"

// Primary granite texture.  This generates the "roughness" of the texture
// when lit by a light source.
var primaryGranite = NoiseBillow {
	seed = 0,
	frequency = 4.0,
	persistence = 0.625,
	lacunarity = 2.18359375,
	octaveCount = 6,
	noiseQuality = NOISE_QUALITY_BEST,
}

// Use Voronoi polygons to produce the small grains for the granite texture.
var baseGrains = NoiseVoronoi {
	seed = 1,
	frequency = 16.0,
	distance = true,
}

// Scale the small grain values so that they may be added to the base
// granite texture.  Voronoi polygons normally generate pits, so apply a
// negative scaling factor to produce bumps instead.
var scaledGrains = NoiseScaleBias {
	sourceModule = baseGrains,
	scale = -0.5,
	bias = 0.0,
}

// Combine the primary granite texture with the small grain texture.
var combinedGranite = NoiseAdd {
	sourceModule0 = primaryGranite,
	sourceModule1 = scaledGrains,
}

// Finally, perturb the granite texture to add realism.
var finalGranite = NoiseTurbulence {
	sourceModule = combinedGranite,
	seed = 2,
	frequency = 4.0,
	power = 1.0 / 8.0,
	roughness = 6,
}

function createTextureColor(renderer){
	renderer.gradient {
		-1.0000: [  0,   0,   0, 255],
		-0.9375: [  0,   0,   0, 255],
		-0.8750: [216, 216, 242, 255],
		 0.0000: [191, 191, 191, 255],
		 0.5000: [210, 116, 125, 255],
		 0.7500: [210, 113,  98, 255],
		 1.0000: [255, 176, 192, 255],	
	}
}

function renderTexture(noiseMap, filename, seamless){
	print "renderTexture"
	
	// Set up us the texture renderer and pass the noise map to it.
	var destTexture = NoiseImage()
	
	var textureRenderer = NoiseRenderer {
		sourceNoiseMap = noiseMap,
		destImage = destTexture,
		wrap = seamless,
		light = true,
		lightAzimuth = 135.0,
		lightElev = 60.0,
		lightContrast = 2.0,
		lightColor = [255, 255, 255, 0],
	}
	
	// Create the color gradients for the texture.
	createTextureColor(textureRenderer)
	
	// Render the texture.
	textureRenderer.render()

	// Write the texture as a Windows bitmap file (*.bmp).
	print "writeBMP"
	destTexture.writeBMP(filename)
	
	renderNormalMap(noiseMap, filename.replace(Regexp("/(\..+?)$/"), "-normalmap$1"))
}

// Given the wood noise module, create a non-seamless texture map, a
// seamless texture map, and a spherical texture map.
var TEXTURE_HEIGHT = 256

function createPlanar(noiseModule, seamless, height, filename){
	var noiseMap = createPlanarMap(noiseModule, seamless, height)
	renderTexture(noiseMap, filename, seamless)
}

createPlanar(finalGranite, false, TEXTURE_HEIGHT, "granite-plane.bmp")
createPlanar(finalGranite, true, TEXTURE_HEIGHT, "granite-seamless.bmp")

renderTexture(createSphericalMap(finalGranite, TEXTURE_HEIGHT), "granite-sphere.bmp")
