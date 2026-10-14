# Valve Node Test Checklist

- [ ] Confirm PlatformIO selects the `d1` ESP8266 environment.
- [ ] Confirm only `src/rf.cpp` is compiled for the current firmware.
- [ ] Build successfully and record PlatformIO, Arduino framework, and RCSwitch library versions.
- [ ] Verify serial startup output at 9600 baud.
- [ ] Test with the valve disconnected or with a safe test load first.
- [ ] Confirm the startup open/close pulse sequence is expected.
- [ ] Confirm receiver wiring to D6 / GPIO12 and driver wiring against the schematic.
- [ ] Verify open code 2561 and close code 1281 for `VAL_NO = 1`.
- [ ] Confirm the driver is compatible with the 6 V DC bi-stable valve and its specified pulse current/duration.
- [ ] Verify the ESP8266 does not reset during a valve pulse.
- [ ] Test actual RF range and missed-command behavior in the intended environment.
- [ ] Verify physical valve movement separately from RF reception.
- [ ] Record results, hardware revision, date, and outstanding issues.
