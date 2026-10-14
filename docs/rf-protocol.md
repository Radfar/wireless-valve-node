# RF Protocol Notes

The node uses the RCSwitch library to receive 433 MHz RF commands from the Arduino UNO controller.

For the compiled valve ID `n`:

- Open / ON: `2560 + n`
- Close / OFF: `1280 + n`

The current source sets `VAL_NO = 1`, so it expects decimal values 2561 (open) and 1281 (close). The controller transmits each command as a 24-bit value twice.

This fixed-code protocol does not provide acknowledgement, encryption, authentication, replay protection, or physical valve-position feedback. A received command does not prove that the valve moved.

Keep the node ID consistent with the controller's valve numbering. Test with the valve isolated before live operation.
