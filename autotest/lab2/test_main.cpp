#include "utils.h"

// vertices and faces of all testing objects
#include "objects.inc"

bool test_all();

int main()
{
    if (test_all()) {
        info("all tests passed, congratulations!");
    } else {
        warn("test failed");
    }

    return 0;
}

bool test_all()
{
    Halfedge_Mesh test_mesh;
    bool result = true;

    for (size_t i = 0; i != object_names.size(); ++i) {
        const char* name = object_names[i].c_str();
        // load the input mesh
        string error_msg = test_mesh.from_poly(object_faces[i], object_vertices[i]);
        if (!error_msg.empty()) {
            warn("failed to load the %s: %s", name, error_msg.c_str());
            result = false;
            continue;
        }
        // load the standard output mesh
        auto [std_vertices, std_edges] = load_std(object_names[i]);

        info("start testing subdivision for %s", name);
        test_mesh.loop_subdivide();
        // test 1: validate the mesh
        info("validate the mesh...");
        auto validation = test_mesh.validate();
        if (validation.has_value()) {
            warn("the result of subdivision is a invalid mesh: %s",
                    validation.value().second.c_str());
            result = false;
        } else {
            info("passed");
            // test 2: vertex positions
            info("compare all vertices...");
            auto [stu_vertices, stu_edges] = extract(test_mesh);
            auto vertices_map = test_and_map_vertices(std_vertices, stu_vertices);
            if (stu_vertices.size() != std_vertices.size()) {
                warn("number of vertices is wrong: %zu (should be %zu)",
                        stu_vertices.size(), std_vertices.size());
                result = false;
            } else if (!vertices_map.has_value()) {
                warn("at least on of the vertices has incorrect position");
                result = false;
            } else {
                unordered_map<size_t, size_t> &std_to_stu = vertices_map.value();
                info("passed");
                // test 3: connectivity
                info("compare all edges...");
                bool edges_equal = test_edges(std_edges, stu_edges, std_to_stu);
                if (stu_edges.size() != std_edges.size()) {
                    warn("number of edges is wrong: %zu (should be %zu)",
                            stu_edges.size(), std_edges.size());
                    result = false;
                } else if (!edges_equal) {
                    warn("at least one of the edges connects incorrect vertices");
                    result = false;
                } else {
                    info("passed");
                }
            }
        }

        // string std_path = "../std/";
        // std_path = std_path + object_names[i] + ".txt";
        // FILE *fout = std::fopen(std_path.c_str(), "w");
        // unordered_map<Index, size_t> id;
        // size_t current_id = 0;
        // fprintf(fout, "%zu\n", static_cast<size_t>(std_mesh.n_vertices()));
        // for (VertexCRef vertex = std_mesh.vertices_begin();
        //         vertex != std_mesh.vertices_end(); ++vertex) {
        //     const Vec3 &pos = vertex->pos;
        //     fprintf(fout, "%f %f %f\n", pos.x, pos.y, pos.z);
        //     if (id.find(vertex->id()) == id.end()) {
        //         id.insert({vertex->id(), current_id});
        //         ++current_id;
        //     }
        // }
        // fprintf(fout, "%zu\n", static_cast<size_t>(std_mesh.n_edges()));
        // for (EdgeCRef edge = std_mesh.edges_begin();
        //         edge != std_mesh.edges_end(); ++edge) {
        //     VertexCRef v1 = edge->halfedge()->vertex();
        //     VertexCRef v2 = edge->halfedge()->twin()->vertex();
        //     size_t id1 = id[v1->id()];
        //     size_t id2 = id[v2->id()];
        //     fprintf(fout, "%zu %zu\n", id1, id2);
        // }
        // std::fclose(fout);

        info("end testing subdivision for %s\n", name);
    }

    return result;
}
