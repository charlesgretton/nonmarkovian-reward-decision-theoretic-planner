#!/usr/bin/ruby -w

def create_problem_world(param, reward_specs, action_specs, problems)
    world = open("test.world", "w")
    if param.has?('problem')
	if !problems.has_key?(param.problem)
	    $stderr.puts "Unknown problem: \"#{param.problem}\""
	    exit(1)
	end
	p = problems[param.problem].new(param)
	p.set_defaults
	world.puts p.to_s
    else
	if !reward_specs.has_key?(param.reward_spec)
	    $stderr.puts "Unknown reward spec: \"#{param.reward_spec}\""
	    exit(1)
	end
	if !action_specs.has_key?(param.action_spec)
	    $stderr.puts "Unknown action spec: \"#{param.action_spec}\""
	    exit(1)
	end
	r = reward_specs[param.reward_spec].new(param)
	a = action_specs[param.action_spec].new(param)
	r.set_defaults
	a.set_defaults

	if param.initial_proposition_value
	    (1..param.n).each do | i |
		world.puts "p#{i} = tt"
	    end
	end
	
	world.puts r.get_reward_string
	world.puts
	world.puts a.to_s
    end
    world.close
end

def time_stats(timed)
    "'#{timed} time: ' readTimer\n" +
	"'#{timed} CPU time: ' readCPUtimer"
end

def create_problem_command(param, command_file='test.cmd')
    action = param.action
    test = open(command_file, "w")
    if param.algorithm != 'spudd' and param.language == 'PLTL'
        test.puts "startStateRequired(1)"
    end
    test.puts "loadWorld('test.world')"
    test.puts "monitorMemory(0.1)" if param.slow_stats
    
    case param.algorithm
    when "LAO-polIt", "LAO-valIt", "polIt", "valIt"
	if param.slow_stats
	    test.puts "expansionMemory(1)"
	end
	
	case param.language
	when "PLTL"
	    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	    if param.preprocessing == "minimise"
		test.puts "preprocess('mPltl')"
	    else
		test.puts "preprocess('pltl')"
	    end
	    test.puts "stopTimer\nstopCPUtimer"
	    test.puts time_stats('preprocessing')
	    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	    test.puts "expand" unless param.algorithm =~ /LAO/
	    if action == 'stats'
		test.puts "stopTimer\nstopCPUtimer"
		test.puts time_stats('expansion')
	    end
	when "FLTL"
	    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	    test.puts "preprocess('')"
	    test.puts "stopTimer\nstopCPUtimer"
	    test.puts time_stats('preprocessing')

	    unless param.algorithm =~ /LAO/
		test.puts "startTimer\nstartCPUtimer" if action == 'stats'
		test.puts "expand"
		if action == 'stats'
		    test.puts "stopTimer\nstopCPUtimer"
		    test.puts time_stats('expansion')
		end
	    end
	end
    when "spudd"
	test.puts "startTimer\nstartCPUtimer" if action == 'stats'
	test.puts "PLTLvarExpand"
	if action == 'stats'
	    test.puts "stopTimer\nstopCPUtimer"
	    test.puts time_stats('NMRDP->MDP conversion')
	end
	
	if param.language != "PLTL"
	    $stderr.puts "#{param.algorithm} does not support #{param.language}"
	    exit(1)
	end
    end

    test.puts "startTimer\nstartCPUtimer" if action == 'stats'
    case param.algorithm
    when "spudd"
	case param.preprocessing
	when 'none'
	when 'auto-constrain'
	    test.puts "autoConstrain"
	else
	    $stderr.puts "#{param.algorithm} does not support '#{param.preprocessing}' preprocessing"
	    exit(1)
	end
	if param.slow_stats
	    test.puts "monitorSpuddPolicyChanges='true'"
	end
	test.puts "spudd(#{param.discount}, #{param.epsilon})"
	if param.slow_stats
	    test.puts "'policy-change-history:' spuddChangeHistory"
	    test.puts "'value-leaf-count:' spuddValueLeaves"
	    test.puts "'value-density:' spuddValueDensity"
	    test.puts "'value-node-count:' spuddValueNodes"
	    test.puts "'value-path-count:' spuddValuePaths"
	    test.puts "'reachable states:' reachableStates"
	elsif action == 'stats'
	    test.puts "'delta-history: ' spuddDeltaHistory"
	end
    when /LAO-(polIt|valIt)/
	test.puts "LAO('#{$1}', #{param.discount}, #{param.epsilon})"
    when "polIt"
	test.puts "polIt(#{param.discount})"
    when "valIt"
	test.puts "valIt(#{param.discount}, #{param.epsilon})"
    else
	$stderr.puts "Unknown algorithm '#{param.algorithm}'"
	exit(1)
    end

    if param.algorithm == 'spudd'
	if action == 'stats'
