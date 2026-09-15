const rows = DATA.rows;
const folderOf = r => r.key.includes('/') ? r.key.split('/')[0] : '(top)';
let folder = null, shown = 0;
const PAGE = 120;
const $ = id => document.getElementById(id);
function fmt(x){ return x === undefined ? '-' : (x === 0 ? '0' : x < 1e-3 ? x.toExponential(1) : x.toFixed(3)); }
const n = r => (r.gold_entries||0) + (r.test_entries||0);
function sig(r){
  const a = r.gold_entries||0, b = r.test_entries||0;
  if (!a && !b) return 0;
  return (r.ks_dist||0) * Math.sqrt(a && b ? a*b/(a+b) : Math.max(a,b));
}
function filtered(){
  const st = $('status').value, q = $('q').value.toLowerCase(), dmin = +$('dmin').value;
  let out = rows.filter(r => (st === 'all' || r.status === st)
    && (!q || r.key.toLowerCase().includes(q))
    && (!folder || folderOf(r) === folder)
    && (r.ks_dist === undefined || r.ks_dist >= dmin));
  const by = $('sort').value;
  const rank = {missing:0, fail:1, new:2, pass:3};
  const cmp = {
    dist: (a,b) => rank[a.status]-rank[b.status] || (b.ks_dist??0)-(a.ks_dist??0) || n(b)-n(a),
    sig: (a,b) => rank[a.status]-rank[b.status] || sig(b)-sig(a),
    prob: (a,b) => rank[a.status]-rank[b.status] || (a.ks_prob??1)-(b.ks_prob??1) || (b.ks_dist??0)-(a.ks_dist??0),
    name: (a,b) => a.key.localeCompare(b.key)
  }[by];
  return out.sort(cmp);
}
function card(r){
  const d = document.createElement('div'); d.className = 'card';
  const img = r.img ? `<a href="${r.img}" target="_blank"><img loading="lazy" src="${r.img}" alt=""></a>`
                    : `<div class="noimg">${r.status === 'pass' ? 'passing plot not published' : 'no plot'}</div>`;
  d.innerHTML = img + `<div class="k"></div><div class="s"><span class="${r.status}">${r.status}</span>`
    + (r.ks_dist !== undefined ? ` &middot; D=${fmt(r.ks_dist)} &middot; p=${fmt(r.ks_prob)}` : '')
    + (r.gold_entries !== undefined ? ` &middot; N gold/new=${r.gold_entries}/${r.test_entries}` : '') + `</div>`;
  d.querySelector('.k').textContent = r.key;
  return d;
}
function render(reset){
  if (reset) { shown = 0; $('grid').innerHTML = ''; }
  const list = filtered();
  $('count').textContent = `${list.length} histograms`;
  const frag = document.createDocumentFragment();
  list.slice(shown, shown + PAGE).forEach(r => frag.appendChild(card(r)));
  $('grid').appendChild(frag);
  shown = Math.min(list.length, shown + PAGE);
  $('more').hidden = shown >= list.length;
}
function folders(){
  const c = {};
  rows.forEach(r => { const f = folderOf(r); c[f] = c[f] || [0,0]; c[f][1]++; if (r.status !== 'pass') c[f][0]++; });
  const box = $('folders'); box.innerHTML = '';
  Object.entries(c).sort((a,b) => b[1][0]-a[1][0] || a[0].localeCompare(b[0])).forEach(([f,[n,t]]) => {
    const d = document.createElement('div');
    d.innerHTML = `<span></span><span class="${n ? 'fail' : 'pass'}">${n}/${t}</span>`;
    d.firstChild.textContent = f;
    d.onclick = () => { folder = folder === f ? null : f;
      [...box.children].forEach(x => x.classList.toggle('on', x === d && folder)); render(true); };
    box.appendChild(d);
  });
}
['status','sort','q','dmin'].forEach(id => $(id).addEventListener('input', () => {
  if (id === 'dmin') $('dminv').textContent = (+$('dmin').value).toFixed(2);
  render(true);
}));
$('more').onclick = () => render(false);
if (!rows.some(r => r.status !== 'pass')) $('status').value = 'all';
folders(); render(true);
