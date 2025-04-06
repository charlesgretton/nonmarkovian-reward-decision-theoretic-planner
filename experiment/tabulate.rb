#!/usr/bin/ruby -w
$display = true
$use_cache = true
$collate = false
$verbose = false
$combine = true
$quiet = false
$cached_only = false
$stop_on_error = false
$pre_cache = true
$list_torun = false
$bundle = false
ARGV.each do | arg |
    case arg
    when '--no-display'
	$display = false
    when '--no-cache'
	$use_cache = false
    when '--collate'
	$collate = true
    when '--verbose'
	$verbose = true
    when '--no-combine'
	$combine = false
    when '--quiet'
	$quiet = true
    when '--cached-only'
	$cached_only = true
    when '--stop-on-error'
	$stop_on_error = true
    when '--no-pre-cache'
	$pre_cache = false
    when '--list-torun'
	$list_torun = true
    when '--bundle'
	$bundle = true
    else
	$stderr.puts "unrecognised option #{arg}"
	exit 1
    end
end

if $bundle
    x = 1
    while File.directory?("b#{x}")
	x += 1
    end
    $bundle_dir = "b#{x}"
    Dir.mkdir($bundle_dir)
end

['data', 'results', 'points', 'runs']. each do | dirname |
    unless File.directory?(dirname)
	Dir.mkdir(dirname)
    end
    if $bundle
	Dir.mkdir("#{$bundle_dir}/#{dirname}")
    end
end

if $collate
    $prt = open('print.tex', 'w')
    $prt.print <<'EOF'
    \documentclass[12pt]{article}
    \usepackage{verbatim}
    \usepackage{graphicx}
    \usepackage{a4}
    
    \addtolength{\textheight}{6.5cm}
    \addtolength{\textwidth}{1.5cm}
    \addtolength{\evensidemargin}{-1.5cm}
    \addtolength{\oddsidemargin}{-1.5cm}
    \addtolength{\topmargin}{-3cm}
    
    
EOF
    $prt.puts('\begin{document}')
    $prt.puts('\tableofcontents')
end

def finish
    if $collate
	$prt.puts '\\end{document}'
	$prt.close
	system("latex print.tex && latex print.tex && dvips print.dvi -o print.ps")
    end
    if $bundle
	system("cp lines.plo #{$bundle_dir}")
	system("tar zcf #{$bundle_dir}.tar.gz #{$bundle_dir}")
	system("cp print.ps #{$bundle_dir}")

	puts "Bundle name: #{$bundle_dir}"
#	system("rm -rf #{bundle_dir}")
    end
    exit
end

class Octave
    def initialize
	@o = nil
    end

    def close
	unless @o.nil?
	    @o.puts "quit"
	    @o.close
	end
    end
    
    def method_missing(methname, *parameters)
	if @o.nil?
	    @o = open("|octave", "w+")
	end
	
	call = "#{methname}("
	call += parameters.collect do | param |
	    param.inspect
	end.join(', ')
	call += ")"
#	puts ">o: #{call}"
	@o.puts call
	@o.puts '"END OF COMMAND"'

	result = nil
	@o.each_line do | line |
#	    puts "<o: #{line}"
	    if line =~ /^ans = END OF COMMAND$/
		break
	    elsif line =~ /^ans =$/
		result = Array.new
	    elsif line =~ /^ans = ([-\d]+)$/
		result = $1.to_i
	    elsif line =~ /^ans = ([-\d\.e]+)$/
		result = $1.to_f
	    elsif line =~ /^ *([-\d]+)$/
		result.push($1.to_i)
	    elsif line =~ /^ *([-\d\.e]+)$/
		result.push($1.to_f)
	    end
	end
	return result
    end
end

$oct = Octave.new
at_exit do
    $oct.close
end

def next_poly_value(so_far)
    dat = [0.0] + so_far
    
    if dat.size == 1
	return 0.0
    end

    last_num = so_far.size + 1

    if dat.size < 4
	p = $oct.polyfit([0] + (2..last_num).to_a, dat, 2)
	return $oct.polyval(p, last_num + 1)
    end
    
    return $oct.spline([0] + (2..last_num).to_a, dat, last_num + 1)
end

def next_exp_value(so_far)
    Math.exp(next_poly_value(so_far.collect {|x| Math.log(x + 1)})) - 1
end

class Array
    def Array.new_float_range(lower, upper, step)
	result = Array.new
	i = lower
	while i <= upper
	    result.push(i)
	    i += step
	end

	return result
    end
end

