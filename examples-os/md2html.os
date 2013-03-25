// convert markdown format to html

if(#arg < 4){
	print "Usage: ${arg[-1]} ${__FILE__} filename.md filename.html"
	terminate()
}

var md_filename, html_filename = arg[1], arg[2]
print("${md_filename}\n => ${html_filename}")

function File.readContents(filename)
{
	var f = File(filename, "rb")
	var content = f.read()
	f.close()
	return content
}

function File.writeContents(filename, content)
{
	var f = File(filename, "wb")
	f.write(content)
	f.close()
}

/*
function Buffer.__lshift(b){	// declare << operator
	return @append(b)
}
*/
Buffer.__lshift = Buffer.append	// make alias << operator to appent method

function String.__get(i){		// declare [] read operator with one single argument
	return @sub(i, 1)
}

function String.__mul(i){		// declare * operator
	var buf = Buffer()
	for(; i >= 1; i--){
		buf << this
	}
	return toString(buf)
}

function String.indent(spaces){
	spaces = spaces || 4
	var buf, indent, len, c = Buffer(), spaces, #this
	for(var i = 0; i < len; i++){
		if((c = this[i]) == "\t"){
			buf << " " * indent
			indent = spaces
		}else{
			buf << c
			if(--indent <= 0) indent = spaces
		}
	}
	return toString(buf)
}

function Object.reverse(){
	var r = {}
	for(var k, v in @reverseIter()){
		r[k] = v
	}
	return r
}

var content = File.readContents(md_filename)

content = content.replace(Regexp("/\r\n/"), "\n")
// content = content.replace(Regexp("/\s*?\n/"), "\n")
content = content.replace(Regexp("/\n{3,}/"), "\n\n")

// File.writeContents(html_filename, content) terminate()

var function convertParagraph(p){
	p = p.replace {
		"<" = "&lt;",
		">" = "&gt;",
	} .. "\n"
	p = p.replace(Regexp("/^#+\s*(.+?)\s*$/m"), "<h2>$1</h2>")
	
	var code_list, code_mark = {}, "HeghjgtdhjbUYTudghjbYUtwdwjhzYHJUsidy7ykjhsfUtu"
	var function inject(m){
		var mark = code_mark..#code_list
		code_list[mark] = "<b>"..m[1].."</b>"
		return mark
	}
	
	var e = Regexp.escape("\\")
	p = p.replace(Regexp("/`(.*?)`/"), inject)
	p = p.replace(Regexp("/${e}{1}(__.+?__)/m"), {|m|
		var mark = code_mark..#code_list
		code_list[mark] = m[1]
		return mark
	})

	p = p.replace(Regexp("/${e}{2}/"), {|m|
		var mark = code_mark..#code_list
		code_list[mark] = "\\"
		return mark
	})

	p = p.replace(Regexp("/__(\*\*)__/m"), inject)
	p = p.replace(Regexp("/\*\*(.+?)\*\*/m"), inject)
	p = p.replace(Regexp("/__(.+?)__/m"), inject)
	
	p = p.replace(Regexp("/^\s*\*\s*/m"), "</li>\n<li>")
	p = p.replace(Regexp("/^\s*\d\.\s*/m"), "</lio>\n<lio>")
	p = p.replace(Regexp("/^<\/li>/"), "<ul>")
	p = p.replace(Regexp("/^<\/lio>/"), "<ol>")
	p = p.replace(Regexp("/\n<\/li>/"), "</li>")
	p = p.replace(Regexp("/\n<\/lio>/"), "</lio>")
	p = p.replace(Regexp("/^<li>.+[^(<\/li>)]$/m"), "$0</li></ul>")
	p = p.replace(Regexp("/^<lio>.+[^(<\/lio>)]$/m"), "$0</lio></ol>")
	p = p.replace(Regexp("/<lio>/"), "<li>")
	p = p.replace(Regexp("/<\/lio>/"), "</li>")
	
	p = p.replace(Regexp("/\[(.+?)\]\((https?:\/\/.+?)\)/"), "<a href='$2' target='_blank'>$1</a>")
	
	p = p.replace(code_list.reverse())
	// print code_list.."\n\n"
	return p.trim()
}

var pp = content.split(Regexp("/\n\s*?\n/"))
content = Buffer()
for(var _, p in pp){
	// print "${_}\n${p}"
	if(!Regexp("/^(\t|\s\s)/").test(p)){
		content << "<p>" << convertParagraph(p) << "</p>\n"
		continue
	}
	content 
		<< "<pre lang=\"C++\">"
		<< p.replace(Regexp("/^.*$/m"), {|m| m[0].indent() })
		<< "</pre>\n"
}
content = content.replace(Regexp("/<\/pre>\n<pre lang=\"C\+\+\">/"), "\n\n")

File.writeContents(html_filename, content)
