" ==============================================================================
"  1. PLUGINS
" ==============================================================================
call plug#begin('~/vimfiles/plugged')
  Plug 'wfxr/minimap.vim'
  Plug 'preservim/nerdtree'
  Plug 'junegunn/fzf'
  Plug 'junegunn/fzf.vim'
call plug#end()

" ==============================================================================
"  2. CORE CONFIGURATION
" ==============================================================================
set nocompatible
set encoding=utf-8
set fileencoding=utf-8
set clipboard=unnamed
set noerrorbells
set visualbell
set t_vb=
set t_Co=256
set termguicolors
syntax enable

" Performance
set lazyredraw
set ttyfast
set autoread
set updatetime=200

" Indentation
set tabstop=2
set softtabstop=2
set shiftwidth=2
set expandtab
set autoindent
set smartindent

" Interface
set number
set cursorline
set scrolloff=8
set laststatus=2
set wildmenu
set wildmode=longest:list,full
set hlsearch
set incsearch
set ignorecase
set smartcase
set splitbelow
set splitright
set signcolumn=yes

" ==============================================================================
"  3. WINDOWS INTEGRATION
" ==============================================================================
source $VIMRUNTIME/mswin.vim
behave mswin

if !isdirectory($HOME . '/vimfiles/swap')
  call mkdir($HOME . '/vimfiles/swap', 'p')
endif
set directory=$HOME/vimfiles/swap//
set nobackup
set nowritebackup

" ==============================================================================
"  4. MAPPINGS
" ==============================================================================
let mapleader = ","

" System
noremap <C-a> ggVG
inoremap <C-a> <Esc>ggVG
noremap <C-Tab> :bnext<CR>
noremap <C-S-Tab> :bprev<CR>

" Window Nav
noremap <C-h> <C-w>h
noremap <C-j> <C-w>j
noremap <C-k> <C-w>k
noremap <C-l> <C-w>l

" Leader
nmap <leader>w :w<CR>
nmap <leader>q :q<CR>
nmap <leader>h :nohlsearch<CR>:diffupdate<CR>:syntax sync fromstart<CR><C-l>
nmap <leader>z :call ToggleGhost()<CR>
nmap <leader>g :call CycleGradient()<CR>

" Plugins
map <C-n> :NERDTreeToggle<CR>
map <C-p> :Files<CR>
map <C-t> :call OpenTerminal()<CR>

" ==============================================================================
"  5. THEME ENGINE: "TRANSPARENT HARMONIC"
" ==============================================================================
try
  colorscheme quiet
catch
  colorscheme desert
endtry

" Force transparency and consistent colors
augroup HarmonicTheme
  autocmd!
  autocmd ColorScheme,VimEnter * call ApplyHarmonicTheme()
augroup END

function! ApplyHarmonicTheme()
  " 1. The Canvas (TRANSPARENT)
  " We set ctermbg=NONE and guibg=NONE to let the terminal background show through
  hi Normal       guifg=#abb2bf guibg=NONE ctermbg=NONE gui=NONE
  hi NonText      guifg=#3b4048 guibg=NONE ctermbg=NONE gui=NONE
  hi LineNr       guifg=#4b5263 guibg=NONE ctermbg=NONE gui=NONE
  hi SignColumn   guifg=NONE    guibg=NONE ctermbg=NONE gui=NONE
  hi EndOfBuffer  guifg=#3b4048 guibg=NONE ctermbg=NONE gui=NONE
  hi VertSplit    guifg=#3b4048 guibg=NONE ctermbg=NONE gui=NONE
  
  " CursorLine needs a slight tint to be visible, but we keep it subtle
  " If you want full transparency for cursor line too, change guibg to NONE and gui=underline
  hi CursorLine   guifg=NONE    guibg=#282c34 ctermbg=236 gui=NONE

  " 2. Syntax Groups
  hi Comment      guifg=#5c6370 gui=italic
  hi String       guifg=#98c379 gui=NONE
  hi Number       guifg=#d19a66 gui=NONE
  hi Boolean      guifg=#d19a66 gui=bold
  hi Float        guifg=#d19a66 gui=NONE
  hi Keyword      guifg=#c678dd gui=bold
  hi Conditional  guifg=#c678dd gui=bold
  hi Repeat       guifg=#c678dd gui=bold
  hi Operator     guifg=#56b6c2 gui=NONE
  hi Function     guifg=#61afef gui=bold
  hi Type         guifg=#e5c07b gui=NONE
  hi Structure    guifg=#e5c07b gui=NONE
  hi Identifier   guifg=#e06c75 gui=NONE
  hi Constant     guifg=#d19a66 gui=NONE
  
  " 3. UI Accents
  hi Pmenu        guifg=#abb2bf guibg=#282c34
  hi PmenuSel     guifg=#1c1c1c guibg=#61afef gui=bold
  hi Search       guifg=#1c1c1c guibg=#e5c07b gui=bold
  hi IncSearch    guifg=#1c1c1c guibg=#98c379 gui=bold
  hi MatchParen   guifg=#61afef guibg=NONE    gui=bold,underline
  
  " 4. Apply Semantic Brackets
  call SemanticBrackets()
