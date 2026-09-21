/* Il foglio: formati, dimensioni, e il disegno che ci sta sopra.
 *
 * Tutte le coordinate dentro l'SVG sono in millimetri, grazie alla viewBox.
 * Un gruppo ribalta l'asse Y, perché in SVG cresce verso il basso mentre
 * sulla macchina cresce verso l'alto: senza quel ribaltamento ogni disegno
 * uscirebbe specchiato, ed è un errore che si scopre tardi e male. */
var Paper = (function () {
  var svg, flip, elPaper, caption, layerTrace;

  var FORMATI = { A4: [210, 297], A5: [148, 210], A6: [105, 148] };

  function nomeFormato(w, h) {
    for (var k in FORMATI) {
      if (FORMATI[k][0] === w && FORMATI[k][1] === h) return k;
      if (FORMATI[k][1] === w && FORMATI[k][0] === h) return k + ' orizzontale';
    }
    return 'Personalizzato';
  }

  function apply() {
    var p = State.data.paper;
    p.name = nomeFormato(p.w, p.h);

    svg.setAttribute('viewBox', '0 0 ' + p.w + ' ' + p.h);
    flip.setAttribute('transform', 'translate(0,' + p.h + ') scale(1,-1)');
    elPaper.style.aspectRatio = p.w + ' / ' + p.h;

    /* Il foglio deve stare dentro il riquadro sia in altezza sia in larghezza:
       si lascia decidere al lato più vincolante. */
    if (p.w / p.h > 1) {
      elPaper.style.width = '100%';
      elPaper.style.height = 'auto';
    } else {
      elPaper.style.height = '100%';
      elPaper.style.width = 'auto';
    }

    caption.textContent = p.name + ' · ' + p.w + ' × ' + p.h + ' mm';
    State.emit('paper');
  }

  function setSize(w, h) {
    State.data.paper.w = w;
    State.data.paper.h = h;
    document.getElementById('paper-w').value = w;
    document.getElementById('paper-h').value = h;
    markPreset(w, h);
    apply();
    demoPath();
  }

  function markPreset(w, h) {
    var btns = document.querySelectorAll('.preset');
    for (var i = 0; i < btns.length; i++) {
      var b = btns[i];
      b.classList.toggle('is-active',
        +b.dataset.w === w && +b.dataset.h === h);
    }
  }

  /* Percorso di esempio, finché non si carica un disegno vero: serve a vedere
     subito se l'anteprima e il ribaltamento dell'asse Y funzionano. */
  function demoPath() {
    var p = State.data.paper;
    var m = Math.min(p.w, p.h) * 0.18;
    var x0 = m, y0 = m, x1 = p.w - m, y1 = p.h - m;
    var seg = [];

    seg.push({ x1: 0, y1: 0, x2: x0, y2: y0, draw: false });
    seg.push({ x1: x0, y1: y0, x2: x1, y2: y0, draw: true });
    seg.push({ x1: x1, y1: y0, x2: x1, y2: y1, draw: true });
    seg.push({ x1: x1, y1: y1, x2: x0, y2: y1, draw: true });
    seg.push({ x1: x0, y1: y1, x2: x0, y2: y0, draw: true });

    var cx = p.w / 2, cy = p.h / 2, r = Math.min(p.w, p.h) * 0.22;
    var px = cx + r, py = cy, N = 72;
    seg.push({ x1: x0, y1: y0, x2: px, y2: py, draw: false });
    for (var i = 1; i <= N; i++) {
      var a = i / N * Math.PI * 2;
      var nx = cx + r * Math.cos(a), ny = cy + r * Math.sin(a);
      seg.push({ x1: px, y1: py, x2: nx, y2: ny, draw: true });
      px = nx; py = ny;
    }

    State.data.path = seg;
    renderPath();
    State.emit('path');
  }

  function renderPath() {
    var seg = State.data.path, out = '';
    for (var i = 0; i < seg.length; i++) {
      var s = seg[i];
      out += '<line class="' + (s.draw ? 'trace-draw' : 'trace-move') +
             '" x1="' + s.x1 + '" y1="' + s.y1 +
             '" x2="' + s.x2 + '" y2="' + s.y2 + '" data-i="' + i + '"></line>';
    }
    layerTrace.innerHTML = out;
  }

  function init() {
    svg = document.getElementById('paper-svg');
    flip = document.getElementById('flip');
    elPaper = document.getElementById('paper');
    caption = document.getElementById('paper-caption');
    layerTrace = document.getElementById('layer-trace');

    var presets = document.querySelectorAll('.preset');
    for (var i = 0; i < presets.length; i++) {
      presets[i].addEventListener('click', function () {
        setSize(+this.dataset.w, +this.dataset.h);
      });
    }

    ['paper-w', 'paper-h'].forEach(function (id) {
      document.getElementById(id).addEventListener('change', function () {
        setSize(+document.getElementById('paper-w').value,
                +document.getElementById('paper-h').value);
      });
    });

    document.getElementById('btn-rotate-paper').addEventListener('click', function () {
      setSize(State.data.paper.h, State.data.paper.w);
    });

    apply();
    demoPath();
  }

  return { init: init, setSize: setSize, renderPath: renderPath };
})();