#	    test.puts "'reachable states:' reachableStates"
	end
    else
	test.puts "iterationCount"
	if param.slow_stats
	    test.puts "expansionPeak"
	end
	if action == 'stats'
	    test.puts "longestLabel"
	    test.puts "domainStateSize"
	    test.puts "averageLabelSize"
	    test.puts "domainMemorySize"
	end
    end
    
    test.puts "stopMonitoringMemory" if param.slow_stats
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
	case param.algorithm
	when "spudd"
	    test.puts "displayDot(spuddPolicyToDot)"
	else
	    test.puts "getPolicy"
	end
    end

    if param.slow_stats
	test.puts "'peak memory usage: ' peakMemoryUsage"
    end

    if action == 'stats'
	test.puts "'#{action} run successful'" if action != 'term'
	if param.show_policy_domain
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
end

def run_test(command_file='test.cmd')
    if !(rv = system("../nmrdpp #{command_file} 2>&1"))
	#puts "Command execution failed: return value = #{rv}"
	puts "nmrdpp returned non-zero status, ignoring"
	#return false
    end
    return true
end

$action_specs = Hash.new
$reward_specs = Hash.new
$problems = Hash.new

class ActionSpec
    def ActionSpec.inherited(sub)
	$action_specs[sub.name] = sub
    end

    attr_accessor :param

    def initialize(param)
	@param = param
    end

    def set_defaults
    end

    def n
	param.n
    end
end

class RewardSpec
    def RewardSpec.inherited(sub)
	$reward_specs[sub.name] = sub
    end

    attr_accessor :param, :propositions

    def initialize(param)
	@param = param
	@propositions = (1..param.num_vars).collect { | i | "p#{i}" }
    end

    def n
	param.n
    end

    def reward_to_string(formulae)
	if formulae.type != Array
	    formulae = [formulae]
	end
	
	result = ""
	formulae.each_index do | i |
	    result += "[r#{i}, #{param.reward}]? #{formulae[i]}\n"
	end
	result
    end

    def get_reward
	case param.language
	when "PLTL"
	    get_pltl_reward
	when "FLTL"
	    get_fltl_reward
	end
    end

    def get_fltl_reward
	$stderr.puts "FLTL not implemented for #{type}"
	exit(1)
    end

    def get_pltl_reward
	$stderr.puts "PLTL not implemented for #{type}"
	exit(1)
    end

    def get_reward_string
	reward_to_string(get_reward)
    end
    
    def set_defaults
    end

    def pltl_first_every(f)
	if param.has?('first') and param.first == 'true'
	    "(#{f}) and ~(prv pdi (#{f}))"
	else
	    f
	end
    end

    def nxt_or_nothing(n)
	if 0 == n
	    return ""
	else
	    return "nxt^#{n}"
	end
    end

    def prv_or_nothing(n)
	if 0 == n
	    return ""
	else
	    return "prv^#{n}"
	end
    end
    
    def fltl_first_every(f, nxt)
	if param.has?('first') and param.first == 'true'
	    "(~(#{f})) fut ((#{f}) and #{nxt_or_nothing(nxt)} $)"
	else
	    "fbx ((#{f}) -> #{nxt_or_nothing(nxt)} $)"
	end
    end
end

