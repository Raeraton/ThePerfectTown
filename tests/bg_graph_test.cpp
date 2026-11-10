
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <background_traffic.hpp>

using namespace game::traffic::backgound;

// --------------------------------------------------
// Helpers
// --------------------------------------------------

template<typename F>
bool throws_string(F&& func)
{
    try {
        func();
    }
    catch (const std::string&) {
        return true;
    }

    return false;
}

void assert_path(
    const std::vector<Node_idx>& actual,
    const std::vector<Node_idx>& expected
) {
    assert(actual == expected);
}

// --------------------------------------------------
// Graph construction
// --------------------------------------------------

void test_add_nodes()
{
    Graph graph;

    assert(graph.add_node(Node{}) == 0);
    assert(graph.add_node(Node{}) == 1);
    assert(graph.add_node(Node{}) == 2);

    assert(graph.is_node_valid(0));
    assert(graph.is_node_valid(1));
    assert(graph.is_node_valid(2));
    assert(!graph.is_node_valid(3));
}

void test_add_edges()
{
    Graph graph;

    graph.add_node(Node{});
    graph.add_node(Node{});
    graph.add_node(Node{});

    assert(graph.add_edge(Edge(50, 100, 1), 0) == 0);
    assert(graph.add_edge(Edge(60, 200, 2), 0) == 1);
    assert(graph.add_edge(Edge(30, 150, 2), 1) == 2);

    assert(graph.is_edge_valid(0));
    assert(graph.is_edge_valid(1));
    assert(graph.is_edge_valid(2));
    assert(!graph.is_edge_valid(3));
}

void test_invalid_edge_source()
{
    Graph graph;

    graph.add_node(Node{});

    assert(throws_string([&] {
        graph.add_edge(Edge(50, 100, 0), 1);
    }));
}

void test_invalid_edge_destination()
{
    Graph graph;

    graph.add_node(Node{});

    assert(throws_string([&] {
        graph.add_edge(Edge(50, 100, 1), 0);
    }));
}

void test_empty_graph()
{
    Graph graph;

    assert(!graph.is_node_valid(0));
    assert(!graph.is_edge_valid(0));

    assert(throws_string([&] {
        graph.assert_node_valid(0);
    }));

    assert(throws_string([&] {
        graph.assert_edge_valid(0);
    }));
}

// --------------------------------------------------
// Shortest path tests
// --------------------------------------------------

void test_direct_path()
{
    Graph graph;

    graph.add_node(Node{});
    graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);

    auto path = graph.calculate_shortest_path(0, 1);

    assert_path(path, {0, 1});
}

void test_multiple_paths()
{
    Graph graph;

    for (int i = 0; i < 4; ++i)
        graph.add_node(Node{});

    // 0 -> 1 -> 3 (total distance 200)
    graph.add_edge(Edge(50, 100, 1), 0);
    graph.add_edge(Edge(50, 100, 3), 1);

    // 0 -> 2 -> 3 (total distance 150)
    graph.add_edge(Edge(50, 50, 2), 0);
    graph.add_edge(Edge(50, 100, 3), 2);

    auto path = graph.calculate_shortest_path(0, 3);

    assert_path(path, {0, 2, 3});
}

void test_weighted_shortest_path()
{
    Graph graph;

    for (int i = 0; i < 4; ++i)
        graph.add_node(Node{});

    // Short path in number of edges, but longer distance:
    // 0 -> 3 = 500

    graph.add_edge(Edge(50, 500, 3), 0);

    // Longer path in number of edges, but shorter distance:
    // 0 -> 1 -> 2 -> 3 = 300

    graph.add_edge(Edge(50, 100, 1), 0);
    graph.add_edge(Edge(50, 100, 2), 1);
    graph.add_edge(Edge(50, 100, 3), 2);

    auto path = graph.calculate_shortest_path(0, 3);

    assert_path(path, {0, 1, 2, 3});
}

