DOCUMENT=design

all: modules.tex design.ps

%.ps: %.dvi
	dvips $< -o $@

$(DOCUMENT).dvi: $(DOCUMENT).tex modules.tex
	-latex $(DOCUMENT) >/dev/null 2>&1 </dev/null
	-latex $(DOCUMENT) >/dev/null 2>&1 </dev/null
	latex $(DOCUMENT)

modules.tex: generate-design.rb
	ruby generate-design.rb

clean:
	rm -f modules.tex *.ps *.dvi *.log *.aux *.eps *.dot *.toc *.lof *.idx *.lot

