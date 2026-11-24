import osmnx as ox
from pyproj import Transformer

from graph_conversion import convert_graph
from lane_graph import create_lane_graph
from walk_path import prepare_walk_graph
from progress import ProgressBar


class MapDownloader:
    def __init__(self):
        self.drive_crs = None
        self.origin_x = 0.0
        self.origin_y = 0.0
        self.transformer = None
        self.drive_graph = None
        self.buildings = None

    def download_drive_graph(self, city: str):
        progress = ProgressBar("Downloading driving graph", None)
        progress.start()
        graph = ox.graph_from_place(city, network_type="drive")
        progress.update(0, "projecting")
        graph = ox.project_graph(graph)

        self.drive_graph = graph
        self.drive_crs = graph.graph["crs"]
        self.origin_x = min(data["x"] for _, data in graph.nodes(data=True))
        self.origin_y = min(data["y"] for _, data in graph.nodes(data=True))
        self.transformer = Transformer.from_crs("EPSG:4326", self.drive_crs, always_xy=True)

        converted = convert_graph(graph, self.origin_x, self.origin_y)
        progress.finish()
        return converted

    def download_walk_graph(self, city: str):
        if self.drive_crs is None:
            raise RuntimeError("Download the driving graph first.")

        download_progress = ProgressBar("Downloading walking graph", None)
        download_progress.start()
        graph = ox.graph_from_place(city, network_type="walk")
        graph = ox.project_graph(graph, to_crs=self.drive_crs)
        download_progress.finish()

        boundary_progress = ProgressBar("Downloading city boundary", None)
        boundary_progress.start()
        boundary_gdf = ox.geocode_to_gdf(city).to_crs(self.drive_crs)
        city_boundary = boundary_gdf.geometry.union_all()
        boundary_progress.finish()

        process_progress = ProgressBar("Processing sidewalk graph", graph.number_of_edges())
        process_progress.start()
        graph = prepare_walk_graph(
            graph,
            self.drive_graph,
            city_boundary,
            buildings=self.buildings,
            origin_x=self.origin_x,
            origin_y=self.origin_y,
            progress_callback=process_progress.update,
        )
        process_progress.finish()

        converted = convert_graph(graph, self.origin_x, self.origin_y)
        return converted

    def download_buildings(self, city: str):
        from building_downloader import download_buildings
        self.buildings = download_buildings(
            city,
            self.drive_crs,
            self.origin_x,
            self.origin_y,
            self.transformer,
        )
        return self.buildings

    def download_lane_graph(self, city: str):
        if self.drive_graph is None:
            raise RuntimeError("Download the driving graph first.")

        return create_lane_graph(
            self.drive_graph,
            self.origin_x,
            self.origin_y,
        )
