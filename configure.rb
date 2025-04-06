#!/usr/bin/ruby -w

$depdir = '.deps'

$compiler_opts = Array.new
$compiler_opts.push("-Wno-deprecated")

# this option is important, even if we are not optimising, since CUDD
# uses this option, so if we don't then we'll crash when we try and
# use some parts of CUDD (okay, only the CUDD internals, but we need
# to use them in order to write the non-caching ADD traversal stuff).

#  -fprefetch-loop-arrays
$optimise_flags = '-O3 -fstrict-aliasing -ffast-math' +
    ' -fforce-mem -finline-functions -funroll-all-loops -fomit-frame-pointer'

$options_file = 'config.opts'

$cudd_path = ''
$mtl_path = ''
$readline_path = ''
$flex_path = ''
$extra_variables = Hash.new
$debug_enabled = false

# parse command line parameters, and return options to be saved
def process_opts(opts)
    out_opts = Array.new
    
    last_opt = ''
    opts.each do | arg |
	
	if arg != '--old-opts'
	    out_opts.push(arg)
	end
	
	if last_opt == ''
	    case arg
	    when '--help'
		puts '--help                display this message'
		puts '--optimise            enable optimisation'
		puts '--cpu cputype         i386/i686/pentium4'
		puts '--sse                 enable SSE'
		puts '--sse2                enable SSE2'
		puts '--cudd-path p         specify location of CUDD'
		puts '--mtl-path p          specify location of MTL'
		puts '--readline-path p     specify location of libreadline'
		puts '--flex-path p         specify location of flex'
		puts '--old-opts            use same options as last time'
		puts '--profile             compile for profiling'
		puts '--debug               enable debugging info'
		puts '--header-redir-errors redirect h++ errors to corresponding c++'
		puts '--align-double        needed when CUDD compiled with that option'
		exit
	    when '--optimise'
		$compiler_opts.push($optimise_flags)
	    when '--sse'
		$compiler_opts.push('-msse')
		$compiler_opts.push('-mfpmath=sse')
	    when '--sse2'
		$compiler_opts.push('-msse2')
		$compiler_opts.push('-mfpmath=sse')
	    when '--optimise-amd' # depreciated
		$compiler_opts.push($optimise_flags + ' -march=pentium3 -mfpmath=sse')
	    when '--optimise-i686' # depreciated
		$compiler_opts.push($optimise_flags + ' -march=i686 -mcpu=i686')
	    when '--profile'
		$compiler_opts.push('-pg')
	    when '--debug'
		$compiler_opts.push('-g')
		$debug_enabled = true
	    when '--align-double'
		$compiler_opts.push('-malign-double')
	    when '--old-opts'
		old_opt_file = open($options_file)
		old_options = old_opt_file.readlines.collect {|a| a.chomp}
		old_opt_file.close
		out_opts += process_opts(old_options)
	    when '--header-redir-errors'
		$extra_variables['MAKE_HEADER_OPTS'] = '--redirect-errors'
	    else
		last_opt = arg
	    end
	else
	    case last_opt
	    when '--cpu'
		$compiler_opts.push(" -mcpu=#{arg}")
	    when '--cudd-path'
		$cudd_path = arg
	    when '--mtl-path'
		$mtl_path = arg
	    when '--readline-path'
		$readline_path = arg
	    when '--flex-path'
		$flex_path = arg
	    when /^#/
	    else
		$stderr.puts "Unrecognised option #{last_opt}"
		exit
	    end
	    last_opt = ''
	end
    end
    if last_opt != ''
	$stderr.puts "Unrecognised option #{last_opt}"
	exit
    end
	
    return out_opts.delete_if {|a| a =~ /^#/}
end

options = process_opts(ARGV).join("\n")
opt_file = open($options_file, "w")
opt_file.puts(options)
opt_file.close

if $cudd_path == ''
    $stderr.puts "Must specify location of CUDD using --cudd-path"
    exit
end
if $mtl_path == ''
    $stderr.puts "Must specify location of MTL using --mtl-path"
    exit
end
if $flex_path == ''
    $stderr.puts "Must specify location of flex using --flex-path"
    exit
end

makefile_inc = open("Makefile.inc", "w")
if `gcc --version` =~ /gcc (GCC) 3./
    makefile_inc.puts("DEPMODE=gcc3")
else
    makefile_inc.puts("DEPMODE=gcc")
end
extra_cflags = $compiler_opts.join(' ')

# profiling and debugging cannot work with -fomit-frame-pointer
if extra_cflags =~ /-pg/ or $debug_enabled
    extra_cflags.gsub!(/-fomit-frame-pointer/, '')
end

makefile_inc.puts("EXTRA_CFLAGS=#{extra_cflags}")
makefile_inc.puts("CUDD=#{$cudd_path}")
makefile_inc.puts('CUDD_LIB=-L$(CUDD) -L$(CUDD)/lib -lobj -lcudd -lmtr -lst -lutil -lepd -lm')
makefile_inc.puts('CUDD_CFLAGS=-I$(CUDD) -I$(CUDD)/include')
makefile_inc.puts("MTL=#{$mtl_path}")
makefile_inc.puts('MTL_LIB=')
makefile_inc.puts('MTL_CFLAGS=-I$(MTL)')

makefile_inc.puts("FLEX_LIB=#{$flex_path}/lib/libfl.a")

$readline_lib = 'READLINE_LIB=-lcurses -lreadline'
if $readline_path != ''
    makefile_inc.puts("READLINE=#{$readline_path}")
    makefile_inc.puts('READLINE_CFLAGS=-I$(READLINE)')
    makefile_inc.puts($readline_lib + ' -L$(READLINE) ')
else
    makefile_inc.puts($readline_lib)
end
$extra_variables.each do | var, val |
    makefile_inc.puts("#{var}=#{val}")
end
makefile_inc.close

def read_makefile_list(var)
    makefile = open("Makefile")
    values = Array.new
    makefile.each_line do | line |
	if line =~ /^#{var}=([^\\]+) *\\?$/
	    values += $1.split(/ +/)
	    break
	end
    end
    makefile.each_line do | line |
	line =~ /^ *([^\\]*) *(\\)?$/
	values += $1.split
	break if $2 != "\\"
    end
    makefile.close
    return values
end

files =
    read_makefile_list("CLASSES") +
    read_makefile_list("NONAUTO") +
    read_makefile_list("GENERATED")

if !File.directory?($depdir)
    Dir.mkdir($depdir)
end
files.each do | file |
    depfile_name = $depdir + "/" + file + ".Po"
    if !File.exist?(depfile_name)
	depfile = open(depfile_name, "w")
	depfile.puts "# dummy"
	depfile.close
    end
end
