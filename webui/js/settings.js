/* La tab Impostazioni. Per ora tiene solo i valori che vivono nel browser:
 * quelli della macchina (passi/mm, estremi del servo) restano nella
 * configurazione di FluidNC, e qui saranno in sola lettura. Due posti in cui
 * scrivere lo stesso numero sono due posti che prima o poi si contraddicono. */
var Settings = (function () {

  function init() {
    var host = document.getElementById('set-host');
    var mock = document.getElementById('set-mock');

    host.value = State.data.host;
    mock.checked = State.data.mock;

    host.addEventListener('change', function () {
      State.set('host', this.value.trim());
      document.getElementById('host').value = State.data.host;
    });

    mock.addEventListener('change', function () {
      State.set('mock', this.checked);
      Api.disconnect();
      Log.add('msg', this.checked
        ? 'Modalità finta attivata.'
        : 'Modalità finta disattivata: serve una macchina raggiungibile.');
    });
  }

  return { init: init };
})();
