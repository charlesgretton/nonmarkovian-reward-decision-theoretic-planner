#!/usr/bin/ruby -w

# ((a snc b) or (c snc b)) snc d
# fbx (d -> ($ and nxt (b -> ($ and nxt ($ fut (~a or ~b or ~c)))))) 

discount = 0.95
epsilon = 0.05
reward = 1

$show_policy_domain = false

class String
    def puts(a)
	if a.type == Array
	    a.each {|x| self.puts(a)}
	elsif a =~ /\n$/
	    self.sub!(/$/, a)
	else
	    self.sub!(/$/, a + "\n")
	end
    end
end

if ARGV.size != 6
    $stderr.puts(
		 "usage: ruby create-prob {action} {language} {problem} {n} {preprocessing} {algorithm}\n" +
		 "  action: stats|display-policy|display-xmdp|term\n" +
		 "  language: PLTL|FLTL\n" +
		 "  problem: ((first|every)(one-true|all-true|sequence|seq-seq))|\n" +
		 "     spudd-linear|spudd-expon|factory|delay|ago|vars|ago-vars\n" +
		 "     random-{seed}-{num actions}-{proportion reachable}-{influence}-{uncertainty}-{rwd-card}\n" +
		 "     linexp-{0..1}|polynomial|kill-pltl|unachievable|cddelay\n" +
         "     prv-in-and-{power}|prv-in-or-{power}|prv-out-and-{power}|prv-out-or-{power}" +
		 "  n: problem size\n" +
		 "  preprocessing: minimise|none|auto-constrain\n" +
		 "  algorithm: spudd|LAO-polIt|LAO-valIt|polIt|valIt\n" +
		 "eg: ruby create-prob.rb stats PLTL every-one-true 8 none spudd")
    exit(1)
end

def pltl_first_only(f)
    "(#{f}) and ~prv pdi (#{f})"
end
def fltl_first_only(f, nxt)
    "(~(#{f})) fut ((#{f}) and nxt ^ #{nxt} $)"
end

def create_random_problem(seed, n_var, n_act, p_reach, influence, uncertainty, n_rwd)
    result = ""

    t = open("create-rand.cmd", "w")
    t.puts "randomSeed(#{seed})"
    t.puts "randomActionSpec(#{n_var}, #{n_act}, #{p_reach}, #{influence}, #{uncertainty})"
    t.puts "randomReward(#{n_rwd})"
    t.puts "rewardSpecification"
    t.puts "actionSpec"
    t.puts "quit"
    t.close

    t = open("|../nmrdpp create-rand.cmd")
    t.each_line do | line |
	if line =~ /^(r\d+) :: (\d+) :: (.*)$/
	    # reward spec line
	    result.puts "[#{$1}, #{$2}]? #{$3}" #.gsub(/[\(\)]/, '')
	else
	    # action spec line
	    result.puts line
	end
    end
    t.close
    
    result
end

def nxt_exp(i, theta)
    if i == 0
	theta
    else
	"nxt^#{i} #{theta}"
    end
end

def nxt_and(i, theta)
    if i == 1
	'tt'
    else
	"nxt and < #{i} #{theta}"
    end
end

