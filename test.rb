#! /usr/bin/ruby

require 'net/http'
require 'uri'

# pocet testovacich pokusu
TEST_CNT = 5


if(ARGV.length > 0)
	CLIENTS_CNT = ARGV[0].to_i
else 
	CLIENTS_CNT = 10
end



class Worker
	def run(index)
		#puts "Starting worker: "+index.to_s
		
		begin
			res = Net::HTTP.start("localhost", 35123) {|http|
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



#methods = ['PTHREADS', 'GCD', 'FORK']
methods = ['GCD']

methods.each { |method| 

=begin
	# spawning thread for webserver
	thread_main = Thread.new {
		#start server
		cmd = "./build/Release/httpserver -m "+method
		puts cmd
		%x(#{cmd})

		puts "konec serveru"
	}
=end
  
	# jednotlive vysledky mereni
	results = Array.new
	puts "Klientu: "+CLIENTS_CNT.to_s
	0.upto(TEST_CNT) do |iteration|
		threads = Array.new

		# puts "spawning clients"
		time_start = Time.now

		# spawning many clients and measuring time of their join
		1.upto(CLIENTS_CNT) do |i|
			threads[i] = Thread.new(i) do
			  Worker.new.run i
			end
		end

		# wait for all threads
		1.upto(CLIENTS_CNT) do |i|
			threads[i].join
		end
	
		# save time of execution
		time_end = Time.now
	
		delta = time_end-time_start
		puts "iterace: "+iteration.to_s+" cas: "+delta.to_s
		results << delta
		
		sleep(0.2)
  end

	# kill server
	#%x(killall httpserver)
  
	# jako vysledek se pouzije nejmensi hodnota
	min = results.inject(0) {|index, num| num < results[index] ? results.find_index(num) : index }
	total = results.inject() {|result, elem| result + elem }

	puts "cas vykonavani pro "+(CLIENTS_CNT.to_s)+": \n\tprumer: "+(total/TEST_CNT).to_f.to_s+"\n\tminimum: "+results[min].to_s+"\n\n"
}

