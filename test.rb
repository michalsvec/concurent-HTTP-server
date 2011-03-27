#! /bin/ruby

require 'net/http'
require 'uri'

require 'rubygems'
require 'ap'


# calling killal to running webservers
kill = %x(killall)

# http://ruby-doc.org/stdlib/libdoc/net/http/rdoc/classes/Net/HTTP.html
url = URI.parse('http://localhost:5000/index.html')

res = Net::HTTP.start(url.host, url.port) {|http|
	http.get('/index.html')
}


if(Net::HTTPSuccess)
	puts res.header
end