class Problem <Hash
    def initialize(values)
	super
	update(values)
    end

    def to_s
	keys.collect do | key |
	    "#{key}=#{fetch(key)}"
	end.join(' ')
    end

    def to_id
	keys.sort.collect do | key |
	    "#{key}=#{fetch(key)}"
	end.join('-')
    end

    def ranged_to_discrete
	result = self.clone
	first = true

	keys.each do | memb |
	    if ['Range', 'Array'].include?(fetch(memb).type.to_s)
		if first
		    result = Hash.new
		    fetch(memb).each do | i |
			result[i] = self.clone
			result[i][memb] = i
		    end
		else
		    old_result = result
		    result = Hash.new
		    fetch(memb).each do | i |
			old_result.each do | key, val |
			    nk = "#{key}-#{i}"
			    result[nk] = val.clone
			    result[nk][memb] = i
			end
		    end
		end
		    
		first = false
	    end
	end
	result
    end

    def description
	result = ''

	first = true
	keys.each do | memb |
	    if !(['Range', 'Array'].include?(fetch(memb).type.to_s))
		if first
		    first = false
		else
		    result += " "
		end
		result += "#{memb}=#{fetch(memb)}"
	    end
	end
	result
    end

    def range_variable
	keys.each do | memb |
	    if ['Range', 'Array'].include?(fetch(memb).type.to_s)
		return memb
	    end
	end
	return nil
    end

    def report
	result = "\n" +
	    "\\subsection{#{description.gsub(/_/, '\_')}}\n" +
	    "\\begin{verbatim}\n"
	['FLTL', 'PLTL'].each do | lang |
	    result += `./create-prob.rb action=print-reward-spec reward_spec=#{to_s} language=#{lang} n=4 k=2`
	    result += "\n"
	end
	result += "\\end{verbatim}\n"
    end
end

$first_action_spec = true

def report_action_spec(action_spec)
    result = ""
    if !$first_action_spec
	result += "\\pagebreak\n"
    end
    $first_action_spec = false

    result = "\n" +
	"\\section{#{action_spec}}" +
	"\\begin{verbatim}\n" +
	`./create-prob.rb action=print-action-spec action_spec=#{action_spec} n=4` +
	"\\end{verbatim}\n" +
	"\n"
end

class Config
    attr_accessor :language, :preprocessing, :algorithm, :description
    def initialize(language, preprocessing, algorithm, description=nil)
	@language = language
	@preprocessing = preprocessing
	@algorithm = algorithm
	if description.nil?
	    @description = algorithm
	else
	    @description = description
	end
    end

    alias to_s description

    def cache_filename(prob)
	"data/#{language}-#{preprocessing}-#{algorithm}-#{prob.to_id}" 
    end

    def cached?(prob)
	File.exist?(cache_filename(prob))
    end

    def run_seed_average(measure, prob, seeds)
	total = 0.0
	num = 0
	seeds.each do | seed |
	    prob['seed'] = seed
	    value = run(measure, prob)
	    if !value.nil?
		total += value
		num += 1
	    end
	end

	if num > 0
	    return total/num
	else
	    return nil
	end
    end

    def run(measure, prob)
	filename = cache_filename(prob)
	if $verbose
