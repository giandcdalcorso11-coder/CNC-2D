/* Avvio: collega i moduli, gestisce le tab e tiene aggiornata la barra in alto. */
(function () {

  /* Quali comandi hanno senso solo con la macchina collegata. Tenerli in un
     elenco unico evita che un pulsante resti attivo per dimenticanza e mandi
     un comando nel vuoto. */
  var NEEDS_LINK = [
    'btn-set-origin', 'btn-corner-bl', 'btn-corner-tr',
    'btn-send', 'btn-pause', 'btn-abort',
    'log-input', 'btn-log-send'
  ];

  var STATE_LABEL = {
    Off: 'Disconnesso', Idle: 'Pronta', Run: 'In movimento',
    Jog: 'Movimento manuale', Hold: 'In pausa', Alarm: 'Allarme',
    Door: 'Sportello', Home: 'Azzeramento', Check: 'Verifica'
  };

  var STATE_CLASS = {
    Off: 'state-off', Idle: 'state-idle', Run: 'state-run', Jog: 'state-run',
    Hold: 'state-hold', Alarm: 'state-alarm', Door: 'state-alarm'
  };

  function tabs() {
    var btns = document.querySelectorAll('.tab');
    for (var i = 0; i < btns.length; i++) {
      btns[i].addEventListener('click', function () {
        for (var j = 0; j < btns.length; j++) btns[j].classList.remove('is-active');
        this.classList.add('is-active');
        var panels = document.querySelectorAll('.panel');
        for (var k = 0; k < panels.length; k++) panels[k].classList.remove('is-active');
        document.getElementById('tab-' + this.dataset.tab).classList.add('is-active');
      });
    }
  }

  function refreshState() {
    var s = State.data.machine;
    var badge = document.getElementById('machine-state');
    badge.textContent = STATE_LABEL[s] || s;
    badge.className = 'state-badge ' + (STATE_CLASS[s] || 'state-off');

    /* Le spie riassumono lo stato: verde quando si può lavorare, giallo
       quando qualcosa richiede attenzione ma non è un guasto, rosso quando
       la macchina è ferma e va sbloccata a mano. */
    setLed('led-green',  s === 'Idle' || s === 'Run' || s === 'Jog');
    setLed('led-yellow', s === 'Hold' || (State.data.connected && !State.data.origin));
    setLed('led-red',    s === 'Alarm' || s === 'Door');
  }

  function setLed(id, on) {
    document.getElementById(id).classList.toggle('is-on', !!on);
  }

  function refreshLink() {
    var on = State.data.connected;
    for (var i = 0; i < NEEDS_LINK.length; i++) {
      var el = document.getElementById(NEEDS_LINK[i]);
      if (el) el.disabled = !on;
    }
    var jogs = document.querySelectorAll('[data-jog], #btn-pen-up, #btn-pen-down');
    for (var j = 0; j < jogs.length; j++) jogs[j].disabled = !on;

    /* La penna resta comunque ferma: il servo non è ancora tarato. */
    document.getElementById('btn-pen-up').disabled = true;
    document.getElementById('btn-pen-down').disabled = true;

    document.getElementById('btn-connect').textContent = on ? 'Disconnetti' : 'Connetti';
  }

  function refreshPos() {
    var p = State.data.pos;
    document.getElementById('dro-x').textContent = p.x.toFixed(2);
    document.getElementById('dro-y').textContent = p.y.toFixed(2);
    document.getElementById('dro-z').textContent = p.z.toFixed(2);
  }

  function refreshCalib() {
    var el = document.getElementById('calib-state');
    if (State.data.origin) {
      el.textContent = 'Origine impostata';
      el.classList.add('is-ok');
    } else {
      el.textContent = 'Area non calibrata';
      el.classList.remove('is-ok');
    }
    refreshState();
  }

  function dropzone() {
    var dz = document.getElementById('dropzone');
    var input = document.getElementById('file-input');

    dz.addEventListener('click', function () { input.click(); });

    ['dragenter', 'dragover'].forEach(function (ev) {
      dz.addEventListener(ev, function (e) {
        e.preventDefault();
        dz.classList.add('is-over');
      });
    });
    ['dragleave', 'drop'].forEach(function (ev) {
      dz.addEventListener(ev, function (e) {
        e.preventDefault();
        dz.classList.remove('is-over');
      });
    });

    dz.addEventListener('drop', function (e) {
      if (e.dataTransfer.files.length) accept(e.dataTransfer.files[0]);
    });
    input.addEventListener('change', function () {
      if (this.files.length) accept(this.files[0]);
    });

    document.getElementById('btn-clear-file').addEventListener('click', function () {
      document.getElementById('file-info').hidden = true;
      input.value = '';
      Log.add('msg', 'Disegno rimosso.');
    });

    function accept(file) {
      document.getElementById('file-name').textContent = file.name;
      document.getElementById('file-info').hidden = false;
      Log.add('msg', 'File ricevuto: ' + file.name +
        ' — la lettura dell\'SVG e la conversione in percorso non sono ancora implementate.');
    }
  }

  function connectBtn() {
    document.getElementById('btn-connect').addEventListener('click', function () {
      if (State.data.connected) { Api.disconnect(); return; }
      State.data.host = document.getElementById('host').value.trim();
      document.getElementById('set-host').value = State.data.host;
      Api.connect().catch(function (e) {
        Log.add('err', 'Connessione fallita: ' + e.message);
      });
    });
  }

  document.addEventListener('DOMContentLoaded', function () {
    Log.init();
    Paper.init();
    Preview.init();
    Jog.init();
    Settings.init();

    tabs();
    dropzone();
    connectBtn();

    State.on('machine', refreshState);
    State.on('connected', function () { refreshLink(); refreshState(); });
    State.on('pos', refreshPos);
    State.on('origin', refreshCalib);

    refreshState();
    refreshLink();
    refreshPos();
    refreshCalib();

    Log.add('msg', 'Interfaccia avviata in modalità finta. Premi Connetti per iniziare.');
  });
})();