endfunction

let g:minimap_width = 10
let g:minimap_auto_start = 1
let g:minimap_auto_start_win_enter = 1

" ==============================================================================
"  6. LOGIC-BASED DELIMITERS
" ==============================================================================
function! SemanticBrackets()
  " 1. Curly Braces {} -> PURPLE (Structure/Scope)
  hi ScopeBraces guifg=#c678dd gui=bold
  syntax match ScopeBraces /[{}]/
  
  " 2. Round Parentheses () -> BLUE (Function Calls/Parameters)
  hi ParamParens guifg=#61afef gui=bold
  syntax match ParamParens /[()]/

  " 3. Square Brackets [] -> GOLD (Array/Data Access)
  hi DataBrackets guifg=#e5c07b gui=bold
  syntax match DataBrackets /[\[\]]/
  
  " 4. Semicolons ; -> CYAN (Terminator)
  hi Terminator guifg=#56b6c2 gui=NONE
  syntax match Terminator /;/
endfunction

" ==============================================================================
"  7. MODERN STATUS BAR
" ==============================================================================
function! GetFileContext()
  let l:mode = mode()
  if l:mode =~# '\v(v|V|)'
    let [line_start, col_start] = getpos("v")[1:2]
    let [line_end, col_end] = getpos(".")[1:2]
    let lines = abs(line_end - line_start) + 1
    return ' SEL: '.lines.'L '
  else
    let l:size = getfsize(expand(@%))
    if l:size <= 0 | return '' | endif
    if l:size < 1024
      return ' ' . l:size . 'B '
    elseif l:size < 1048576
      return ' ' . printf('%.1f', l:size/1024.0) . 'KB '
    else
      return ' ' . printf('%.1f', l:size/1048576.0) . 'MB '
    endif
  endif
endfunction

function! GetCurrentLineLen()
  return strwidth(getline('.'))
endfunction

function! ModeLabel()
  let l:m = mode()
  if l:m ==# 'n' | return ' NORMAL ' | endif
  if l:m ==# 'i' | return ' INSERT ' | endif
  if l:m ==# 'R' | return ' REPLACE ' | endif
  if l:m =~# '\v(v|V|)' | return ' VISUAL ' | endif
  return ' '.toupper(l:m).' '
endfunction

function! UpdateStatusColors()
  let l:m = mode()
  
  " We keep the Status Bar Opaque (Dark) so it reads clearly 
  " against a potentially noisy terminal background image.
  let l:bg_bar   = '#181a1f'
  let l:fg_text  = '#abb2bf'
  let l:fg_mute  = '#5c6370'
  
  if l:m ==# 'i'      
    let l:accent = '#c678dd'
    let l:contrast = '#ffffff'
  elseif l:m =~# '\v(v|V|)' 
    let l:accent = '#e5c07b'
    let l:contrast = '#000000'
  elseif l:m ==# 'R'  
    let l:accent = '#e06c75'
    let l:contrast = '#ffffff'
  else                
    let l:accent = '#61afef'
    let l:contrast = '#000000'
  endif

  exe 'hi User1 guifg='.l:contrast.' guibg='.l:accent.' gui=bold'
  exe 'hi User2 guifg='.l:fg_text.' guibg='.l:bg_bar.' gui=NONE'
  exe 'hi User3 guifg='.l:fg_mute.' guibg='.l:bg_bar.' gui=NONE'
  exe 'hi User4 guifg='.l:accent.' guibg='.l:bg_bar.' gui=bold'
  
  exe 'hi StatusLine guifg='.l:fg_text.' guibg='.l:bg_bar.' gui=NONE'
  exe 'hi StatusLineNC guifg='.l:fg_mute.' guibg='.l:bg_bar.' gui=NONE'
  
  exe 'hi CursorLineNr guifg='.l:accent.' guibg=NONE gui=bold'
  exe 'hi ModeMsg      guifg='.l:accent.' guibg=NONE gui=bold'
