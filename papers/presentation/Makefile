# Makefile                                                       -*-makefile-*-
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Slide export machinery, after cppnow26/trees: org -> reveal.js html
# via a project-local batch Emacs with org-re-reveal.

EMACS := $(shell command -v emacs 2> /dev/null)

ORGFILES := $(wildcard *.org)

default: slides

.PHONY: slides
slides: d4270-slides.html ## Export the D4270 deck

reveal.js:
	git clone --depth 1 https://github.com/hakimel/reveal.js.git reveal.js

%-slides.html : %.org | reveal.js
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el \
	-f package-initialize \
	--eval "(setq enable-local-variables :all)" \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 're-reveal \"$@\")"
	echo $@ : \\ > $@.deps
	sed -n "s/^.*\[\[file:\(\S*\)::.*$$/\1/p" < $< | sort -u | xargs printf "  %s \\\\\\n" >> $@.deps

-include $(ORGFILES:%.org=%-slides.html.deps)

%.md : %.org
	$(EMACS) --init-directory=.emacs.d/ \
	--batch --load .emacs.d/init.el \
	-f package-initialize \
	--eval "(setq enable-local-variables :all)" \
	--visit $< \
	--eval "(org-transclusion-mode t)" \
	--eval "(org-export-to-file 'gfm \"$@\")"

.PHONY: clean
clean:
	-rm -f *-slides.html *.deps

# Help target
.PHONY: help
help: ## Show this help.
	@awk 'BEGIN {FS = ":.*?## "} /^[.a-zA-Z_%-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST) | sort
