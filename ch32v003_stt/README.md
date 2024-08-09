# ch32v003_stt

## Description

Documentation is work in progress.

## Projects

* `stt.c`

* `misc/`

* `training/ch32v003/dump_raw_audio/`

* `training/ch32v003/i2c_dump_audio/`

* `training/ch32v003/i2c_receive_audio/`

* `training/desktop/go.c`
  * See [here](training/desktop/README.md) for more information.

## Share WCH-LinkE serial port to `wsl2`

<https://learn.microsoft.com/en-us/windows/wsl/connect-usb>

```bash
usbipd list
usbipd attach --wsl --busid 1-4
usbipd detach --busid 1-4
```
