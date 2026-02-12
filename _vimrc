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
"  8. MATHEMATICAL GRADIENT ENGINE (RGB INTERPOLATION)
" ==============================================================================

" --- 1. CONFIGURATION ---
let g:gradient_resolution = 100 

" --- 2. MATH HELPERS ---

" Hex to RGB
function! s:HexToRGB(hex)
  let l:hex = substitute(a:hex, '#', '', '')
  return [str2nr(l:hex[0:1], 16), str2nr(l:hex[2:3], 16), str2nr(l:hex[4:5], 16)]
endfunction

" RGB to Hex
function! s:RGBToHex(rgb)
  let l:r = float2nr(a:rgb[0])
  let l:g = float2nr(a:rgb[1])
  let l:b = float2nr(a:rgb[2])
  return printf("#%02x%02x%02x", l:r, l:g, l:b)
endfunction

" Linear Interpolation
function! s:Lerp(start, end, step, total_steps)
  let l:factor = (1.0 * a:step) / a:total_steps
  return a:start + (a:end - a:start) * l:factor
endfunction

" Generate Palette
function! GenerateDynamicPalette(keyframes, total_steps)
  let l:palette = []
  let l:segments = len(a:keyframes) - 1
  if l:segments < 1 | return a:keyframes | endif
  
  let l:steps_per_segment = a:total_steps / l:segments

  for i in range(l:segments)
    let l:start_rgb = s:HexToRGB(a:keyframes[i])
    let l:end_rgb   = s:HexToRGB(a:keyframes[i+1])
    
    for j in range(l:steps_per_segment)
      let l:r = s:Lerp(l:start_rgb[0], l:end_rgb[0], j, l:steps_per_segment)
      let l:g = s:Lerp(l:start_rgb[1], l:end_rgb[1], j, l:steps_per_segment)
      let l:b = s:Lerp(l:start_rgb[2], l:end_rgb[2], j, l:steps_per_segment)
      call add(l:palette, s:RGBToHex([l:r, l:g, l:b]))
    endfor
  endfor
  
  " Ensure we explicitly add the final color to close the loop
  call add(l:palette, a:keyframes[-1])
  return l:palette
endfunction

" --- 3. THEME DEFINITIONS (ANCHOR POINTS) ---
let g:gradient_anchors = {}
let g:gradient_anchors['Cyberpunk'] = ['#00f0ff', '#5000ff', '#ff0070']
let g:gradient_anchors['Sunset']    = ['#2d004d', '#ff00ff', '#ffff00']
let g:gradient_anchors['Matrix']    = ['#002200', '#00ff00', '#ffffff']
let g:gradient_anchors['Magma']     = ['#0000aa', '#ff0000', '#ffff00']
let g:gradient_anchors['Frost']     = ['#ffffff', '#00d4ff', '#003032']
let g:gradient_anchors['Viridis']   = ['#440154', '#21908c', '#fde725']
let g:gradient_anchors['Plasma']    = ['#0d0887', '#cc4678', '#f0f921']
let g:gradient_anchors['Inferno']   = ['#000004', '#bb3754', '#fcffa4']
let g:gradient_anchors['Forest']    = ['#001100', '#228833', '#aaffbb']
let g:gradient_anchors['DeepSea']   = ['#000022', '#0044aa', '#00ffff']
let g:gradient_anchors['Fire']      = ['#220000', '#ff0000', '#ffff00']
let g:gradient_anchors['Grey']      = ['#222222', '#888888', '#ffffff']
let g:gradient_anchors['Kelvin'] = ['#ff8912', '#ffc58f', '#ffffff', '#c9dfff', '#809eff']
let g:gradient_anchors['Blackbody'] = ['#000000', '#550000', '#aa0000', '#ff0000', '#ffaa00', '#ffff00', '#ffffff']
let g:gradient_anchors['Blueprint'] = ['#001133', '#002266', '#004499', '#3377cc', '#88bbff', '#ffffff']
let g:gradient_anchors['Spectrum'] = ['#ff0000', '#ffa500', '#ffff00', '#00ff00', '#0000ff', '#4b0082', '#ee82ee']
let g:gradient_anchors['Synthwave'] = ['#240046', '#7b2cbf', '#ff00ff', '#00ffff', '#ffffff']
let g:gradient_anchors['Jungle'] = ['#001a00', '#003300', '#006600', '#339933', '#66cc66', '#99ff99', '#ccffcc']
let g:gradient_anchors['Dusk'] = ['#ff8800', '#ff4444', '#cc0066', '#880088', '#4400aa', '#000044']
let g:gradient_anchors['Dawn'] = ['#000033', '#2d004d', '#800080', '#ff4500', '#ffcc00', '#87ceeb']
let g:gradient_anchors['BladeRunner'] = ['#003333', '#006666', '#00ffff', '#ffffff', '#ffaa00', '#ff5500']
let g:gradient_anchors['Candy'] = ['#ff99cc', '#cc99ff', '#99ccff', '#99ffcc', '#ffff99']
let g:gradient_anchors['Toxic'] = ['#000000', '#1a3300', '#4d9900', '#ccff00', '#ffffcc', '#ffffff']
let g:gradient_anchors['Patriot'] = ['#b22234', '#ffffff', '#3c3b6e']
let g:gradient_anchors['Golden'] = ['#2e1500', '#5c2a00', '#b8860b', '#ffd700', '#ffffcc']
let g:gradient_anchors['Borealis'] = ['#002211', '#005533', '#00aa77', '#00ddaa', '#4466aa', '#8844aa']
let g:gradient_anchors['Miami'] = ['#ff00cc', '#ff66cc', '#ccaaff', '#66ccff', '#00ffff']
let g:gradient_anchors['CottonCandy'] = ['#ffaddb', '#e0b0ff', '#b0c4ff', '#9bf6ff']
let g:gradient_anchors['Rust'] = ['#2e1000', '#5c2200', '#8a3700', '#cc5500', '#ff9966', '#ffccaa']
let g:gradient_anchors['Dracula'] = ['#282a36', '#6272a4', '#8be9fd', '#50fa7b', '#ffb86c', '#ff79c6', '#bd93f9']