class Problem
    def Problem.inherited(sub)
	$problems[sub.name] = sub
    end

    attr_accessor :param

    def initialize(param)
	@param = param
    end
end

class RandomDomain
    attr_accessor :action_spec, :attainable_rewards,:unattainable_rewards

    def initialize(param, num_attainable, num_unattainable)
	@param = param
	
	t = open("create-rand.cmd", "w")
	t.puts "randomSeed(#{param.seed})"
	t.puts "randomActionSpec(#{param.num_vars}, #{param.num_actions}, #{param.p_reach}, #{param.influence}, #{param.uncertainty})"
	t.puts "randomReward(#{num_attainable}, #{num_unattainable})"
	t.puts "rewardSpecification"
	t.puts "actionSpec"
	t.puts "quit"
	t.close

	@action_spec = ""
	@attainable_rewards = Array.new
	@unattainable_rewards = Array.new

	t = open("|../nmrdpp create-rand.cmd 2>/dev/null")
	t.each_line do | line |
	    if line =~ /^(r\d+) :: (\d+) :: (.*)$/
		@attainable_rewards.push($3)
	    elsif line =~ /^(u\d+) :: (\d+) :: (.*)$/
		@unattainable_rewards.push($3)
	    else
		@action_spec += line
	    end
	end
	t.close
    end

    def to_s
	result = ""
	
	attainable_rewards.each_index do | k |
	    form = attainable_rewards[k]
	    result += "[r#{k}, #{@param.reward}]? "
	    if param.language == 'FLTL'
		result += "fbx(#{form})\n"
	    else
		result += "#{form}\n"
	    end
	end

	result += action_spec

	return result
    end
end

module RandomDefaults
    def set_defaults
	param.default('seed', 1.0)
	param.default('influence', 0.3)
	param.default('uncertainty', 0)
	param.default('p_reach', 0.04)
	param.default('num_reachable_rewards', param.n / 2)
	param.default('num_unreachable_rewards', param.n / 2)
    end
end


################ Problems ###################

class RandomProblem <Problem
    include RandomDefaults
    
    def to_s
	RandomDomain.new(param, param.num_reachable_rewards, param.num_unreachable_rewards).to_s
    end
end

class RandomProblemPrv <Problem
    include RandomDefaults
    
    def to_s
	result = "[r0, #{param.reward}]? "
	rd = RandomDomain.new(param, 2, 1)

	if rd.unattainable_rewards.size < 1
	    $stderr.puts "Failed to generate sufficient unattainable rewards"
	    exit(1)
	end
	if rd.attainable_rewards.size < 2
	    $stderr.puts "Failed to generate sufficient attainable rewards"
	    exit(1)
	end

	r1 = rd.attainable_rewards[0]
	r2 = rd.attainable_rewards[1]
	if param.r1 =~ /^u/
	    r1 = rd.unattainable_rewards[0]
	elsif param.r2 =~ /^u/
	    r1 = rd.unattainable_rewards[0]
	end
	
	if param.language == 'PLTL'
	    result += "(#{r1}) and prv^#{param.n} (#{r2})"
	end
	if param.language == 'FLTL'
	    result += "fbx ((#{r2}) -> nxt^#{param.n} ((#{r1}) -> $))"
	end

	result += "\n\n" + rd.action_spec
    end
end

################ Action spec ###################
class LinExp <ActionSpec
    def set_defaults
	param.default('discount', 0.99)
	param.default('epsilon', 0.01)
	param.default('reward', 10000000000000000)
    end
    
    def to_s
	result = ""
	
	linexp_level = ((1 - param.exp_prop.to_f) * (param.n - 1)).to_i + 1
	# the first linexp_level actions behave like linear
	(1..linexp_level).each do | i |
	    result += "action a#{i}\n"
	    (1..i-1).each do | j |
		result += "  p#{j} (0.0)\n"
	    end
	    result += "  p#{i} "
	    result += "(1.0)\n"
	    result += "endaction\n"
	end
	# the rest behave like expon
	(linexp_level+1..param.n).each do | i |
	    result += "action a#{i}\n"
	    (1..i-1).each do | j |
		result += "  p#{j} (0.0)\n"
	    end
	    result += "  p#{i} "
	    (linexp_level..i-1).each do | j |
		result += "(p#{j} "
	    end
	    result += " (1.0)"
	    result += ' (0.0))' * (i - linexp_level) + "\n"
	    result += "endaction\n"
	end

	result
    end
