/* Ottimizzazione del percorso.
 *
 * Il criterio non e' la distanza percorsa ma il TEMPO, e su questa macchina i
 * due non coincidono: ogni alzata di penna costa il tempo che il servo impiega
 * a muoversi e a smettere di oscillare, dell'ordine di mezzo secondo. Uno
 * spostamento di cinquanta millimetri, a 2000 mm/min, ne costa uno e mezzo.
 * Eliminare un'alzata vale quindi piu' che accorciare di dieci millimetri uno
 * spostamento, ed e' il motivo per cui la saldatura dei tratti contigui viene
 * prima del riordino.
 */
var Optimize = (function () {

  var JOIN_TOL = 0.15;   // mm: sotto questa distanza due capi sono lo stesso punto
  var LIFT_S = 0.45;     // secondi persi per ogni alzata e riabbassata di penna

  function d(a, b) {
    return Math.sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
  }

  function first(p) { return p[0]; }
  function last(p) { return p[p.length - 1]; }
  function isClosed(p) { return d(first(p), last(p)) <= JOIN_TOL; }

  function lengthOf(p) {
    var L = 0;
    for (var i = 1; i < p.length; i++) L += d(p[i - 1], p[i]);
    return L;
  }

  /* --- 1. doppioni esatti ---------------------------------------------- */

  function key(p) {
    var a = first(p), b = last(p);
    var r = function (v) { return Math.round(v * 20) / 20; };
    var s1 = r(a.x) + ',' + r(a.y) + '|' + r(b.x) + ',' + r(b.y);
    var s2 = r(b.x) + ',' + r(b.y) + '|' + r(a.x) + ',' + r(a.y);
    /* Stessa chiave nei due versi: una linea disegnata al contrario e' lo
       stesso segno sulla carta. */
    return (s1 < s2 ? s1 : s2) + '#' + Math.round(lengthOf(p) * 10);
  }

  function dedupe(polys) {
    var seen = {}, out = [];
    for (var i = 0; i < polys.length; i++) {
      var k = key(polys[i]);
      if (seen[k]) continue;
      seen[k] = 1;
      out.push(polys[i]);
    }
    return out;
  }

  /* --- 2. saldatura dei tratti contigui -------------------------------- */

  function join(polys) {
    var list = polys.slice(), out = [];

    while (list.length) {
      var cur = list.shift();
      if (isClosed(cur)) { out.push(cur); continue; }

      var grew = true;
      while (grew) {
        grew = false;
        for (var i = 0; i < list.length; i++) {
          var o = list[i];
          if (d(last(cur), first(o)) <= JOIN_TOL) { cur = cur.concat(o.slice(1)); }
          else if (d(last(cur), last(o)) <= JOIN_TOL) { cur = cur.concat(o.slice().reverse().slice(1)); }
          else if (d(first(cur), last(o)) <= JOIN_TOL) { cur = o.concat(cur.slice(1)); }
          else if (d(first(cur), first(o)) <= JOIN_TOL) { cur = o.slice().reverse().concat(cur.slice(1)); }
          else continue;
          list.splice(i, 1);
          grew = true;
          break;
        }
      }
      out.push(cur);
    }
    return out;
  }

  /* --- 3. ordine e verso ----------------------------------------------- */

  /* Vicino piu' prossimo, con due liberta' in piu' rispetto alla versione
     scolastica: un tratto aperto si puo' percorrere da entrambi i capi, e un
     contorno chiuso si puo' cominciare da uno qualsiasi dei suoi punti. La
     seconda conta molto su un logo, che e' fatto quasi solo di contorni
     chiusi: senza di essa la penna raggiunge sempre il punto in cui il
     disegnatore ha cominciato la forma, che non c'entra nulla con dove si
     trova adesso. */
  function order(polys, start) {
    var pool = polys.slice(), out = [], cur = start || { x: 0, y: 0 };

    while (pool.length) {
      var bestI = 0, bestD = Infinity, bestPoly = null;

      for (var i = 0; i < pool.length; i++) {
        var p = pool[i];

        if (isClosed(p)) {
          var bi = 0, bd = Infinity;
          for (var j = 0; j < p.length - 1; j++) {
            var dd = d(cur, p[j]);
            if (dd < bd) { bd = dd; bi = j; }
          }
          if (bd < bestD) {
            bestD = bd; bestI = i;
            bestPoly = rotate(p, bi);
          }
        } else {
          var df = d(cur, first(p)), dl = d(cur, last(p));
          if (df < bestD) { bestD = df; bestI = i; bestPoly = p; }
          if (dl < bestD) { bestD = dl; bestI = i; bestPoly = p.slice().reverse(); }
        }
      }

      pool.splice(bestI, 1);
      out.push(bestPoly);
      cur = last(bestPoly);
    }
    return out;
  }

  function rotate(p, i) {
    /* Il contorno e' chiuso: si taglia in i, si riattacca la coda davanti e si
       richiude sul nuovo punto di partenza. */
    var body = p.slice(0, p.length - 1);
    var r = body.slice(i).concat(body.slice(0, i));
    r.push({ x: r[0].x, y: r[0].y });
    return r;
  }

  /* --- misure ----------------------------------------------------------- */

  function stats(polys, feedDraw, feedTravel) {
    var draw = 0, travel = 0, cur = { x: 0, y: 0 };
    for (var i = 0; i < polys.length; i++) {
      travel += d(cur, first(polys[i]));
      draw += lengthOf(polys[i]);
      cur = last(polys[i]);
    }
    travel += d(cur, { x: 0, y: 0 });   // ritorno all'origine

    return {
      tratti: polys.length,
      punti: polys.reduce(function (n, p) { return n + p.length; }, 0),
      disegno: draw,
      spostamento: travel,
      alzate: polys.length,
      secondi: time(polys, feedDraw, feedTravel)
    };
  }

  /* Stima del tempo con profilo trapezoidale: accelerazione, tratto a
     velocita' costante, frenata. Una polilinea continua si tratta come un
     unico movimento, perche' il look-ahead di FluidNC mantiene la velocita'
     fra segmenti consecutivi invece di fermarsi a ogni vertice. E' una stima:
     ignora il rallentamento sugli spigoli vivi. */
  function time(polys, feedDraw, feedTravel) {
    var ACC = 200;                      // mm/s^2, come in configurazione
    var vD = feedDraw / 60, vT = feedTravel / 60;
    var t = 0, cur = { x: 0, y: 0 };

    for (var i = 0; i < polys.length; i++) {
      t += trapezoid(d(cur, first(polys[i])), vT, ACC);
      t += LIFT_S;
      t += trapezoid(lengthOf(polys[i]), vD, ACC);
      cur = last(polys[i]);
    }
    t += trapezoid(d(cur, { x: 0, y: 0 }), vT, ACC);
    return t;
  }

  function trapezoid(L, v, a) {
    if (L <= 0) return 0;
    var lAcc = v * v / a;               // spazio per accelerare e poi frenare
    if (L < lAcc) return 2 * Math.sqrt(L / a);
    return 2 * (v / a) + (L - lAcc) / v;
  }

  /* --- catena completa -------------------------------------------------- */

  function run(polys, feedDraw, feedTravel) {
    var prima = stats(polys, feedDraw, feedTravel);
    var p = order(join(dedupe(polys)), { x: 0, y: 0 });
    var dopo = stats(p, feedDraw, feedTravel);
    return { polys: p, prima: prima, dopo: dopo };
  }

  return { run: run, stats: stats, join: join, order: order, dedupe: dedupe };
})();
