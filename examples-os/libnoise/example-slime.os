require "lib"

// Large slime bubble texture.
var largeSlime = NoiseBillow {
	seed = 0,
	frequency = 4.0,
	lacunarity = 2.12109375,
	octaveCount = 2,
	noiseQuality = NOISE_QUALITY_BEST,
}

// Base of the small slime bubble texture.  This texture will eventually
// appear inside cracks in the large slime bubble texture.
var smallSlimeBase = NoiseBillow {
	seed = 1,
	frequency = 24.0,
	lacunarity = 2.14453125,
	octaveCount = 2,
	noiseQuality = NOISE_QUALITY_BEST,
}

// Scale and lower the small slime bubble values.
var smallSlime = NoiseScaleBias {
	sourceModule = smallSlimeBase,
	scale = 0.5,
	bias = -0.5,
}

// Create a map that specifies where the large and small slime bubble
// textures will appear in the final texture map.
var slimeMap = NoiseRidgedMulti {
	seed = 0,
	frequency = 2.0,
	lacunarity = 2.20703125,
	octaveCount = 3,
	noiseQuality = NOISE_QUALITY_STD,
}

// Choose between the large or small slime bubble textures depending on the
// corresponding value from the slime map.  Choose the small slime bubble
// texture if the slime map value is within a narrow range of values,
// otherwise choose the large slime bubble texture.  The edge falloff is
// non-zero so that there is a smooth transition between the two textures.
var slimeChooser = NoiseSelect {
	sourceModule0 = largeSlime,
	sourceModule1 = smallSlime,
	controlModule = slimeMap,
	edgeFalloff = 0.5,
}
slimeChooser.setBounds(-0.375, 0.375)

// Finally, perturb the slime texture to add realism.
var finalSlime = NoiseTurbulence {
	sourceModule = slimeChooser,
	seed = 2,
	frequency = 8.0,
	power = 1.0 / 32.0,
	roughness = 2,
}

function createTextureColor(renderer){
	renderer.gradient {
		-1.0: [160,  64,  42, 255],
		 0.0: [ 64, 192,  64, 255],
		 1.0: [128, 255, 128, 255],
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

createPlanar(finalSlime, false, TEXTURE_HEIGHT, "slime-plane.bmp")
createPlanar(finalSlime, true, TEXTURE_HEIGHT, "slime-seamless.bmp")

renderTexture(createSphericalMap(finalSlime, TEXTURE_HEIGHT), "slime-sphere.bmp")
