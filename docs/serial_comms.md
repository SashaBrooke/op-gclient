# Serial Comms Design Decisions

## Streaming
The serial device will be assumed to boot with streaming disabled. Therefore, it is expected that no packets will be sent to the client until we either deliberately enable streaming or request information (and expect an ACK).