endfunction

augroup StatusColors
  autocmd!
  autocmd ColorScheme,ModeChanged,VimEnter * call UpdateStatusColors()
augroup END

set statusline=
set statusline+=%1*\ %{ModeLabel()}\             
set statusline+=%2*\ %f\                         
set statusline+=%4*%m%r                          
set statusline+=%3*\ %{v:hlsearch?'\ \ [S:'.@/.\']':''} 
set statusline+=%=                               
set statusline+=%3*\ %y\ \|\ %{&enc}\            
set statusline+=%3*\ \|\ Len:                    
set statusline+=%4*%{GetCurrentLineLen()}\       
set statusline+=%3*\|\ Ln\                       
set statusline+=%4*%l/%L                         
set statusline+=%3*\ :\                          
set statusline+=%4*%c\                           
set statusline+=%2*%{GetFileContext()}\          

" ==============================================================================
"  8. GRADIENT ENGINE
" ==============================================================================
let g:gradient_themes = {}
let g:gradient_themes['Cyberpunk'] = ['#00f0ff', '#00e0ff', '#00c0ff', '#00a0ff', '#0080ff', '#5000ff', '#8000ff', '#b000ff', '#e000ff', '#ff00d0', '#ff00a0', '#ff0070']
let g:gradient_themes['Matrix']    = ['#003300', '#004400', '#005500', '#006600', '#007700', '#008800', '#009900', '#00aa00', '#00bb00', '#00cc00', '#00dd00', '#00ee00', '#00ff00', '#55ff55', '#aaffaa', '#ffffff']
let g:gradient_themes['Magma']     = ['#0000ff', '#4400ff', '#8800ff', '#cc00ff', '#ff00cc', '#ff0088', '#ff0044', '#ff0000', '#ff4400', '#ff8800', '#ffcc00', '#ffff00']
let g:gradient_themes['Frost']     = ['#ffffff', '#e0f7fa', '#b2ebf2', '#80deea', '#4dd0e1', '#26c6da', '#00bcd4', '#00acc1', '#0097a7', '#00838f', '#006064', '#003032']
let g:gradient_themes['Sunset']    = ['#2d004d', '#4b0082', '#800080', '#ba55d3', '#ff00ff', '#ff1493', '#ff4500', '#ff8c00', '#ffa500', '#ffd700', '#ffff00', '#ffffbd']

let g:current_gradient = 'Cyberpunk'
let g:gradient_keys = keys(g:gradient_themes)

function! ApplyGradientHighlighting()
  let l:palette = g:gradient_themes[g:current_gradient]
  for i in range(len(l:palette))
    let l:color = l:palette[i]
    exe 'hi GradientLvl' . i . ' guifg=' . l:color . ' guibg=NONE gui=bold'
    exe 'sign define GSign' . i . ' numhl=GradientLvl' . i
  endfor
endfunction

" INITIALIZE
call ApplyGradientHighlighting()

function! PlaceGradientSigns()
  if line('$') > 3000 | return | endif
  silent! sign unplace * group=GradientGroup
  let l:total = line('$')
  if l:total == 0 | let l:total = 1 | endif
  let l:palette = g:gradient_themes[g:current_gradient]
  let l:spec_len = len(l:palette)

  for l:lnum in range(1, l:total)
    let l:idx = ((l:lnum - 1) * l:spec_len) / l:total
    if l:idx >= l:spec_len | let l:idx = l:spec_len - 1 | endif
    exe 'sign place ' . l:lnum . ' line=' . l:lnum . ' name=GSign' . l:idx . ' group=GradientGroup file=' . expand('%:p')
  endfor
endfunction

command! -nargs=1 Gradient let g:current_gradient = <args> | call ApplyGradientHighlighting() | call PlaceGradientSigns() | echo "Gradient: " . g:current_gradient

function! CycleGradient()
  let l:idx = index(g:gradient_keys, g:current_gradient)
  let l:idx = (l:idx + 1) % len(g:gradient_keys)
  let g:current_gradient = g:gradient_keys[l:idx]
  call ApplyGradientHighlighting()
  call PlaceGradientSigns()
  echo "Gradient Theme: " . g:current_gradient
