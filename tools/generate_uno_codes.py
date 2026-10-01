"""Generate the AVR (Arduino Uno R3) PROGMEM code table.

Uses the same verified protocol timings as tools/generate_codes.py
(IRremoteESP8266 constants), stored as flat microsecond duration arrays so
the bit-banging sketch needs no unpacking logic on the ATmega328P.
"""
import os


def nec(data):
    out = [(9000, 4500)]
    out += [(560, 1690) if (data >> (31 - i)) & 1 else (560, 560)
            for i in range(32)]
    out.append((560, 5000))
    return out


def samsung(data):
    out = [(4480, 4480)]
    out += [(560, 1680) if (data >> (31 - i)) & 1 else (560, 560)
            for i in range(32)]
    out.append((560, 5000))
    return out


def sony(data):
    out = [(2400, 600)]
    out += [(1200, 600) if (data >> i) & 1 else (600, 600)
            for i in range(12)]
    out.append((600, 5000))
    return out


def rc5(addr, cmd, toggle):
    h = 889  # kRc5T1
    bits = [1, toggle]
    bits += [(addr >> i) & 1 for i in range(4, -1, -1)]
    bits += [(cmd >> i) & 1 for i in range(5, -1, -1)]
    levels = [1]  # S1 mark; its leading space is never transmitted
    for b in bits:
        levels += [0, 1] if b else [1, 0]
    runs = []
    for lvl in levels:
        if runs and runs[-1][0] == lvl:
            runs[-1][1] += 1
        else:
            runs.append([lvl, 1])
    pairs, i = [], 0
    while i < len(runs):
        m = runs[i]
        s = runs[i + 1] if i + 1 < len(runs) else [0, 1]  # trailing gap
        pairs.append((h * m[1], h * s[1]))
        i += 2
    return pairs


def flat(pairs):
    return [v for p in pairs for v in p]


FRAMES = [
    # (name, khz, extra_repeats, durations)
    ('kFrameNecToshiba', 38, 0, flat(nec(0x02FD48B7))),
    ('kFrameNecToshiba2', 38, 0, flat(nec(0x02FD08F7))),
    ('kFrameNecLg', 38, 0, flat(nec(0x20DF10EF))),
    ('kFrameSamsung', 38, 0, flat(samsung(0xE0E040BF))),
    ('kFrameSamsung2', 38, 0, flat(samsung(0xE0E09966))),
    ('kFrameSony', 40, 2, flat(sony(0x0A90))),
    ('kFrameRc5Toggle0', 36, 0, flat(rc5(0, 12, 0))),
    ('kFrameRc5Toggle1', 36, 0, flat(rc5(0, 12, 1))),
]

lines = [
    '// GENERATED FILE -- same verified timings as the ESP8266/ESP32 tables.',
    '// Regenerate with: python tools/generate_uno_codes.py',
    '#pragma once',
    '#include <avr/pgmspace.h>',
    '',
]
for name, _khz, _reps, durations in FRAMES:
    lines.append('const uint16_t %s[] PROGMEM = {%s};'
                 % (name, ','.join(str(d) for d in durations)))
lines.append('')

lines.append('struct UnoFrame {')
lines.append('  const uint16_t* data;  // PROGMEM durations, mark/space pairs (us)')
lines.append('  uint8_t pairs;')
lines.append('  uint8_t khz;')
lines.append('  uint8_t repeats;      // extra frames after the first')
lines.append('};')
lines.append('')

def is_eu_only(name):
    return name.startswith('kFrameRc5')

# NEC/Samsung/Sony frames ship in both regions; RC-5 is EU-only.
def region_rows(region):
    return ['{%s, %d, %d, %d}' % (name, len(durations) // 2, khz, reps)
            for name, khz, reps, durations in FRAMES
            if region == 'EU' or not is_eu_only(name)]

na_rows, eu_rows = region_rows('NA'), region_rows('EU')
lines.append('#if defined(TVBGONE_REGION_NA)')
lines.append('const UnoFrame kFrames[] PROGMEM = {%s};' % ','.join(na_rows))
lines.append('const uint8_t kFrameCount = %d;' % len(na_rows))
lines.append('#else  // TVBGONE_REGION_EU')
lines.append('const UnoFrame kFrames[] PROGMEM = {%s};' % ','.join(eu_rows))
lines.append('const uint8_t kFrameCount = %d;' % len(eu_rows))
lines.append('#endif')
lines.append('')
lines.append('#if !defined(TVBGONE_REGION_NA) && !defined(TVBGONE_REGION_EU)')
lines.append('#define TVBGONE_REGION_NA  // default region')
lines.append('#warning "No region selected; building the NA/Asia set"')
lines.append('#endif')

os.makedirs('Uno-Tv-B-Gone', exist_ok=True)
with open('Uno-Tv-B-Gone/uno_codes.hpp', 'w', newline='\n') as f:
    f.write('\n'.join(lines) + '\n')
print('written: Uno-Tv-B-Gone/uno_codes.hpp (%d frames)' % len(FRAMES))