def create_problem_file(lang, when_reward, problem, n, reward)
    world = open("test.world", "w")
    propositions = (1..n).collect { | i | "b#{i}" }
    case problem
    when /^random-(\d+)-(\d+)-([0-9\.]+)-([0-9\.]+)-([0-9\.]+)-(\d+)/
	seed = $1
	n_act = $2
	p_reach = $3
	influence = $4
	uncertainty = $5
	n_rwd = $6
	create_random_problem(seed, n, n_act, p_reach, influence, uncertainty, n_rwd)
    when "factory"
	t = open("#{problem}.world")
	t.each_line do | line |
	    world.puts line
	end
	t.close
    else
	world.print "[r, #{reward}]? "
	condition = ""
	
	case lang
	when "FLTL"
      #Charles:: prv-in... is a problem of the form $prv (P_1 PCop P_2...)$
      case problem
      when /^prv-in-and-(\d+)/, /^prv-out-and-(\d+)/
        power = $1
        
        world.puts ("fbx ( (" + (1..n).collect do | i |
                      "p#{i}"
                    end.join(" and ") + ") -> nxt ^ #{power} $)")
      when /^prv-in-or-(\d+)/, /^prv-out-or-(\d+)/
        power = $1
        
        world.puts ("fbx ( (" + (1..n).collect do | i |
                      "p#{i}"
                    end.join(" or ") + ") -> nxt ^ #{power} $)")
      when "kill-pltl"
		# fbx (F -> ($ and nxt (B -> ($ and nxt ($ fut (~ A or ~B or ~ C or ~D or ~E))))))
        
		world.print "fbx (A -> ($ and nxt (B -> ($ and nxt ($ fut ("
		world.print((1..n).collect do | i |
		    "(~p#{i})"
		end.join(" or "))
		world.puts "))))))"
	    when "unachievable"
		world.puts "fbx a"
		(1..n).each do | i |
		    world.print "[r#{i}, 10000]? "
		    # a and nxt <
		    world.print "fbx((a and nxt^#{i} c and "
		    tmp = (0..i).collect do | j |
			"(#{nxt_exp(j, 'a')} and #{nxt_exp(j, nxt_and(i - j + 1, '(~a and ~b)'))})"
		    end
		    world.print tmp.join(" or ")
		    world.puts ")-> nxt^#{i} $)"