#	    puts "Cache filename: #{filename}"
	end

	if !File.exist?(filename) or !$use_cache
	    if $cached_only
		return nil
	    end
	    
	    param = "language=#{@language} #{prob.to_s} preprocessing=#{@preprocessing} algorithm=#{@algorithm}"
	    
	    trap "INT" do
		File.delete(filename + '.tmp')
		$stderr.puts "file '#{filename}.tmp' deleted"
		exit(1)
	    end
	    trap "TERM" do
		File.delete(filename + '.tmp')
		$stderr.puts "file '#{filename}.tmp' deleted"
		exit(1)
	    end

	    to_run = "ruby create-prob.rb #{param} >'#{filename}.tmp'"
	    if $verbose
		puts "\nrunning: #{to_run}"
	    end
	    good = system(to_run)

	    good &= system("grep '^stats run successful' '#{filename}.tmp' >/dev/null")
	    good &= !system("grep '^Command execution failed:' '#{filename}.tmp' >/dev/null")
	    good &= !system("grep '(Parse Error) --  Invalid domain specification.' '#{filename}.tmp' >/dev/null")

	    if not good
		$stderr.puts
		$stderr.puts "test execution failed.  \"#{filename}.tmp\" may contain more details."
		if $stop_on_error
		    exit(1)
		else
		    return nil
		end
	    else
		File.rename(filename + '.tmp', filename)
	    end
	    
	    trap("INT", "DEFAULT")
	end

	results = Hash.new

	file = open(filename)
	file.each_line do | line |
	    if line =~ /^----------results----------$/
		file.each_line do | line |
		    if line =~ /^----------.*----------$/
			break
		    elsif line =~ /^The domain is currently comprised of (\d+) e-states./
			results['State count'] = $1.to_i

		    elsif line =~ /^Algorithm execution time: ([-0-9\.]+)/
			results['Algorithm time'] = $1.to_f
		    elsif line =~ /^Algorithm execution CPU time: ([-0-9\.]+)/
			results['Algorithm CPU time'] = $1.to_f

		    elsif line =~ /^preprocessing time: ([-0-9\.]+)/
			results['Preprocessing time'] = $1.to_f
		    elsif line =~ /^preprocessing CPU time: ([-0-9\.]+)/
			results['Preprocessing CPU time'] = $1.to_f

		    elsif line =~ /^expansion time: ([-0-9\.]+)/
			results['Expansion time'] = $1.to_f
		    elsif line =~ /^expansion CPU time: ([-0-9\.]+)/
			results['Expansion CPU time'] = $1.to_f

		    elsif line =~ /^NMRDP->MDP conversion time: ([-0-9\.]+)/
			results['NMR conversion time'] = $1.to_f
		    elsif line =~ /^NMRDP->MDP conversion CPU time: ([-0-9\.]+)/
			results['NMR conversion CPU time'] = $1.to_f

		    elsif line =~ /^delta-history: (.*)$/
			results['Delta history'] = $1.split(/,/)
		    elsif line =~ /^converged after (\d+) iterations with deltaMax = /
			results['Iterations'] = $1.to_i
		    elsif line =~ /^The longest label is comprised of (\d+) operators./
			results['Longest label length'] = $1.to_i
		    elsif line =~ /^Iteration number: *(\d+)/
			results['Iterations'] = $1.to_i
		    elsif line =~ /^The average label is comprised of (\d+) operators./
			results['Average label length'] = $1.to_i
		    elsif line =~ /^The domain is currently (\d+) Kb in size./
			results['Domain memory'] = $1.to_i
		    elsif line =~ /^policy-change-history: *([01]+)$/
			results['Policy change'] = $1
			results['Iterations to correct policy'] = $1.rindex('1') + 1
		    elsif line =~ /value-leaf-count: *(\d+)/
			results['Value leaf count'] = $1.to_i
		    elsif line =~ /value-node-count: *(\d+)/
			results['Value node count'] = $1.to_i
		    elsif line =~ /value-path-count: *(\d+)/
			results['Value path count'] = $1.to_i
		    elsif line =~ /value-density: *([-0-9\.]+)/
			results['Value density'] = $1.to_f
		    elsif line =~ /peak memory usage: *([-0-9\.]+)/
			results['Peak memory usage'] = $1.to_f
		    elsif line =~ /reachable states: *([-0-9\.]+)/
			results['Reachable states'] = $1.to_f
		    end
		end
	    end
	end
	
	case measure
	when 'Total time'
	    total = 0
	    results.keys.collect {|x| x if ((x =~ /time/i) and x !~ /CPU/)}.compact.each do | key |
		total += results[key]
	    end
	    return total
	when 'Total CPU time'
	    total = 0
	    results.keys.collect {|x| x if (x =~ /CPU time/i)}.compact.each do | key |
		total += results[key]
	    end
	    return total
	else
	    if results.has_key?(measure)
		return results[measure]
	    else
		if $verbose
		    $stderr.puts "\nWarning: Request for unknown measure #{measure} (#{prob.description})"
		end
		return nil
	    end
	end
    end
end

# action, language, when_reward, problem, n, preprocessing, algorithm
def run_tests(*args)
    tests = Array.new
    tests.push("")
    args.each do | param |
	case param
	when String, Fixnum
	    tests = tests.collect do | test |
		test + " '#{param.to_s}'"
	    end
	when Range, Array
	    ntests = Array.new
	    param.each do | i |
		ntests += tests.collect do | test |
		    test + " '#{i.to_s}'"
		end
	    end
	    tests = ntests
	end
    end

    puts "#{tests.size} tests to be run:"
    tests.each do | test |
	print "#{test}: "
	output = `ruby create-prob.rb #{test} 2>/dev/null`
	if output =~ /time taken: ([0-9\.]+)/
	    puts $1
	else
	    puts "-"
	end
    end
end

