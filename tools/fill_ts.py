#!/usr/bin/env python3
# SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
# Trägt die Übersetzungen ein:
#   * Englisch:  translations/viviabreathe_en.ts (Desktop/Kern) aus tools/translations_en.py,
#                mobile/translations/viviabreathe_mobile_en.ts (Touch-Oberfläche) zusätzlich aus
#                tools/translations_mobile_en.py
#   * Deutsch:   nur die Mehrzahlformen der %n-Texte (…_de.ts)
# Ablauf nach Textänderungen (siehe README.md bzw. mobile/README.md):
#   lupdate ViviaBreathe.pro
#   lupdate -pluralonly -no-obsolete src -ts translations/viviabreathe_de.ts
#   lupdate -no-obsolete mobile/qml mobile/src -ts mobile/translations/viviabreathe_mobile_en.ts
#   lupdate -pluralonly -no-obsolete mobile/qml mobile/src -ts mobile/translations/viviabreathe_mobile_de.ts
#   python3 tools/fill_ts.py
# Fehlende Übersetzungen werden gemeldet und bleiben „unfinished“ (dann erscheint der deutsche Text).
import os
import sys
import xml.etree.ElementTree as ET

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(here, '..')
sys.path.insert(0, here)
from translations_en import EN, EN_CONTEXT, DE_NUMERUS  # noqa: E402
from translations_mobile_en import EN_MOBILE, DE_NUMERUS_MOBILE  # noqa: E402

missing = []
done = 0


def write(tree, path):
    tree.write(path, encoding='utf-8', xml_declaration=True)
    with open(path, encoding='utf-8') as f:
        data = f.read()
    data = data.replace("<?xml version='1.0' encoding='utf-8'?>", '<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>', 1)
    with open(path, 'w', encoding='utf-8') as f:
        f.write(data)


def fill_en(path, table):
    global done
    if not os.path.exists(path):
        return
    tree = ET.parse(path)
    for ctx in tree.getroot().findall('context'):
        name = ctx.find('name').text
        for msg in ctx.findall('message'):
            src = msg.find('source').text or ''
            tr = msg.find('translation')
            if tr is None:
                tr = ET.SubElement(msg, 'translation')
            en = EN_CONTEXT.get((name, src), table.get(src))
            if en is None:
                missing.append((os.path.basename(path) + ':' + name, src))
                continue
            for child in list(tr):
                tr.remove(child)
            tr.text = None
            if msg.get('numerus') == 'yes':
                forms = en if isinstance(en, list) else [en, en]
                for f in forms:
                    ET.SubElement(tr, 'numerusform').text = f
            else:
                tr.text = en[0] if isinstance(en, list) else en
            tr.attrib.pop('type', None)
            done += 1
    write(tree, path)


def fill_de(path, table):
    if not os.path.exists(path):
        return
    tree = ET.parse(path)
    for ctx in tree.getroot().findall('context'):
        for msg in ctx.findall('message'):
            src = msg.find('source').text or ''
            forms = table.get(src)
            if forms is None:
                missing.append((os.path.basename(path) + ':' + ctx.find('name').text, src))
                continue
            tr = msg.find('translation')
            for child in list(tr):
                tr.remove(child)
            tr.text = None
            tr.attrib.pop('type', None)
            for fm in forms:
                ET.SubElement(tr, 'numerusform').text = fm
    write(tree, path)


fill_en(os.path.join(root, 'translations', 'viviabreathe_en.ts'), EN)
fill_de(os.path.join(root, 'translations', 'viviabreathe_de.ts'), DE_NUMERUS)
fill_en(os.path.join(root, 'mobile', 'translations', 'viviabreathe_mobile_en.ts'), {**EN, **EN_MOBILE})
fill_de(os.path.join(root, 'mobile', 'translations', 'viviabreathe_mobile_de.ts'), {**DE_NUMERUS, **DE_NUMERUS_MOBILE})

print(f'{done} übersetzt, {len(missing)} fehlen')
for c, s in missing:
    print(f'  FEHLT [{c}]: {s!r}')
sys.exit(1 if missing else 0)
