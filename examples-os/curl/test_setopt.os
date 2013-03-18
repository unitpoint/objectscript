var curl = Curl()

var header_file = File("header.txt", "wb")
var content_file = File("content.txt", "wb")

curl.options {
    url         = "http://objectscript.org",
    file        = content_file,
    header_file = header_file,
    // httpheader  = ["Content-Type: text/html; charset=utf-8", "Transfer-Encoding: chunked"],
}

curl.perform(true) // keep open

header_file.close()
content_file.close()

var header = ""
var saveHeader = function(data) {
    header = header..data
}

curl.options {
    file = null, //set default value
    headerfunction = saveHeader,
}

content = curl.perform()

function File.readContent(filename, mode){
	var f = File(filename, mode)
	var content = f.read()
	f.close()
	return content
}

function File.writeContent(content, filename, mode){
	var f = File(filename, mode)
	f.write(content)
	f.close()
}

File.writeContent(header, "header-dub.txt")
File.writeContent(content, "content-dub.txt")

var check_header = File.readContent("header.txt")
var check_content = File.readContent("content.txt")

// check_header != header because of date header

if (check_content == content) {
    print "Test successful finished"
} else {
    print "Test failed"
	printf("content %d <=> %d\n", #check_content, #content)
}
