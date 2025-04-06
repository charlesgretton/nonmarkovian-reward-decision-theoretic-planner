#!/usr/bin/ruby -w
class Item
    attr_accessor :descrip, :files, :user
end

class String
    def latex_escape
	# we can't use gsub to do this because there doesn't seem to be any
	# way to insert a single \ for a substitution
	
	result = ""
	each_byte do | byte |
	    char = sprintf('%c', byte)
	    
	    if char == '~'
		result += '\~{}'
	    else
		if char =~ /[&_\$#\^{}]/
		    result += "\\" + char
		elsif char =~ /[<>]/
		    result += "$" + char + "$"
		elsif char =~ /\\/
		    result += "\\verb+#{char}+"
		else
		    result += char
		end
	    end
	end
	result
    end
    def make_latex_safe
	gsub(/_/, '\_').gsub(/</, '\<').gsub(/>/, '\>').
	    gsub(/#/, '\#').gsub(/\^/, "\\^").gsub(/\$/, "\\$").
	    gsub(/\>/, "\\>").gsub(/\</, "\\<").gsub(/&/, "\\\\&")
    end
end

stuff = Hash.new
date = ""

fname = ""
inp = open "|cvs log"
inp.each_line do | line |
    if line =~ /^date:\s*([^ ]+ [^ ]+):..; *author: +([^ ;]+);/
	date = $1
	user = $2

	thing = ""
	inp.each_line do | line |
	    break if line =~ /[-=]{5,}/
	    thing += line
	end
	if !stuff.has_key?(date)
	    stuff[date] = Item.new
	    stuff[date].descrip = thing
	    stuff[date].files = ""
	end
	if stuff[date].files != ""
	    stuff[date].files += ", "
	end
	stuff[date].files += fname
	stuff[date].user = user
    elsif line =~ /Working file: (.*)$/
	fname = $1
    end
end
inp.close

stuff.keys.sort.each do | date |
    print `date -d "#{date} UTC" '+%a, %b %d, %I:%M%p'`
    puts " & #{stuff[date].user}" +
	" & #{stuff[date].files.latex_escape}" +
	" & #{stuff[date].descrip.latex_escape} \\\\"
    puts "\\hline"
end
