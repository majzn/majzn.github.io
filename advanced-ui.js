'use strict';
// Reorganize existing, already-bound controls rather than creating duplicate inputs.
(function(){
 const $=id=>document.getElementById(id),root=$('advancedControls');if(!root)return;
 const names=[['compose','Composition'],['expression','Development'],['harmony','Harmony & evolution'],['sound','Sound & color']];
 const form=$('evolutionForm'),rangeWrap=id=>$(id)?.closest('.slider-control'),control=id=>$(id)?.closest('.musical-rule,.voices-control,.opening-control,.staff-palette-controls,.soundfont-import,.entropy-setting,.beat-unit,.slider-control,.automation-row')||$(id)?.closest('label')||$(id);
 const make=(tag,cls,text)=>{const n=document.createElement(tag);if(cls)n.className=cls;if(text)n.textContent=text;return n;};
 const tabs=make('div','settings-tabs');tabs.setAttribute('role','tablist');tabs.setAttribute('aria-label','Advanced settings categories');
 const shell=make('div','settings-shell'),panes=new Map();
 for(const [id,name]of names){const button=make('button','settings-tab',name);button.id='settings-tab-'+id;button.type='button';button.setAttribute('role','tab');button.setAttribute('aria-controls','settings-pane-'+id);button.setAttribute('aria-selected',id==='compose'?'true':'false');button.tabIndex=id==='compose'?0:-1;
 const pane=make('section','settings-pane');pane.id='settings-pane-'+id;pane.setAttribute('role','tabpanel');pane.setAttribute('aria-labelledby',button.id);pane.hidden=id!=='compose';
 button.addEventListener('click',()=>activate(id));tabs.append(button);shell.append(pane);panes.set(id,pane);}
 root.append(tabs,shell);
 function activate(id){for(const [key,pane]of panes){const on=key===id;pane.hidden=!on;const tab=$('settings-tab-'+key);tab.setAttribute('aria-selected',String(on));tab.tabIndex=on?0:-1;}root.scrollTop=0;}
 tabs.addEventListener('keydown',e=>{if(!['ArrowRight','ArrowLeft','Home','End'].includes(e.key))return;const options=[...tabs.children],index=options.indexOf(document.activeElement);if(index<0)return;e.preventDefault();const next=e.key==='Home'?0:e.key==='End'?options.length-1:(index+(e.key==='ArrowRight'?1:-1)+options.length)%options.length;options[next].click();options[next].focus();});
 const intro=(pane,title,subtitle)=>{const header=make('div','settings-intro');header.append(make('h2','',title),make('p','',subtitle));pane.append(header);};
 const group=(pane,title,subtitle,cls='')=>{const section=make('section','settings-card '+cls);const heading=make('div','settings-card-heading');heading.append(make('h3','',title));if(subtitle)heading.append(make('p','',subtitle));const grid=make('div','settings-fields');section.append(heading,grid);pane.append(section);return grid;};
 const append=(target,...ids)=>{for(const id of ids){const item=control(id);if(item&&!target.contains(item))target.append(item);}};
 const composition=panes.get('compose');intro(composition,'The musical essentials','Start here. These controls shape the sound and structure of the fugue.');
 append(group(composition,'Pace & texture','Keep the writing clear before adding ornamentation.'),'tempo','density','voices');
 append(group(composition,'Musical character','Counterpoint and form control the discipline of the entire cycle.'),'counterpointStyle','formMode','mood');
 const moodHelp=make('p','settings-help','Mood only biases the automatic scale choice; a selected scale takes precedence.');composition.append(moodHelp);
 const refreshMood=()=>{$('mood').disabled=$('scale').value!=='auto';moodHelp.hidden=$('scale').value==='auto';};$('scale').addEventListener('change',refreshMood);refreshMood();
 const describe=make('p','settings-help','For a traditional result, choose Strict counterpoint and Classical form. The base note rhythms follow the restored original theme vocabulary.');composition.append(describe);
 const expression=panes.get('expression');intro(expression,'Thematic development','Choose how subjects interact and how the exposition begins.');
 
 append(group(expression,'Thematic development','Use a single subject for a traditional fugue, or add counterthemes for dialogue.'),'themeCount','callResponse');
 const themeHelp=make('p','settings-help','Theme interplay and call-and-response require at least two independent themes.');expression.append(themeHelp);
 const refreshThemes=()=>{const enabled=+$('themeCount').value>1;for(const id of ['callResponse','thematicInterplay'])$(id).disabled=!enabled;themeHelp.hidden=enabled;};$('themeCount').addEventListener('change',refreshThemes);refreshThemes();
 const extra=make('details','settings-expert');extra.append(make('summary','','More melodic and opening options'));
 const extraGrid=make('div','settings-fields');extra.append(extraGrid);expression.append(extra);
 append(extraGrid,'adventure','thematicInterplay','development','openingMode','openingTexture','openingVoices','liveEntropy');
 const harmony=panes.get('harmony');intro(harmony,'Harmony & evolution','Choose a key and meter, then optionally automate changes through the cycle.');
 // All evolution values and the submit button must remain in their real form for browser validation.
 const formHost=make('div','settings-form-host');harmony.append(formHost);formHost.append(form);form.className='settings-evolution-form';
 const music=group(form,'Key & meter','These are fixed until automation requests a change.');append(music,'meterNumerator','tonic','scale','microtiming','variation');
 const beat=control('beatUnit')||document.querySelector('.beat-unit');if(beat)music.insertBefore(beat,music.children[1]||null);
 const modes=group(form,'Change over time','Fixed keeps the selected value. Other modes evolve it by cycle, section or bar.','settings-automation-card');modes.append($('automationRows'));
 const bounds=make('details','settings-expert');bounds.append(make('summary','','Meter variation range'));const boundFields=make('div','settings-fields');bounds.append(boundFields);form.append(bounds);append(boundFields,'meterLow','meterHigh');
 form.append($('meter'));const apply=$('evolutionForm').querySelector('.toolbar-apply');if(apply){apply.classList.add('settings-apply');form.append(apply);}const evolutionHelp=make('p','settings-help','Use Apply evolution after changing the harmony and automation fields.');form.append(evolutionHelp);
 const sound=panes.get('sound');intro(sound,'Sound & appearance','Set your ensemble and mixing balance. Individual voices are edited in the instrument workspace.');
 append(group(sound,'Master mix','Applies to browser playback and exported audio.'),'volume','reverb');
 const ensembleCard=group(sound,'Ensemble','Choose a registered instrument combination. All voices can still be individually edited in the soundstage.');append(ensembleCard,'palette','sampleBank');
 const quick=make('button','settings-choir-action','Surprise me');quick.type='button';quick.id='randomEnsemble';quick.title='Choose a different ensemble without changing the composition seed';
 const preview=make('p','settings-help');preview.id='ensembleLineup';preview.setAttribute('aria-live','polite');ensembleCard.append(quick,preview);
 const refreshEnsemble=()=>{const count=+$('voices').value||4,patches=ContinuumEnsembles.instruments($('palette').value,count),labels=patches.map(id=>INSTRUMENTS.find(i=>i[0]===id)?.[1]||id);preview.textContent='Parts (high → low): '+labels.join(' · ')+(count!==((window.continuum?.cycle?.roles.length)||count)?' · takes effect next cycle':'');};
 quick.onclick=()=>{const all=ContinuumEnsembles.presets.filter(p=>p.id!==$('palette').value);let index=0;try{index=crypto.getRandomValues(new Uint32Array(1))[0]%all.length;}catch{index=Math.floor(Math.random()*all.length);}if(!all.length)return;$('palette').value=all[index].id;$('palette').dispatchEvent(new Event('change',{bubbles:true}));refreshEnsemble();};
 $('palette').addEventListener('change',refreshEnsemble);$('voices').addEventListener('change',refreshEnsemble);window.refreshEnsembleLineup=refreshEnsemble;refreshEnsemble();
 const soundfont=group(sound,'Custom SoundFonts','Import optional sampled instruments for individual voices.');append(soundfont,'soundfontFile');
 const choirButton=$('assignChoirFont');if(choirButton){soundfont.append(choirButton);choirButton.classList.add('settings-choir-action');}
 const sourceNote=document.querySelector('.choir-source-note');if(sourceNote){const sources=make('details','settings-expert settings-sources');sources.append(make('summary','','Free sample-library references'));sources.append(sourceNote);soundfont.append(sources);}
 const midi=group(sound,'MIDI output','Optional hardware synthesis.');append(midi,'connectMidi','midiOutput');if($('soundStatus')){const status=$('soundStatus');status.classList.add('settings-help');midi.append(status);} 
 append(group(sound,'Staff colors','Optional color coding; notation remains readable with the default monochrome palette.'),'staffColorToggle');
 // Keep the complete color controls together, including their existing event handlers.
 const palette=$('staffPalette'),swatches=$('staffColorSwatches'),button=$('randomizePalette');if(palette&&swatches&&button){const widget=$('staffColorToggle').closest('.staff-palette-controls');widget.classList.add('settings-color-widget');}
 // The voice mix is edited through circles, not a duplicate legacy set of controls.
 const voiceMix=$('voiceMix');if(voiceMix){voiceMix.hidden=true;root.append(voiceMix);}form.querySelector('.toolbar-harmony')?.remove();
 root.replaceChildren(tabs,shell,voiceMix||document.createComment('no duplicate mixer'));
 root.classList.add('settings-refactored');root.setAttribute('aria-label','Advanced settings');root.setAttribute('role','region');
 const advanced=$('advancedToggle');if(advanced){advanced.setAttribute('aria-controls','advancedControls');advanced.setAttribute('aria-haspopup','true');}
 document.addEventListener('keydown',e=>{if(e.key==='Escape'&&document.body.classList.contains('advanced-open')&&!document.querySelector('dialog[open]')){document.body.classList.remove('advanced-open');advanced?.setAttribute('aria-expanded','false');if(advanced?.lastElementChild)advanced.lastElementChild.textContent='⌄';advanced?.focus();}});
 document.addEventListener('pointerdown',e=>{if(!document.body.classList.contains('advanced-open')||root.contains(e.target)||$('topToolbar').querySelector('.toolbar-main')?.contains(e.target)||e.target.closest('#instrumentInspector'))return;document.body.classList.remove('advanced-open');advanced?.setAttribute('aria-expanded','false');if(advanced?.lastElementChild)advanced.lastElementChild.textContent='⌄';},{passive:true});
 // Hide dependent fields that do not influence the current musical mode.
 const refresh=()=>{const meterMode=$('meterMode')?.value;bounds.hidden=meterMode==='fixed';};
 for(const id of ['meterMode'])$(id)?.addEventListener('change',refresh);refresh();
 // All visible controls remain bound to their original application state.
})();
