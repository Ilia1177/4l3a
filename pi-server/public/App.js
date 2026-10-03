// ---------------------------------------------------------------------------
// Session state
// ---------------------------------------------------------------------------
let sessionToken = null;

// ---------------------------------------------------------------------------
// Session management
// ---------------------------------------------------------------------------
function join() {
  return fetch('/api/minitel/join', { method: 'POST' })
    .then(r => r.json().then(d => ({ ok: r.ok, d })))
    .then(({ ok, d }) => {
      if (ok) {
        sessionToken = d.token;
        document.getElementById('sessionStatus').textContent = 'Connecté au laboratoire';
        return true;
      } else {
        document.getElementById('sessionStatus').textContent = 'Session labo non disponible';
        return false;
      }
    });
}

function callMinitel(path, options = {}) {
  if (!sessionToken) return Promise.reject('no session');
  return fetch(path, {
    ...options,
    headers: { ...(options.headers || {}), 'X-Session-Token': sessionToken }
  });
}

function checkSession() {
  if (!sessionToken) {
    document.getElementById('sessionStatus').textContent = 'Not connected';
    return;
  }

  fetch('/api/minitel/validSession', {
    method: 'POST',
    headers: { 'X-Session-Token': sessionToken }
  })
    .then(r => r.json().then(d => ({ ok: r.ok, d })))
    .then(({ ok, d }) => {
      document.getElementById('sessionStatus').textContent =
        ok ? 'Connecté au laboratoire' : 'Session expirée';
      if (!ok) sessionToken = null; // stop treating a dead token as valid
    })
    .catch(() => {
      document.getElementById('sessionStatus').textContent = 'Connexion perdue';
    });
}

// Release the session when the tab closes, if possible.
window.addEventListener('beforeunload', () => {
  if (sessionToken) {
    navigator.sendBeacon('/api/minitel/leave', new Blob());
  }
});

// ---------------------------------------------------------------------------
// Passcode submission
// ---------------------------------------------------------------------------
document.getElementById('passcodeBtn').addEventListener('click', () => {
  const code = document.getElementById('passcodeInput').value;
  if (!code) return;

  callMinitel('/api/minitel/hazardousLab', {
    method: 'POST',
    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
    body: new URLSearchParams({ code })
  })
    .then(r => r.json().then(d => ({ ok: r.ok, d })))
    .then(({ ok, d }) => {
      document.getElementById('passcodeResult').textContent =
        ok ? 'Status: ' + d.status : 'Error: ' + d.error;
    })
    .catch(() => {
      document.getElementById('passcodeResult').textContent = 'Failed to reach server';
    });
});

// ---------------------------------------------------------------------------
// "Get your code" / print button — joins first, then prints
// ---------------------------------------------------------------------------
document.getElementById('printBtn').addEventListener('click', () => {
  const ensureJoined = sessionToken ? Promise.resolve(true) : join();

  ensureJoined.then(success => {
    if (!success) {
      document.getElementById('printResult').textContent = 'Could not join session';
      return;
    }

    callMinitel('/api/minitel/printcode')
      .then(r => r.json().then(d => ({ ok: r.ok, d })))
      .then(({ ok, d }) => {
        document.getElementById('printResult').textContent =
          ok ? 'Status: ' + d.status : 'Error: ' + d.error;
      })
      .catch(() => {
        document.getElementById('printResult').textContent = 'Failed to reach server';
      });
  });
});

// ---------------------------------------------------------------------------
// Server status / visit counter
// ---------------------------------------------------------------------------
fetch('/api/status')
  .then(r => r.json())
  .then(d => {
    document.getElementById('status').textContent =
      `Server status: ${d.status} — total visits so far: ${d.visits}`;
  })
  .catch(() => {
    document.getElementById('status').textContent = 'Could not reach /api/status';
  });

// ---------------------------------------------------------------------------
// Periodic session check (every 15s)
// ---------------------------------------------------------------------------
setInterval(checkSession, 15000);
checkSession();
