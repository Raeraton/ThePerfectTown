import osmnx as ox
from pyproj import Transformer

from graph_conversion import convert_graph
from lane_graph import create_lane_graph


class MapDownloader:
    def __init__(self):
        self.drive_crs = None
        self.origin_x = 0.0
        self.origin_y = 0.0
        self.transformer = None
        self.drive_graph = None

    def download_drive_graph(self, city: str):
        graph = ox.graph_from_place(city, network_type="drive")
        graph = ox.project_graph(graph)

        self.drive_graph = graph
        self.drive_crs = graph.graph["crs"]
        self.origin_x = min(data["x"] for _, data in graph.nodes(data=True))
        self.origin_y = min(data["y"] for _, data in graph.nodes(data=True))
        self.transformer = Transformer.from_crs("EPSG:4326", self.drive_crs, always_xy=True)

        return convert_graph(graph, self.origin_x, self.origin_y)

    def download_walk_graph(self, city: str):
        if self.drive_crs is None:
            raise RuntimeError("Download the driving graph first.")

        graph = ox.graph_from_place(city, network_type="walk")
        graph = ox.project_graph(graph, to_crs=self.drive_crs)

        sidewalk_edges = {
            (u, v, key)
            for u, v, key, data in graph.edges(keys=True, data=True)
            #if data.get("footway") == "sidewalk"
        }

        graph.remove_edges_from([
            (u, v, key)
            for u, v, key in graph.edges(keys=True)
            if (u, v, key) not in sidewalk_edges
        ])
        graph.remove_nodes_from([
            node for node, degree in graph.degree()
            if degree == 0
        ])

        return convert_graph(graph, self.origin_x, self.origin_y)

    def download_buildings(self, city: str):
        from building_downloader import download_buildings
        return download_buildings(
            city,
            self.drive_crs,
            self.origin_x,
            self.origin_y,
            self.transformer,
        )

    def download_lane_graph(self, city: str):
        if self.drive_graph is None:
            raise RuntimeError("Download the driving graph first.")

        return create_lane_graph(
            self.drive_graph,
            self.origin_x,
            self.origin_y,
        )
