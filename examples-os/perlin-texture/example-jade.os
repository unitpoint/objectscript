require "lib"

// Primary jade texture.  The ridges from the ridged-multifractal module
// produces the veins.
var primaryJade = NoiseRidgedMulti {
	seed = 0,
	frequency = 2.0,
	lacunarity = 2.20703125,
	octaveCount = 6,
	noiseQuality = NOISE_QUALITY_STD,
}

// Base of the secondary jade texture.  The base texture uses concentric
// cylinders aligned on the z axis, which will eventually be perturbed.
var baseSecondaryJade = NoiseCylinders {
	frequency = 2.0,
}

// Rotate the base secondary jade texture so that the cylinders are not
// aligned with any axis.  This produces more variation in the secondary
// jade texture since the texture is parallel to the y-axis.
var rotatedBaseSecondaryJade = NoiseRotatePoint {
	sourceModule = baseSecondaryJade,
	xAngle = 90,
	yAngle = 25,
	zAngle = 5,
}

// Slightly perturb the secondary jade texture for more realism.
var perturbedBaseSecondaryJade = NoiseTurbulence {
	sourceModule = rotatedBaseSecondaryJade,
	seed = 1,
	frequency = 4.0,
	power = 1.0 / 4.0,
	roughness = 4,
}

// Scale the secondary jade texture so it contributes a small part to the
// final jade texture.
var secondaryJade = NoiseScaleBias {
	sourceModule = perturbedBaseSecondaryJade,
	scale = 0.25,
	bias = 0.0,
}

// Add the two jade textures together.  These two textures were produced
// using different combinations of coherent noise, so the final texture will
// have a lot of variation.
var combinedJade = NoiseAdd {
	sourceModule0 = primaryJade,
	sourceModule1 = secondaryJade,
}

// Finally, perturb the combined jade textures to produce the final jade
// texture.  A low roughness produces nice veins.
var finalJade = NoiseTurbulence {
	sourceModule = combinedJade,
	seed = 2,
	frequency = 4.0,
	power = 1.0 / 16.0,
	roughness = 2,
}

function createTextureColor(renderer){
	renderer.gradient {
		-1.000: [ 24, 146, 102, 255],
		 0.000: [ 78, 154, 115, 255],
		 0.250: [128, 204, 165, 255],
		 0.375: [ 78, 154, 115, 255],
		 1.000: [ 29, 135, 102, 255],	
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
		light = false,
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

createPlanar(finalJade, false, TEXTURE_HEIGHT, "jade-plane.bmp")
createPlanar(finalJade, true, TEXTURE_HEIGHT, "jade-seamless.bmp")

renderTexture(createSphericalMap(finalJade, TEXTURE_HEIGHT), "jade-sphere.bmp")