class RunInstance
    attr_reader :problem, :solution_method, :problem_instances, :runtimes, :next_est_time, :finished

    def initialize(problem, solution_method)
	@problem = problem
	@solution_method = solution_method
	@problem_instances = problem.ranged_to_discrete
	if @problem_instances.type != Hash
	    @problem_instances = [@problem_instances]
	else
	    @problem_instances = @problem_instances.keys.sort.collect do | key |
		@problem_instances[key]
	    end
	end
	@runtimes = Array.new
	@next_est_time = 0
	@finished = false
    end

    def next_cached?
	solution_method.cached?(problem_instances[runtimes.size])
    end

    def next_instance
	problem_instances[runtimes.size]
    end

    def run_next
	time = solution_method.run("Total time", problem_instances[runtimes.size])
	if time.nil?
	    @finished = true
	else
	    runtimes.push(time)
	    @next_est_time = next_exp_value(runtimes)
	    if runtimes.size == problem_instances.size
		@finished = true
	    end
	end
    end

    def size
	problem_instances.size
    end
end

def priority_run(problems, solution_methods)
    if $list_torun
	problems.each do | prob |
	    puts prob.description
	end
	exit
    end

    ris = Array.new

    num_runs = 0
    
    problems.each do | prob |
	solution_methods.each do | sol_method |
	    ri = RunInstance.new(prob, sol_method)
	    num_runs += ri.size
	    ris.push(ri)
	end
    end

    run_num = 0
    while !ris.empty?
	# we take the absolute time, since timing errors (with CPU
	# time) sometimes give negative results - we want these to be
	# penalised, the more they are below zero
	
	next_prob = ris.min {|a, b| a.next_est_time.abs <=> b.next_est_time.abs}

	run_num += 1
	estimate = next_prob.next_est_time

	puts
	puts next_prob.runtimes.join(', ')
	if next_prob.next_cached?
	    print "*cached*"
	end
	print "#{run_num}/#{num_runs}: #{next_prob.next_instance.description} " +
	    "method=#{next_prob.solution_method}: ETA=#{estimate} "
	$stdout.flush
	next_prob.run_next
	puts "#{next_prob.runtimes.last} seconds NETA=#{next_prob.next_est_time}"

	if next_prob.finished
	    ris.delete(next_prob)
	end
    end
end

