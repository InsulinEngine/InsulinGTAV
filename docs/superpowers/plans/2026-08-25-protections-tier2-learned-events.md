# Tier 2 learned events — benign baseline (Task 14 groundwork)

Source: dumped from a live LSO session on build prot-t2-preview-1 (2026-08-26),
`/data/Ozark/insulingtav.log` via the "Dump Learned Hashes" menu button.

## Session character

- **script_event: 32 distinct** args[0] values (below). Benign baseline candidates.
- **weapon / sound / explosion: 0** — a peaceful session (no combat / explosions /
  weapon-give). Needs a combat session to populate.
- give_control filter confirmed LIVE-firing on console (`Detected Give Control ...`).
- Players present: 0, 1, 4, 6.

## Open question (why this is a LEARN tier, not a ported block list)

None of the 32 hashes resolve in CodeWalker's joaat string index, so script-event
args[0] is NOT the joaat of a known event-name string — it is a runtime value.
Whether it is stable enough per event-type to block on can only be answered by
diffing this benign set against a known-attack session. Until then: **benign,
never block.**

## Benign script_event args[0] (32 distinct, with hit counts from the latest dump)

| args[0] | hits | first player |
|---|---|---|
| 0xb64d9e01 | 9 | 6 |
| 0x5d7bb983 | 229 | 1 |
| 0xeb0d0e8c | 2 | 4 |
| 0x76ec940d | 45 | 4 |
| 0xedb26815 | 7 | 6 |
| 0x10790999 | 7 | 6 |
| 0xd6fd319d | 5 | 4 |
| 0x3578139e | 1 | 1 |
| 0xc60b6f9e | 2 | 1 |
| 0xb5623a28 | 5 | 4 |
| 0x320907ab | 1 | 1 |
| 0x75d38830 | 1 | 4 |
| 0x1344f138 | 10 | 4 |
| 0x9d140f3d | 18 | 4 |
| 0x2edba0be | 4 | 6 |
| 0x72e56b3e | 1 | 4 |
| 0x37cf4bc0 | 2 | 1 |
| 0x1cefcbc3 | 1 | 4 |
| 0x625201c9 | 1 | 0 |
| 0x79734e4d | 1 | 6 |
| 0xa62640ce | 1 | 6 |
| 0xc3959757 | 13 | 6 |
| 0xe5002ada | 3 | 4 |
| 0x39ffe55e | 1 | 4 |
| 0xedb875de | 2 | 4 |
| 0xa5a04266 | 10 | 6 |
| 0x6c252fe6 | 18 | 4 |
| 0xe1ffdaf2 | 2 | 0 |
| 0x2fb16f75 | 16 | 4 |
| 0x27bc6af5 | 2 | 4 |
| 0xbc70d7f5 | 3 | 6 |
| 0xa371697e | 1 | 4 |
