Diversification software suite allows estimating diversification and fossilization rates from the fossil record

Console mode softwares are estimate, sample, assess, complexity (type 'make all' to compile)

---------------------------------------------------------------------------------

usage: estimate [options] <input file> [<output file>]

Estimate the diversification rates of the tree contained in the input file.
The input file has to be in Newick format with special tags for fossils ages and origin and end of the diversification, 
it returns a text report with the estimates.

Options are:
	-o <options file name>	: load the settings of the optimizer. <options file name> has to be in the format:
		:SPE [0;1] :EXT [0;1] :FOS [0:1] :TRI 10 :TOL 1.E-7 :ITE 500
	-h	: display help

---------------------------------------------------------------------------------

usage: sample [options] [<output file>]

sample simulates random trees and fossils finds and saves them in Newick format

Options:
	-h : display help
	-b <birth>	: set birth rate
	-d <death>	: set death rate
	-f <fossil>	: set fossil find rate
	-m <min>	: set minimum number of contemporary species of a simulation to be considered
	-M <size>	: set maximum size of a simulation to be considered
	-i <niter>	: set the number of simulations
	-t <time> : the end time of the diversification (start is always 0)

---------------------------------------------------------------------------------

usage: assess [options] [<output file>]

assess simulates random trees and fossils finds, estimates speciation and extinction rates and returns the mean absolute error


Options:
	-h : display help
	-b <birth>	: set birth rate
	-d <death>	: set death rate
	-f <fossil>	: set fossil find rate
	-m <min>	: set minimum number of contemporary species of a simulation to be considered
	-M <size>	: set maximum size of a simulation to be considered
	-i <niter>	: set the number of simulations
	-l <start> <end> <step> : the range of the end time of the diversification (start is always 0)
	-o <options file name>	: load the settings of the optimizer. <options file name> has to be in the format:
		:SPE [0;1] :EXT [0;1] :FOS [0:1] :TRI 10 :TOL 1.E-7 :ITE 500
	-x <number>	: set the number of threads
	-c <complexity>	: set the max complexity index of a simulation to be considered

---------------------------------------------------------------------------------

usage: complexity [options] [<output file>]

complexity simulates random trees and fossils finds an return a CSV file with lines
	<complexity index>	<likelihood computation time>	<tree size>	<fossil number>

Options:
	-h : display help
	-b <birth>	: set birth rate
	-d <death>	: set death rate
	-f <fossil>	: set fossil find rate
	-m <min>	: set minimum number of contemporary species of a simulation to be considered
	-M <size>	: set maximum size of a simulation to be considered
	-c <complexity>	: set the max complexity index of a simulation to be considered
	-i <niter>	: set the number of simulations
	-l <start> <end> <step> : the range of the end time of the diversification (start is always 0)



