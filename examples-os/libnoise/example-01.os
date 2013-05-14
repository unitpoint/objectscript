
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

// var myModule = NoisePerlin()
var myModule = NoiseRidgedMulti()
// var myModule = NoiseBillow()

var TEXTURE_HEIGHT = 256
var SEAMLESS = true

var heightMap = NoiseMap()
var heightMapBuilder = NoiseMapBuilderPlane {
	sourceModule = myModule,
	destNoiseMap = heightMap,
	seamless = SEAMLESS,
}
heightMapBuilder.setDestSize(TEXTURE_HEIGHT, TEXTURE_HEIGHT)
heightMapBuilder.setBounds(0.0, 5.0, 0.0, 5.0)
heightMapBuilder.build()

var image = NoiseImage()
var renderer = NoiseRenderer {
	sourceNoiseMap = heightMap,
	destImage = image,
	wrap = SEAMLESS,
	light = true,
	lightContrast = 3.0, // Triple the contrast
	lightBrightness = 2.0, // Double the brightness
}
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
renderer.render()

image.writeBMP("example-01.bmp")
