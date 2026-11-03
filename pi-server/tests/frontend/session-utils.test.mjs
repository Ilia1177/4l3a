import assert from "node:assert/strict";
import test from "node:test";

import {
	cameraStreamUrl,
	encodePasscode,
	serverStatusText,
	sessionStatusText,
	withSessionToken,
} from "../../public/session-utils.mjs";

test("sessionStatusText describes the session state", () => {
	assert.equal(sessionStatusText(true), "Connecté au laboratoire");
	assert.equal(sessionStatusText(false), "Session expirée");
});

test("withSessionToken preserves existing headers", () => {
	const headers = { "Content-Type": "application/json" };

	const result = withSessionToken(headers, "abc123");

	assert.deepEqual(result, {
		"Content-Type": "application/json",
		"X-Session-Token": "abc123",
	});
	assert.deepEqual(headers, { "Content-Type": "application/json" });
});

test("withSessionToken replaces an existing session header", () => {
	const result = withSessionToken({ "X-Session-Token": "old" }, "new");

	assert.equal(result["X-Session-Token"], "new");
});

test("encodePasscode form-encodes the code", () => {
	assert.equal(encodePasscode("A B"), "code=A+B");
	assert.equal(encodePasscode("é"), "code=%C3%A9");
});

test("cameraStreamUrl adds a cache-busting timestamp", () => {
	assert.equal(cameraStreamUrl(123), "/camera/stream?t=123");
});

test("serverStatusText includes status and visit count", () => {
	assert.equal(
		serverStatusText("online", 5),
		"Server status: online — total visits so far: 5",
	);
});
