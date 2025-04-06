#!/usr/bin/ruby -w

# Extracts documentation from source files, to produce a latex
# document.

# Only C-style comments are extracted

# Comments preceding a class are treated as documentation for that
# class.

# Comments preceding a public method are treated as documentation for
# that method.

# If a file contains multiple classes, it is treated as a module,
# which has its own documentation (first comment in the file).

# All comments are in latex - so be sure to escape '_' etc.

# Documentation for classes and modules is broken into named sections.
# The name of each section is given by "\paragraph{:name:}".  Text
# before the first section is ignored.

###########################################################################

# Should we create a class diagram for each class showing it's ancestors?
$include_class_diagrams = true

# Should we output a line of the form "Inherits: " then a list of the
# classes direct ancestors (if it has any).
$state_direct_ancestors = false

# Should we include each classes subclasses in it's diagram?
$include_subclasses = true

# Should we keep going with subsubclasses (and so on - likely to
# create diagrams that don't fit on the page)
$show_subsubclasses = false

# this doesn't work properly yet
$produce_graph_of_all_classes = false

# shoud we show inheritance types on our graphs
$show_inheritance_types = true

# If a method has no documentation, should we still produce a
# paragraph for it?
$paragraphs_for_undocumented = false

# Should we include the public interface for classes in their class
# diagram?
$interface_in_diagram = true

###########################################################################

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
		result += "\\" if char =~ /[&_\$]/
		result += char
	    end
	end
	result
    end

    def dot_escape
	result = ""
	each_char do | char |
	    result += "\\" if char =~ /[ <>\|\{\}]/
	    result += char
	end
	result
    end

    def each_char
	each_byte do | byte |
	    char = sprintf('%c', byte)
	    yield(char)
	end
    end

    def dot_comma_wrap(wrap_after)
	result = ""
	lcount = 0
	each_char do | char |
	    result += char
	    lcount += 1
	    if char == ',' and lcount > wrap_after
		lcount = 4
		result += '\l' + (' ' * lcount)
	    end
	end
	result
    end
end

class CommentBlock < Array
    attr_accessor :active, :sections
    
    def initialize
	@active = true
    end

    def parameterised_parse
	@sections = Hash.new
	section = ''
	section_text = Array.new
	each do | line |
	    if line =~ /\\paragraph\{:(.*):\}/
		if !section.empty?
		    @sections[section] = section_text
		end
		section = $1
		section_text = Array.new
	    else
		section_text.push(line)
	    end
	end
	if !section.empty?
	    @sections[section] = section_text
	end
    end

    def formatted_sections
	result = ''
	parameterised_parse
	sections.each do | secname, sec |
	    result += "\\paragraph\{#{secname}\}"
	    result += sec.join("\n")
	end
	result
    end
    
end

class ClassGroup < Hash
    attr_accessor :name, :doc

    def initialize(name)
	@name = name
    end

    def add(sysclass)
	self[sysclass.name] = sysclass
    end

    def each_class
	each_value do | sysclass |
	    yield(sysclass)
	end
    end
end

class SysMethod
    attr_accessor :doc, :name, :access_type

    def initialize(name, access_type, doc)
	@name = name.gsub(/\)const/, ') const').gsub(/= *0/, ' = 0').gsub(/ +/, ' ').
	    gsub(/^ */, '').gsub(/ *$/, '')
	@access_type = access_type
	@doc = doc
    end
end

