#!/usr/bin/env python3
import os
import json

version = os.environ.get('VERSION', '')
base = 'releases'
releases = []

if os.path.isdir(base):
    for arch in sorted(os.listdir(base)):
        d = os.path.join(base, arch)
        if not os.path.isdir(d):
            continue
        binary = None
        control = None
        sql = None
        for f in os.listdir(d):
            if f.endswith('.so'):
                binary = f
            elif f.endswith('.control'):
                control = f
            elif f.endswith('.sql'):
                sql = f
        releases.append({
            'platform': arch,
            'binary': f"{base}/{arch}/{binary}" if binary else None,
            'control': f"{base}/{arch}/{control}" if control else None,
            'sql': f"{base}/{arch}/{sql}" if sql else None,
        })

out = {'version': version, 'releases': releases}

with open('manifest.json', 'w') as fh:
    json.dump(out, fh, indent=2)
