# Tabela de simbolos (inclui funcoes static) para o perfilador por amostragem de PC.
# Uso: python profsyms.py out_dir > syms.txt     (linhas: "ENDERECO_HEX nome")
# e depois: python profsyms.py out_dir samples.txt   -> histograma por funcao
import re, sys, os, collections

out = sys.argv[1]
mp = open(os.path.join(out, 'akalabeth.map'), encoding='latin1').read()
glob = {}          # nome -> (addr, modulo)
for m in re.finditer(r'^\s+([0-9A-F]{8})\s+(_\w+)\s+(\w+)\s*$', mp, re.M):
    glob[m.group(2)] = (int(m.group(1), 16), m.group(3))

syms = {}
for f in os.listdir(out):
    if not f.endswith('.sym'):
        continue
    mod = f[:-4]
    t = open(os.path.join(out, f), encoding='latin1').read()
    loc = []
    for m in re.finditer(r'^\s*(\d)\s+(_\w+)\s+([0-9A-F]{8})\s+(\w*)\s*$', t, re.M):
        loc.append((m.group(2), int(m.group(3), 16), m.group(1)))
    base = None
    for n, a, ar in loc:
        if n in glob and glob[n][1] == mod:
            base = glob[n][0] - a
            break
    if base is None:
        continue
    for n, a, ar in loc:
        if n.startswith('___str') or ar != '0':
            continue
        syms[base + a] = n

for n, (a, mod) in glob.items():
    if a >= 0x100 and a < 0xD000:
        syms.setdefault(a, n)

table = sorted(syms.items())
if len(sys.argv) == 2:
    for a, n in table:
        print('%04X %s' % (a, n))
else:
    import bisect
    addrs = [a for a, n in table]
    cnt = collections.Counter()
    tot = 0
    for l in open(sys.argv[2]):
        l = l.strip()
        if not l:
            continue
        pc = int(l, 16)
        tot += 1
        i = bisect.bisect_right(addrs, pc) - 1
        cnt[table[i][1] if i >= 0 and pc < 0xD000 else ('<ROM/other %04X>' % (pc & 0xF000))] += 1
    for n, c in cnt.most_common(30):
        print('%6.1f%% %5d %s' % (100.0 * c / tot, c, n))
    print('total', tot)
