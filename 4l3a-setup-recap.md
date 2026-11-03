# 4l3a: Pi server exposed through hazardous-editorial.net

Recap of the working setup (November 2026).

## Goal

Serve the Raspberry Pi web server on `hazardous-editorial.net` with a valid HTTPS certificate, without depending on third-party services such as DuckDNS or Cloudflare. Constraint: only external port **30777** can be forwarded on the Freebox (80 and 443 are not available).

## Architecture

```
Browser
  │  https://www.hazardous-editorial.net
  ▼
Porkbun URL forwarding (302)
  │  → https://4l3a.hazardous-editorial.net:30777
  ▼
DNS (Porkbun):  4l3a.hazardous-editorial.net  CNAME  ilia.freeboxos.fr  → current public IP
  ▼
Freebox: external TCP 30777  →  Pi LAN IP : 443
  ▼
Caddy (on the Pi, listens on 443)
  ├─ /camera/*  → 127.0.0.1:8081  (camera stream)
  └─ everything else → 127.0.0.1:8080  (pi-server, C++ web server)
```

Why the port stays visible: a redirect cannot hide a port, and without 80/443 on the Freebox there is no way to serve the site on the default HTTPS port over IPv4.

## Components

### 1. Freebox

- Dynamic name `ilia.freeboxos.fr`, maintained by the Freebox, always points to the current public IP.
- Port forwarding: external **TCP 30777** → Pi LAN IP, internal port **443**.
- The Pi should have a fixed LAN IP (DHCP reservation).

### 2. Porkbun DNS

| Type  | Host                          | Value                  | Purpose                              |
|-------|-------------------------------|------------------------|--------------------------------------|
| CNAME | `4l3a.hazardous-editorial.net`| `ilia.freeboxos.fr`    | Follows the Freebox IP               |
| ALIAS | `www`                         | `uixie.porkbun.com`    | Porkbun's records for URL forwarding |
| CNAME | `*.www`                       | `uixie.porkbun.com`    | Same                                 |
| MX    | bare domain                   | `fwd1/fwd2.porkbun.com`| Email forwarding (keep)              |
| TXT   | bare domain                   | SPF record             | Email (keep)                         |
| A / AAAA | bare domain                | OVH server             | Leftover, unrelated to this setup    |

### 3. Porkbun URL forwarding

- Host: `www`
- Destination: `https://4l3a.hazardous-editorial.net:30777`
- Type: Temporary (302), Cloak off

Result: `www.hazardous-editorial.net` redirects to the real site. The address bar then shows `4l3a.hazardous-editorial.net:30777`.

### 4. Porkbun API (for the DNS challenge)

Since Let's Encrypt can only validate over ports 80/443, Caddy proves domain ownership through DNS instead (the DNS-01 challenge), by creating a temporary `_acme-challenge` TXT record through the Porkbun API.

- **API ACCESS** enabled on the domain `hazardous-editorial.net` (domain Details).
- API key pair created at porkbun.com/account/api (permissions: "Manage, but no purchases", no IP restriction because the home IP changes).
- The secret key is displayed **only once** at creation. If lost, create a new pair.

### 5. Caddy on the Pi

Custom Caddy binary that includes the Porkbun DNS module (downloaded from Caddy's download service, since `xcaddy` is not in the Pi's apt repositories):

```
curl -o caddy "https://caddyserver.com/api/download?os=linux&arch=arm64&p=github.com/caddy-dns/porkbun"
chmod +x caddy
./caddy list-modules | grep porkbun     # must show dns.providers.porkbun
sudo systemctl stop caddy
sudo mv caddy /usr/bin/caddy
sudo systemctl start caddy
```

API keys are passed through the systemd override (`sudo systemctl edit caddy`):

```
[Service]
Environment=PORKBUN_API_KEY=pk1_xxxxxxxx
Environment=PORKBUN_API_SECRET_KEY=sk1_xxxxxxxx
```

Then `sudo systemctl daemon-reload && sudo systemctl restart caddy`.

`/etc/caddy/Caddyfile` (final working state):

