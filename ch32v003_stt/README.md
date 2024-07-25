## Share WCH-LinkE serial port to `wsl2`

<https://learn.microsoft.com/en-us/windows/wsl/connect-usb>

```bash
usbipd list
usbipd attach --wsl --busid 1-4
usbipd detach --busid 1-4
```
