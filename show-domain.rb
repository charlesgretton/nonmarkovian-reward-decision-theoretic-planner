#!/usr/bin/ruby -w
dotfile = open("tmp.dot", "w")
dotfile.puts 'digraph "Domain specification" {'
dotfile.puts 'center = true;'

state = ""
action = ""
reward = ""
$stdin.each_line do | line |
    case line
    when /^\s*State at address 0(x.*)\s*$/
	state = $1
	reward = ""
    when /^REWARD = (.*)$/
	reward = $1
    when /^\s*Propositions that are true in this state are ::\s*$/
	line = $stdin.readline.chomp
	line.sub!(/, *$/, '')
	if line.empty?
	    line = '-'
	end
	dotfile.puts "#{state} [label=\"#{line}\\nReward=#{reward}\"]"
    when /^\s*On action (.*) the following states may be reached ::\s*$/
	action = $1
	while true
	    line = $stdin.readline.chomp
	    if line.empty?
		break
	    end
	    if line =~ /^\s*On action (.*) the following states may be reached ::\s*$/
		action = $1
		line = $stdin.readline.chomp
	    end
	    line =~ /0(x[a-f0-9]+) *([0-9\.]+)/
	    dotfile.puts "#{state} -> #{$1} [label=\"#{action}(#{$2})\"]"
	end
    end
end
dotfile.puts '}'
dotfile.close
if ARGV.size == 0
    system("dot -Tps tmp.dot -o tmp.ps && gv tmp.ps")
else
    if ARGV[0] =~ /\.fig/
	system("dot -Tfig tmp.dot -o #{ARGV[0]}")
    else
	#system("dot -Tfig tmp.dot -o tmp.fig && fig2dev -Leps tmp.fig #{ARGV[0]}")
	system("dot -Tps tmp.dot -o tmp.ps && ps2epsi tmp.ps #{ARGV[0]}")
    end
end
