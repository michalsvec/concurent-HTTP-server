#! /bin/ruby

require 'net/http'
require 'uri'

require 'rubygems'
require 'ap'




if(ARGV.length > 0)
	CLIENTS_CNT = ARGV[0].to_i
else 
	CLIENTS_CNT = 10
end


class Worker
	def run(index)
		#puts "Starting worker: "+index.to_s
		
		# http://ruby-doc.org/stdlib/libdoc/net/http/rdoc/classes/Net/HTTP.html
		#url = URI.parse('http://localhost:5000/index.html')
		
		
		begin
			res = Net::HTTP.start("localhost", 5000) {|http|
				http.get('/index.html')
			}
			
		rescue TimeoutError
			puts index.to_s+" TimeoutExcetion"
			return
		end
		
		# No exception
		if(!Net::HTTPSuccess && !Net::HTTPRedirection)
			puts index.to_s+" UNsuccessfull"
		end
	end
end




# calling killall to running webservers to kill server running from xcode
#kill = %x(killall httpserver)

#methods = ['PTHREADS', 'GCD', 'FORK']
methods = ['GCD']

methods.each { |method| 

	# spawning thread for webserver
	#	thread_main = Thread.new {
	#	#start server
	#	cmd = "build/Debug/httpserver -m "+method
	#	puts cmd
	#	%x(#{cmd})
	#}

	threads = Array.new
	
	#	puts "spawning clients"
	time_start = Time.now

	# spawning many clients and measuring time of their join
	1.upto(CLIENTS_CNT) do |i|
		threads[i] = Thread.new(i) do
			Worker.new.run i
		end
	end

	#wait for all threads
	1.upto(CLIENTS_CNT) do |i|
		threads[i].join
		#puts "--- "+i.to_s+" joined"
	end
	
	# save time of execution
	time_end = Time.now
	
	# kill it
	#%x(killall httpserver)
	
	time_total = time_end - time_start
	puts "cas vykonavani: "+time_total.to_f.to_s
}

