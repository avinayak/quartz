"""Verify installed shared font matching; no preferences are changed."""
import importlib.util
from pathlib import Path
import subprocess

spec = importlib.util.spec_from_file_location('typography', Path(__file__).resolve().parents[2] / 'theme/Quartz-System6/typography.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)
count = 0
for family, sizes in m.catalog().items():
    requests = set(sizes + [max(1, sizes[0] - 2), sizes[-1] + 7])
    for lower, upper in zip(sizes, sizes[1:]):
        middle = (lower + upper) / 2
        requests.update((middle - .1, middle, middle + .1))
    for request in sorted(requests):
        expected = min(sizes, key=lambda size: (abs(size - request), size))
        for expression in [f'pixelsize={request}'] + [f'size={request * 72 / dpi}:dpi={dpi}' for dpi in (96, 144, 192)]:
            actual = subprocess.check_output(['fc-match', '-f', '%{family[0]}|%{pixelsize}|%{antialias}|%{matrix}', family + ':' + expression], text=True).split('|')
            assert actual[0] == family and float(actual[1]) == expected and actual[2] == 'False' and actual[3] == '1 0 0 1', (family, request, expression, expected, actual)
            count += 1
print(f'Passed {count} nearest-size checks across all Typography families (pixels, points, midpoints, and out-of-range sizes).')
