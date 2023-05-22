# Arduino CAN Fragmentation Protocol

- Source:       5 bits
   - Destination:  5 bits
   - Type:         1 bit
   - Remain:       8 bits
   - Identifier:   10 bits

   The \b Source and \b Destination fields must match the source and destiantion addressses in the CSP packet.
   The \b Type field is used to distinguish the first and subsequent frames in a fragmented CSP
   packet. Type is BEGIN (0) for the first fragment and MORE (1) for all other fragments.
   The \b Remain field indicates number of remaining fragments, and must be decremented by one for each fragment sent.
   The \b identifier field serves the same purpose as in the Internet Protocol, and should be an auto incrementing
   integer to uniquely separate sessions.
