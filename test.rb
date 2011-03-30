#! /bin/ruby

require 'net/http'
require 'uri'

require 'rubygems'
require 'ap'


# calling killall to running webservers to kill server running from xcode
kill = %x(killall httpserver)


methods = ['PTHREADS', 'GCD', 'OPENMPI', 'FORK']

methods.each { |method| 

	#start server
	cmd = "build/Debug/httpserver -m "+method
	puts cmd
	%x(#{cmd})
	
	# kill it
	%x(killall httpserver)
}





# http://ruby-doc.org/stdlib/libdoc/net/http/rdoc/classes/Net/HTTP.html
url = URI.parse('http://localhost:5000/index.html')

res = Net::HTTP.start(url.host, url.port) {|http|
	http.get('/index.html')
}


if(Net::HTTPSuccess)
	puts res.header
end
