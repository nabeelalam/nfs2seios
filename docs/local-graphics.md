# Local graphics for WSL development

The bounded local runner defaults to software Mesa for repeatable checks. On a
WSL installation that already has GPU support and Mesa's D3D12 backend, pass
`--graphics d3d12` to request the existing GPU path. No installation or persistent
Windows, WSL, driver or shell setting is changed.

Run from the repository root inside an owned Xvfb display, using the built app
and an imported working copy of the original assets:

```sh
xvfb-run -a -s "-screen 0 640x480x24" python3 scripts/wsl_smoke.py \
  .build/classic-ui-build-v1/nfs_game_app "$NFS_WORKING_COPY" \
  .build/local-ui-check --graphics d3d12
```

The app writes settings and records, so supply a working copy. Existing bounded
capture, shutdown and application-cleanup requirements remain in effect.

The option applies `EGL_PLATFORM=x11` and `GALLIUM_DRIVER=d3d12` to the game child
only, removing conflicting software and adapter overrides from that child. It
does not force or verify a particular physical adapter. Check renderer evidence
separately; a driver request or loaded library alone is not proof of the active
renderer. A local hardware window/context probe has succeeded, while an ordinary
race-to-Victory observation remains unverified.

Local frame rates and screenshots do not establish iPad performance, energy use,
or physical-device acceptance.
