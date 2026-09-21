/* Stato centrale dell'applicazione.
 *
 * Un solo oggetto con i dati veri e un minimo sistema di notifiche: i moduli
 * non si chiamano fra loro, scrivono nello stato e ascoltano i cambiamenti.
 * Serve a evitare che, crescendo, ogni pezzo debba conoscere tutti gli altri.
 */
var State = (function () {
  var data = {
    connected: false,
    mock: true,
    host: 'cnc2d.local',

    machine: 'Off',          // Off | Idle | Run | Hold | Alarm
    pos: { x: 0, y: 0, z: 0 },

    paper: { w: 148, h: 210, name: 'A5' },

    art: null,               // il disegno caricato, quando ci sarà
    path: [],                // segmenti del percorso: {x1,y1,x2,y2,draw}
    origin: null,            // origine impostata a mano
    bounds: null,            // area calibrata: {x0,y0,x1,y1}

    scrub: 0,                // 0..1, avanzamento dell'anteprima
    jogStep: 1
  };

  var listeners = {};

  function on(key, fn) {
    (listeners[key] || (listeners[key] = [])).push(fn);
  }

  function emit(key) {
    var fns = listeners[key] || [];
    for (var i = 0; i < fns.length; i++) fns[i](data);
  }

  function set(key, value) {
    data[key] = value;
    emit(key);
  }

  return { data: data, on: on, set: set, emit: emit };
})();
