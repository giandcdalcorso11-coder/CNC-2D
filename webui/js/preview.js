/* La colonna di destra: il cursore che fa scorrere la penna lungo il percorso.
 *
 * Serve a rispondere a una domanda precisa prima di far partire il disegno —
 * "in che ordine si muoverà, e dove passerà la penna?" — che su un plotter è
 * la differenza fra accorgersi di un errore in anteprima o a foglio rovinato.
 *
 * L'avanzamento è calcolato sulla lunghezza percorsa, non sul numero di
 * segmenti: un arco è fatto di tanti segmenti corti e uno spostamento lungo
 * di uno solo, quindi contare i segmenti darebbe un avanzamento bugiardo. */
var Preview = (function () {
  var slider, pct, posOut, dot, playBtn;
  var lengths = [], total = 0, playing = false, raf = null;

  function measure() {
    var seg = State.data.path;
    lengths = [];
    total = 0;
    for (var i = 0; i < seg.length; i++) {
      var s = seg[i];
      var d = Math.sqrt((s.x2 - s.x1) * (s.x2 - s.x1) + (s.y2 - s.y1) * (s.y2 - s.y1));
      lengths.push(d);
      total += d;
    }
    update();
  }

  /* Dalla frazione di percorso alla posizione della penna. */
  function pointAt(t) {
    var seg = State.data.path;
    if (!seg.length || total === 0) return { x: 0, y: 0, i: -1 };

    var target = t * total, acc = 0;
    for (var i = 0; i < seg.length; i++) {
      if (acc + lengths[i] >= target || i === seg.length - 1) {
        var k = lengths[i] > 0 ? (target - acc) / lengths[i] : 1;
        k = Math.max(0, Math.min(1, k));
        var s = seg[i];
        return {
          x: s.x1 + (s.x2 - s.x1) * k,
          y: s.y1 + (s.y2 - s.y1) * k,
          i: i
        };
      }
      acc += lengths[i];
    }
    return { x: 0, y: 0, i: -1 };
  }

  function update() {
    var t = State.data.scrub;
    var p = pointAt(t);

    dot.setAttribute('cx', p.x.toFixed(2));
    dot.setAttribute('cy', p.y.toFixed(2));

    pct.textContent = Math.round(t * 100) + '%';
    posOut.textContent = 'X ' + p.x.toFixed(1).replace('.', ',') +
                         ' · Y ' + p.y.toFixed(1).replace('.', ',');

    /* I segmenti già percorsi cambiano colore: si vede a colpo d'occhio
       quanto manca senza leggere la percentuale. */
    var lines = document.querySelectorAll('#layer-trace line');
    for (var i = 0; i < lines.length; i++) {
      lines[i].classList.toggle('trace-done', i <= p.i && lines[i].classList.contains('trace-draw'));
    }
  }

  function setScrub(t) {
    State.data.scrub = Math.max(0, Math.min(1, t));
    slider.value = Math.round(State.data.scrub * 1000);
    update();
  }

  function play() {
    if (playing) return stop();
    if (State.data.scrub >= 1) setScrub(0);
    playing = true;
    playBtn.textContent = 'Ferma';
    var last = performance.now();
    (function step(now) {
      if (!playing) return;
      var dt = (now - last) / 1000;
      last = now;
      setScrub(State.data.scrub + dt / 12);   // dodici secondi per l'intero percorso
      if (State.data.scrub >= 1) return stop();
      raf = requestAnimationFrame(step);
    })(last);
  }

  function stop() {
    playing = false;
    playBtn.textContent = 'Riproduci';
    if (raf) cancelAnimationFrame(raf);
  }

  function init() {
    slider = document.getElementById('scrub');
    pct = document.getElementById('scrub-pct');
    posOut = document.getElementById('scrub-pos');
    dot = document.getElementById('pen-dot');
    playBtn = document.getElementById('btn-play');

    slider.addEventListener('input', function () {
      stop();
      setScrub(this.value / 1000);
    });
    playBtn.addEventListener('click', play);

    State.on('path', measure);
    measure();
  }

  return { init: init, measure: measure };
})();
