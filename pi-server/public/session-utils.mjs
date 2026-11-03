export function sessionStatusText(valid) {
	return valid ? "Connecté au laboratoire" : "Session expirée";
}

export function withSessionToken(headers = {}, token) {
	return { ...headers, "X-Session-Token": token };
}

export function encodePasscode(code) {
	return new URLSearchParams({ code }).toString();
}

export function cameraStreamUrl(timestamp) {
	return `/camera/stream?t=${timestamp}`;
}

export function serverStatusText(status, visits) {
	return `Server status: ${status} — total visits so far: ${visits}`;
}
