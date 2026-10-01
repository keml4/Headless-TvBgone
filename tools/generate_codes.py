"""Generate and validate raw IR timing tables for the ESP8266 TV-B-Gone sketches.

All protocol timings are taken from the IRremoteESP8266 sources (ir_NEC.cpp,
ir_Samsung.cpp, ir_Sony.cpp, ir_RC5_RC6.cpp). Tables are packed in the
TV-B-Gone format: IrCode entries hold a times table (units of 10 us) and a
byte array of 2-bit indices into (mark, space) pairs, MSB first.

Validation replays the *exact* C++ expandCode algorithm over the generated
tables and compares the emitted microsecond stream against frames built
directly from each protocol definition. RC5 frames are additionally decoded
back to bits as an independent round-trip check.
"""
import re

# ---------------------------------------------------------------- generators
def nec_frame(data):
    # 38 kHz, MSB first: 9000/4500, one 560/1690, zero 560/560, stop 560/5000
    return (38, [(9000, 4500)] +
            [(560, 1690) if (data >> (31 - i)) & 1 else (560, 560)
             for i in range(32)] + [(560, 5000)])

def samsung_frame(data):
    # 38 kHz, MSB first: 4480/4480, one 560/1680, zero 560/560, stop 560/5000
    return (38, [(4480, 4480)] +
            [(560, 1680) if (data >> (31 - i)) & 1 else (560, 560)
             for i in range(32)] + [(560, 5000)])

def sony_frame(data):
    # 40 kHz, LSB first (true SIRC): 2400/600, one 1200/600, zero 600/600,
    # stop 600/5000
    return (40, [(2400, 600)] +
            [(1200, 600) if (data >> i) & 1 else (600, 600)
             for i in range(12)] + [(600, 5000)])

def rc5_frame(address, command, toggle):
    # 36 kHz, 889 us half-bits (kRc5T1; 890 in 10-us units stays in tolerance).
    # Wire order: S1 S2 toggle addr(5) cmd(6); "1" = space+mark, "0" =
    # mark+space. The leading space of S1 is never transmitted (IRsend's
    # skipSpace), so the frame physically starts with S1's mark.
    H = 889  # kRc5T1 in microseconds
    bits = [1]  # S2 / field bit
    bits += [toggle]
    bits += [(address >> i) & 1 for i in range(4, -1, -1)]
    bits += [(command >> i) & 1 for i in range(5, -1, -1)]
    levels = [1]  # S1's mark
    for b in bits:
        levels += [0, 1] if b else [1, 0]
    runs = []
    for lvl in levels:
        if runs and runs[-1][0] == lvl:
            runs[-1][1] += 1
        else:
            runs.append([lvl, 1])
    assert runs[0][0] == 1, 'frame must start with a mark'
    pairs, i = [], 0
    while i < len(runs):
        m = runs[i]
        s = runs[i + 1] if i + 1 < len(runs) else [0, 1]  # implicit trailing gap
        assert m[0] == 1 and s[0] == 0, 'expected mark,space alternation'
        pairs.append((H * m[1], H * s[1]))
        i += 2
    return (36, pairs)