#		    world.puts "`fbx ((a and nxt^#{i} (c and a)) -> nxt^#{i} ($ and nxt ($ fut (~a and ~b))))"
#		    world.puts "fbx (((a and nxt^#{i} (c -> $)) fut a) and fbx (a and nxt ^#{i} (c and ((~a and ~b) -> ($ and nxt ($ fut (~a and ~b)))))))"
		end
	    else
		nxt = 0
		case problem
		when "test"
		    condition = "(b1 and ~b2) and nxt (b2 and ~b1)"
		    nxt = 1
		when "cddelay"
		    condition = "((A or B or C or D or E or F) and G)"
		    nxt = n
		when "one-true"
		    condition = "b1"
		when /^(all-true|spudd-expon|spudd-linear|linexp-.*)$/
		    condition = propositions.join(' and ')
		when "sequence"
		    condition = (1..n).collect do | i |
			"(nxt ^ #{i - 1} b#{i})"
		    end.join(' and ')
		    nxt = n - 1
		when "polynomial"
		    condition = "(nxt or < #{n} P) and (nxt ^ #{n} Q)"
		    nxt = n
		when "ago", "ago-vars"
		    condition = "a"
		    nxt = n
		when "vars"
		    condition = "a"
		    nxt = 0
		when "delay"
		    condition = "(nxt ^ #{n} a) and (nxt and < #{n} b)"
		    nxt = n
		when "seq-seq"
		    condition = (0..n**2-1).collect do | i |
			"nxt ^ #{i} b#{(i % n) + 1}"
		    end.join(' and ')
		    nxt = n**2 - 1
		else
		    $stderr.puts "problem #{problem} not implemented for #{lang}"
		    exit(1)
		end
		if when_reward == "first"
		    world.puts fltl_first_only(condition, nxt)
		elsif when_reward == "every"
		    world.puts "fbx ((#{condition}) -> nxt ^ #{nxt} $)"
		else
		    $stderr.puts "when \"#{when_reward}\" unknown"
		    exit(1)
		end
	    end
	when "PLTL"
      #Charles:: prv-in... is a problem of the form $prv (P_1 PCop P_2...)$
      case problem
      when /^prv-in-and-(\d+)/
        power = $1
        
        world.puts ("prv ^ #{power}(" + (1..n).collect do | i |
                      "p#{i}"
                    end.join(" and ") + ")")
      when /^prv-in-or-(\d+)/
        power = $1
        world.puts ("prv ^ #{power}(" + (1..n).collect { | i |
                      "p#{i}"
                    }.join(" or ") + ")")
      #Charles:: prv-in... is a problem of the form $prv (P_1) PCop prv (P_2)...$
      when /^prv-out-and-(\d+)/
        power = $1
        world.puts ((1..n).collect { | i |
                      "prv ^ #{power} (p#{i})"
                    }.join(" and ") + ")")
      when /^prv-out-or-(\d+)/
        power = $1
        world.puts ((1..n).collect { | i |
                      "prv ^ #{power} (p#{i})"
                    }.join(" or ") + ")")
      when "unachievable"
		world.puts "a"
		(1..n).each do | i |
          world.puts "[r#{i}, 10000.0]? (prv ^ #{i} a) and c and ((~b) snc a)"
          #world.puts "[r#{i}, 10000.0]? (prv ^ #{i} a) and c and ~(b snc a)"
		end
	    else
		case problem
		when "kill-pltl"
		    condition = "(" + (1..n).collect do | i |
			"(p#{i} snc A)"
		    end.join(" or ") + ") snc B"
		when "cddelay"
		    condition = "prv ^ #{n} ((A or B or C or D or E or F) and G)"
		when "one-true"
		    # reward b1
		    condition = "b1"
		when /^(all-true|spudd-expon|spudd-linear|linexp-.*)$/
		    # reward all propositions true
		    condition = propositions.join(' and ')
		when "sequence"
		    # reward b1 followed by b2 followed by b3 and so on
		    condition = (1..n).collect do | i |
			"(prv ^ #{n - i} b#{i})"
		    end.join(' and ')
		when "seq-seq"
		    # reward b1 followed by b2 followed by b3 and so on, with the
		    # whole sequence repeated `n' times
		    condition = (1..n**2).collect do | i |
			"(prv ^ #{n**2 - i} b#{((i-1) % n) + 1})"
		    end.join(' and ')
		when "polynomial"
		    condition = "Q and prv or < #{n} P"
		when "ago", "ago-vars"
		    condition = "prv ^ #{n} a"
		when "vars"
		    condition = "a"
		when "delay"
		    condition = "a and (prv and < #{n} b)"
		else
		    $stderr.puts "problem #{problem} not implemented for #{lang}"
		    exit(1)
		end
		if when_reward == "first"
		    world.puts pltl_first_only(condition)
		elsif when_reward == "every"
		    world.puts condition
		else
		    $stderr.puts "when \"#{when_reward}\" unknown"
		    exit(1)
		end
	    end
	else
	    $stderr.puts "unknown language: #{lang}"
	    exit(1)
	end
	world.puts ""

	# action definitions
	case problem
	when "unachievable"
	    world.puts "action a"
	    world.puts "   a (1)"
	    world.puts "   b (0)"
	    world.puts "   c (0)"
	    world.puts "endaction"
	    world.puts "action b"
	    world.puts "   a (0)"
	    world.puts "   b (a (1) (0))"
	    world.puts "   c (0)"
	    world.puts "endaction"
	    world.puts "action c"
	    world.puts "   a (0)"
	    world.puts "   b (0)"
	    world.puts "   c (b (1) (0))"
	    world.puts "endaction"
	when "spudd-expon", "spudd-linear", /^prv-in-or-(\d+)/,  /^prv-in-and-(\d+)/,  /^prv-our-and-(\d+)/,  /^prv-our-or-(\d+)/
	    (1..n).each do | i |
		world.puts "action a#{i}"
		(1..i-1).each do | j |
		    world.puts "  b#{j} (0.0)"
		end

		world.print "  b#{i} "
		if problem == "spudd-expon"
		    (1..i-1).each do | j |
			world.print "(b#{j} "
		    end
		    world.print " (1.0)"
		    world.puts ' (0.0))' * (i - 1)
		elsif problem =~ /^linexp-(.*)$/
		    linexp_prop = $1.to_f
		    if linexp_prop < 0 || linexp_prop > 1
			$stderr.puts "linexp proportion must be in the range [0..1]"
			exit 1
		    end
		    level = i - ((n - 1) * linexp_prop).to_i
		    if level < 1
			level = 1
		    end
		    (level..i-1).each do | j |
			world.print "(b#{j} "
		    end
		    world.print " (1.0)"
		    world.print ' (0.0))' * (i - level)
		    world.puts
		elsif problem == ("spudd-linear" or /^prv-in-or-(\d+)/ or
		/^prv-in-and-(\d+)/ or  /^prv-our-and-(\d+)/ or  /^prv-our-or-(\d+)/)
		    world.puts "(1.0)"
		end
		
		world.puts "endaction"
	    end
	when /linexp-(.*)$/
	    linexp_level = ((1 - $1.to_f) * (n - 1)).to_i + 1
	    # the first linexp_level actions behave like linear
	    (1..linexp_level).each do | i |
		world.puts "action a#{i}"
		(1..i-1).each do | j |
		    world.puts "  b#{j} (0.0)"
		end
		world.print "  b#{i} "
		world.puts "(1.0)"
		world.puts "endaction"
	    end
	    # the rest behave like expon
	    (linexp_level+1..n).each do | i |
		world.puts "action a#{i}"
		(1..i-1).each do | j |
		    world.puts "  b#{j} (0.0)"
		end
		world.print "  b#{i} "
		(linexp_level..i-1).each do | j |
		    world.print "(b#{j} "
		end
		world.print " (1.0)"
		world.puts ' (0.0))' * (i - linexp_level)
		world.puts "endaction"
	    end
	when "delay"
	    world.puts "action a"
	    world.puts "  a (1.0)"
	    world.puts "  b (0.0)"
	    world.puts "endaction"
	    world.puts "action b"
	    world.puts "  a (0.0)"
	    world.puts "  b (1.0)"
	    world.puts "endaction"
	when "polynomial"
	    world.puts "action mess"
	    world.puts "        Q       (Q      (0.50)"
	    world.puts "                        (0.50))"
	    world.puts "        P       (P      (0.50)"
	    world.puts "                        (0.50))"
	    world.puts "endaction"
	when "kill-pltl"
	    world.puts "action mess"
	    world.puts "   A (0.5)"
	    world.puts "   B (0.5)"
	    (1..n).each do | i |
		world.puts "   p#{i} (0.5)"
	    end
	    world.puts "endaction"
	when "ago"
	    world.puts "action a"
	    world.puts "  a (1.0)"
	    world.puts "endaction"
	when "vars", "ago-vars"
	    world.puts "action a"
	    world.puts "  a (1.0)"
	    world.puts "endaction"
	    (1..n).each do | i |
		world.puts "action b#{n}"
		world.puts "  b#{n} (1.0)"
		world.puts "endaction"
	    end
	else
	    (1..n).each do | i |
		world.puts "action a#{i}"
		world.puts "  b#{i} (b#{i} (1.0) (0.8))"
		world.puts "endaction"
		world.puts "action r#{i}"
		world.puts "  b#{i} (b#{i} (0.2) (0.0))"
		world.puts "endaction"
		world.puts ""
	    end
	end
    end
    
    world.close