class Graph
    def initialize(measure, title, outfile, problem, solution_methods, opts=[])
	@measure = measure
	@title = title
	@outfile = outfile
	@problem = problem
	@solution_methods = solution_methods
	@opts = opts

	@seeds = nil

	if @opts.include?('average-seed') and @problem.include?('seed') and @problem['seed'].type == Range
	    @problem = @problem.clone
	    @seeds = @problem['seed']
	    @problem.delete('seed')
	end

	setup_defaults
	produce_datafile
	run
    end

    def problem_results(measure, prob, sol_method)
	problems = prob.ranged_to_discrete

	if problems.type == Problem
	    $stderr.puts "We don't seem to have anything to graph on the X axis"
	    exit(1)
	end

	result = Hash.new

	problems.keys.sort.each do | key |
	    problem_i = problems[key]
	    
	    value = nil
	    if @seeds.nil?
		value = sol_method.run(@measure, problem_i)
	    else
		value = sol_method.run_seed_average(@measure, problem_i, @seeds)
	    end
	    if !value.nil? and @opts.include?('div-state-count') and 'State count' == @measure
		value = value.to_f / 2 ** problem_i['n']
	    end

	    result[key] = value
	end
	return result
    end

    def setup_defaults
	fix_param = nil
	fix_param_name = nil
	if @problem.type == Array
	    fix_param = @solution_methods
	    fix_param_name = "Solution method"
	else
	    fix_param = @problem
	    fix_param_name = "Problem"
	end
	if @title.empty?
	    @title = "#{fix_param_name}: #{fix_param.description}"
	end
	if @outfile.empty?
	    @outfile = "#{fix_param.description}-#{@measure}".gsub(/ /, '-').gsub(/\(/, '').gsub(/\)/, '')
	end

	@ylabel = case @measure
		 when /time/
		     "#{@measure} (sec)"
		 when /memory/i
		     "#{@measure} (MB)"
		 else
		     @measure
		 end

	if @opts.include?('div-state-count') and 'State count' == @measure
	    @ylabel += '/(2^n)'
	end
	
	if @opts.include?('nolog-state-count') and 'State count' == @measure
	    @opts.delete('log')
	end
    end
    
    def produce_datafile
	@point_filename = "points/#{@outfile}.data"
	
	pl = open(@point_filename, "w")

	@results = Hash.new

	if @solution_methods.type == Array
	    @solution_methods.each do | cat |
		@results[cat] = problem_results(@measure, @problem, cat)
	    end
	elsif @problem.type == Array
	    @problem.each do | cat |
		@results[cat] = problem_results(@measure, cat, @solution_methods)
	    end
	end

	xvalues = @results.keys.collect do | key |
	    @results[key].keys
	end.flatten.uniq.sort

	@categories = @results.keys.sort {|a,b| a.description <=> b.description }

	xvalues.each do | xvalue |
	    pl.print "#{xvalue},"
	    @categories.each do | category |
		pl.print "#{@results[category][xvalue]},"
	    end
	    pl.puts
	end
	pl.close
	if $bundle
	    system("cp '#{@point_filename}' #{$bundle_dir}/points")
	end
    end

    def run
	output_filename = "results/#{@outfile}.eps"

	if $verbose
	    puts "Output filename: #{output_filename}"
	end

	ploticus_opts = ""
	if @opts.include?('log')
	    ploticus_opts += 'ylog=log '
	end
	if @opts.include?('logx')
	    ploticus_opts += 'xlog=log '
	end
	if @opts.include?('log+1')
	    ploticus_opts += 'ylog=log+1 '
	end

	same_as = Hash.new
	@categories.each do | s1 |
	    same_as[s1] = Array.new
	    @categories.each do | s2 |
		if @results[s1] == @results[s2]
		    same_as[s1].push(s2)
		end
	    end
	end

	first_problem = nil
	if @problem.type == Array
	    first_problem = @problem.first
	else
	    first_problem = @problem
	end

	if false # nothing good?
	    $stderr.puts "Couldn't get measure \"#{@measure}\" from any of the test runs.\n#{@title}"
	    if $cached_only
		return
	    end
	else
	    # run ploticus
	    @torun = "ploticus lines.plo -eps -o '#{output_filename}' title='#{@title}' data='#{@point_filename}' delim=comma " +
		ploticus_opts + "xlbl='#{first_problem.range_variable}' ylbl='#{@ylabel}' x=1 "
	end
	
	index = 1
	@categories.each do | category |
	    descr = ''
	    if $combine
		same_as[category].each do | other |
		    descr += ', ' if !descr.empty?
		    descr += other.description
		    if other != category
			same_as[other].clear
		    end
		end
	    else
		descr = category.description
	    end
	    if !descr.empty?
		tmp = index == 1 ? '' : index
		@torun += " name#{tmp}='#{descr}' y#{tmp}=#{@categories.index(category) + 2}"
		index += 1
	    end
	end
	
	if $verbose
	    $stdout.puts "running: #{@torun}"
	end
	if system("#{@torun}")
	    r = open("runs/#{@outfile}.sh", "w")
	    r.puts "#!/bin/sh"
	    r.puts @torun
	    r.puts "gv '#{output_filename}'"
	    r.close
	    system("chmod +x 'runs/#{@outfile}.sh'")
	    
	    if $display
		system("gv '#{output_filename}' &")
	    end
	    if $bundle
		system("cp 'runs/#{@outfile}.sh' #{$bundle_dir}/runs")
		system("cp '#{output_filename}' #{$bundle_dir}/results")
	    end
	    if $collate
		$prt.puts("\\includegraphics[width=0.9\\textwidth]{#{output_filename}} \n\n")
		$prt.puts "\\tiny{\\verb+runs/#{@outfile}.sh+}\n\n"
	    end
	else
	    $stderr.puts "Graph creation failed: #{@title}"
	    $stderr.puts @torun
	end
    end
end

def run_test(problem, n, config, measure)
    config.run(measure, n, problem)
end

def tabulate(*args)
    v1 = v2 = nil
    for arg in args
	if arg.type == Array or arg.type == Range
	    if v1.nil?
		v1 = arg
	    elsif v2.nil?
		v2 = arg
	    else
		fail "too many variable arguments"
	    end
	end
    end

    if v1.nil? or v2.nil?
	fail "not enough variable arguments (need 2)"
    end

    first_cw = v1.collect{|s| s.to_s.size}.max

    print ' ' * first_cw
    for vv2 in v2
	print '|' + vv2.to_s
    end
    puts
    print '-' * first_cw
    for vv2 in v2
	print '+' + ('-' * vv2.size)
    end
    puts

    v1.each do | vv1 |
	print vv1.to_s.ljust(first_cw)
	v2.each do | vv2 |
	    test = Array.new
	    args.each do | arg |
		if arg == v1
		    test.push(vv1)
		elsif arg == v2
		    test.push(vv2)
		else
		    test.push(arg)
		end
	    end

	    print '|' + run_test(*test).to_s.ljust(vv2.to_s.size)
	end
	puts
    end
end