end

class SpuddExpon <LinExp
    def initialize(param)
	super
	param.default('exp_prop', 1.0)
    end
end

class SpuddLinear <LinExp
    def initialize(param)
	super
	param.default('exp_prop', 0.0)
    end

    def set_defaults
	# do nothing here, we just don't want LinExp to set its defaults
    end
end

class Balls <ActionSpec
    def to_s
	result = ""
	(1..param.n).each do | i |
	    result += "action a#{i}\n"
	    result += "  p#{i} (p#{i} (1.0) (0.8))\n"
	    result += "endaction\n"
	    result += "action r#{i}\n"
	    result += "  p#{i} (p#{i} (0.2) (0.0))\n"
	    result += "endaction\n"
	end

	result
    end
end

class FiftyFifty <ActionSpec
    def to_s
	"action all\n" + (1..param.num_vars).collect do | i |
	    "   p#{i} (0.5)\n"
	end.join + "endaction"
    end
end

class FiftyFiftyLessG <ActionSpec
    def to_s
	"action all\n" + (1..param.num_vars).collect do | i |
	    "   p#{i} (0.5)\n"
	end.join + "   G (0.0)\nendaction"
    end
end

class CompleteNoP <ActionSpec
    def to_s
	(1..param.num_actions).collect do | act |
	    "action do#{act}\n" + (1..param.num_vars).collect do | i |
		if i == act
		    "   p#{i} (#{act.to_f/(n + 1)})\n"
		else
		    "   p#{i} (0.5)\n"
		end
	    end.join + "   p (0.0)\n   c (0.5)\nendaction"
	end
    end
end


class CompleteNoC <ActionSpec
    def to_s
	(1..param.num_actions).collect do | act |
	    "action do#{act}\n" + (1..param.num_vars).collect do | i |
		if i == act
		    "   p#{i} (#{act.to_f/(n + 1)})\n"
		else
		    "   p#{i} (0.5)\n"
		end
	    end.join + "   c (0.0)\n   p (0.5)\nendaction"
	end
    end
end



class Complete <ActionSpec
    def to_s
	(1..param.num_actions).collect do | act |
	    "action do#{act}\n" + (1..param.num_vars).collect do | i |
		if i == act
		    "   p#{i} (#{act.to_f/(n + 1)})\n"
		else
		    "   p#{i} (0.5)\n"
		end
	    end.join + "endaction"
	end
    end
end

class RandomActionSpec <ActionSpec
    include RandomDefaults
    
    def to_s
	RandomDomain.new(param, param.num_reachable_rewards, param.num_unreachable_rewards).action_spec
    end
end

################## Reward spec ###################

class TempLin <RewardSpec
    def get_fltl_reward
	"nxt ^ #{param.n-1} $" #"
    end
    
    def get_pltl_reward
	"prv^ #{param.n-1} ~ prv tt"
    end
end


class AllTrue <RewardSpec
    def get_fltl_reward
	fltl_first_every(propositions.join(' and '), 0)
    end
    
    def get_pltl_reward
	pltl_first_every(propositions.join(' and '))
    end
end

class SomeTrue <RewardSpec
    def get_fltl_reward
	fltl_first_every(propositions.join(' or '), 0)
    end
    
    def get_pltl_reward
	pltl_first_every(propositions.join(' or '))
    end
end

class OneTrue <RewardSpec
    def get_fltl_reward
	fltl_first_every('p1', 0)
    end
    
    def get_pltl_reward
	pltl_first_every('p1')
    end
end

