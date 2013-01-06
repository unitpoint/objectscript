var function range(a, b){
	var arr = {}
	for(; a <= b; a++){
		arr[] = a
	}
	return arr
}

var function memory_get_usage()
{
	return GC.allocatedBytes - GC.cachedBytes
}

var start_memory = memory_get_usage()
var array = range(1, 100000)
printf("%d bytes\n", memory_get_usage() - start_memory)