void test_unreachable_node()
{
    Graph graph;

    graph.add_node(Node{});
    graph.add_node(Node{});
    graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);

    auto path = graph.calculate_shortest_path(0, 2);

    assert(path.empty());
}

void test_same_origin_and_destination()
{
    Graph graph;

    graph.add_node(Node{});

    auto path = graph.calculate_shortest_path(0, 0);

    assert_path(path, {0});
}

// --------------------------------------------------
// Cycles
// --------------------------------------------------

void test_cycle()
{
    Graph graph;

    for (int i = 0; i < 3; ++i)
        graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);
    graph.add_edge(Edge(50, 100, 2), 1);
    graph.add_edge(Edge(50, 100, 0), 2);

    auto path = graph.calculate_shortest_path(0, 2);

    assert_path(path, {0, 1, 2});
}

void test_cycle_with_shortcut()
{
    Graph graph;

    for (int i = 0; i < 4; ++i)
        graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);
    graph.add_edge(Edge(50, 100, 2), 1);
    graph.add_edge(Edge(50, 100, 0), 2);

    // Shortcut
    graph.add_edge(Edge(50, 50, 3), 0);

    auto path = graph.calculate_shortest_path(0, 3);

    assert_path(path, {0, 3});
}

// --------------------------------------------------
// Path validation
// --------------------------------------------------

void test_valid_path()
{
    Graph graph;

    for (int i = 0; i < 3; ++i)
        graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);
    graph.add_edge(Edge(50, 100, 2), 1);

    assert(graph.path_is_valid({0, 1, 2}));
}

void test_invalid_path()
{
    Graph graph;

    for (int i = 0; i < 3; ++i)
        graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);
    graph.add_edge(Edge(50, 100, 2), 1);

    // No edge from 2 to 0.
    assert(!graph.path_is_valid({0, 1, 2, 0}));

    // No direct edge from 0 to 2.
    assert(!graph.path_is_valid({0, 2}));
}

void test_invalid_node_in_path()
{
    Graph graph;

    graph.add_node(Node{});

    assert(!graph.path_is_valid({0, 99}));
}

// --------------------------------------------------
// Distance table
// --------------------------------------------------

void test_distance_table()
{
    Graph graph;

    for (int i = 0; i < 4; ++i)
        graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);
    graph.add_edge(Edge(50, 100, 2), 1);
    graph.add_edge(Edge(50, 100, 3), 2);

    auto table = graph.create_full_distance_table(0);

    assert(table.at(0) == 0);
    assert(table.at(1) == 0);
    assert(table.at(2) == 1);
    assert(table.at(3) == 2);
}

void test_distance_table_unreachable()
{
    Graph graph;

    for (int i = 0; i < 3; ++i)
        graph.add_node(Node{});

    graph.add_edge(Edge(50, 100, 1), 0);

    auto table = graph.create_full_distance_table(0);

    assert(table.at(0) == 0);
    assert(table.at(1) == 0);
    assert(table.find(2) == table.end());
}

void test_distance_table_invalid_origin()
{
    Graph graph;

    graph.add_node(Node{});

    assert(throws_string([&] {
        graph.create_full_distance_table(99);
    }));
}

// --------------------------------------------------
// Main
// --------------------------------------------------

int main()
{
    test_empty_graph();
    test_add_nodes();
    test_add_edges();
    test_invalid_edge_source();
    test_invalid_edge_destination();
    test_direct_path();
    test_multiple_paths();
    test_weighted_shortest_path();
    test_unreachable_node();
    test_same_origin_and_destination();
    test_cycle();
    test_cycle_with_shortcut();
    test_valid_path();
    test_invalid_path();
    test_invalid_node_in_path();
    test_distance_table();
    test_distance_table_unreachable();
    test_distance_table_invalid_origin();
    
    std::cout << "All Graph tests passed!\n";
    

    return 0;
}