class ConstantFactor <RewardSpec
    def get_fltl_reward
	"fbx ((" + (0..2).collect do | i |
	    nxt_or_nothing(i) + '(' + propositions.join(' and ') + ')'
	end.join(' and ') + ") -> #{nxt_or_nothing(2)} $)"
    end
    
    def get_pltl_reward
	(0..2).collect do | i |
	    prv_or_nothing(i) + '(' + propositions.join(' and ') + ')'
	end.join(' and ')
    end
end

class RewardLinear <RewardSpec
    def get_fltl_reward
	"fbx (" + (1..n - 1).collect do | i |
	    "(p#{i} and nxt p#{i + 1} -> nxt $)"
	end.join(' and ') + ")"
    end

    def get_pltl_reward
	(1..n - 1).collect do | i |
	    "(prv p#{i} and p#{i + 1})"
	end.join(' or ')
    end
end

class Exponential <RewardSpec
    def get_fltl_reward
      "fbx ( (" + (1..param.n).collect do | i |
	    "p#{i}"
      end.join(' and ') + ") -> #{nxt_or_nothing(param.n)} $)"
    end
    
    def get_pltl_reward
      "prv^#{param.n} (" + (1..param.n).collect do | i |
	    "p#{i}"
      end.join(' and ') + ")"
    end
end

class Sequence <RewardSpec
    def get_fltl_reward
	condition = (1..param.n).collect do | i |
	    "(#{nxt_or_nothing(i - 1)} p#{i})"
	end.join(' and ')
	fltl_first_every(condition, param.n - 1)
    end

    def get_pltl_reward
	condition = (1..param.n).collect do | i |
	    "(#{prv_or_nothing(n - i)} p#{i})"
	end.join(' and ')
	pltl_first_every(condition)
    end
end

class SeqSeq <RewardSpec
    def get_fltl_reward
	condition = "p1" + (n > 1 ? ' and ' : '')
	condition += (2..n**2).collect do | i |
	    "#{nxt_or_nothing(i - 1)} p#{((i - 1) % n) + 1}"
	end.join(' and ')
	fltl_first_every(condition, n**2 - 1)
    end

    def get_pltl_reward
	condition = (1..n**2 - 1).collect do | i |
	    "(#{prv_or_nothing(n**2 - i)} p#{((n**2  - (i - 1)) % n) + 1})"
	end.join(' and ')
	condition += ' and' if n > 1
	condition += " p#{n}"
	pltl_first_every(condition)
    end
end

class Polynomial <RewardSpec
    def get_fltl_reward
	fltl_first_every("(nxt or < #{param.n} p1) and (#{nxt_or_nothing(param.n)} p2)", param.n)
    end

    def get_pltl_reward
	pltl_first_every("p1 and prv or < #{param.n} p2")
    end
end

class Poly2 <RewardSpec
    def get_fltl_reward
	"fbx (((nxt or < #{param.n} p1) and (#{nxt_or_nothing(param.n)} p2)) -> nxt^#{param.n} $)"
    end

    def get_pltl_reward
	"prv^#{param.n} tt and p2 and prv or < #{param.n} p1"
    end
end

class Sequence2 <RewardSpec
    def get_fltl_reward
	"fbx (" + (1..param.n - 1).collect do | i |
	    "(p#{i} and nxt p#{i + 1} -> nxt $)"
	end.join(' and ') + ")"
    end

    def get_pltl_reward
	(1..param.n - 1).collect do | i |
	    "(prv p#{i} and p#{i + 1})"
	end.join(' or ')
    end
end

class Piano <RewardSpec
    def get_fltl_reward
	(2..param.n).collect do | i |
	    "fbx ( p1 -> ( ((p#{i - 1} and (nxt p#{i})) -> nxt($)) fut p#{n} ) )"
	end
    end
    
    def get_pltl_reward
	(2..param.n - 1).collect do | i |
	    "((~p#{n} and (prv tt)) snc p1) and (prv p#{i - 1}) and p#{i}"
	end + ["((~(prv p#{n})) snc p1) and (prv p#{n - 1}) and p#{n}"]
    end
end