methods = [
    Config.new('PLTL', 'minimise', 'valIt', 'PLTL-valIt (minimised)'),
    Config.new('FLTL', 'none', 'valIt', 'FLTL-valIt'),
#    Config.new('PLTL', 'minimise', 'polIt', 'PLTL-polIt (minimised)'), #
#    Config.new('FLTL', 'none', 'polIt', 'FLTL-polIt'), #
#    Config.new('PLTL', 'none', 'valIt', 'PLTL-valIt'), #
#    Config.new('PLTL', 'none', 'polIt', 'PLTL-polIt'), #
    Config.new('FLTL', 'none', 'LAO-valIt', 'FLTL-LAO-valIt'),
#    Config.new('PLTL', 'none', 'LAO-valIt', 'PLTL-LAO-valIt'), #
#    Config.new('PLTL', 'none', 'LAO-polIt', 'PLTL-LAO-polIt'), #
    Config.new('PLTL', 'none', 'spudd'),
    Config.new('PLTL', 'auto-constrain', 'spudd', 'spudd (auto-constrained)'),
]

methods = [
#    Config.new('PLTL', 'minimise', 'valIt', 'PLTL-valIt (minimised)'),
    Config.new('FLTL', 'none', 'valIt', 'FLTL-valIt'),
#    Config.new('PLTL', 'minimise', 'polIt', 'PLTL-polIt (minimised)'), #
    Config.new('FLTL', 'none', 'polIt', 'FLTL-polIt'), #
#    Config.new('PLTL', 'none', 'valIt', 'PLTL-valIt'), #
#    Config.new('PLTL', 'none', 'polIt', 'PLTL-polIt'), #
#    Config.new('FLTL', 'none', 'LAO-valIt', 'FLTL-LAO-valIt'),
#    Config.new('PLTL', 'none', 'LAO-valIt', 'PLTL-LAO-valIt'), #
#    Config.new('PLTL', 'none', 'LAO-polIt', 'PLTL-LAO-polIt'), #
#    Config.new('PLTL', 'none', 'spudd'),
#    Config.new('PLTL', 'auto-constrain', 'spudd', 'spudd (auto-constrained)'),
]

if false
    Graph.new('Reachable states',
	  '',
	  '',
	  Problem.new('action_spec'=>'RandomActionSpec',
		      'reward_spec'=>'AllTrue',
		      'seed'=>3..3,
		      'influence'=>1,
		      'uncertainty'=>0,
		      'p_reach'=>Array.new_float_range(0.0, 1.0, 0.1),
		      'n'=>8),
	  [
	      Config.new('PLTL', 'auto-constrain', 'spudd', 'spudd (auto-constrained)'),
	  ],
	  ['log', 'div-state-count', 'average-seed']
	  )
    finish
end

if false
    Graph.new('Total CPU time',
	      '',
	      '',
	      [
		  Problem.new('action_spec'=>'SpuddLinear',
			      'reward_spec'=>'AllTrue',
			      'n'=>1..5),
		  Problem.new('action_spec'=>'SpuddLinear',
			      'reward_spec'=>'OneTrue',
			      'n'=>1..5),
	      ],
#	      [
#		  Config.new('PLTL', 'none', 'spudd', 'spudd'),
		  Config.new('PLTL', 'minimise', 'valIt', 'PLTL-valIt'),
#	      ],
	      ['log', 'div-state-count']
	      )
    finish
end

if false
    Graph.new('State count',
	  '',
	  '',
	  Problem.new('action_spec' => 'Complete', 'reward_spec' => 'PbxIn',
		      'n' => 2..4, 'conjdisj' => 'and'),
	  [
	      Config.new('FLTL', 'none', 'valIt', 'FLTL-valIt'),
#	      Config.new('PLTL', 'minimise', 'valIt', 'PLTL-valIt'),
	  ],
	  ['div-state-count']
	  )
    finish
end

measures = [
	    'Total CPU time',
#	    'Total time',
#	    'Expansion CPU time',
#	    'Expansion time',
#	    'Preprocessing CPU time',
#	    'Preprocessing time',
#	    'Peak memory usage',
#	    'Iterations',
#	    'State count',
#	    'Longest label length',
#	    'Average label length',
	]

