" Vim syntax file
" Language:		Circa

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" Literal values
syn match circaColor    "#\([0-9]\|[a-f]\)*"
syn match circaInteger	"\<0[xX]\x\+\%(_\x\+\)*\>"								display
syn match circaInteger	"\<\%(0[dD]\)\=\%(0\|[1-9]\d*\%(_\d\+\)*\)\>"						display
syn match circaInteger	"\<0[oO]\=\o\+\%(_\o\+\)*\>"								display
syn match circaInteger	"\<0[bB][01]\+\%(_[01]\+\)*\>"								display
syn match circaFloat	"\<\%(0\|[1-9]\d*\%(_\d\+\)*\)\.\d\+\%(_\d\+\)*\>"					display
syn match circaFloat	"\<\%(0\|[1-9]\d*\%(_\d\+\)*\)\%(\.\d\+\%(_\d\+\)*\)\=\%([eE][-+]\=\d\+\%(_\d\+\)*\)\>"	display
syn region circaString   start=+"+ end=+"+ skip=+\\"+
syn region circaString   start=+'+ end=+'+ skip=+\\'+
syn region circaString   start=+<<<+ end=+>>>+
syn match circaSymbol   ":\([A-Z0-9a-z_]\)*"
syn region circaMultilineComment start="{-" end="-}" contains=circaMultilineComment

" Keywords
syn keyword circaKeyword def type if elif else for state return in true false namespace discard break continue while not and or require import package section

" Comments
syn region circaLineComment start="--" skip="\\$" end="$"

hi def link circaColor          Number
hi def link circaInteger        Number
hi def link circaFloat          Number
hi def link circaString         String
hi def link circaSymbol         Constant
hi def link circaKeyword        Keyword
hi def link circaLineComment    Comment
hi def link circaMultilineComment    Comment

let b:current_syntax = "circa"

" vim: nowrap sw=2 sts=2 ts=8 ff=unix:
