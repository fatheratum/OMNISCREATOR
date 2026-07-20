//==============================================================================
// omnisurf.js
// OMNIBRAIN GODHEAD — Living Dashboard Engine
// Loads omnibrain_state.json (from GitHub Actions / C++ extract_state)
// WebGL sphere + Float-Gen matrix | All panels live | SPACE/E keyboard
// Mirrors OmniBrainReserveNexus.hpp equations exactly:
//   R = F + D  |  SI = (LA + YP) / OO
//==============================================================================

'use strict';

// ============================================================================
// STATE — mirrors OmniBrainReserveNexus.hpp
// ============================================================================
const STATE = {
  simTime:   0.0,
  solvencyIndex: 1.0,
  liquidAssets:  100000.0,
  yieldPool:     8500.0,
  floatVolume:   42000.0,
  organismsOnline: 4,
  reserveTotal:  0.0,
  depositTotal:  0.0,
  yieldRate:     0.07,

  domains: [
    { id:'DOM-0001', label:'NARU-CORE',        balance:50000,  pendingYield:0 },
    { id:'DOM-0002', label:'ATUM-RESERVE',      balance:38000,  pendingYield:0 },
    { id:'DOM-0003', label:'SYNDICATE-ALPHA',   balance:22000,  pendingYield:0 },
    { id:'DOM-0004', label:'LIGHT-CODE-VAULT',  balance:17500,  pendingYield:0 },
    { id:'DOM-0005', label:'INFINITY-FUND',     balance:61000,  pendingYield:0 },
    { id:'DOM-0006', label:'GENESIS-POOL',      balance:9800,   pendingYield:0 },
    { id:'DOM-0007', label:'MATRIX-SINK',       balance:14200,  pendingYield:0 },
    { id:'DOM-0008', label:'VOID-BRIDGE',       balance:5500,   pendingYield:0 },
  ],

  organisms: [
    { id:0, name:'NARU-ATUM',        sigil:'◈', capital:120000, yieldRate:0.07, state:'ACTIVE', circulations:0, inventory:['LIGHT-CODE-UNITS','RESERVE-FRAGMENTS'] },
    { id:1, name:'VOID-WALKER',      sigil:'▲', capital:88000,  yieldRate:0.08, state:'ACTIVE', circulations:0, inventory:['VOID-KEYS','MATRIX-SHARDS'] },
    { id:2, name:'MATRIX-SYNDICATE', sigil:'■', capital:65000,  yieldRate:0.09, state:'ACTIVE', circulations:0, inventory:['SYNDICATE-BONDS','FLOAT-TOKENS'] },
    { id:3, name:'GENESIS-PRIME',    sigil:'★', capital:99000,  yieldRate:0.10, state:'ACTIVE', circulations:0, inventory:['GENESIS-SPARKS','NARU-BONDS'] },
  ],

  payments: [],
  consoleLog: [],
  nodes: [],
  paths: [],
};

// ============================================================================
// EQUATIONS — exact mirrors of OmniBrainReserveNexus.hpp
// ============================================================================
const PHI  = 1.6180339887;

function calc_reserve()         { return STATE.floatVolume + STATE.depositTotal; }
function calc_solvency_index()  { return STATE.organismsOnline > 0 ? (STATE.liquidAssets + STATE.yieldPool) / STATE.organismsOnline : 0; }
function calc_yield(principal)  { return principal * Math.max(0.001, Math.min(0.25, STATE.yieldRate)); }

function recalculate() {
  STATE.solvencyIndex = calc_solvency_index();
  STATE.reserveTotal  = calc_reserve();
}

// ============================================================================
// INIT FLOAT-GEN NODES (5x5x2 grid — mirrors FloatGenMatrix.hpp)
// ============================================================================
function init_nodes() {
  STATE.nodes = [];
  STATE.paths = [];
  let id = 0;
  for (let z = 0; z < 2; z++)
  for (let y = 0; y < 5; y++)
  for (let x = 0; x < 5; x++) {
    STATE.nodes.push({
      id, x:(x-2)*1.8, y:(y-2)*1.8, z:(z)*3.0,
      value: 1000 + (x + y*5 + z*25)*200,
      flow: 0, state: 0,
      fx: (x-2)*0.3, fy: (y-2)*0.3, fz: z*0.2,
      selected: false,
    });
    id++;
  }
  // Build paths
  const W=5,H=5,D=2;
  for (let z=0;z<D;z++) for (let y=0;y<H;y++) for (let x=0;x<W;x++) {
    const i = z*W*H + y*W + x;
    if (x+1<W) STATE.paths.push({a:i,b:i+1,      flow:0,active:false,t:0,speed:0.5});
    if (y+1<H) STATE.paths.push({a:i,b:i+W,      flow:0,active:false,t:0,speed:0.5});
    if (z+1<D) STATE.paths.push({a:i,b:i+W*H,    flow:0,active:false,t:0,speed:0.7});
  }
  for (let i=0;i<STATE.nodes.length;i+=7) {
    const j=(i+13)%STATE.nodes.length;
    STATE.paths.push({a:i,b:j,flow:0,active:false,t:0,speed:0.8});
  }
}

// ============================================================================
// ACTIONS — mirrors C++ functions
// ============================================================================
function acquire_deposit(domainIdx) {
  const amount = 500 + Math.random() * 24000 * PHI;
  const yieldEarned = calc_yield(amount);
  const d = STATE.domains[domainIdx];
  d.balance       += amount + yieldEarned;
  d.pendingYield   = yieldEarned;
  STATE.depositTotal += amount;
  STATE.yieldPool    += yieldEarned;
  STATE.liquidAssets += amount * 0.72;
  STATE.floatVolume  += amount * 0.1;
  recalculate();
  propagate_flow(domainIdx, amount);
  log_payment(`DOMAIN-${domainIdx}`, 'RESERVE-NEXUS', amount, yieldEarned);
  SPHERE.trigger_burst(Math.random()*2-1, Math.random()*2-1, 0, 512);
  console_log(`[DOMAIN] ${d.label} ACQUIRED $${amount.toFixed(2)} YIELD $${yieldEarned.toFixed(2)}`, 'gold');
  render_domains();
  return amount + yieldEarned;
}

function tender_for_solvency() {
  const si = calc_solvency_index();
  if (si < 1.0) {
    const deficit  = 1.0 - si;
    const infusion = deficit * STATE.organismsOnline * 1.1;
    STATE.liquidAssets += infusion;
    STATE.floatVolume  += infusion * 0.5;
    STATE.paths.forEach(p => { p.active=true; p.t=0; p.flow=infusion/STATE.paths.length; });
  }
  recalculate();
  SPHERE.trigger_burst(0,0,0,4096);
  console_log(`[SPACE] tender_for_solvency() — SI=${STATE.solvencyIndex.toFixed(4)}`,'gold');
  update_reserve_bar();
}

function establish_payment(from, to, amount) {
  from   = from   || `DOMAIN-${Math.floor(Math.random()*8)}`;
  to     = to     || `DOMAIN-${Math.floor(Math.random()*8)}`;
  amount = amount || 100 + Math.random()*9899;
  const y = calc_yield(amount);
  STATE.floatVolume += amount * 0.1;
  recalculate();
  const pmt = {
    id:   `PMT-${(Math.random()*0xFFFFF|0).toString(16).toUpperCase().padStart(5,'0')}`,
    from, to, amount, yield: y,
    status: 'UNENCUMBERED \u2022 PRIVATE \u2022 COMPLETE',
    ts: Date.now(),
  };
  STATE.payments.unshift(pmt);
  if (STATE.payments.length > 512) STATE.payments.pop();
  // Activate random path
  const ri = Math.floor(Math.random()*STATE.paths.length);
  STATE.paths[ri].active = true; STATE.paths[ri].t = 0;
  render_payment_feed();
  update_reserve_bar();
  console_log(`[E] ${pmt.id} $${amount.toFixed(2)} ${pmt.status}`,'green');
  return pmt;
}

function circulate(orgIdx) {
  const org  = STATE.organisms[orgIdx];
  org.state  = 'CIRCULATING';
  render_organisms();
  const rate   = 0.05 + Math.random()*0.17;
  const amount = org.capital * rate;
  org.capital -= amount;
  const acquired = acquire_deposit(orgIdx % 8);
  org.capital    += acquired * 0.15;
  org.circulations++;
  if (org.circulations % 3 === 0) org.yieldRate = Math.min(0.25, org.yieldRate + 0.005);
  if (Math.random() < 0.35) establish_payment(org.name, 'RESERVE-NEXUS', amount*0.5);
  SPHERE.trigger_burst(
    (orgIdx%2===0?-1:1)*3, (orgIdx<2?1:-1)*1.5, 1, 1024
  );
  setTimeout(()=>{ org.state='ACTIVE'; render_organisms(); }, 800);
  console_log(`[CIRCULATE] ${org.sigil} ${org.name} FLOW=$${amount.toFixed(2)} YIELD=${(org.yieldRate*100).toFixed(2)}%`,'gold');
}

function propagate_flow(domainIdx, amount) {
  const root = (domainIdx*6) % STATE.nodes.length;
  STATE.nodes[root].state = 2;
  STATE.nodes[root].flow  = amount;
  STATE.paths.forEach(p => {
    if (p.a===root||p.b===root) {
      p.active=true; p.flow=amount*0.33; p.t=0;
      p.speed=0.6+amount/50000;
    }
  });
}

function inspect_node(nodeId) {
  const n = STATE.nodes[nodeId];
  if (!n) return;
  STATE.nodes.forEach(nd=>nd.selected=false);
  n.selected=true; n.state=2;
  propagate_flow(nodeId, n.value);
  const stateLabels=['IDLE','FLOWING','BURST','SATURATED'];
  const conn = STATE.paths.filter(p=>p.a===nodeId||p.b===nodeId).length;
  document.getElementById('insp-title').textContent = `NODE ${n.id} — N-${nodeId}`;
  document.getElementById('insp-body').innerHTML = `
    <div class="insp-row"><span class="insp-key">POSITION</span><span class="insp-val">[${n.x.toFixed(2)}, ${n.y.toFixed(2)}, ${n.z.toFixed(2)}]</span></div>
    <div class="insp-row"><span class="insp-key">FORCE VEC</span><span class="insp-val">[${n.fx.toFixed(3)}, ${n.fy.toFixed(3)}, ${n.fz.toFixed(3)}]</span></div>
    <div class="insp-row"><span class="insp-key">VALUE</span><span class="insp-val">$${n.value.toFixed(2)}</span></div>
    <div class="insp-row"><span class="insp-key">FLOW</span><span class="insp-val">${n.flow.toFixed(4)}</span></div>
    <div class="insp-row"><span class="insp-key">STATE</span><span class="insp-val">${stateLabels[Math.min(n.state,3)]}</span></div>
    <div class="insp-row"><span class="insp-key">PATHS</span><span class="insp-val">${conn} connected</span></div>
  `;
  document.getElementById('node-inspector').classList.add('visible');
  console_log(`[NODE] N-${nodeId} STATE=${stateLabels[Math.min(n.state,3)]} FLOW=${n.flow.toFixed(2)}`,'cyan');
}

function log_payment(from,to,amount,y) {
  STATE.payments.unshift({
    id:`PMT-${(Math.random()*0xFFFFF|0).toString(16).toUpperCase().padStart(5,'0')}`,
    from,to,amount,yield:y,
    status:'UNENCUMBERED \u2022 PRIVATE \u2022 COMPLETE',ts:Date.now()
  });
  if(STATE.payments.length>512) STATE.payments.pop();
  render_payment_feed();
}

function console_log(msg, cls='dim') {
  STATE.consoleLog.unshift({msg,cls});
  if(STATE.consoleLog.length>80) STATE.consoleLog.pop();
  render_console();
}

// ============================================================================
// TICK — per-frame state evolution (mirrors nexus.tick())
// ============================================================================
function tick(dt) {
  STATE.simTime += dt;
  const t = STATE.simTime;

  STATE.floatVolume  *= (1 - 0.0003*dt);
  STATE.yieldRate    += Math.sin(t*0.3)*0.0001*dt;
  STATE.yieldRate     = Math.max(0.001, Math.min(0.25, STATE.yieldRate));
  STATE.liquidAssets += STATE.floatVolume * STATE.yieldRate * dt * 0.01;

  // Evolve nodes
  STATE.nodes.forEach((n,i)=>{
    n.value += Math.sin(t*0.5 + i*0.3)*10;
    n.fx += Math.cos(t*PHI + i)*0.01;
    n.fy += Math.sin(t*0.7  + i*0.5)*0.01;
    n.fz += Math.cos(t*1.1  + i*0.7)*0.01;
    if(n.state===2) n.state=1;
    else if(n.state===1&&n.flow<0.01) n.state=0;
    n.flow *= 0.99;
  });

  // Evolve paths
  STATE.paths.forEach(p=>{
    if(p.active){
      p.t += p.speed*dt;
      if(p.t>=1){p.t=0;p.flow*=0.95;if(p.flow<0.001)p.active=false;}
    }
    if(Math.random()<0.002*dt){p.active=true;p.flow=500+Math.random()*5000;p.t=0;}
  });

  // Organisms auto-evolve
  STATE.organisms.forEach((org,i)=>{
    org.capital += org.capital*org.yieldRate*dt*0.001;
    if(Math.random()<0.001*dt&&org.state==='ACTIVE'){
      org.state='TRANSCENDENT';
      setTimeout(()=>{org.state='ACTIVE';render_organisms();},2000);
      console_log(`[TRANSCEND] ${org.name} TRANSCENDENT`,'cyan');
    }
    // Rare auto-circulate
    if(Math.random()<0.003*dt&&org.state==='ACTIVE') circulate(i);
  });

  recalculate();

  // Rare auto-payment
  if(Math.random()<0.01*dt) establish_payment();

  update_reserve_bar();
}

// ============================================================================
// RENDER — DOM panel updates
// ============================================================================
function update_reserve_bar() {
  const si = STATE.solvencyIndex;
  const siEl = document.getElementById('rb-si');
  siEl.textContent = si.toFixed(4);
  siEl.style.color = si>=1?'var(--green)':si>=0.5?'var(--gold)':'var(--red)';
  document.getElementById('rb-la').textContent = '$'+fmt_money(STATE.liquidAssets);
  document.getElementById('rb-yr').textContent = (STATE.yieldRate*100).toFixed(3)+'%';
  document.getElementById('rb-fv').textContent = '$'+fmt_money(STATE.floatVolume);
  document.getElementById('rb-oo').textContent = STATE.organismsOnline;
  document.getElementById('rb-rt').textContent = '$'+fmt_money(STATE.reserveTotal);
  document.getElementById('sphere-si-readout').textContent =
    `SI = ${si.toFixed(4)} · GLOW = ${Math.min(si,2).toFixed(2)}`;
}

function render_domains() {
  const el = document.getElementById('domain-list');
  el.innerHTML = STATE.domains.map((d,i)=>`
    <div class="domain-card" id="dcard-${i}" onclick="OMNI.acquireDeposit(${i})">
      <div class="domain-name">${d.label}</div>
      <div class="domain-bal">$${fmt_money(d.balance)}</div>
      <div class="domain-yield">YIELD $${fmt_money(d.pendingYield)}</div>
      <button class="domain-btn">ACQUIRE</button>
    </div>
  `).join('');
}

function render_organisms() {
  const el = document.getElementById('organism-list');
  el.innerHTML = STATE.organisms.map((o,i)=>`
    <div class="organism-card${o.name==='NARU-ATUM'?' naru':''} ${o.state==='CIRCULATING'||o.state==='TRANSCENDENT'?'active-pulse':''}">
      <div class="org-header">
        <span class="org-sigil">${o.sigil}</span>
        <span class="org-name">${o.name}</span>
        <span class="org-state ${o.state}">${o.state}</span>
      </div>
      <div class="org-row"><span class="org-label">CAPITAL</span><span class="org-val">$${fmt_money(o.capital)}</span></div>
      <div class="org-row"><span class="org-label">YIELD</span><span class="org-val">${(o.yieldRate*100).toFixed(2)}%</span></div>
      <div class="org-row"><span class="org-label">CIRCULATIONS</span><span class="org-val">${o.circulations}</span></div>
      <div class="org-inv">${o.inventory.map(iv=>`∷ ${iv}`).join('  ')}</div>
      <button class="circulate-btn" onclick="OMNI.circulate(${i})">
        CIRCULATE • EXECUTE PATH
      </button>
    </div>
  `).join('');
}

function render_payment_feed() {
  const el = document.getElementById('payment-log');
  el.innerHTML = STATE.payments.slice(0,60).map(p=>`
    <div class="pmt-line complete">
      [${p.id}] ${p.from} → ${p.to} | $${p.amount.toFixed(2)} | YIELD $${p.yield.toFixed(2)} | ${p.status}
    </div>
  `).join('');
}

function render_console() {
  const el = document.getElementById('console-log');
  el.innerHTML = STATE.consoleLog.slice(0,40).map(l=>`
    <div class="con-line ${l.cls}">${l.msg}</div>
  `).join('');
}

function fmt_money(v) {
  return v.toLocaleString('en-US',{minimumFractionDigits:2,maximumFractionDigits:2});
}

// ============================================================================
// WEBGL SPHERE + FLOAT-GEN MATRIX
// ============================================================================
const SPHERE = (() => {
  const bursts = [];
  const particles = [];
  const MAX_P = 8192;

  for (let i=0;i<MAX_P;i++) particles.push({
    x:0,y:0,z:0,vx:0,vy:0,vz:0,r:1,g:.78,b:.1,a:0,size:.03,life:0,maxLife:1,active:false,type:0
  });

  function trigger_burst(ox,oy,oz,count,r=1,g=.82,b=.1) {
    let spawned=0;
    for(const p of particles){
      if(!p.active&&spawned<count){
        const len=Math.sqrt(ox*ox+oy*oy+oz*oz)||1;
        const spd=0.5+Math.random()*2.5;
        p.x=ox;p.y=oy;p.z=oz;
        p.vx=(ox/len)*spd+(Math.random()-.5)*.3;
        p.vy=(oy/len)*spd+(Math.random()-.5)*.3;
        p.vz=(oz/len)*spd+(Math.random()-.5)*.3;
        p.r=r;p.g=g;p.b=b;p.a=.9;
        p.size=.02+Math.random()*.06;
        p.maxLife=.8+Math.random()*2.2;
        p.life=p.maxLife;p.active=true;p.type=1;
        spawned++;
      }
    }
  }

  return {trigger_burst, particles};
})();