def get_problems(sz, act)
    past_temporal_operators = ['pbx', 'pdi', 'prv', 'Asnc', 'sncA']
    two_level_problems = past_temporal_operators.collect do | l1 |
	past_temporal_operators.collect do | l2 |
	    ['and', 'or'].collect do | cd |
		Problem.new('action_spec'=>act,
			    'reward_spec'=>'TwoLevel',
			    'level1'=>l1,
			    'level2'=>l2,
			    'conjdisj'=>cd,
			    'n'=>2..sz)
	    end
	end
    end.flatten

    prv_in_out = [
	Problem.new('action_spec' => act, 'reward_spec' => 'PrvIn',
		    'n' => 2..sz, 'conjdisj' => 'and'),
	Problem.new('action_spec' => act, 'reward_spec' => 'PrvOut',
		    'n' => 2..sz, 'conjdisj' => 'and')
    ]

    pbx_in_out = [
	Problem.new('action_spec' => act, 'reward_spec' => 'PbxIn',
		    'n' => 1..sz, 'conjdisj' => 'and'),
	Problem.new('action_spec' => act, 'reward_spec' => 'PbxOut',
		    'n' => 1..sz, 'conjdisj' => 'and'),
    ]
    
    state_space_size = [
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'RewardLinear',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'AllTrue',
		    'first'=>'true',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'ConstantFactor',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Sequence',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Sequence2',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Poly2',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Exponential',
		    'power'=>3,
		    'n'=>2..sz),
    ]

    other = [
	Problem.new('action_spec' => 'SpuddExpon',
		    'reward_spec' => 'AllTrue',
		    'n' => 2..sz),
	Problem.new('action_spec' => 'SpuddLinear',
		    'reward_spec' => 'AllTrue',
		    'n' => 2..sz),
	Problem.new('action_spec' => 'SpuddLinear',
		    'reward_spec' => 'DCDelay',
		    'n' => 2..sz),
    ]

    selected = [
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'AllTrue',
		    'first'=>'true',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Sequence',
		    'first'=>'true',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'LinearSequence',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Sequence2',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Poly2',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'Exponential',
		    'power'=>3,
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'TempLin',
		    'n'=>2..sz),
    ]

    exploitPLTL = [
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'NonMinPLTL_Min',
		    'n'=>1..8),
    ]

    timing = [
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'AchievableExactTiming',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'UnachievableExactTiming',
		    'n'=>2..sz),
	Problem.new('action_spec'=>act,
		    'reward_spec'=>'TimePeriod',
		    'k'=>1..12,
		    'n'=>6),
    ]

    if 'SpuddLinear' != act
	timing = []
    end

    return selected + other


#    return selected + pbx_in_out + prv_in_out
#    return state_space_size + pbx_in_out
#    return pbx_in_out
#    return selected
#    return pbx_in_out + state_space_size # + two_level_problems + prv_in_out + other
end

def get_random_problems(num_points)
    r_sz = 6
    random_problems = [
	Problem.new('action_spec'=>'RandomActionSpec',
		    'seed'=>1..num_points,
		    'influence'=>Array.new_float_range(0.1, 1.0, 0.1),
		    'uncertainty'=>0.3,
		    'p_reach'=>0.04,
		    'n'=>r_sz),
	Problem.new('action_spec'=>'RandomActionSpec',
		    'first'=>'true',
		    'seed'=>1..num_points,
		    'influence'=>0.3,
		    'uncertainty'=>Array.new_float_range(0.0, 1.0, 0.1),
		    'p_reach'=>0.04,
		    'n'=>r_sz),
#	Problem.new('action_spec'=>'RandomActionSpec',
#		    'seed'=>1..num_points,
#		    'influence'=>0.3,
#		    'uncertainty'=>0.3,
#		    'p_reach'=>Array.new_float_range(0.0, 1.0, 0.1),
#		    'n'=>r_sz),
    ].collect do | prob |
	[
	    prob.clone.update('reward_spec'=>'TempLin'),
#	    prob.clone.update('reward_spec'=>'AllTrue', 'first'=>'true'),
	]
    end.flatten
end

def get_random_problems2
end

if false
    (2..12).each do | i |
	['SpuddLinear', 'Balls'].each do | act |
	    for experiment in get_problems(i, act)
		for measure in measures
		    Graph.new(measure, '', '', experiment, methods, ['log', 'div-state-count'])
		end
	    end
	end
    end
    finish
end

sz = 12
#action_specs = ['Balls', 'SpuddExpon', 'SpuddLinear', 'Complete']
#action_specs = ['Balls']
action_specs = ['SpuddLinear']
problems = action_specs.collect do | act |
    get_problems(sz, act)
end.flatten

methods = [
    Config.new('PLTL', 'minimise', 'valIt', 'PLTL-valIt (minimised)'),
#    Config.new('FLTL', 'none', 'valIt', 'FLTL-valIt'),
#    Config.new('PLTL', 'minimise', 'polIt', 'PLTL-polIt (minimised)'), #
#    Config.new('FLTL', 'none', 'polIt', 'FLTL-polIt'), #
    Config.new('PLTL', 'none', 'valIt', 'PLTL-valIt'), #
#    Config.new('PLTL', 'none', 'polIt', 'PLTL-polIt'), #
#    Config.new('FLTL', 'none', 'LAO-valIt', 'FLTL-LAO-valIt'),
#    Config.new('PLTL', 'none', 'LAO-valIt', 'PLTL-LAO-valIt'), #
#    Config.new('PLTL', 'none', 'LAO-polIt', 'PLTL-LAO-polIt'), #
#    Config.new('PLTL', 'none', 'spudd'),
#    Config.new('PLTL', 'auto-constrain', 'spudd', 'spudd (auto-constrained)'),
]

