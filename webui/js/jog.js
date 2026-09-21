/* Movimento manuale, origine e penna.
 *
 * Il passo scelto vale anche come sicurezza: su una macchina senza
 * azzeramento automatico, un comando da 50 mm dato per sbaglio vicino a fine
 * corsa manda il carrello in battuta. Il passo resta quindi esplicito e
 * visibile, mai implicito. */
var Jog = (function () {

  function init() {
    var pad = document.querySelectorAll('[data-jog]');
    for (var i = 0; i < pad.length; i++) {
      pad[i].addEventListener('click', function () {
        var d = this.dataset.jog;
        if (d === 'stop') { Api.feedHold(); return; }
        var axis = d[0];
        var sign = d[1] === '-' ? -1 : 1;
        Api.jog(axis, (sign * State.data.jogStep).toFixed(3), 1000);
      });
    }

    var steps = document.querySelectorAll('.step');
    for (var j = 0; j < steps.length; j++) {
      steps[j].addEventListener('click', function () {
        for (var k = 0; k < steps.length; k++) steps[k].classList.remove('is-active');
        this.classList.add('is-active');
        State.data.jogStep = +this.dataset.step;
      });
    }

    document.getElementById('btn-set-origin').addEventListener('click', function () {
      Api.setOrigin();
      State.data.origin = { x: 0, y: 0 };
      State.emit('origin');
    });

    /* La calibrazione dell'area sostituisce i finecorsa di riferimento: si
       porta la penna nei due angoli opposti del foglio e si registra dove
       sono. È la decisione presa al posto dell'azzeramento automatico. */
    document.getElementById('btn-corner-bl').addEventListener('click', function () {
      Log.add('msg', 'Angolo in basso a sinistra: da implementare.');
    });
    document.getElementById('btn-corner-tr').addEventListener('click', function () {
      Log.add('msg', 'Angolo in alto a destra: da implementare.');
    });

    document.getElementById('btn-abort').addEventListener('click', function () {
      Api.reset();
    });
    document.getElementById('btn-pause').addEventListener('click', function () {
      Api.feedHold();
    });
  }

  return { init: init };
})();