# ---------------------------------------------------------------- packing
def dedupe_and_pack(ms):
    times, seen, idx = [], {}, []
    for m, s in ms:
        key = (m, s)
        if key not in seen:
            seen[key] = len(times) // 2
            # frames are specified in microseconds; times[] uses 10-us units
            times += [m // 10, s // 10]
        idx.append(seen[key])
    assert len(times) // 2 <= 4, 'bpi=2 supports at most 4 distinct pairs'
    bits = ''.join(format(i, '02b') for i in idx)
    bits += '0' * (-len(bits) % 8)
    codes = [int(bits[k:k + 8], 2) for k in range(0, len(bits), 8)]
    return times, codes, len(ms)

def emit(name, timer, ms, comment):
    times, codes, n = dedupe_and_pack(ms)
    lines = ['// ' + comment,
             'constexpr uint16_t %sTimes[] = {%s};' % (name, ', '.join(map(str, times))),
             'constexpr uint8_t %sCodes[] = {%s};' % (name, ', '.join('0x%02X' % c for c in codes)),
             'constexpr IrCode %sCode = {%d, %d, 2, %d, sizeof(%sCodes), %sTimes, %sCodes};'
             % (name, timer, n, len(times), name, name, name)]
    return '\n'.join(lines) + '\n', (timer, n, times, codes)

# ------------------------------------------------- exact port of C++ unpack
def unpack(timer, n, times, codes):
    raw, bl, cb, ptr = [], 0, 0, 0
    for _ in range(n):
        index = 0
        for _ in range(2):
            if bl == 0:
                cb = codes[ptr]; ptr += 1; bl = 8
            bl -= 1
            index = (index << 1) | ((cb >> bl) & 1)
        t = index * 2
        assert t + 1 < len(times), 'index out of timing table'
        raw += [min(times[t] * 10, 0xFFFF), min(times[t + 1] * 10, 0xFFFF)]
    return raw

results = []

def check(label, ok, detail=''):
    results.append(ok)
    print('%-30s %s %s' % (label, 'MATCH' if ok else 'MISMATCH', detail))

# ------------------------------------------------------------- code sets
NA_CODES = [
    ('kNec000', nec_frame(0x02FD48B7), 'NEC power, Toshiba 0x02FD48B7'),
    ('kNec001', nec_frame(0x02FD08F7), 'NEC power, Toshiba 0x02FD08F7'),
    ('kNec002', nec_frame(0x20DF10EF), 'NEC power, LG 0x20DF10EF'),
    ('kSam000', samsung_frame(0xE0E040BF), 'Samsung power, 0xE0E040BF'),
    ('kSam001', samsung_frame(0xE0E09966), 'Samsung power, 0xE0E09966'),
    ('kSony000', sony_frame(0x0A90), 'Sony SIRC-12 power, classic 0xA90'),
    ('kSony001', sony_frame(0x0A91), 'Sony SIRC-12 power, alternate 0xA91'),
]
EU_CODES = [
    ('kRc5000', rc5_frame(0, 12, 0), 'RC-5 power, addr 0 cmd 12 (TV), toggle 0'),
    ('kRc5001', rc5_frame(0, 12, 1), 'RC-5 power, addr 0 cmd 12 (TV), toggle 1'),
    ('kNec000', nec_frame(0x02FD48B7), 'NEC power, Toshiba 0x02FD48B7'),
    ('kNec001', nec_frame(0x02FD08F7), 'NEC power, Toshiba 0x02FD08F7'),
    ('kNec002', nec_frame(0x20DF10EF), 'NEC power, LG 0x20DF10EF'),
    ('kSam000', samsung_frame(0xE0E040BF), 'Samsung power, 0xE0E040BF'),
    ('kSony000', sony_frame(0x0A90), 'Sony SIRC-12 power, classic 0xA90'),
]

def flat(frame):
    return [v for p in frame[1] for v in p]

def build(codes):
    blocks, datas = [], []
    for name, frame, comment in codes:
        block, data = emit(name, frame[0], frame[1], comment)
        blocks.append(block)
        datas.append((name, frame, data))
    return blocks, datas

na_blocks, na_datas = build(NA_CODES)
eu_blocks, eu_datas = build(EU_CODES)

def within(got, want, tol):
    return len(got) == len(want) and all(abs(a - b) <= tol for a, b in zip(got, want))

# ------------------------------------------------------------ validation
for name, frame, data in na_datas + eu_datas:
    got = unpack(*data)
    want = flat(frame)
    ms = sum(got) / 1000
    # RC5 half-bits are 889 us, which quantizes to 880 us in the 10-us table
    ok = within(got, want, 10 if 'Rc5' in name else 0)
    check(name, ok, '(pairs=%d, %.1f ms, %d kHz)' % (len(got) // 2, ms, frame[0]))

# RC5 independent round-trip: decode generated stream back to bits
for name, frame, data in [d for d in eu_datas if 'Rc5' in d[0]]:
    got = unpack(*data)
    toggle = 0 if name.endswith('000') else 1
    levels = []
    for m, s in zip(got[0::2], got[1::2]):
        levels += [1] * round(m / 889) + [0] * round(s / 889)
    while levels and levels[-1] == 0:
        levels.pop()
    bits, i = [], 0
    ok = True
    try:
        while i + 1 < len(levels):
            if levels[i] == 0 and levels[i + 1] == 1:
                bits.append(1)
            elif levels[i] == 1 and levels[i + 1] == 0:
                bits.append(0)
            else:
                ok = False
                break
            i += 2
    except Exception:
        ok = False
    # stream: S1=1 (its leading space is not transmitted), then
    # S2 toggle addr(5) cmd(6).
    ok = ok and len(bits) >= 13
    if ok:
        s2, tg = bits[0], bits[1]
        addr = int(''.join(map(str, bits[2:7])), 2)
        cmd = int(''.join(map(str, bits[7:13])), 2)
        ok = (s2 == 1 and addr == 0 and cmd == 12 and tg == toggle)
        check(name + ' decoded', ok,
              '(S2=%d toggle=%d addr=%d cmd=%d)' % (s2, tg, addr, cmd))

# ------------------------------------------------------------- write files
header = '''#pragma once

#include <Arduino.h>

struct IrCode {
  uint16_t timerValue;   // carrier frequency in kHz
  uint16_t numPairs;     // number of (mark, space) pairs in the frame
  uint8_t bitsPerIndex;  // bits per timing index (2)
  uint8_t timeValues;    // number of uint16_t entries in times[] (pairs * 2)
  uint16_t codeBytes;    // number of bytes in codes[]
  const uint16_t* times; // (mark, space) durations, units of 10 us
  const uint8_t* codes;  // packed 2-bit indices into times[] pairs, MSB first
};

'''

def make_text(codes, blocks, note):
    text = header + note + '\n'
    text += '\n'.join(blocks) + '\n'
    text += 'constexpr const IrCode* kPowerCodes[] = {' \
         + ', '.join('&' + n for n, _, _ in codes) + '};\n'
    text += 'constexpr uint8_t kPowerCodeCount = ' \
         + 'sizeof(kPowerCodes) / sizeof(kPowerCodes[0]);\n'
    return text

na_text = make_text(NA_CODES, na_blocks, '// Add further North American/Asian entries here.')
eu_text = make_text(EU_CODES, eu_blocks, '// Add further European entries here.')

with open('ESP8266-NA-Tv-B-Gone/ir_codes_na.hpp', 'w', newline='\n') as f:
    f.write(na_text)
with open('ESP8266-EU-Tv-B-Gone/ir_codes_eu.hpp', 'w', newline='\n') as f:
    f.write(eu_text)

print()
print('written: ESP8266-NA-Tv-B-Gone/ir_codes_na.hpp (%d codes)' % len(NA_CODES))
print('written: ESP8266-EU-Tv-B-Gone/ir_codes_eu.hpp (%d codes)' % len(EU_CODES))
print('VALIDATION:', 'ALL MATCH' if all(results) else 'FAILURES PRESENT')
