var function range(a, b, arr){
	for(; a <= b; a++){
		arr[] = a
	}
	return arr
}

var function usedBytes()
{
	return GC.allocatedBytes - GC.cachedBytes
}

GC.full()
printf("==== START MEMORY USAGE ====\n")
printf("     used %3.0f Kb\n", usedBytes() / 1024)
printf("allocated %.0f Kb\n\n", GC.allocatedBytes / 1024)

var start_memory_allocated = GC.allocatedBytes
var start_memory = usedBytes()

var values = 1024*1024
var test = range(1, values, [])
printf("==== ARRAY of %d values\n", values)
printf("     used %3.0f Kb, per item %d bytes (approximately)\n", usedBytes() / 1024, usedBytes() / values)
printf("allocated %.0f Kb\n\n", GC.allocatedBytes / 1024)

test = null // reset var so the one could be garbage collected
GC.full() // do not use GC.full in general, OS frees unused values automaticaly
printf("==== FREE UNUSED MEMORY ====\n")
printf("     used %3.0f Kb\n", usedBytes() / 1024)
printf("allocated %.0f Kb\n\n", GC.allocatedBytes / 1024)

var test = range(1, values, {})
printf("==== OBJECT of %d values\n", values)
printf("     used %3.0f Kb, per item %d bytes (approximately)\n", usedBytes() / 1024, usedBytes() / values)
printf("allocated %.0f Kb\n\n", GC.allocatedBytes / 1024)
