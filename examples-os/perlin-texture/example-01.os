require "lib"

var test = NoisePerlin { // Perlin noise to use for the wood grain.
	// seed = 0,
	frequency = 2,
	// persistence = 0.5,
	// lacunarity = 2.20703125,
	// octaveCount = 1,
	noiseQuality = NOISE_QUALITY_BEST,
}

function createTextureColor(renderer){
	// renderer.buildTerrainGradient()
	renderer.gradient {
		-1.0000: [  0,   0, 128, 255], // deeps
		-0.2500: [  0,   0, 255, 255], // shallow
		 0.0000: [  0, 128, 255, 255], // shore
		 0.0625: [240, 240,  64, 255], // sand
		 0.1250: [ 32, 160,   0, 255], // grass
		 0.3750: [224, 224,   0, 255], // dirt
		 0.7500: [128, 128, 128, 255], // rock
		 1.0000: [255, 255, 255, 255], // snow	
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
		// light = false,
	}
	
	// Create the color gradients for the texture.
	createTextureColor(textureRenderer)

	// Render the texture.
	textureRenderer.render()

	// Write the texture as a Windows bitmap file (*.bmp).
	print "writeBMP"
	destTexture.writeBMP(filename)
	
	renderNormalMap(noiseMap, filename.replace(Regexp("/(\..+?)$/"), "-normalmap$1"), 10)
}

var TEXTURE_HEIGHT = 256

function createPlanar(noiseModule, seamless, height, filename){
	var noiseMap = createPlanarMap(noiseModule, seamless, height)
	renderTexture(noiseMap, filename, seamless)
}

createPlanar(test, false, TEXTURE_HEIGHT, "example-01-plane.bmp")
createPlanar(test, true, TEXTURE_HEIGHT, "example-01-seamless.bmp")

renderTexture(createSphericalMap(test, TEXTURE_HEIGHT), "example-01-sphere.bmp")
