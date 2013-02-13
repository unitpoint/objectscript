var c = Curl()

c.options { url = "www.objectscript.org" }
c.perform(true) // keep open

print "-=== TEST STRING INFO ===-"
print c.getInfo("content_type")            
print c.getInfo("effective_url")           
print c.getInfo("ftp_entry_path")          
print c.getInfo("redirect_url")            
print c.getInfo("primary_ip")              
print c.getInfo("local_ip")                
print c.getInfo("rtsp_session_id")         
print ""

print "-=== TEST DOUBLE INFO ===-"
print c.getInfo("total_time")              
print c.getInfo("namelookup_time")         
print c.getInfo("connect_time")            
print c.getInfo("appconnect_time")         
print c.getInfo("pretransfer_time")        
print c.getInfo("starttransfer_time")      
print c.getInfo("size_upload")             
print c.getInfo("size_download")           
print c.getInfo("speed_download")          
print c.getInfo("speed_upload")            
print c.getInfo("content_length_download") 
print c.getInfo("content_length_upload")   
print c.getInfo("redirect_time")           
print ""

print "-=== TEST LONG INFO ===-"
print c.getInfo("response_code")           
print c.getInfo("http_connectcode")        
print c.getInfo("filetime")                
print c.getInfo("header_size")             
print c.getInfo("request_size")            
print c.getInfo("ssl_verifyresult")        
print c.getInfo("redirect_count")          
print c.getInfo("httpauth_avail")          
print c.getInfo("proxyauth_avail")         
print c.getInfo("os_errno")                
print c.getInfo("num_connects")            
print c.getInfo("lastsocket")              
print c.getInfo("primary_port")            
print c.getInfo("local_port")              
print c.getInfo("condition_unmet")         
print c.getInfo("rtsp_client_cseq")        
print c.getInfo("rtsp_server_cseq")        
print c.getInfo("rtsp_cseq_recv")          
print ""

print "-=== TEST STRING LIST INFO ===-"
print c.getInfo("ssl_engines")  
print c.getInfo("cookielist")   
print ""

print "-=== TEST CERTINFO INFO ===-"
print c.getInfo("certinfo")     
print ""

print "-=== GET ALL CURLINFO VALUE ===-"
print info = c.getInfo() 
print ""

print "-=== PRINT INFO OBJECT  ===-"
for(var i, v in info) {
    print i.." => "..v
}

c.close()
