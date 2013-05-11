require "lib"

// Cut the wood texture a small distance from the center of the "log".
var translatedWood = NoiseTranslatePoint {
// 	Slightly perturb the wood texture for more realism.
	sourceModule = NoiseTurbulence {
		// Add the wood grain texture to the base wood texture.
		sourceModule = NoiseAdd {
			// Base wood texture.  The base texture uses concentric cylinders aligned
			// on the z axis, like a log.
			sourceModule0 = NoiseCylinders {
				frequency = 16.0,
			},
			// Scale the wood-grain values so that they may be added to the base wood
			// texture.
			sourceModule1 = NoiseScaleBias {
				// Stretch the Perlin noise in the same direction as the center of the
				// log.  This produces a nice wood-grain texture.
				sourceModule = NoiseScalePoint {
					sourceModule = NoisePerlin { // Perlin noise to use for the wood grain.
						seed = 0,
						frequency = 48,
						persistence = 0.5,
						lacunarity = 2.20703125,
						octaveCount = 3,
						noiseQuality = NOISE_QUALITY_BEST,
					},
					yScale = 0.25,
				},
				scale = 0.25,
				bias = 0.125,
			},
		},
		seed = 1,
		frequency = 4.0,
		power = 1.0 / 256.0,
		roughness = 4,
	},
	zTranslation = 1.48,
}

// Cut the wood texture on an angle to produce a more interesting wood
// texture.
var rotatedWood = NoiseRotatePoint()
rotatedWood.sourceModule = translatedWood
rotatedWood.setAngles(84.0, 0.0, 0.0)
  
// Finally, perturb the wood texture to produce the final texture.
var finalWood = NoiseTurbulence {
	sourceModule = rotatedWood,
	seed = 2,
	frequency = 2.0,
	power = 1.0 / 64.0,
	roughness = 4,
}

function createTextureColor(renderer){
	renderer.gradient {
		-1.00: [189, 94, 4, 255],
		 0.50: [144, 48, 6, 255],
		 1.00: [60, 10, 8, 255],
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

createPlanar(finalWood, false, TEXTURE_HEIGHT, "wood-plane.bmp")
createPlanar(finalWood, true, TEXTURE_HEIGHT, "wood-seamless.bmp")

renderTexture(createSphericalMap(finalWood, TEXTURE_HEIGHT), "wood-sphere.bmp")