class NonMinPLTL_Min <RewardSpec
    def get_fltl_reward
	(1..param.n).collect do | i |
	    "fbx ( p#{i} -> nxt $)"
	end
    end
    
    def get_pltl_reward
	(1..param.n).collect do | i |
	    "prv p#{i}"
	end 
    end
end

class RewardIrrelevance <RewardSpec
    def get_pltl_reward
      ["p and prv c", 
        "p and prv ^ #{param.k} prv c", 
        "p and prv and < #{param.k} c", 
        "p and prv ^ #{param.k} c"]
    end
    
    def get_fltl_reward
      ["fbx (c -> fbx (p -> $))", 
        "fbx (c -> nxt ^ #{param.k} fbx (p -> $))", 
        "fbx ((nxt and < #{param.k} c and nxt ^ #{param.k} p) -> nxt ^ #{param.k - 1} $)", 
        "fbx ((c and nxt ^ #{param.k} p) -> nxt ^ #{param.k} $)"]
    end
end


class LinearSequence <RewardSpec
    def get_fltl_reward
	(1..param.n).collect do | i |
	    "( nxt ^ #{i} p#{i})"
	end.join(' and ') + " -> nxt^#{param.n} $" #"
    end

    def get_pltl_reward
	(1..param.n).to_a.reverse.collect do | i |
	    "#{prv_or_nothing(i-1)} p#{i}"
	end.join(" and ") + " and  prv^#{param.n} ~ prv tt" #"
    end
end

class DCDelay <RewardSpec
    def get_fltl_reward
	fltl_first_every("( (" + propositions[0..param.n-2].join(' or ') +") and G)", param.n)
    end

    def get_pltl_reward
	pltl_first_every("prv ^ #{param.n} ( (" + propositions[0..param.n-2].join(' or ') + ") and G)")
    end
end

class DCDelayFixed <RewardSpec
    def get_fltl_reward
	fltl_first_every("( (" + propositions[0..param.n-2].join(' or ') +") and G)", param.delay)
    end

    def get_pltl_reward
	pltl_first_every("prv ^ #{param.delay} ( (" + propositions[0..param.n-2].join(' or ') + ") and G)")
    end
end

class Unachievable <RewardSpec
    def nxt_exp(i, theta)
	if i == 0
	    theta
	elsif i == 1
	    "nxt #{theta}"
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
    
    def get_fltl_reward
	(1..param.n).collect do | i |
	    result = "fbx((p1 and nxt^#{i} p3 and "
	    tmp = (0..i).collect do | j |
		"(#{nxt_exp(j, 'p1')} and #{nxt_exp(j, nxt_and(i - j + 1, '(~p1 and ~p2)'))})"
	    end
	    result += tmp.join(" or ")
	    result += ")-> nxt^#{i} $)"

	    result
	end
    end

    def get_pltl_reward
	(1..param.n).collect do | i |
	    "(prv ^ #{i} p1) and p3 and ((~p2) snc p1)"
	end
    end
end

class TwoLevel <RewardSpec
    def get_pltl_reward
	result = ''
	case param.level1
	when 'Asnc'
	    result += 'A snc'
	when 'sncA'
	else
	    result += param.level1
	end
	result += '('
	case param.level2
	when 'Asnc'
	    result += 'A snc'
	when 'sncA'
	else
	    result += param.level2
	end
	result += '('
	result += propositions.join(' ' + param.conjdisj + ' ')

	result += ')'
	if 'sncA' == param.level2
	    result += 'snc A'
	end
	
	result += ')'
	if 'sncA' == param.level1
	    result += 'snc A'
	end

	return result
    end
end

# should In/Out be named the other way around?  I guess it depends on how you look at it.
class PrvIn <RewardSpec
    def get_pltl_reward
	"prv ^ #{param.n}(" + (1..param.n).collect { | i |
	    "p#{i}"
	}.join(' ' + param.conjdisj + ' ') + ")"
    end

    def get_fltl_reward
	"fbx (#{nxt_or_nothing(param.n)}(" + (1..param.n).collect do | i |
	    "p#{i}"
	end.join(' ' + param.conjdisj + ' ') + ") -> #{nxt_or_nothing(param.n)} $)"
    end