end
    
# program execution command script

def time_stats(timed)
    "'#{timed} time: ' readTimer\n" +
	"'#{timed} CPU time: ' readCPUtimer"
end

def run_test(action, lang, preprocessing, algorithm, discount, epsilon)
    test = open("test.cmd", "w")
    test.puts "loadWorld('test.world')"
    test.puts "monitorMemory(0.1)" if action == 'slow-stats'
    
    case algorithm
    when "LAO-polIt", "LAO-valIt", "polIt", "valIt"
	if action == 'slow-stats'
	    test.puts "expansionMemory(1)"
	end
	
	case lang
	when "PLTL"
	    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	    if preprocessing == "minimise"
		test.puts "preprocess('mPltl')"
	    else
		test.puts "preprocess('pltl')"
	    end
	    test.puts "stopTimer\nstopCPUtimer"
	    test.puts time_stats('preprocessing')
	    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	    test.puts "expand" unless algorithm =~ /LAO/
	    if action == 'stats'
		test.puts "stopTimer\nstopCPUtimer"
		test.puts time_stats('expansion')
	    end
	when "FLTL"
	    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	    test.puts "preprocess('')"
	    test.puts "stopTimer\nstopCPUtimer"
	    test.puts time_stats('preprocessing')

	    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	    test.puts "expand" unless algorithm =~ /LAO/
	    if action == 'stats'
		test.puts "stopTimer\nstopCPUtimer"
		test.puts time_stats('expansion')
	    end
	end
    when "spudd"
	test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	test.puts "PLTLvarExpand"
	if action == 'stats'
	    test.puts "stopTimer\nstopCPUtimer"
	    test.puts time_stats('NMRDP->MDP conversion')
	end
	
	if lang != "PLTL"
	    $stderr.puts "#{algorithm} does not support #{lang}"
	    exit(1)
	end
    end

    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
    case algorithm
    when "spudd"
	case preprocessing
	when 'none'
	    test.puts "autoConstrain='false'"
	when 'auto-constrain'
	    test.puts "autoConstrain='true'"
	else
	    $stderr.puts "#{algorithm} does not support '#{preprocessing}' preprocessing"
	    exit(1)
	end
	if action == 'slow-stats'
	    test.puts "monitorSpuddPolicyChanges='true'"
	end
	test.puts "spudd(#{discount}, #{epsilon})"
	if action == 'stats'
	    test.puts "'delta-history: ' spuddDeltaHistory"
	elsif action == 'slow-stats'
	    test.puts "'policy-change-history:' spuddChangeHistory"
	    test.puts "'value-leaf-count:' spuddValueLeaves"
	    test.puts "'value-density:' spuddValueDensity"
	    test.puts "'value-node-count:' spuddValueNodes"
	    test.puts "'value-path-count:' spuddValuePaths"
	    test.puts "'reachable states:' reachableStates"
	end
    when /LAO-(polIt|valIt)/
	test.puts "LAO('#{$1}', #{discount}, #{epsilon})"
    when "polIt"
	test.puts "polIt(#{discount})"
    when "valIt"
	test.puts "valIt(#{discount}, #{epsilon})"
    else
	$stderr.puts "Unknown algorithm '#{algorithm}'"
	exit(1)
    end

    if algorithm == 'spudd'
	if action == 'stats'
