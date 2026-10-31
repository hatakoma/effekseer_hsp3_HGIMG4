"""Package built DLLs, authored assets, source and tests; exclude build caches."""
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
root=Path(__file__).resolve().parents[1]
output=root/'effekseer_hgimg4-1.80.7-win32-collision.zip'
files=[root/name for name in ('LICENSE','README.md','CMakeLists.txt','build.ps1','test.ps1')]
for folder in ('dist','src','tests','tools'):
    for file in (root/folder).rglob('*'):
        if not file.is_file() or set(file.parts)&{'bin','obj','__pycache__'}:
            continue
        if file.suffix.lower() in {'.ax','.rgba','.log','.pyc'}:
            continue
        files.append(file)
for required in ('dist/effekseer_hgimg4.dll','dist/effekseer_hgimg4dx.dll','dist/assets/collision_rain.efk'):
    if not (root/required).is_file():
        raise SystemExit('Missing '+required)
with ZipFile(output,'w',ZIP_DEFLATED) as archive:
    for file in sorted(files):
        archive.write(file,file.relative_to(root).as_posix())
print(output)