```
4l3a.duckdns.org {
    handle_path /camera/* {
        reverse_proxy 127.0.0.1:8081 {
            flush_interval -1
        }
    }
    reverse_proxy 127.0.0.1:8080
}

4l3a.hazardous-editorial.net {
    tls {
        dns porkbun {
            api_key {env.PORKBUN_API_KEY}
            api_secret_key {env.PORKBUN_API_SECRET_KEY}
        }
    }
    handle_path /camera/* {
        reverse_proxy 127.0.0.1:8081 {
            flush_interval -1
        }
    }
    reverse_proxy 127.0.0.1:8080
}
```

Do **not** keep blocks for `hazardous-editorial.net` or `www.hazardous-editorial.net` in the Caddyfile. Without a `tls dns` block, Caddy tries HTTP/TLS-ALPN validation, which cannot work on port 30777, and it fills the logs with failing retries. The `4l3a.duckdns.org` block can be removed if DuckDNS is no longer used.

Do not write `:30777` in the Caddyfile: Caddy listens on 443 locally, and the Freebox maps 30777 to it.

## Verification

```
dig +short 4l3a.hazardous-editorial.net        # ilia.freeboxos.fr. then the public IP
sudo caddy validate --config /etc/caddy/Caddyfile
sudo journalctl -u caddy -n 50 --no-pager      # look for "certificate obtained successfully"
```

Test from outside the home network (phone on mobile data), because many boxes cannot loop back to their own public IP:

- `https://4l3a.hazardous-editorial.net:30777`
- `https://www.hazardous-editorial.net` (redirects to the above)

Test the API keys independently of Caddy:

```
curl -s -X POST https://api.porkbun.com/api/json/v3/ping \
  -H 'Content-Type: application/json' \
  -d '{"apikey":"pk1_...","secretapikey":"sk1_..."}'
```

Expected: `"status":"SUCCESS"` with the public IP.

## Troubleshooting log (problems met and their causes)

| Symptom | Cause | Fix |
|---|---|---|
| "Client sent an HTTP request to an HTTPS server" | Redirect used `http://` but Caddy serves HTTPS on that port | Use `https://` in the destination |
| `SSL_ERROR_INTERNAL_ERROR_ALERT` after redirect to `ilia.freeboxos.fr` | Caddy has no site/certificate for that hostname | Redirect to a hostname Caddy serves (`4l3a.hazardous-editorial.net`) |
| Same error with the right hostname | No certificate issued yet: Porkbun returned `Invalid API key (002)` | Create a new API key pair, copy the secret immediately, test with curl, update the systemd override |
| Logs: `invalid port 30777: must be 80 or 443` | Let's Encrypt HTTP/TLS-ALPN validation for blocks without a `tls dns` config | Remove those blocks, use the DNS challenge only |
| `www.hazrdous-editorial.net` not found | Typo in the URL | Spell `hazardous` correctly |
| Logs mention `acme-staging` | Caddy automatically retries on Let's Encrypt staging after a failure | Not a configuration problem; disappears once the real issuance succeeds |

## Maintenance notes

- **Certificate renewal** is automatic (Caddy renews through the DNS challenge). The Porkbun API keys must stay valid and **API ACCESS** must stay enabled on the domain.
- **Caddy updates:** the binary was replaced manually, so `apt upgrade` will not update it. Use `sudo caddy upgrade`, which keeps the added modules, or repeat the download step.
- **Freebox IP changes** are handled by `ilia.freeboxos.fr`; no update script is needed.
- **Secrets:** the API secret key lives only in the systemd override on the Pi. Delete any old or leaked key pairs at porkbun.com/account/api. An `EnvironmentFile` with `chmod 600` is a tidier alternative to `Environment=` lines.

## Possible improvements

- **Clean URL without port:** only possible over IPv6 (open 80/443 in the Freebox IPv6 firewall to the Pi, add an AAAA record for `www`). Visitors without IPv6 would not be able to connect.
- **Keep `www.hazardous-editorial.net` in the address bar** would require serving the domain directly on 443, which is not possible with only port 30777 forwarded.