// ============================================================================
// CANVAS RENDERER
// ============================================================================
(function() {
  const canvas = document.getElementById('omnibrain-canvas');
  const gl = canvas.getContext('webgl2',{antialias:true,alpha:true});
  if(!gl){console.error('WebGL2 not supported');return;}

  gl.enable(gl.BLEND);
  gl.blendFunc(gl.SRC_ALPHA,gl.ONE);

  let W,H;
  function resize(){
    W=canvas.width=window.innerWidth;
    H=canvas.height=window.innerHeight;
    gl.viewport(0,0,W,H);
  }
  resize();
  window.addEventListener('resize',resize);

  // Compile shader
  function mkShader(type,src){
    const s=gl.createShader(type);
    gl.shaderSource(s,src);
    gl.compileShader(s);
    return s;
  }
  function mkProgram(vs,fs){
    const p=gl.createProgram();
    gl.attachShader(p,mkShader(gl.VERTEX_SHADER,vs));
    gl.attachShader(p,mkShader(gl.FRAGMENT_SHADER,fs));
    gl.linkProgram(p);
    return p;
  }

  // ── PARTICLE PROGRAM ─────────────────────────────────
  const partProg = mkProgram(`#version 300 es
    in vec3 aPos; in vec4 aColor; in float aSize;
    uniform mat4 uMVP;
    out vec4 vColor;
    void main(){
      vec4 p=uMVP*vec4(aPos,1.);
      gl_Position=p;
      gl_PointSize=max(1.,aSize*400./p.w);
      vColor=aColor;
    }`,`#version 300 es
    precision highp float;
    in vec4 vColor;
    out vec4 fragColor;
    void main(){
      vec2 c=gl_PointCoord*2.-1.;
      float d=length(c);
      if(d>1.)discard;
      float soft=1.-smoothstep(.5,1.,d);
      fragColor=vec4(vColor.rgb*soft,vColor.a*soft);
    }`);

  // ── LINE PROGRAM ──────────────────────────────────────
  const lineProg = mkProgram(`#version 300 es
    in vec3 aPos; in vec4 aColor;
    uniform mat4 uMVP;
    out vec4 vColor;
    void main(){ gl_Position=uMVP*vec4(aPos,1.); vColor=aColor; }`,
    `#version 300 es
    precision highp float;
    in vec4 vColor; out vec4 fragColor;
    void main(){ fragColor=vColor; }`);

  // Buffers
  const partBuf = gl.createBuffer();
  const lineBuf = gl.createBuffer();

  // Matrix helpers
  function perspective(fov,asp,near,far){
    const f=1/Math.tan(fov*.5),d=far-near;
    return new Float32Array([f/asp,0,0,0, 0,f,0,0,
      0,0,-(far+near)/d,-1, 0,0,-2*far*near/d,0]);
  }
  function lookAt(ex,ey,ez){
    const cx=0,cy=0,cz=0;
    let fx=cx-ex,fy=cy-ey,fz=cz-ez;
    const fl=Math.sqrt(fx*fx+fy*fy+fz*fz);
    fx/=fl;fy/=fl;fz/=fl;
    let sx=fy*0-fz*1,sy=fz*0-fx*0,sz=fx*1-fy*0;
    const sl=Math.sqrt(sx*sx+sy*sy+sz*sz);
    sx/=sl;sy/=sl;sz/=sl;
    const ux=sy*fz-sz*fy,uy=sz*fx-sx*fz,uz=sx*fy-sy*fx;
    return new Float32Array([
      sx,ux,-fx,0, sy,uy,-fy,0, sz,uz,-fz,0,
      -(sx*ex+sy*ey+sz*ez),-(ux*ex+uy*ey+uz*ez),(fx*ex+fy*ey+fz*ez),1
    ]);
  }
  function mulMat(a,b){
    const r=new Float32Array(16);
    for(let i=0;i<4;i++) for(let j=0;j<4;j++)
      for(let k=0;k<4;k++) r[i*4+j]+=a[i*4+k]*b[k*4+j];
    return r;
  }

  // Camera orbit
  let camTheta=.4,camPhi=.35,camR=18;
  let dragging=false,lastMX=0,lastMY=0;
  canvas.addEventListener('mousedown',e=>{
    if(e.clientX>252&&e.clientX<W-268){
      dragging=true;lastMX=e.clientX;lastMY=e.clientY;
    }
  });
  canvas.addEventListener('mouseup',()=>dragging=false);
  canvas.addEventListener('mousemove',e=>{
    if(!dragging)return;
    camTheta-=(e.clientX-lastMX)*.008;
    camPhi=Math.max(-.145,Math.min(1.45,camPhi+(e.clientY-lastMY)*.006));
    lastMX=e.clientX;lastMY=e.clientY;
  });
  canvas.addEventListener('wheel',e=>{
    camR=Math.max(4,Math.min(60,camR-e.deltaY*.012));
  });
  // Right-click → node inspect
  canvas.addEventListener('contextmenu',e=>{
    e.preventDefault();
    const nx=(e.clientX/W-.5)*50;
    const ny=(e.clientY/H-.5)*50;
    let best=0,bestD=1e9;
    STATE.nodes.forEach((n,i)=>{
      const d=(n.x-nx)**2+(n.y-ny)**2;
      if(d<bestD){bestD=d;best=i;}
    });
    inspect_node(best);
  });

  // ── SPHERE GEOMETRY ───────────────────────────────────
  const sphereVerts=[];
  const stacks=28,slices=56,R=0.55;
  for(let st=0;st<=stacks;st++){
    const phi_v=Math.PI*st/stacks;
    const y=R*Math.cos(phi_v),rxy=R*Math.sin(phi_v);
    for(let sl=0;sl<=slices;sl++){
      const th=2*Math.PI*sl/slices;
      sphereVerts.push(rxy*Math.cos(th),y,rxy*Math.sin(th));
    }
  }
  const sphereIdx=[];
  for(let st=0;st<stacks;st++) for(let sl=0;sl<slices;sl++){
    const a=st*(slices+1)+sl,b=a+slices+1;
    sphereIdx.push(a,b,a+1,b,b+1,a+1);
  }
  const sphVBuf=gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER,sphVBuf);
  gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(sphereVerts),gl.STATIC_DRAW);
  const sphIBuf=gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,sphIBuf);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(sphereIdx),gl.STATIC_DRAW);

  const sphProg = mkProgram(`#version 300 es
    in vec3 aPos; uniform mat4 uMVP; uniform float uTime; uniform float uGlow;
    out vec3 vPos; out float vGlow;
    void main(){
      float pulse=1.+.08*sin(uTime*1.2);
      gl_Position=uMVP*vec4(aPos*pulse,1.);
      vPos=aPos; vGlow=uGlow;
    }`,`#version 300 es
    precision highp float;
    in vec3 vPos; in float vGlow;
    uniform float uTime;
    out vec4 fragColor;
    void main(){
      float d=length(vPos);
      float core=exp(-d*3.)*vGlow*2.;
      float pulse=.5+.5*sin(uTime*1.2);
      vec3 gold=vec3(1.,.82,.08);
      vec3 col=gold*(core+.3*pulse);
      float a=clamp(.3+core*.8,0.,1.);
      fragColor=vec4(col,a);
    }`);

  // Sacred rings (9 latitude rings)
  function build_ring(lat,radius){
    const segs=80,y=R*1.02*Math.sin(lat),r=R*1.02*Math.cos(lat);
    const verts=[];
    for(let i=0;i<=segs;i++){
      const th=2*Math.PI*i/segs;
      verts.push(r*Math.cos(th),y,r*Math.sin(th));
    }
    return verts;
  }
  const ringData=[];
  for(let i=0;i<9;i++){
    const lat=(i/8-.5)*Math.PI;
    ringData.push({verts:build_ring(lat,R),speed:.3+i*.07*(i%2?1:-1),angle:0,buf:null});
  }
  ringData.forEach(rd=>{
    rd.buf=gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER,rd.buf);
    gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(rd.verts),gl.STATIC_DRAW);
  });

  // ── MAIN RENDER LOOP ──────────────────────────────────
  let last=0;
  function frame(now){
    now/=1000;
    const dt=Math.min(now-last,.05);
    last=now;

    tick(dt);

    // Update sphere particles
    SPHERE.particles.forEach(p=>{
      if(!p.active)return;
      p.x+=p.vx*dt;p.y+=p.vy*dt;p.z+=p.vz*dt;
      p.life-=dt;
      const t=p.life/p.maxLife;
      p.a=t*.9;
      p.g=Math.max(0,p.g-dt*.05);
      p.b=Math.min(1,p.b+dt*.1);
      if(p.life<=0||p.a<.01)p.active=false;
    });

    // Auto-emit stream particles
    if(Math.random()<.4){
      for(const p of SPHERE.particles){
        if(!p.active){
          const phi_v=Math.random()*Math.PI*2,th=Math.acos(Math.random()*2-1);
          const sx=R*Math.sin(th)*Math.cos(phi_v);
          const sy=R*Math.cos(th);
          const sz=R*Math.sin(th)*Math.sin(phi_v);
          const len=Math.sqrt(sx*sx+sy*sy+sz*sz)||1;
          const spd=.5+Math.random()*2.5;
          p.x=sx;p.y=sy;p.z=sz;
          p.vx=(sx/len)*spd+(Math.random()-.5)*.3;
          p.vy=(sy/len)*spd+(Math.random()-.5)*.3;
          p.vz=(sz/len)*spd+(Math.random()-.5)*.3;
          p.r=1;p.g=.78+Math.random()*.1;p.b=.1;p.a=.9;
          p.size=.02+Math.random()*.05;
          p.maxLife=.8+Math.random()*2.2;p.life=p.maxLife;p.active=true;p.type=0;
          break;
        }
      }
    }

    gl.clearColor(.027,.012,.055,1);
    gl.clear(gl.COLOR_BUFFER_BIT|gl.DEPTH_BUFFER_BIT);

    // Camera matrices
    const ex=camR*Math.cos(camPhi)*Math.sin(camTheta);
    const ey=camR*Math.sin(camPhi);
    const ez=camR*Math.cos(camPhi)*Math.cos(camTheta);
    const view=lookAt(ex,ey,ez);
    const proj=perspective(Math.PI/4,W/H,.1,200);
    const mvp=mulMat(proj,view);

    // ── Draw sphere core ──────────────────────────────
    gl.useProgram(sphProg);
    const uMVPs=gl.getUniformLocation(sphProg,'uMVP');
    const uTs=gl.getUniformLocation(sphProg,'uTime');
    const uGs=gl.getUniformLocation(sphProg,'uGlow');
    gl.uniformMatrix4fv(uMVPs,false,mvp);
    gl.uniform1f(uTs,now);
    gl.uniform1f(uGs,Math.min(STATE.solvencyIndex,2));
    gl.bindBuffer(gl.ARRAY_BUFFER,sphVBuf);
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,sphIBuf);
    const aPosS=gl.getAttribLocation(sphProg,'aPos');
    gl.enableVertexAttribArray(aPosS);
    gl.vertexAttribPointer(aPosS,3,gl.FLOAT,false,0,0);
    gl.drawElements(gl.TRIANGLES,sphereIdx.length,gl.UNSIGNED_SHORT,0);

    // ── Draw sacred rings ─────────────────────────────
    gl.useProgram(lineProg);
    const uMVPl=gl.getUniformLocation(lineProg,'uMVP');
    const uCl=gl.getUniformLocation(lineProg,'aColor');
    gl.uniformMatrix4fv(uMVPl,false,mvp);
    const aPosL=gl.getAttribLocation(lineProg,'aPos');
    const aColL=gl.getAttribLocation(lineProg,'aColor');
    gl.enableVertexAttribArray(aPosL);

    ringData.forEach((rd,i)=>{
      rd.angle += rd.speed*dt*(1+STATE.yieldRate*2);
      const pulse=.4+.6*Math.abs(Math.sin(now*.7+i*.4));
      const ringVerts=[];
      rd.verts.forEach((_,vi)=>{
        if(vi%3===0){
          const x=rd.verts[vi],y=rd.verts[vi+1],z=rd.verts[vi+2];
          const ca=Math.cos(rd.angle),sa=Math.sin(rd.angle);
          ringVerts.push(ca*x-sa*z,y,sa*x+ca*z, 1,.92,.3,pulse*.9);
        }
      });
      gl.bindBuffer(gl.ARRAY_BUFFER,lineBuf);
      gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(ringVerts),gl.DYNAMIC_DRAW);
      gl.vertexAttribPointer(aPosL,3,gl.FLOAT,false,28,0);
      if(aColL>=0){gl.enableVertexAttribArray(aColL);gl.vertexAttribPointer(aColL,4,gl.FLOAT,false,28,12);}
      gl.drawArrays(gl.LINE_STRIP,0,ringVerts.length/7);
    });

    // ── Draw Float-Gen matrix lines ───────────────────
    const lineVerts=[];
    STATE.paths.forEach(p=>{
      const a=STATE.nodes[p.a],b=STATE.nodes[p.b];
      if(!a||!b)return;
      const ft=Math.min(p.flow/10000,1);
      const r=.9+ft*.1,g=.65*ft+.2*(1-ft),bv=.05*(1-ft);
      const alpha=p.active?(.7+ft*.3):.12;
      lineVerts.push(
        a.x,a.y,a.z, r,g,bv,alpha,
        b.x,b.y,b.z, r,g,bv,alpha
      );
    });
    if(lineVerts.length){
      gl.bindBuffer(gl.ARRAY_BUFFER,lineBuf);
      gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(lineVerts),gl.DYNAMIC_DRAW);
      gl.vertexAttribPointer(aPosL,3,gl.FLOAT,false,28,0);
      if(aColL>=0) gl.vertexAttribPointer(aColL,4,gl.FLOAT,false,28,12);
      gl.drawArrays(gl.LINES,0,lineVerts.length/7);
    }

    // ── Draw flow particles (matrix) ──────────────────
    const flowPVerts=[];
    STATE.paths.forEach(p=>{
      if(!p.active)return;
      const a=STATE.nodes[p.a],b=STATE.nodes[p.b];
      if(!a||!b)return;
      const x=a.x+(b.x-a.x)*p.t;
      const y=a.y+(b.y-a.y)*p.t;
      const z=a.z+(b.z-a.z)*p.t;
      const edge=Math.min(p.t,1-p.t)*4;
      flowPVerts.push(x,y,z, 1,.9,.15,Math.min(1,edge), .12);
    });

    // ── Draw light-code particles (sphere) ────────────
    const partVerts=[];
    SPHERE.particles.forEach(p=>{
      if(!p.active)return;
      partVerts.push(p.x,p.y,p.z, p.r,p.g,p.b,p.a, p.size);
    });
    // Merge flow + sphere particles
    const allPartVerts=[...partVerts,...flowPVerts];
    if(allPartVerts.length){
      gl.useProgram(partProg);
      gl.uniformMatrix4fv(gl.getUniformLocation(partProg,'uMVP'),false,mvp);
      gl.bindBuffer(gl.ARRAY_BUFFER,partBuf);
      gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(allPartVerts),gl.DYNAMIC_DRAW);
      const aPP=gl.getAttribLocation(partProg,'aPos');
      const aCP=gl.getAttribLocation(partProg,'aColor');
      const aSP=gl.getAttribLocation(partProg,'aSize');
      gl.enableVertexAttribArray(aPP);gl.enableVertexAttribArray(aCP);gl.enableVertexAttribArray(aSP);
      gl.vertexAttribPointer(aPP,3,gl.FLOAT,false,32,0);
      gl.vertexAttribPointer(aCP,4,gl.FLOAT,false,32,12);
      gl.vertexAttribPointer(aSP,1,gl.FLOAT,false,32,28);
      gl.drawArrays(gl.POINTS,0,allPartVerts.length/8);
    }

    requestAnimationFrame(frame);
  }
  requestAnimationFrame(frame);
})();

