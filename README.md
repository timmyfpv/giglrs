# GIGLRS

GIGLRS is a GIGFPV-focused fork of the ExpressLRS firmware codebase. It starts from upstream ELRS and is intended to diverge only where changes have been tested and verified for GIGFPV hardware and flying needs.

The original project is GPL-3.0 licensed. This fork keeps the upstream copyright and license notices in the source tree and documents GIGLRS-specific changes here.

## Current Focus

- Radio startup hardening for TX and RX firmware.
- GIGFPV-oriented target and release handling.
- Keeping useful upstream ELRS changes mergeable after they have been reviewed and tested.

## Radio Startup Robustness

GIGLRS retries radio chipset bring-up before declaring the radio failed. This is meant to reduce rare boot-time failures caused by transient LoRa radio wake-up, SPI, or reset timing issues.

Defaults:

- `GIGLRS_RADIO_BEGIN_ATTEMPTS=5`
- `GIGLRS_RADIO_BEGIN_RETRY_DELAY_MS=100`

Both values can be overridden with build flags if a target needs different timing.

## Target Handling

PlatformIO build environments live in `src/targets`.

Hardware target metadata is stored separately in:

https://github.com/timmyfpv/giglrs-targets

During local ESP builds, the metadata repo is cloned into `src/hardware` when that folder is missing.

## Build And Test

From `src`:

```sh
pio test -e native
PLATFORMIO_BUILD_FLAGS="-DRegulatory_Domain_ISM_2400" pio run -e Unified_ESP32_2400_RX_via_UART
PLATFORMIO_BUILD_FLAGS="-DRegulatory_Domain_ISM_2400" pio run -e Unified_ESP32_2400_TX_via_UART
```

## Upstream

GIGLRS is forked from:

https://github.com/ExpressLRS/ExpressLRS

Upstream changes should be reviewed and tested before merging into this fork.
