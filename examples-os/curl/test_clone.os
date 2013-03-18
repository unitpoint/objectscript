var c = Curl()

var func = function(buf) {
    print buf
}

c.options {
    url:"www.ya.ru",
	timeout: 5,
    // httpheader: ["Content-Type: text/html; charset=utf-8", "Transfer-Encoding: chunked"],
    writefunction: func,
}

c1 = c.clone()

c1.options {
    url:"www.objectscript.org",
}

try{ 
	c.perform()
}catch(e){
	printf("%d: %s\n", e.code, e.message);
}

c1.perform()
