/* Lettura di un file SVG e conversione in polilinee in millimetri.
 *
 * L'appiattimento delle curve lo fa il browser, non noi: ogni forma vettoriale
 * espone getTotalLength e getPointAtLength, quindi si campiona per lunghezza
 * d'arco e si ottengono punti equidistanti lungo la curva. Riscrivere a mano
 * la matematica delle Bézier significherebbe rifare, peggio, una cosa che il
 * motore grafico fa già in modo esatto.
 *
 * Il campionamento per lunghezza d'arco regala anche il riconoscimento dei
 * sotto-tracciati: due campioni consecutivi non possono mai distare piu' del
 * passo scelto, quindi un salto piu' lungo e' per forza uno stacco di penna
 * dentro lo stesso elemento (il foro di una "o", per dire). Senza questo
 * controllo comparirebbe una riga che attraversa la lettera.
 */
var SvgImport = (function () {

  var SELECTOR = 'path,line,polyline,polygon,circle,ellipse,rect';

  function parse(text) {
    var holder = document.createElement('div');
    /* Fuori schermo ma non display:none: un elemento non disegnato non ha
       matrice di trasformazione, e i conti sulle coordinate fallirebbero. */
    holder.style.cssText = 'position:absolute;left:-10000px;top:0;width:900px;height:900px;overflow:hidden';
    holder.innerHTML = text;
    document.body.appendChild(holder);

    try {
      var svg = holder.querySelector('svg');
      if (!svg) throw new Error('Il file non contiene un elemento <svg>.');

      var rootInv = svg.getScreenCTM();
      if (!rootInv) throw new Error('L’SVG non ha una geometria leggibile.');
      rootInv = rootInv.inverse();

      var els = svg.querySelectorAll(SELECTOR);
      var polys = [];

      for (var i = 0; i < els.length; i++) {
        var el = els[i];
        if (typeof el.getTotalLength !== 'function') continue;

        var len = 0;
        try { len = el.getTotalLength(); } catch (e) { continue; }
        if (!isFinite(len) || len <= 0) continue;

        /* Il passo: fitto abbastanza da non tagliare le curve, non cosi' fitto
           da produrre centomila punti su una forma sola. Il diradamento
           successivo toglie comunque i punti inutili sui tratti dritti. */
        var step = Math.max(len / 3000, 0.35);
        var m = rootInv.multiply(el.getScreenCTM());
        var pt = svg.createSVGPoint();

        var cur = [], prev = null, s;
        for (s = 0; s <= len + 1e-6; s += step) {
          var raw = el.getPointAtLength(Math.min(s, len));
          pt.x = raw.x; pt.y = raw.y;
          var p = pt.matrixTransform(m);

          if (prev && dist(prev, p) > step * 1.8 * scaleOf(m)) {
            if (cur.length > 1) polys.push(cur);
            cur = [];
          }
          cur.push({ x: p.x, y: p.y });
          prev = p;
        }
        if (cur.length > 1) polys.push(cur);
      }

      if (!polys.length) throw new Error('Nessun tracciato trovato nel file.');
      return polys;

    } finally {
      holder.remove();
    }
  }

  function scaleOf(m) {
    return Math.sqrt(Math.abs(m.a * m.d - m.b * m.c)) || 1;
  }

  function dist(a, b) {
    return Math.sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
  }

  function bbox(polys) {
    var b = { x0: Infinity, y0: Infinity, x1: -Infinity, y1: -Infinity };
    for (var i = 0; i < polys.length; i++) {
      for (var j = 0; j < polys[i].length; j++) {
        var p = polys[i][j];
        if (p.x < b.x0) b.x0 = p.x;
        if (p.y < b.y0) b.y0 = p.y;
        if (p.x > b.x1) b.x1 = p.x;
        if (p.y > b.y1) b.y1 = p.y;
      }
    }
    b.w = b.x1 - b.x0;
    b.h = b.y1 - b.y0;
    return b;
  }

  /* Porta le polilinee nelle coordinate del foglio, in millimetri, centrate.
     Qui avviene anche il ribaltamento dell'asse Y: in SVG cresce verso il
     basso, sul foglio verso l'alto. */
  function toSheet(polys, sheet, percent) {
    var b = bbox(polys);
    var margin = 0.92;
    var base = Math.min(sheet.w * margin / b.w, sheet.h * margin / b.h);
    var k = base * (percent / 100);

    var w = b.w * k, h = b.h * k;
    var ox = (sheet.w - w) / 2, oy = (sheet.h - h) / 2;

    var out = [];
    for (var i = 0; i < polys.length; i++) {
      var src = polys[i], dst = [];
      for (var j = 0; j < src.length; j++) {
        dst.push({
          x: ox + (src[j].x - b.x0) * k,
          y: oy + (b.y1 - src[j].y) * k
        });
      }
      out.push(dst);
    }
    return out;
  }

  /* Diradamento di Douglas-Peucker: toglie i punti che non spostano la linea
     piu' della tolleranza. Un tratto dritto campionato ogni mezzo millimetro
     torna ad essere due punti, e il pianificatore di FluidNC puo' accelerare
     invece di frenare a ogni vertice inutile. */
  function simplify(polys, tol) {
    var out = [];
    for (var i = 0; i < polys.length; i++) {
      var r = dp(polys[i], 0, polys[i].length - 1, tol);
      if (r.length > 1) out.push(r);
    }
    return out;
  }

  function dp(pts, first, last, tol) {
    if (last <= first + 1) return [pts[first], pts[last]];
    var maxD = -1, idx = -1;
    for (var i = first + 1; i < last; i++) {
      var d = segDist(pts[i], pts[first], pts[last]);
      if (d > maxD) { maxD = d; idx = i; }
    }
    if (maxD <= tol) return [pts[first], pts[last]];
    var a = dp(pts, first, idx, tol);
    var b = dp(pts, idx, last, tol);
    return a.slice(0, a.length - 1).concat(b);
  }

  function segDist(p, a, b) {
    var dx = b.x - a.x, dy = b.y - a.y;
    var l2 = dx * dx + dy * dy;
    if (l2 === 0) return dist(p, a);
    var t = ((p.x - a.x) * dx + (p.y - a.y) * dy) / l2;
    t = Math.max(0, Math.min(1, t));
    return dist(p, { x: a.x + t * dx, y: a.y + t * dy });
  }

  return { parse: parse, toSheet: toSheet, simplify: simplify, bbox: bbox, dist: dist };
})();