endfunction

augroup GradientEngine
  autocmd!
  autocmd VimEnter * call ApplyGradientHighlighting()
  autocmd BufReadPost,BufWritePost,TextChanged,TextChangedI * call PlaceGradientSigns()
augroup END

" ==============================================================================
"  9. ADVANCED UTILITIES
" ==============================================================================
augroup HighlightYank
  autocmd!
  autocmd TextYankPost * silent! call matchadd('IncSearch', '\%'.v:event.visual.start[1].'l\%'.v:event.visual.start[2].'v\%>'.(v:event.visual.end[1]).'l\|\%'.v:event.visual.start[1].'l\%'.v:event.visual.start[2].'v.\+\%<'.(v:event.visual.end[1]+1).'l\|\%>'.v:event.visual.start[1].'l\%<'.v:event.visual.end[1].'l.*\|\%>'.v:event.visual.start[1].'l\%'.v:event.visual.end[1].'l\%'.v:event.visual.end[2].'v\|\%'.v:event.visual.end[1].'l\%'.v:event.visual.end[2].'v', -1, -1, {'window': win_getid()}) | call timer_start(300, {-> clearmatches()})
augroup END

if has("persistent_undo")
  let target_path = expand('~/vimfiles/undo')
  if !isdirectory(target_path) | call mkdir(target_path, "p") | endif
  let &undodir = target_path
  set undofile
endif

autocmd BufReadPost * if line("'\"") > 0 && line("'\"") <= line("$") | exe "normal! g`\"" | endif

let s:hidden_all = 0
function! ToggleGhost()
    if s:hidden_all  == 0
        let s:hidden_all = 1
        set nonumber | set nocursorline | set laststatus=0 | set noshowmode | set noruler | set signcolumn=no
        if exists(':MinimapClose') | MinimapClose | endif
    else
        let s:hidden_all = 0
        set number | set cursorline | set laststatus=2 | set ruler | set signcolumn=yes
        if exists(':Minimap') | Minimap | endif
        call PlaceGradientSigns()
    endif
endfunction

function! OpenTerminal()
  if bufexists(bufnr('!cmd'))
      execute "bd! !cmd"
  else
      belowright terminal ++rows=12 ++close cmd
  endif
endfunction

" ==============================================================================
"  10. C/C++ HYPER-SUITE
" ==============================================================================

autocmd FileType c,cpp setlocal cindent cinoptions=:0,l1,t0,g0,(0

autocmd FileType c,cpp nnoremap <buffer> <F5> :call CompileRunC()<CR>
function! CompileRunC()
  exec "w"
  if &filetype == 'c' | let l:cc = "gcc" | else | let l:cc = "g++" | endif
  let l:src = shellescape(expand("%"))
  let l:exe = shellescape(expand("%:r") . (has("win32") ? ".exe" : ""))

  if has("win32") || has("win64")
    exec '!start cmd /c "' . l:cc . ' ' . l:src . ' -o ' . l:exe . ' && ' . l:exe . ' & pause"'
  else
    exec '! ' . l:cc . ' ' . l:src . ' -o ' . l:exe . ' && ./' . l:exe
  endif
endfunction

autocmd FileType c,cpp,objc nnoremap <buffer> <F4> :call SwitchSourceHeader()<CR>
function! SwitchSourceHeader()
  let l:base = expand("%:r")
  let l:ext = expand("%:e")
  if l:ext == "c" || l:ext == "cpp" || l:ext == "m" || l:ext == "cc"
    if filereadable(l:base . ".h") | execute "e " . l:base . ".h"
    elseif filereadable(l:base . ".hpp") | execute "e " . l:base . ".hpp"
    else | echo "No header found!" | endif
  elseif l:ext == "h" || l:ext == "hpp"
    if filereadable(l:base . ".c") | execute "e " . l:base . ".c"
    elseif filereadable(l:base . ".cpp") | execute "e " . l:base . ".cpp"
    else | echo "No source found!" | endif
  endif
endfunction

autocmd FileType c inoremap <buffer> <expr> <Tab> getline('.') =~ '^\s*main$' ? '<Esc>cc#include <stdio.h><CR><CR>int main() {<CR>  <C-r>"<CR>  return 0;<CR>}<Esc>kko' : '<Tab>'