class ClassGraph
    def initialize(classgroups)
	@allclasses = Hash.new
	classgroups.each do | group |
	    group.each_class do | sysclass |
		@allclasses[sysclass.name] = sysclass
	    end
	end
	@ingraph = Array.new
	@nodes = ''
	@edges = ''
	@global_attributes = ''

	# name -> id
	@allocated_nodes = Hash.new

	@next_node_id = 1
    end

    def get_node_id(name)
	if @allocated_nodes.member?(name)
	    @allocated_nodes[name]
	else
	    new_id = 'n' + @next_node_id.to_s
	    @next_node_id += 1
	    @allocated_nodes[name] = new_id
	    new_id
	end
    end

    def add_inheritance(from, to, types)
	if $show_inheritance_types
	    extra_param = 'fontsize=9 label="' + types.join('\n') + '"'
	end
	@edges += "     #{get_node_id(to)} -> #{get_node_id(from)} [dir=back arrowtail=empty arrowsize=\"1.5\" #{extra_param}];\n"
    end
    
    def addclass(name, include_subclasses, include_superclasses = true)
	if !@ingraph.member?(name)
	    @ingraph.push(name)
	    name_stripped = name.gsub(/<.*$/, '')
	    sysclass = @allclasses[name_stripped]
	    extra_param = nil
	    if block_given?
		extra_param = yield(sysclass)
	    end
	    if extra_param.nil?
		extra_param = ''
	    end
	    lbl = if extra_param =~ /label *=/
		      ''
		  else
		      "label=\"#{name}\""
		  end
	    @nodes += "    #{get_node_id(name)} [#{lbl} #{extra_param}];\n"
	    if sysclass.nil?
		$stderr.puts "unrecognised class: " + name
	    else
		if include_superclasses
		    sysclass.ancestors.each do | ancestor, types |
			add_inheritance(name, ancestor, types)
			if block_given?
			    addclass(ancestor, false) do | t |
				yield(t)
			    end
			else
			    addclass(ancestor, false)
			end
		    end
		end
		if include_subclasses
		    @allclasses.each do | oclassname, oclass |
			if oclass.ancestors.member?(name_stripped)
			    add_inheritance(oclassname, sysclass.name, oclass.ancestors[name_stripped])
			    if block_given?
				addclass(oclassname, $show_subsubclasses, false) do | t |
				    yield(t)
				end
			    else
				addclass(oclassname, $show_subsubclasses, false)
			    end
			end
		    end
		end
	    end
	end
    end

    def add_global_attribute(attr)
	@global_attributes += '    ' + attr + ";\n"
    end

    def write(name)
	dotfile = open(name + '.dot.tmp', "w")
	dotfile.puts("digraph ClassDiagram {")
	dotfile.puts(@global_attributes)
	dotfile.puts(@nodes)
	dotfile.puts(@edges)
	dotfile.puts("}")
	dotfile.close

	need_to_run_dot = true
	
	if File.exist?(name + '.dot')
	    if system("diff --brief \"#{name}.dot.tmp\" \"#{name}.dot\" >/dev/null")
		need_to_run_dot = false
	    end
	end

	if need_to_run_dot
	    File.rename(name + '.dot.tmp', name + '.dot')
	    system("dot -Tps \"#{name}.dot\" -o #{name}.eps")
	else
	    File.delete(name + '.dot.tmp')
	end
    end

    # note: must call write before you call this
    def width(name)
	eps_file = open("#{name}.eps")
	eps_file.each_line do | line |
	    if line =~ /%%BoundingBox: *(\d+) +(\d+) +(\d+) +(\d+)/
		return $3.to_i - $1.to_i
	    end
	end
	eps_file.close
    end
end

class SysClass
    attr_accessor :methods, :ancestors, :name, :doc, :inheritance_types

    def initialize(name, inheritance_string, doc)
	@name = name
	@methods = Array.new
	@doc = doc
	
	num_lt = 0
	@ancestors = Hash.new
	types = Array.new
	ancestor = ''
	inheritance_string.each_byte do | byte |
	    char = sprintf('%c', byte)

	    next if char == ':'
	    
	    if char == '<'
		num_lt += 1
	    elsif char == '>'
		num_lt -= 1
	    end
	    
	    if num_lt == 0 && char == ','
		@ancestors[ancestor.gsub(/ /, '')] = types
		ancestor = ''
		types = Array.new
	    else
		ancestor += char
	    end
	    
	    if ancestor =~ /(public|private|protected|virtual)/
		types.push(ancestor.gsub(/ /, ''))
		ancestor = ''
	    end
	end
	ancestor.gsub!(/ /, '')
	if !ancestor.empty?
	    @ancestors[ancestor] = types
	end

    end
end

class BackbufferedIO
    def initialize(io)
	@io = io
	@backbuffer = Array.new
    end

    def unget(line)
	@backbuffer.push(line)
    end

    def readline
	@io.readline
    end
    
    def each_line()
	@io.each_line do | line |
	    while !@backbuffer.empty?
		item = @backbuffer.pop
		yield(item)
	    end
	    yield(line)
	end
    end

    def read_until(first_line, regexp)
	result = String.new(first_line)
	return result if result =~ regexp
	@io.each_line do | line |
	    if line =~ regexp
		unget(line)
		break
	    end
	    result += ' ' + line
	end
	result
    end
    
end