" Set Default
let g:current_gradient_name = 'Kelvin'
let g:generated_cache = []

" --- 4. ENGINE CORE ---

function! ApplyGradientHighlighting()
  " 1. Generate the palette based on current anchor points
  if !has_key(g:gradient_anchors, g:current_gradient_name)
    let g:current_gradient_name = 'Cyberpunk'
  endif
  let l:anchors = g:gradient_anchors[g:current_gradient_name]
  let g:generated_cache = GenerateDynamicPalette(l:anchors, g:gradient_resolution)

  " 2. Define Highlights
  for i in range(len(g:generated_cache))
    let l:color = g:generated_cache[i]
    exe 'hi GradientLvl' . i . ' guifg=' . l:color . ' guibg=NONE gui=bold'
    exe 'sign define GSign' . i . ' numhl=GradientLvl' . i
  endfor
endfunction

function! PlaceGradientSigns()
  if line('$') > 5000 | return | endif
  silent! sign unplace * group=GradientGroup

  let l:total_lines = line('$')
  if l:total_lines == 0 | let l:total_lines = 1 | endif
  let l:palette_size = len(g:generated_cache)
  if l:palette_size == 0 
    call ApplyGradientHighlighting()
    let l:palette_size = len(g:generated_cache)
  endif

  for l:lnum in range(1, l:total_lines)
    let l:idx = ((l:lnum - 1) * l:palette_size) / l:total_lines
    if l:idx >= l:palette_size | let l:idx = l:palette_size - 1 | endif
    exe 'sign place ' . l:lnum . ' line=' . l:lnum . ' name=GSign' . l:idx . ' group=GradientGroup file=' . expand('%:p')
  endfor
endfunction

" --- 5. COMMANDS & TRIGGERS ---

command! -nargs=1 Gradient let g:current_gradient_name = <args> | call ApplyGradientHighlighting() | call PlaceGradientSigns() | echo "Gradient: " . g:current_gradient_name

function! CycleGradient()
  let l:keys = keys(g:gradient_anchors)
  let l:idx = index(l:keys, g:current_gradient_name)
  let l:idx = (l:idx + 1) % len(l:keys)
  let g:current_gradient_name = l:keys[l:idx]
  call ApplyGradientHighlighting()
  call PlaceGradientSigns()
  echo "Gradient Theme: " . g:current_gradient_name
endfunction

augroup GradientEngine
  autocmd!
  autocmd VimEnter * call ApplyGradientHighlighting()
  autocmd BufReadPost,BufWritePost,TextChanged,TextChangedI * call PlaceGradientSigns()
augroup END

call ApplyGradientHighlighting()

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