end

class PrvOut <RewardSpec
    def get_pltl_reward
	"("+(1..param.n).collect { | i |
	    "prv ^ #{param.n} (p#{i})"
	}.join(' ' + param.conjdisj + ' ') + ")"
    end

    def get_fltl_reward
	"fbx (" + (1..param.n).collect do | i |
	    "(p#{i} -> #{nxt_or_nothing(param.n)} $)"
	end.join(' ' + param.conjdisj + ' ') + ")"
    end
end

class PbxIn <RewardSpec
    def set_defaults
	param.default('initial_proposition_value', true)
    end
    
    def get_pltl_reward
	"pbx " * param.n + "(" + (1..param.n).collect { | i |
	    "p#{i}"
	}.join(' ' + param.conjdisj + ' ') + ")"
    end
    
    def get_fltl_reward
	"fbx " * (param.n - 1) + "$ fut ~ (" + (1..param.n).collect do | i |
	    "p#{i}"
	end.join(' ' + param.conjdisj + ' ') + ")"
    end
end

class PbxOut <RewardSpec
    def set_defaults
	param.default('initial_proposition_value', true)
    end
    
    def get_pltl_reward
	"(" + (1..param.n).collect { | i |
	    "pbx " * param.n + "p#{i}"
	}.join(' ' + param.conjdisj + ' ') + ")"
    end
    
    def get_fltl_reward
      "(" + (1..param.n).collect do | i |
	    "(" + "fbx " * (param.n - 1) + "$ fut ~ p#{i})"
      end.join(' ' + param.conjdisj + ' ') + ")"
    end
end


class KillFLTL <RewardSpec
    def get_pltl_reward
	"(prv ^ #{param.power} ("+(1..param.n).collect { | i |
	    "p#{i}"
	}.join(' ' + param.conjdisj + ' ') + ") and G)"
    end

    def get_fltl_reward
	"fbx ((" + (1..n).collect do | i |
	    "p#{i}"
	end.join(' ' + param.conjdisj + ' ') + " and #{nxt_or_nothing(param.power)} G) -> #{nxt_or_nothing(param.power)} $)"
    end
end

class ExpensivePhaseI <RewardSpec
    def get_pltl_reward
	"prv ^ #{param.power} ("+(1..param.n).collect { | i |
	    "p#{i}"
	}.join(' ' + param.conjdisj + ' ') + ")"
    end

    def get_fltl_reward
	"fbx ((" + (1..n).collect do | i |
	    "p#{i}"
	end.join(' ' + param.conjdisj + ' ') + ") -> #{nxt_or_nothing(param.power)} $)"
    end
end

class AchievableExactTiming <RewardSpec
    def get_pltl_reward
	propositions.join(' and ') + " and prv^#{n-2} ~p#{n-2}"
    end

    def get_fltl_reward
	"fbx((~p#{n-2}) -> nxt^#{n-2} ((#{propositions.join(' and ')}) -> $))"
    end
end

class UnachievableExactTiming <RewardSpec
    def get_pltl_reward
	propositions.join(' and ') + " and prv^#{n-2} ~p#{n-1}"
    end

    def get_fltl_reward
	"fbx((~p#{n-1}) -> nxt^#{n-2} ((#{propositions.join(' and ')}) -> $))"
    end
end

class TimePeriod <RewardSpec
    def get_pltl_reward
	propositions.join(' and ') + " and prv or <#{param.k + 1} ~p#{n-1}"
    end

    def get_fltl_reward
	"fbx((~p#{n-1}) -> nxt and <#{param.k + 1} ((#{propositions.join(' and ')}) -> $))"
    end
end

