#!/usr/bin/env python3
import os
import json
import glob
from datetime import datetime

def main():
    repo = os.getenv('REPO', 'abiji-2020/pg_ask')
    version = os.getenv('VERSION', 'main')
    pages_url = os.getenv('PAGES_URL', 'https://abiji-2020.github.io/pg_ask')
    
    manifest = {
        'repository': repo,
        'version': version,
        'base_url': pages_url,
        'generated_at': datetime.utcnow().isoformat() + 'Z',
        'releases': {}
    }
    
    # Scan releases directory
    for arch_dir in glob.glob('releases/*'):
        if os.path.isdir(arch_dir):
            arch = os.path.basename(arch_dir)
            files = []
            for file in glob.glob(f'{arch_dir}/*'):
                if os.path.isfile(file):
                    file_stat = os.stat(file)
                    files.append({
                        'name': os.path.basename(file),
                        'path': f'releases/{arch}/{os.path.basename(file)}',
                        'url': f'{pages_url}/releases/{arch}/{os.path.basename(file)}',
                        'size': file_stat.st_size
                    })
            
            if files:
                manifest['releases'][arch] = files
                print(f"  {arch}: {len(files)} files")
    
    # Write manifest
    with open('manifest.json', 'w') as f:
        json.dump(manifest, f, indent=2)
    
    print(f"\nGenerated manifest.json with {len(manifest['releases'])} architectures")
    print(f"Total files: {sum(len(files) for files in manifest['releases'].values())}")

if __name__ == '__main__':
    main()