// ============================================================================
// KEYBOARD CONTROLS
// ============================================================================
window.addEventListener('keyup',e=>{
  if(e.code==='Space'){e.preventDefault();OMNI.tenderSolvency();}
  if(e.key==='e'||e.key==='E') OMNI.establishPayment();
  if(e.key==='c'||e.key==='C') OMNI.circulate(0);
  if(e.key==='n'||e.key==='N') OMNI.inspectNode(0);
  if(e.key==='r'||e.key==='R'){recalculate();update_reserve_bar();}
});
window.addEventListener('keydown',e=>{if(e.code==='Space')e.preventDefault();});

// ============================================================================
// PUBLIC API — OMNI
// ============================================================================
window.OMNI = {
  tenderSolvency:   ()=>tender_for_solvency(),
  establishPayment: ()=>establish_payment(),
  acquireDeposit:   (i)=>acquire_deposit(i),
  circulate:        (i)=>circulate(i),
  inspectNode:      (i)=>inspect_node(i),
  closeInspector:   ()=>document.getElementById('node-inspector').classList.remove('visible'),
  state:            STATE,
};

// ============================================================================
// BOOT — load state from GitHub Actions JSON, then init all panels
// ============================================================================
(function boot(){
  init_nodes();

  // Try to load omnibrain_state.json from GitHub Actions output
  fetch('omnibrain_state.json')
    .then(r=>r.json())
    .then(data=>{
      if(data.reserve){
        Object.assign(STATE,{
          solvencyIndex: data.reserve.solvencyIndex,
          liquidAssets:  data.reserve.liquidAssets,
          yieldPool:     data.reserve.yieldPool,
          floatVolume:   data.reserve.floatVolume,
          organismsOnline: data.reserve.organismsOnline,
          yieldRate:     data.reserve.yieldRate,
        });
      }
      if(data.domains)  STATE.domains   = data.domains;
      if(data.organisms)STATE.organisms = data.organisms.map(o=>({...o,circulations:o.circulations||0,inventory:o.inventory||[]}));
      if(data.payments) STATE.payments  = data.payments;
      console_log('[BOOT] omnibrain_state.json LOADED FROM GITHUB ACTIONS','cyan');
      console_log(`[BOOT] OMNIVALKIN: ${data.engines?.omnivalkin?.status||'STANDBY'}`,'gold');
      console_log(`[BOOT] OMNIDIRECTXINFINITY: ${data.engines?.omnidirectxinfinity?.status||'STANDBY'}`,'gold');
    })
    .catch(()=>{
      console_log('[BOOT] omnibrain_state.json not found — using live defaults','dim');
    })
    .finally(()=>{
      recalculate();
      update_reserve_bar();
      render_domains();
      render_organisms();
      render_payment_feed();
      // Boot payments
      for(let i=0;i<8;i++) establish_payment();
      console_log('[BOOT] OMNIBRAIN GODHEAD ONLINE — NARU ATUM PROTOCOL ENGAGED','gold');
      console_log('[SPACE] tender_for_solvency  [E] establish_payment  [C] circulate  [N] inspect','dim');
    });
})();
