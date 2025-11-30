import matplotlib.pyplot as plt
import matplotlib.patches as patches

# Read the file
filename = 'tiled_quadtree_image.txt'

try:
    with open(filename, 'r') as f:
        lines = f.readlines()
except FileNotFoundError:
    print(f"Error: Could not find '{filename}'. Make sure it's in the same folder as this script!")
    exit()

fig, ax = plt.subplots(figsize=(16, 9))

max_x = 0
max_y = 0

for line in lines:
    # Skip empty lines just in case
    if not line.strip():
        continue
        
    parts = line.split(",")
    dim_color = parts[0]
    x = int(parts[1])
    y = int(parts[2])
    
    dims, color = dim_color.split('/')
    w, h = map(int, dims.split('-'))
    
    # Draw rectangle
    rect = patches.Rectangle((x, y), w, h, linewidth=1, facecolor='#' + color, edgecolor=(0, 0, 0, 0.5))
    ax.add_patch(rect)
    
    max_x = max(max_x, x + w)
    max_y = max(max_y, y + h)

# Set limits
ax.set_xlim(0, max_x)
# INVERT Y-AXIS: 0 at top, max height at bottom
ax.set_ylim(max_y, 0) 

ax.xaxis.tick_top()
ax.set_aspect('equal')
plt.grid(True, linestyle='--', alpha=0.3)
plt.title(f"Visualized: {filename}", y=1.08)
plt.show()