#	    test.puts "'reachable states:' reachableStates"
	end
    else
	test.puts "iterationCount"
	if action == 'stats'
	    test.puts "longestLabel"
	    test.puts "domainStateSize"
	    test.puts "averageLabelSize"
	    test.puts "domainMemorySize"
	elsif action == 'slow-stats'
	    test.puts "expansionPeak"
	end
    end
    
    test.puts "stopMonitoringMemory" if action == 'slow-stats'
    if action == 'stats'
	test.puts "stopTimer\nstopCPUtimer"
	test.puts time_stats('Algorithm execution')
    end

    if action == 'display-xmdp'
	case algorithm
	when "spudd"
	    $stderr.puts "cannot display XMDP when using symbolic algorithm"
	else
	    test.puts "printDomain('') | 'ruby -w ../show-domain.rb'"
	end
    end

    if action == 'display-policy'
	case algorithm
	when "spudd"
	    test.puts "displayDot(spuddPolicyToDot)"
	else
	    test.puts "getPolicy"
	end
    end

    if action == 'slow-stats'
	test.puts "'peak memory usage: ' peakMemoryUsage"
    end

    if action == 'stats'
	test.puts "'#{action} run successful'" if action != 'term'
	if $show_policy_domain
	    if algorithm != 'spudd'
		test.puts "'----------domain----------'"
		test.puts 'printDomain("")'
	    end
	    test.puts "'----------policy----------'"
	    if algorithm == 'spudd'
		test.puts 'spuddPolicyToDot'
	    else
		test.puts 'getPolicy'
	    end
	end
    end

    if action != 'term'
	test.puts "quit"
    end
    test.close
    if !(rv = system("../nmrdpp test.cmd 2>&1"))
	puts "Command execution failed: return value = #{rv}"
	return false
    end
    return true
end

action = ARGV[0]
lang = ARGV[1]
problem = ARGV[2]
n = ARGV[3].to_i
preprocessing = ARGV[4]
algorithm = ARGV[5]

when_reward = 'every'
if problem =~ /^(every|first)-(.*)$/
    when_reward = $1
    problem = $2
end

if problem == 'spudd-expon' || problem =~ /^linexp-/
    reward = 10000000000000000
    discount = 0.99
    epsilon = 0.01
end

good = true

puts "----------command line----------"
puts "'" + ARGV.join("' '") + "'"
    
create_problem_file(lang, when_reward, problem, n, reward)
puts "----------domain specification----------"
system("cat test.world")
puts "----------results----------"
if action == 'stats'
    good &= run_test('slow-stats', lang, preprocessing, algorithm, discount, epsilon)
    File.rename('test.cmd', 'test-slow.cmd')
end
if good
    good &= run_test(action, lang, preprocessing, algorithm, discount, epsilon)
end
puts "----------slow command script----------"
system("cat test-slow.cmd")
puts "----------command script----------"
system("cat test.cmd")
exit(good ? 0 : 1)
