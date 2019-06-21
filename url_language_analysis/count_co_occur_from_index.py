import operator

f1 = open("inverted_index_url_lang.txt")
f2 = open("cooccurrences_by_url.txt", "w")
f3 = open("cooccurrences_by_domain.txt", "w")

d = {}
domain_d = {}

while True:
    line1 = f1.readline()
    line2 = f1.readline()
    if not line1 or not line2: break

    domain = line1.split("/")[0]
    lang_ls = line2.strip().split(", ")
    langs = tuple(sorted(lang_ls))

    if domain in domain_d:
        domain_d[domain].update(lang_ls)
    else:
        domain_d[domain] = set(lang_ls)

    if langs in d:
        d[langs] += 1
    else:
        d[langs] = 1

ks = sorted(d.keys(), key=operator.itemgetter(0))

total = sum(list(d.values()))
for k in ks:
    c = d[k]
    f2.write("+".join(k) + "\t" + str(c) + "\t" + str(round(c * 100. / total, 5)) + "%\n")

f1.close()
f2.close()

new_d = {}
for _, v in domain_d.items():
    langs = tuple(sorted(list(v)))
    if langs in new_d:
        new_d[langs] += 1
    else:
        new_d[langs] = 1

new_ks = sorted(new_d.keys(), key=operator.itemgetter(0))
new_total = sum(list(new_d.values()))
for k in new_ks:
    c = new_d[k]
    f3.write("+".join(k) + "\t" + str(c) + "\t" + str(round(c * 100. / total, 5)) + "%\n")

f3.close()
