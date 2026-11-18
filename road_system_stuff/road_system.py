
import osmnx as ox
from dataclasses import dataclass
import matplotlib.pyplot as plt


@dataclass
class Node:
    id: int
    x: float  # Meters from bottom-left corner
    y: float  # Meters from bottom-left corner


@dataclass
class Edge:
    id: int
    nfrom: int  # Starting node ID
    nto: int    # Destination node ID


# Define the city
city = 'Barcs, Hungary'

# Download the driving road network
G = ox.graph_from_place(city, network_type='drive')

# Project graph to a local metric coordinate system (UTM)
G = ox.project_graph(G)




# Find the bottom-left corner
min_x = min(data['x'] for _, data in G.nodes(data=True))
min_y = min(data['y'] for _, data in G.nodes(data=True))

# Map original OSM IDs to sequential IDs
osm_to_node_id = {
    osm_id: new_id
    for new_id, osm_id in enumerate(G.nodes)
}

# Convert original nodes
nodes: list[Node] = []

for osm_id, data in G.nodes(data=True):
    nodes.append(Node(
        id=osm_to_node_id[osm_id],
        x=data['x'] - min_x,
        y=data['y'] - min_y
    ))

# Next available node ID
next_node_id = len(nodes)

# Convert edges, adding nodes for road curvature
edges: list[Edge] = []

for u, v, key, data in G.edges(keys=True, data=True):

    # Get road geometry
    geometry = data.get('geometry')

    if geometry is not None:
        coordinates = list(geometry.coords)

        # Make sure geometry goes from u to v
        ux, uy = G.nodes[u]['x'], G.nodes[u]['y']
        vx, vy = G.nodes[v]['x'], G.nodes[v]['y']

        start = coordinates[0]
        end = coordinates[-1]

        if ((start[0] - vx)**2 + (start[1] - vy)**2 <
            (start[0] - ux)**2 + (start[1] - uy)**2):
            coordinates.reverse()

    else:
        coordinates = [
            (G.nodes[u]['x'], G.nodes[u]['y']),
            (G.nodes[v]['x'], G.nodes[v]['y'])
        ]

    # Build sequence of node IDs along this road
    road_nodes = [osm_to_node_id[u]]

    # Add intermediate curvature nodes
    for x, y, *rest in coordinates[1:-1]:

        node_id = next_node_id
        next_node_id += 1

        nodes.append(Node(
            id=node_id,
            x=x - min_x,
            y=y - min_y
        ))

        road_nodes.append(node_id)

    road_nodes.append(osm_to_node_id[v])

    # Connect consecutive nodes
    for i in range(len(road_nodes) - 1):
        edges.append(Edge(
            id=len(edges),
            nfrom=road_nodes[i],
            nto=road_nodes[i + 1]
        ))




def plotGraph(nodes: list[Node], edges: list[Edge]) -> None:
    """Plot the graph using matplotlib."""

    import matplotlib.pyplot as plt

    # Fast node lookup by ID
    node_map = {node.id: node for node in nodes}

    fig, ax = plt.subplots(figsize=(12, 12))

    # Draw edges
    for edge in edges:
        n1 = node_map[edge.nfrom]
        n2 = node_map[edge.nto]

        ax.plot(
            [n1.x, n2.x],
            [n1.y, n2.y],
            color='gray',
            linewidth=0.7,
            zorder=1
        )

    # Draw nodes
    ax.scatter(
        [node.x for node in nodes],
        [node.y for node in nodes],
        s=3,
        color='red',
        zorder=2
    )

    ax.set_aspect('equal')
    ax.set_title('Road Network')
    ax.set_xlabel('X (meters)')
    ax.set_ylabel('Y (meters)')
    ax.grid(False)

    plt.show()



def graphToTextFile(nodes: list[Node], edges: list[Edge]) -> None:
    with open("graph.txt", "wt") as file:
        file.write( f"nodes: {len(nodes)} edges: {len(edges)}\n" )
        for node in nodes:
            file.write( f"id: {node.id} x: {node.x} y: {node.y}\n" )
        for edge in edges:
            file.write( f"id: {edge.id} from: {edge.nfrom} to: {edge.nto}\n" )


print(f"number of nodes: {len(nodes)}")
print(f"number of edges: {len(edges)}")
plotGraph(nodes, edges)
graphToTextFile(nodes, edges)