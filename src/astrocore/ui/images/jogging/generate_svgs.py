import math

def rotate(x, y, angle):
    rad = math.radians(angle)
    cx, cy = 12, 12
    dx = x - cx
    dy = y - cy
    nx = dx * math.cos(rad) - dy * math.sin(rad) + cx
    ny = dx * math.sin(rad) + dy * math.cos(rad) + cy
    # Round to avoid float precision issues, 3 decimal places is enough for SVG
    return round(nx, 3), round(ny, 3)

points = {'P1':(12,21), 'P2':(12,5), 'P3':(4,13), 'P5':(20,13)}
angles = {
    'down': 180,
    'left': 270,
    'right': 90,
    'right_up': 45,
    'right_down': 135,
    'left_down': 225,
    'left_up': 315
}

header = '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" stroke-width="3" fill="none" stroke="currentColor" stroke-linecap="round" stroke-linejoin="round">'
footer = '</svg>'

for name, angle in angles.items():
    r = {}
    for k, p in points.items():
        r[k] = rotate(p[0], p[1], angle)
    
    # Check for integers to make it cleaner
    for k in r:
        if r[k][0].is_integer(): r[k] = (int(r[k][0]), r[k][1])
        if r[k][1].is_integer(): r[k] = (r[k][0], int(r[k][1]))

    # Path 1: P1 to P2
    path1 = f'<path d="M{r["P1"][0]} {r["P1"][1]}L{r["P2"][0]} {r["P2"][1]}"/>'
    # Path 2: P3 -> P2 -> P5
    path2 = f'<path d="M{r["P3"][0]} {r["P3"][1]}L{r["P2"][0]} {r["P2"][1]}L{r["P5"][0]} {r["P5"][1]}"/>'
    
    # Use &&& separator
    print(f'--- {name} ---')
    print(header)
    print(f'  {path1}')
    print(f'  {path2}')
    print(footer)
    print('&&&')