class System
    attr_accessor :groups
    
    def initialize
	@groups = Array.new
    end
    
    def extract_doc(filename)
	filename =~ /\/([^\/]+)\.h\+\+/
	modulename = $1
	group = ClassGroup.new(modulename)
	file = open(filename)
	block = nil
	state = 'outside-class'
	class_indent = ''
	current_method = ''
	method_indent = ''
	access_type = ''
	in_module_doc_region = true
	current_class = nil
	look_ahead = Array.new
	bb = BackbufferedIO.new(file)
	bb.each_line do | line |

	    if line =~ /^ *class[^;]*$/
		line = bb.read_until(line, /\{/)
	    end

	    # handle cases of the form "class Something{};"
	    if line =~ /^( *)(class.*)(\{)(\};) *$/
		line = $1 + $2
		bb.unget($1 + $4)
		bb.unget($1 + $3)
	    end

	    # ignore cases of the form "class Something;"
	    if line =~ /^ *class.*; *$/
		line = ''
	    end

	    line.chomp!
	    line.gsub!(/\n/, ' ')

	    #printf("%-14s:%s\n", state, line)

	    # comment handling
	    if line =~ /^ *\/\* *(.*)$/
		# start of comment
		tmp = $1
		block = CommentBlock.new
		if tmp =~ /^(.*)\*\/$/
		    # comment ends on same line
		    tmp = $1
		    block.active = false
		end
		block.push(tmp)
	    elsif !block.nil? and block.active
		if line =~ /^ *\*? *(.*)\*\/$/
		    # end of comment
		    block.push($1) if $1.length > 0
		    block.active = false
		else
		    # continuing comment
		    line =~ / *\*? *(.*)$/
		    block.push($1)
		end
	    else
		# handling of code and non-C-style comments
		
		if in_module_doc_region
		    group.doc = block
		    block = nil
		end
		in_module_doc_region = false
		
		case state
		when 'outside-class'
		    if line =~ /^ *class +([^ :]+)(.*)$/
			current_class = SysClass.new($1, $2, block)
			block = nil
			group.add(current_class)
			state = 'class-decl'
		    end
		    
		when 'class-decl'
		    if line =~ /^( *)\{$/
			state = 'inside-class'
			class_indent = $1
		    end
		    
		when 'inside-class'
		    if line =~ /^#{class_indent}\}; *$/
			state = 'outside-class'
		    elsif line =~ /^( *)([^\(]*\([^\{;]+) *(([\{;])(.*))?$/
			m = $~
			current_method = m[2]
			if current_method =~ /^(.*\(.*\).*) *:/
			    current_method = $1
			end
			if m[4] == ';' || (m[4] == '{' && m[5] =~ /\} *$/)
			    state = 'inside-class'
			    current_class.methods.push(SysMethod.new(current_method, access_type, block))
			    block = nil
			elsif m[4] == '{'
			    state = 'method'
			    method_indent = m[1]
			else
			    state = 'method-decl'
			end
		    elsif line =~ /^ *([a-z]+):/
			access_type = $1
		    end
		    
		when 'method-decl'
		    if line =~ /^( *)\{$/
			state = 'method'
			method_indent = $1
		    else
			line =~ /^( *)([^\{;]+) *([\{;]?)(.*)$/
			m = $~
			current_method += m[2]
			if m[3] == ';' || (m[3] == '{' && m[4] =~ /\}$/)
			    state = 'inside-class'
			    current_class.methods.push(SysMethod.new(current_method, access_type, block))
			    block = nil
			elsif m[3] == '{'
			    state = 'method'
			    method_indent = m[1]
			end
		    end
		    
		when 'method'
		    if line =~ /^#{method_indent}\}$/
			state = 'inside-class'
			current_class.methods.push(SysMethod.new(current_method, access_type, block))
			block = nil
		    end
		end
		
	    end
	    
	end
	file.close
	@groups.push(group)
    end
end

sys = System.new
#sys.extract_doc("../formulaNormaliser.h++")
#exit

Dir.glob("../*.h++").each do | hppfilename |
    sys.extract_doc(hppfilename)
end

output = open("modules.tex", "w")
$defout = output

if $produce_graph_of_all_classes
    cg = ClassGraph.new(sys.groups)
    cg.add_global_attribute('size="16,10"')
    cg.add_global_attribute('ratio=0.15')
    sys.groups.each do | group |
	group.each_class do | sysclass |
	    cg.addclass(sysclass.name, false)
	end
    end
    cg.write("all-classes")
end

sys.groups.each do | group |
    level = 'sub'
    if group.size > 1
	puts "\\subsection{Module #{group.name.latex_escape}}"
	level += 'sub'

	if group.doc
	    puts group.doc.formatted_sections
	end
    end
    group.each_class do | aclass |
	puts "\\#{level}section{Class #{aclass.name.latex_escape}}"

	if $include_class_diagrams
	    cg = ClassGraph.new(sys.groups)
	    cg.add_global_attribute('node [shape=box]')
	    cg.addclass(aclass.name, $include_subclasses) do | class_adding |
		if $interface_in_diagram
		    if !class_adding.nil? && class_adding.name == aclass.name
			# 'color="light_grey" style="filled"'
			'shape=record label="{' + class_adding.name + '|' + class_adding.methods.collect do | method |
			    method.name.dot_escape.dot_comma_wrap(50) + '\l'
			end.join + '}"'
		    end
		end
	    end
	    cg.write(aclass.name)
	    scale_cmd = ''
	    if cg.width(aclass.name) > 300
		scale_cmd = '[width=1\textwidth]'
	    end
	    puts "\\includegraphics#{scale_cmd}{#{aclass.name}.eps}\\\\"
	end
	
	if !aclass.ancestors.empty? && $state_direct_ancestors
	    puts "\\textbf{Inherits:} #{aclass.ancestors.join(', ').latex_escape}\\\\"
	end

	if aclass.doc
	    puts aclass.doc.formatted_sections
	end
	
	aclass.methods.each do | method |
	    if method.access_type == 'public'
		if method.doc || $paragraphs_for_undocumented
		    puts "\\paragraph{\\textbf{#{method.name.latex_escape}}}"
		end
		if method.doc
		    puts method.doc.join("\n")
		end
	    end
	end
	puts ''
    end
end
output.close

