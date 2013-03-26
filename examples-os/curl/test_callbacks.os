// see http://pycurl.sourceforge.net/doc/callbacks.html

// Example: Callbacks for document header and body

// This example prints the header data to stderr and the body data to stdout.
// Also note that neither callback returns the number of bytes written.
// For WRITEFUNCTION and HEADERFUNCTION callbacks, returning None implies that all bytes where written.

var body = function(buf) {
    print buf
}

var header = function(buf) {
    print buf
}

var curl = Curl()
curl.options {
    url            = "http://www.python.org/",
    writefunction  = body,
    headerfunction = header,
}
curl.perform()


// Example: Download/upload progress callback

// This example shows how to use the progress callback.
// When downloading a document, the arguments related to uploads are zero, and vice versa.

Curl {
    url              = "http://slashdot.org/",
    noprogress       = false,
    progressfunction = function(dltotal, dlnow, ultotal, ulnow) {
		print ("Total to download", dltotal)
		print ("Total downloaded", dlnow)
		print ("Total to upload", ultotal)
		print ("Total uploaded", ulnow)
	},
}.perform()


// Example: Debug callbacks

// This example shows how to use the debug callback.
// The debug message type is an integer indicating the type of debug message.
// The VERBOSE option must be enabled for this callback to be invoked.

var function test(debug_type, debug_msg) {
    printf ("debug(%d): %s", debug_type, debug_msg)
}

Curl {
    url           = "http://ya.ru/",
    verbose       = true,
    debugfunction = test,
}.perform()

