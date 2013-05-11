require "../std"

function __get(name){
	print "global property \"${name}\" is not declared"
	print "back trace"
	printBackTrace(1)
	print ""
}

function printBackTrace(skipNumFuncs){
	for(var i, t in debugBackTrace(skipNumFuncs + 1)){ // skip printBackTrace
		printf("#${i} ${t.file}%s: %s, args: ${t.arguments}\n",
			t.line > 0 ? "(${t.line},${t.pos})" : "",
			t.object && t.object !== _G ? "<${typeOf(t.object)}#${t.object.id}>.${t.name}" : t.name)
	}
}

function createPlanarMap(noiseModule, seamless, height){
	print "createPlanarTexture"
	// Map the output values from the noise module onto a plane.  This will
	// create a two-dimensional noise map which can be rendered as a flat
	// texture map.
	var plane = NoiseMapBuilderPlane()
	var noiseMap = NoiseMap()
	plane.setBounds(-1.0, 1.0, -1.0, 1.0)
	plane.setDestSize(height, height)
	plane.sourceModule = noiseModule
	plane.destNoiseMap = noiseMap
	plane.seamless = seamless
	
	plane.callback = function(row){
		++row;
		printf("plane build: %d, %.1f%%\n", row, row * 100 / height)
	}

	print "NoiseMapBuilderPlane.build"
	plane.build()

	return noiseMap
}

function createSphericalMap(noiseModule, height){
	// Map the output values from the noise module onto a sphere.  This will
	// create a two-dimensional noise map which can be rendered as a spherical
	// texture map.
	var sphere = NoiseMapBuilderSphere()
	var noiseMap = NoiseMap()
	sphere.setBounds(-90.0, 90.0, -180.0, 180.0) // degrees
	sphere.setDestSize(height * 2, height)
	sphere.sourceModule = noiseModule
	sphere.destNoiseMap = noiseMap

	sphere.callback = function(row){
		++row;
		printf("sphere build: %d, %.1f%%\n", row, row * 100 / height)
	}

	print "NoiseMapBuilderSphere.build"
	sphere.build ();

	return noiseMap
}

function renderNormalMap(noiseMap, filename, bumpHeight){
	print "renderNormalMap"
	
	// Set up us the texture renderer and pass the noise map to it.
	var destTexture = NoiseImage()
	var renderer = NoiseRendererNormalMap {
		sourceNoiseMap = noiseMap,
		destImage = destTexture,
		bumpHeight = bumpHeight || 1,
	}
	
	// Render the texture.
	renderer.render()

	// Write the texture as a Windows bitmap file (*.bmp).
	print "writeBMP normal map"
	destTexture.writeBMP(filename)
}