################# Command line ###################
def show_help
    $stderr.puts(
		 "usage: ruby create-prob option=value option=value...\n" +
		 "  action(stats): stats|display-policy|print-domain|display-xmdp|term\n" +
		 "  language: PLTL|FLTL\n" +
		 "  problem: #{$problems.keys.join('|')}\n" +
		 "  action_spec: #{$action_specs.keys.join('|')}\n" +
		 "  reward_spec: #{$reward_specs.keys.join('|')}\n" +
		 "  n(4): problem size\n" +
		 "  preprocessing(none): minimise|none|auto-constrain\n" +
		 "  algorithm: spudd|LAO-polIt|LAO-valIt|polIt|valIt\n" +
		 "eg: ruby create-prob.rb language=PLTL action_spec=SpuddExpon reward_spec=OneTrue algorithm=spudd")
    exit(1)
end

class Parameters
    def initialize(args)
	@default_list = Array.new
	
	args.each do | arg |
	    if arg =~ /^(.*)=([-0-9]+)$/
		set($1, $2.to_i)
	    elsif arg =~ /^(.*)=([-0-9\.]+)$/
		set($1, $2.to_f)
	    elsif arg =~ /^(.*)=(.*)$/
		set($1, $2)
	    else
		$stderr.puts "all parameters are of the form option=value"
		show_help
	    end
	end
    end

    def create_proc(key)
	if !methods.include?(key)
	    instance_eval <<-"end_eval"
	    def self.#{key}
		    @#{key}
	    end
	    end_eval
	end
    end

    def set(key, val)
	create_proc(key)
	instance_eval <<-"end_eval"
	@#{key} = #{val.inspect}
	end_eval

	if @default_list.include?(key)
	    @default_list.delete(key)
	end
    end

    def method_missing(methname)
	$stderr.puts "Need value for parameter \"#{methname}\""
	exit(1)
    end

    def default(key, val)
	if !methods.include?(key) || @default_list.include?(key)
	    create_proc(key)
	    instance_eval <<-"end_eval"
	    @#{key} = #{val.inspect}
	    end_eval
	    
	    if !@default_list.include?(key)
		@default_list.push(key)
	    end
	end
    end

    def has?(method)
	methods.include?(method)
    end
end

if ARGV.size == 0
    show_help
end
param = Parameters.new(ARGV)

param.default('discount', 0.95)
param.default('epsilon', 0.05)
param.default('reward', 1000000)
param.default('preprocessing', 'none')
param.default('action', 'stats')
param.default('n', 4)
param.default('num_vars', param.n)
param.default('num_actions', param.n)
param.default('show_policy_domain', false)
param.default('initial_proposition_value', false)

##################################################

good = true

case param.action
when 'stats'
    create_problem_world(param, $reward_specs, $action_specs, $problems)
    puts "----------command line----------"
    puts "'" + ARGV.join("' '") + "'"
    param.set('slow_stats', 'spudd' != param.algorithm)
    create_problem_command(param)
    puts "----------domain specification----------"
    system("cat test.world")
    puts "----------results----------"
    good &= run_test
    if 'spudd' == param.algorithm
	param.set('slow_stats', true)
	create_problem_command(param, 'test-slow.cmd')
	good &= run_test('test-slow.cmd')
	puts "----------slow command script----------"
	system('cat test-slow.cmd')
    end
    puts "----------command script----------"
    system("cat test.cmd")
when 'print-domain'
    create_problem_world(param, $reward_specs, $action_specs, $problems)
    system("cat test.world")
when 'print-action-spec'
    if !$action_specs.has_key?(param.action_spec)
	$stderr.puts "Unknown action spec: \"#{param.action_spec}\""
	exit(1)
    end
    
    a = $action_specs[param.action_spec].new(param)
    a.set_defaults
    puts a.to_s
when 'print-reward-spec'
    if !$reward_specs.has_key?(param.reward_spec)
	$stderr.puts "Unknown reward spec: \"#{param.reward_spec}\""
	exit(1)
    end
    
    r = $reward_specs[param.reward_spec].new(param)
    r.set_defaults
    puts r.get_reward_string
when 'term'
    param.set('slow_stats', false)
    create_problem_world(param, $reward_specs, $action_specs, $problems)
    create_problem_command(param)
    run_test
end
exit(good ? 0 : 1)