if false
    problems = get_random_problems(5)
    if !$cached_only and $pre_cache
	[5].each do | num_points | # , 10, 20, 40, 60, 80, 100
	    problems = get_random_problems(num_points)
	    priority_run(problems, methods)
	end
    end
end

if false
    sz = 10
    problems = [
	Problem.new('action_spec' => 'Balls',
		    'reward_spec' => 'DCDelay',
		    'n' => 2..sz),
	Problem.new('action_spec'=>'Balls',
		    'reward_spec'=>'NonMinPLTL_Min',
		    'n'=>1..sz),
    ]
    problems = [
	Problem.new('action_spec' => 'Balls',
		    'reward_spec' => 'DCDelayFixed',
		    'delay' => 8,
		    'n' => 1..sz),
    ]
end

methods = [
    Config.new('PLTL', 'minimise', 'valIt', 'PLTL-valIt (minimised)'),
    Config.new('PLTL', 'none', 'valIt', 'PLTL-valIt (non-minimised)'),
]

if !$cached_only and $pre_cache
    puts "Pre-caching run data:"
    priority_run(problems, methods)
end

if false
    for experiment in problems
	if $collate
	    $prt.puts experiment.report
	end
	for measure in measures
	    Graph.new(measure, '', '', experiment, methods, ['log', 'div-state-count', 'nolog-state-count'])
	end
    end
    finish
end

if false
    methods.each do | sol_method |
	Graph.new('Total CPU time', '', '', [
		      Problem.new('action_spec'=>'SpuddLinear',
				  'reward_spec'=>'AchievableExactTiming',
				  'n'=>2..sz),
		      Problem.new('action_spec'=>'SpuddLinear',
				  'reward_spec'=>'UnachievableExactTiming',
				  'n'=>2..sz),
		  ], sol_method, ['log'])
    end
    finish
end    

if false
    methods = [
	Config.new('PLTL', 'minimise', 'valIt', 'PLTL-valIt (minimised)'),
	Config.new('FLTL', 'none', 'valIt', 'FLTL-valIt'),
	#    Config.new('PLTL', 'minimise', 'polIt', 'PLTL-polIt (minimised)'), #
	#    Config.new('FLTL', 'none', 'polIt', 'FLTL-polIt'), #
	#    Config.new('PLTL', 'none', 'valIt', 'PLTL-valIt'), #
	#    Config.new('PLTL', 'none', 'polIt', 'PLTL-polIt'), #
	#Config.new('FLTL', 'none', 'LAO-valIt', 'FLTL-LAO-valIt'),
	#    Config.new('PLTL', 'none', 'LAO-valIt', 'PLTL-LAO-valIt'), #
	#    Config.new('PLTL', 'none', 'LAO-polIt', 'PLTL-LAO-polIt'), #
	Config.new('PLTL', 'none', 'spudd'),
	Config.new('PLTL', 'auto-constrain', 'spudd', 'spudd (auto-constrained)'),
    ]
    sz = 12
    act = 'Balls'
    methods.each do | sol_method |
	Graph.new('Total CPU time', '', '', [
		      Problem.new('action_spec' => act, 'reward_spec' => 'PrvIn',
				  'n' => 2..sz, 'conjdisj' => 'and'),
		      Problem.new('action_spec' => act, 'reward_spec' => 'PrvOut',
				  'n' => 2..sz, 'conjdisj' => 'and')
		  ], sol_method, ['log'])
    end
    finish
end    

puts "Generating graphs"
if false
    for experiment in problems
	if $collate
	    $prt.puts experiment.report
	end
	for measure in measures
	    Graph.new(measure, '', '', experiment, methods, ['log', 'div-state-count', 'nolog-state-count', 'average-seed'])
	end
    end
else
    action_specs.each do | act |
	if $collate
	    $prt.puts report_action_spec(act)
	end
	for experiment in get_problems(sz, act)
	    if $collate
		$prt.puts experiment.report
	    end
	    for measure in measures
		Graph.new(measure, '', '', experiment, methods, ['log', 'div-state-count', 'nolog-state-count'])
	    end
	end
    end
end

finish
