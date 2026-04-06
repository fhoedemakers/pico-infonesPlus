These are the fixes done for [#111](https://github.com/fhoedemakers/pico-infonesPlus/issues/111)

This fixed an issue with some sound effects missing in Battletoads and Battletoads and Double Dragon. 

This however caused some roms not to play sound anymore like:

- Castlevania
- Goonies
- and more...

This occured only in this specific configuration:

- No PSRAM is used.
- Pico PIO USB is used as usb driver for gamepads.

This seems totally unrelated to the changes done in the emulator code itself.
