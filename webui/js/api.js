/* Comunicazione con FluidNC.
 *
 * Tutto il traffico verso la macchina passa da qui, per due motivi.
 *
 * Il primo è la modalità finta: senza macchina accesa l'interfaccia deve
 * comunque funzionare, altrimenti non ci si può lavorare sopra. Con `mock`
 * attivo le risposte sono simulate e nessuna richiesta esce dal browser.
 *
 * Il secondo è l'origine della pagina. FluidNC espone il suo HTTP senza
 * intestazioni CORS, quindi una pagina servita da un dominio diverso non può
 * interrogarlo: le richieste partono, il browser rifiuta la risposta. Perché
 * funzioni davvero, questa pagina va caricata nella memoria dell'ESP32 e
 * servita dalla macchina stessa. Fino ad allora si resta in modalità finta.
 */
var Api = (function () {

  var pollTimer = null;

  function base() {
    return 'http://' + State.data.host;
  }

  /* FluidNC accetta comandi sulla rotta /command, e risponde in testo. */
  function send(line) {
    Log.add('cmd', line);

    if (State.data.mock) {
      return mockReply(line);
    }

    return fetch(base() + '/command?commandText=' + encodeURIComponent(line))
      .then(function (r) { return r.text(); })
      .then(function (txt) {
        if (txt && txt.trim()) Log.add('msg', txt.trim());
        return txt;
      })
      .catch(function (err) {
        Log.add('err', 'Richiesta fallita: ' + err.message);
        setConnected(false);
        throw err;
      });
  }

  /* Il rapporto di stato di Grbl: <Idle|MPos:0.000,0.000,0.000|FS:0,0> */
  function parseStatus(txt) {
    var m = /<([A-Za-z]+)[^>]*?\|MPos:(-?[\d.]+),(-?[\d.]+),(-?[\d.]+)/.exec(txt || '');
    if (!m) return null;
    return {
      state: m[1],
      pos: { x: parseFloat(m[2]), y: parseFloat(m[3]), z: parseFloat(m[4]) }
    };
  }

  function applyStatus(st) {
    if (!st) return;
    State.set('machine', st.state);
    State.data.pos = st.pos;
    State.emit('pos');
  }

  function connect() {
    if (State.data.mock) {
      setConnected(true);
      Log.add('msg', 'Modalità finta attiva: nessuna macchina collegata.');
      State.set('machine', 'Idle');
      return Promise.resolve();
    }
    return send('?').then(function (txt) {
      var st = parseStatus(txt);
      if (!st) throw new Error('risposta non riconosciuta');
      setConnected(true);
      applyStatus(st);
      startPolling();
    });
  }

  function disconnect() {
    stopPolling();
    setConnected(false);
    State.set('machine', 'Off');
  }

  function setConnected(v) {
    if (State.data.connected !== v) State.set('connected', v);
  }

  function startPolling() {
    stopPolling();
    pollTimer = setInterval(function () {
      if (State.data.mock) return;
      fetch(base() + '/command?commandText=' + encodeURIComponent('?'))
        .then(function (r) { return r.text(); })
        .then(function (t) { applyStatus(parseStatus(t)); })
        .catch(function () { disconnect(); });
    }, 500);
  }

  function stopPolling() {
    if (pollTimer) { clearInterval(pollTimer); pollTimer = null; }
  }

  /* --- comandi di alto livello, così i moduli non scrivono G-code a mano --- */

  function jog(axis, delta, feed) {
    return send('$J=G91 ' + axis + delta + ' F' + (feed || 1000));
  }

  function setOrigin() {
    return send('G92 X0 Y0');
  }

  function led(index, on) {
    return send((on ? 'M62 P' : 'M63 P') + index);
  }

  function feedHold()  { return send('!'); }
  function cycleStart() { return send('~'); }
  function reset()      { return send(String.fromCharCode(0x18)); }

  /* ------------------------- modalità finta ------------------------- */

  function mockReply(line) {
    var reply = 'ok';
    if (line === '?') {
      reply = '<' + State.data.machine + '|MPos:' +
        State.data.pos.x.toFixed(3) + ',' +
        State.data.pos.y.toFixed(3) + ',' +
        State.data.pos.z.toFixed(3) + '|FS:0,0>';
    } else if (line.indexOf('$J=') === 0) {
      var m = /([XYZ])(-?[\d.]+)/g, g;
      while ((g = m.exec(line))) {
        var k = g[1].toLowerCase();
        State.data.pos[k] += parseFloat(g[2]);
      }
      State.emit('pos');
    } else if (line.indexOf('G92') === 0) {
      State.data.pos = { x: 0, y: 0, z: 0 };
      State.emit('pos');
    }
    Log.add('msg', reply);
    return Promise.resolve(reply);
  }

  return {
    send: send, connect: connect, disconnect: disconnect,
    jog: jog, setOrigin: setOrigin, led: led,
    feedHold: feedHold, cycleStart: cycleStart, reset: reset,
    parseStatus: parseStatus
  };
})();
